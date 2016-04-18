//
// Copyright (C) 
// 
// File: pluginMain.cpp
//
// Author: Maya Plug-in Wizard 2.0
//

#include "MetaDataNode.h"
#include "UpdateMetaDataManagerCmd.h"
#include "ListRootsCmd.h"
#include "LoadRigCmd.h"
#include "RemoveRigCmd.h"
#include "GetMetaNodeConnectionCmd.h"
#include "GetMetaChildByIdCmd.h"
#include "MetaRootNode.h"
#include "MDGlobalNode.h"
#include "MDHipNode.h"
#include "MDSpineNode.h"
#include "MetaDataManagerNode.h"
#include "MyErrorChecking.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "", "2012", "Any");

    /* Register all commands */
    status = plugin.registerCommand( "updateMetaDataManager", UpdateMetaDataManagerCmd::creator, UpdateMetaDataManagerCmd::newSyntax );

    MyCheckStatusReturn(status, "registerCommand updateMetaDataManager failed");

    status = plugin.registerCommand( "listMetaRoots", ListRootsCmd::creator );

    MyCheckStatusReturn(status, "registerCommand listMetaRoots failed");

    status = plugin.registerCommand("loadRig", LoadRigCmd::creator, LoadRigCmd::newSyntax );

    MyCheckStatusReturn(status, "registerCommand loadRig failed");

    status = plugin.registerCommand("removeRig", RemoveRigCmd::creator, RemoveRigCmd::newSyntax );

    MyCheckStatusReturn(status, "registerCommand removeRig failed");

    status = plugin.registerCommand( "getMetaNodeConnection", GetMetaNodeConnectionCmd::creator, GetMetaNodeConnectionCmd::newSyntax );

    MyCheckStatusReturn(status, "registerCommand getMetaNodeConnection failed");

    status = plugin.registerCommand( "getMetaChildById", GetMetaChildByIdCmd::creator, GetMetaChildByIdCmd::newSyntax );

    MyCheckStatusReturn(status, "registerCommand getMetaChildById failed");

    /* Register all nodes */
	status = plugin.registerNode( "MetaDataNode", MetaDataNode::id, MetaDataNode::creator,
								  MetaDataNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    status = plugin.registerNode( "MetaRootNode", MetaRootNode::id, MetaRootNode::creator,
                                  MetaRootNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    status = plugin.registerNode( "MDGlobalNode", MDGlobalNode::id, MDGlobalNode::creator,
                                  MDGlobalNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    status = plugin.registerNode( "MDHipNode", MDHipNode::id, MDHipNode::creator,
                                  MDHipNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    status = plugin.registerNode( "MDSpineNode", MDSpineNode::id, MDSpineNode::creator,
                                  MDSpineNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    status = plugin.registerNode( "MetaDataManagerNode", MetaDataManagerNode::id, MetaDataManagerNode::creator,
                                  MetaDataManagerNode::initialize );
    MyCheckStatusReturn(status, "registerNode failed");

    //MStringArray items;
    //items.append(MString("Metadata Rigging"));
    //plugin.removeMenuItem(items);
    //plugin.addMenuItem("Metadata Rigging","MayaWindow","","");

    status = plugin.registerUI("InitMetaDataUI","UninitMetaDataUI");
    MyCheckStatusReturn(status, "registerUI failed");

	return status;
}

MStatus uninitializePlugin( MObject obj)
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin plugin( obj );

    status = plugin.deregisterCommand( "updateMetaDataManager" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

    status = plugin.deregisterCommand( "listMetaRoots" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

    status = plugin.deregisterCommand( "loadRig" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

    status = plugin.deregisterCommand( "removeRig" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

    status = plugin.deregisterCommand( "getMetaNodeConnection" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

    status = plugin.deregisterCommand( "getMetaChildById" );
    if (!status) {
        status.perror("deregisterCommand failed");
        return status;
    }

	status = plugin.deregisterNode( MetaDataNode::id );
	if (!status) {
		status.perror("deregisterNode failed");
		return status;
	}

    status = plugin.deregisterNode( MetaRootNode::id );
    if (!status) {
        status.perror("deregisterNode failed");
        return status;
    }

    status = plugin.deregisterNode( MetaDataManagerNode::id );
    if (!status) {
        status.perror("deregisterNode failed");
        return status;
    }

    status = plugin.deregisterNode( MDGlobalNode::id );
    if (!status) {
        status.perror("deregisterNode failed");
        return status;
    }

    status = plugin.deregisterNode( MDHipNode::id );
    if (!status) {
        status.perror("deregisterNode failed");
        return status;
    }

    status = plugin.deregisterNode( MDSpineNode::id );
    if (!status) {
        status.perror("deregisterNode failed");
        return status;
    }

	return status;
}
