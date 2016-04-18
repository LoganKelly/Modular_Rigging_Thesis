/***********************************************************
* Summary: Creates a MetaDataNode DG Node and connects it  *
*          to the selected node.                           *
*  Author: Logan Kelly                                     *
*    Date: 8/31/12                                         *
***********************************************************/

#include "MakeMetaDataNodeCmd.h"
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFnTransform.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include "MyErrorChecking.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <string>
using namespace std;
#include "XmlGuide.h"

MStatus MakeMetaDataNodeCmd::doIt ( const MArgList &args )
{
    MStatus stat;

    MGlobal::executeCommand( MString("polyCube -ch on -o on -cuv 4 ;") );

    // Get a list of currently selected objects
    MSelectionList selection;
    MGlobal::getActiveSelectionList( selection );

    MDagPath dagPath;
    MFnTransform transformFn;
    MString name;

    //Iterate over all the selected nodes
    MItSelectionList iter( selection, MFn::kTransform );
    for(;!iter.isDone(); iter.next() )
    {
        iter.getDagPath( dagPath );
        transformFn.setObject( dagPath );

        MFnMessageAttribute mAttr;

        MObject transformAttr = mAttr.create("metaParent", "metaParent");

        transformFn.addAttribute(transformAttr);


        MStatus status;
        //get the node name from the xml file      
        MString filePath = MString("./rigDefinitions/test.xml");
        XmlGuide* xmlGuide = new XmlGuide(filePath, false);
        MString name;
        status = xmlGuide->getName(name);
        delete xmlGuide;
        MyCheckStatusReturn(status, "getXMLname failed");
        //create MetaDataManager node
        MObject metaDataManagerNodeObj = dgMod.createNode( "MetaDataManagerNode", &status );
        MyCheckStatusReturn(status, "createNode failed");
        MFnDependencyNode depManagerNodeFn( metaDataManagerNodeObj );
        //create and set up root node
        MObject metaRootNodeObj = dgMod.createNode( "MetaRootNode", &status );
        MyCheckStatusReturn(status, "createNode failed");
        MFnDependencyNode depRootNodeFn( metaRootNodeObj );
        MString rootNodeName = "MRN_";
        rootNodeName += name;
        status = dgMod.newPlugValueString( depRootNodeFn.findPlug("xmlPath"), filePath );
        dgMod.renameNode(metaRootNodeObj, rootNodeName);
        MyCheckStatusReturn(status, "newPlugValueString failed");
        //create MetaDataNode
        MString nodeName = "MDN_";
        nodeName += name;
        MObject metaDataNodeObj = dgMod.createNode( "MetaDataNode", &status );
        MyCheckStatusReturn(status, "createNode failed");
        dgMod.renameNode(metaDataNodeObj, nodeName);
        MFnDependencyNode depNodeFn( metaDataNodeObj );
        

        //create meta data network connections
        status = dgMod.connect( depManagerNodeFn.findPlug("metaRoots"), depRootNodeFn.findPlug("metaParent") );
        MyCheckStatusReturn(status, "connect failed");
        status = dgMod.connect( depRootNodeFn.findPlug("metaChildren"), depNodeFn.findPlug("metaParent") );
        MyCheckStatusReturn(status, "connect failed");
        status = dgMod.connect( depNodeFn.findPlug("metaChildren"), transformFn.findPlug("metaParent") );
        MyCheckStatusReturn(status, "connect failed");

        status = dgMod.commandToExecute( MString("scriptNode -beforeScript \"updateMetaDataManager;\" -st 1 -n \"updateMDM\"; ") );
        MyCheckStatusReturn(status, "scriptNode command failed");

    }

    return redoIt();
}

MStatus MakeMetaDataNodeCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus MakeMetaDataNodeCmd::redoIt()
{
    return dgMod.doIt();
}