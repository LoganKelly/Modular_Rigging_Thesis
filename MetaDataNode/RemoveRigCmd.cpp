/************************************************************
* Summary: Removes a metadata rig network based upon the    *
*          name of the given metaroot node.                 *
*  Author: Logan Kelly                                      *
*    Date: 10/15/12                                         *
************************************************************/

#include "MetaDataManagerNode.h"
#include <maya/MGlobal.h>
#include "RemoveRigCmd.h"
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MFileIO.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MSelectionList.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMessageAttribute.h>
#include "MyErrorChecking.h"
#include "XmlGuide.h"
#include "LoadRigUtils.h"
#include "Rig.h"
#include <boost/lexical_cast.hpp>
#include <sstream>

MStatus RemoveRigCmd::doIt ( const MArgList &args )
{
    MStatus status;

    status = parseArgs(args);

    if(MS::kSuccess != status )
        return status;

    //get the MObject for the given metaroot name
    MObject rootNodeObj;
    status = lrutils::getObjFromName(this->m_rootName, rootNodeObj);
    MFnDependencyNode rootNodeFn(  rootNodeObj );

    //get the xml path string held in the xmlPath attribute
    MPlug rootXmlPath = rootNodeFn.findPlug(MString("xmlPath"),true,&status);
    MyCheckStatusReturn(status,"findPlug failed");
    MString xmlString;
    rootXmlPath.getValue(xmlString);

    //unload the rig and all its components from the scene
    Rig* aRig = new Rig(xmlString, rootNodeObj);
    aRig->remove(dgMod);
    delete aRig;

    //unload the reference files for the geometry nodes
    //connected to the meta root node
    //this->removeGeoNodes(rootObj);

    //delete the meta root node
    //MPlug metaParentPlug = rootNodeFn.findPlug(MString("metaParent"),true,&status);
    //MPlugArray MDMChildPlugs;
    //metaParentPlug.connectedTo(MDMChildPlugs,true,false,&status);
    //MyCheckStatusReturn(status, "MPlug.connectedTo failed");
    //for(unsigned int i = 0; i < MDMChildPlugs.length(); i++) {
    //    MPlug metaChildPlug = MDMChildPlugs[i];
    //    dgMod.disconnect(metaChildPlug, metaParentPlug);
    //    dgMod.doIt();
    //}
    //status = dgMod.deleteNode(rootNodeObj);
    //MyCheckStatusReturn(status, "dgMod.deleteNode failed");

    return redoIt();
}

MStatus RemoveRigCmd::parseArgs(const MArgList & args )
{
    MStatus status;
    if( args.length() == 0 ) {
        status = MStatus::kFailure;
        status.perror("removeRig command requires name parameter\n");
        MGlobal::displayError("removeRig command requires name parameter\n");
        return status;
    }

    MArgDatabase argData(syntax(), args);

    if (argData.isFlagSet(RemoveRigCmd::FileParam())) {
        MString tmp;
        status = argData.getFlagArgument(RemoveRigCmd::FileParam(), 0, tmp);
        if (!status) {
            status.perror("path flag parsing failed");
            return status;
        }
        this->m_rootName = tmp;
    }

    return MS::kSuccess;
}

MStatus RemoveRigCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus RemoveRigCmd::redoIt()
{
    return dgMod.doIt();
}

MSyntax RemoveRigCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(RemoveRigCmd::FileParam(), RemoveRigCmd::FileParamLong(), MSyntax::kString);

    return syntax;
}

MStatus RemoveRigCmd::removeGeoNodes(MObject rootNode) {
    MStatus status;

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

        //remove the reference node
        {
            //dgMod.disconnect(geometryPlug,geometryPlugs[j]);
            //dgMod.doIt();
            stringstream tmp;
            tmp << "file -removeReference -referenceNode \"" << sNamespace.c_str() << "\";";
            MString referenceCommand = MString(tmp.str().c_str());
            MGlobal::executeCommand(referenceCommand);
        }
    }

    return status;
}