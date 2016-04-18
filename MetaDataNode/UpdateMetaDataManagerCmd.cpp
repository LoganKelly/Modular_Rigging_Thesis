/***********************************************************
* Summary: Updates all of the meta data node networks      *
*           connected to the MetaDataManager based upon    *
*          their associated XML rig definitions.           *
*  Author: Logan Kelly                                     *
*    Date: 9/19/12                                         *
***********************************************************/

#include "MetaDataManagerNode.h"
#include "UpdateMetaDataManagerCmd.h"
#include <maya/MItDependencyNodes.h>
#include <maya/MFn.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include "MyErrorChecking.h"
#include "LoadRigUtils.h"
#include "Rig.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <rapidxml.hpp>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
using namespace rapidxml;

MStatus UpdateMetaDataManagerCmd::doIt ( const MArgList &args )
{
    MStatus stat;

    MStatus paramStatus = parseArgs(args);

    for( MItDependencyNodes nodeIt(MFn::kPluginDependNode);
        !nodeIt.isDone(); nodeIt.next() ) {
        MFnDependencyNode nodeFn( nodeIt.item() );
        if(nodeFn.typeId() == MetaDataManagerNode::id) {
            //Get the plug for the list of metaRoots
            MPlug metaRootsPlug = nodeFn.findPlug(MString("metaRoots"),true,&stat);
            MyCheckStatusReturn(stat,"findPlug failed");
            //MGlobal::displayInfo( metaRootsPlug.name() );
            
            //follow its connection to all the plugs on connected metaRoots
            MPlugArray rootPlugs;
            metaRootsPlug.connectedTo(rootPlugs,false,true,&stat);
            MyCheckStatusReturn(stat,"MPlug.connectedTo failed");
            //stringstream ss;
            //ss << "rootPlugs length = " << rootPlugs.length();
            //MGlobal::displayInfo( ss.str().c_str() );

            //iterate over the rootPlugs and find their corresponding metaRoot nodes
            for(unsigned int i = 0; i < rootPlugs.length(); i++) {
                MObject rootNodeObj = rootPlugs[i].node(&stat);
                MyCheckStatusReturn(stat,"MPlug.node() failed");
                MFnDependencyNode rootNodeFn( rootNodeObj );

                if( paramStatus == MS::kSuccess ) {
                    if( rootNodeFn.name() != this->m_rootNodeName )
                        continue;
                }

                //get the xml path string held in the xmlPath attribute
                MString xmlString;
                if (m_alternateXML) {
                    xmlString = m_alternateXMLPath;
                } else {
                    MPlug rootXmlPath = rootNodeFn.findPlug(MString("xmlPath"),true,&stat);
                    MyCheckStatusReturn(stat,"findPlug failed");
                    rootXmlPath.getValue(xmlString);
                }

                //get the version number held in the version attribute
                MPlug rootVersionPlug = rootNodeFn.findPlug(MString("version"),true,&stat);
                MyCheckStatusReturn(stat,"findPlug failed");
                float nodeVersion = 0.0;
                rootVersionPlug.getValue(nodeVersion);

                //check to see if the version of the root node matches its xml file
                m_xmlGuide = new XmlGuide(xmlString, true);
                bool versionMatch = checkXmlFileVersion(nodeVersion);
                //if the version doesn't match, update the loaded rig from the xml file
                if(!versionMatch || this->m_forceUpdate) {
                    float xmlVersion;
                    m_xmlGuide->getVersion(xmlVersion);
                    rootVersionPlug.setValue(xmlVersion);
                    Rig* aRig = new Rig(xmlString, rootNodeObj);
                    aRig->update(this->m_forceUpdate,this->m_globalPos);
                    delete aRig;
                    setResult(rootNodeFn.name());
                }
                delete m_xmlGuide;
            }

        }
    }    

    return redoIt();
}

MStatus UpdateMetaDataManagerCmd::undoIt()
{
    return dgMod.undoIt();
}

bool UpdateMetaDataManagerCmd::checkXmlFileVersion(float version)
{
    MStatus status;
    bool success = false;

    //get info from the xml file      
    float xmlVersion;
    status = m_xmlGuide->getVersion(xmlVersion);
    MyCheckStatus(status, "getVersion failed");
    
    //check to see if the versions match
    if(xmlVersion == version) {
        success = true;
    } else {
        success = false;
    }    

    return success;
}

MStatus UpdateMetaDataManagerCmd::updateHeaderInfo(MObject rootNode) {
    MStatus stat;

    MFnDependencyNode rootNodeFn( rootNode );

    MString xmlName;
    m_xmlGuide->getName(xmlName);
    float xmlVersion;
    m_xmlGuide->getVersion(xmlVersion);
    
    MPlug rootVersionPlug = rootNodeFn.findPlug(MString("version"),true,&stat);
    MyCheckStatusReturn(stat,"findPlug failed");
    rootVersionPlug.setValue(xmlVersion);

    MString rootNodeName = "MRN_";
    rootNodeName += xmlName;
    dgMod.renameNode(rootNode, rootNodeName);

    return MS::kSuccess;
}

MStatus UpdateMetaDataManagerCmd::redoIt()
{
    return dgMod.doIt();
}

MStatus UpdateMetaDataManagerCmd::updateGeoNodes(MObject rootNode) {
    MStatus status = MS::kSuccess;

    MFnDependencyNode rootNodeFn( rootNode );
    
    MPlug geometryPlug = rootNodeFn.findPlug(MString("geometry"),true,&status);
    MyCheckStatusReturn(status,"findPlug geometry failed");
    MPlugArray geometryPlugs;
    geometryPlug.connectedTo(geometryPlugs,false,true,&status);
    MyCheckStatusReturn(status, "MPlug.connectedTo failed");
    for(unsigned int j = 0; j < geometryPlugs.length(); j++) {
        MObject geoObj = geometryPlugs[j].node(&status);
        MFnTransform geoObjFn( geoObj );
        MyCheckStatusReturn(status, "MPlug.node() failed");

        //find the reference namespace
        string str ( geoObjFn.name().asChar() );
        string key (":");
        size_t found = str.rfind(key);
        string sNamespace = str.substr(0,found);
        sNamespace.append("RN" );

        //store the original transformation matrix, and name of geometry transform node
        MTransformationMatrix origGeoTransMat = geoObjFn.transformation();
        MString origGeoName = geoObjFn.name();


        //remove the reference node, and load the reference file from the new path
        {
            stringstream tmp;
            tmp << "file -removeReference -referenceNode \"" << sNamespace.c_str() << "\";";
            MString referenceCommand = MString(tmp.str().c_str());
            MGlobal::executeCommand(referenceCommand);

            //get info from the xml file      
            MString geoFilePath;
            status = m_xmlGuide->getGeoFilePath(geoFilePath);
            MyCheckStatus(status, "getGeoFilePath failed");
            MString name;
            status = m_xmlGuide->getName(name);
            MyCheckStatus(status, "getName failed");
            MString geoName;
            status = m_xmlGuide->getGeoName(geoName);
            MyCheckStatus(status, "getGeoName failed");

            MObject geoObj;
            lrutils::loadGeoReference(geoFilePath,geoName,name, geoObj);
            m_xmlGuide->setName(name);
            MFnTransform transformFn;
            transformFn.setObject(geoObj);

            transformFn.set(origGeoTransMat);

            status = dgMod.connect( rootNodeFn.findPlug("geometry"), transformFn.findPlug("metaParent") );
            MyCheckStatusReturn(status, "connect failed");
        }

    }

    return status;
}

MSyntax UpdateMetaDataManagerCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(UpdateMetaDataManagerCmd::FileParam(), UpdateMetaDataManagerCmd::FileParamLong(), MSyntax::kString);
    syntax.addFlag(UpdateMetaDataManagerCmd::XMLParam(), UpdateMetaDataManagerCmd::XMLParamLong(), MSyntax::kString);
    syntax.addFlag(UpdateMetaDataManagerCmd::ForceParam(), UpdateMetaDataManagerCmd::ForceParamLong(), MSyntax::kNoArg);
    syntax.addFlag(UpdateMetaDataManagerCmd::GlobalPosParam(), UpdateMetaDataManagerCmd::GlobalPosParamLong(), MSyntax::kNoArg);

    return syntax;
}

MStatus UpdateMetaDataManagerCmd::parseArgs(const MArgList & args )
{
    MStatus status;
    if( args.length() == 0 ) {
        this->m_forceUpdate = false;
        this->m_globalPos = false;
        this->m_alternateXML = false;
        return MS::kNotFound;
    }

    MArgDatabase argData(syntax(), args);

    if (argData.isFlagSet(UpdateMetaDataManagerCmd::FileParam())) {
        MString tmp;
        status = argData.getFlagArgument(UpdateMetaDataManagerCmd::FileParam(), 0, tmp);
        if (!status) {
            status.perror("path flag parsing failed");
            return status;
        }
        this->m_rootNodeName = tmp;
    }

    if (argData.isFlagSet(UpdateMetaDataManagerCmd::ForceParam())) {
        this->m_forceUpdate = true;
    } else {
        this->m_forceUpdate = false;
    }

    if (argData.isFlagSet(UpdateMetaDataManagerCmd::GlobalPosParam())) {
        this->m_globalPos = true;
    } else {
        this->m_globalPos = false;
    }

    if (argData.isFlagSet(UpdateMetaDataManagerCmd::XMLParam())) {
        MString tmp;
        status = argData.getFlagArgument(UpdateMetaDataManagerCmd::XMLParam(), 0, tmp);
        if (!status) {
            status.perror("xml flag parsing failed");
            return status;
        }
        this->m_alternateXML = true;
        this->m_alternateXMLPath = tmp;
    } else {
        this->m_alternateXML = false;
    }

    return MS::kSuccess;
}