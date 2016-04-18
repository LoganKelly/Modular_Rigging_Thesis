/************************************************************
* Summary: Returns the name or dag path of a node connected *
*          to a meta data node. Finds the connection based  *
*          upon the connection name provided.               *
*  Author: Logan Kelly                                      *
*    Date: 4/9/13                                           *
************************************************************/

#include "GetMetaNodeConnectionCmd.h"
#include "LoadRigUtils.h"
#include "MyErrorChecking.h"
#include <maya/MFn.h>
#include <maya/MFnDagNode.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>

MStatus GetMetaNodeConnectionCmd::doIt ( const MArgList &args )
{
    MStatus status;

    MStatus paramStatus = parseArgs(args);

    MObject metaNodeObj;
    status = lrutils::getObjFromName(this->m_metaNodeName, metaNodeObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");

    MObject connectedObj;
    status = lrutils::getMetaNodeConnection(metaNodeObj, connectedObj, this->m_connectionName);
    MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");

    if( connectedObj.hasFn(MFn::kDagNode) ) {
        MFnDagNode connectedFn(connectedObj);
        MString dagPath = connectedFn.fullPathName();
        setResult(dagPath);
    }
    else {
        MFnDependencyNode connectedFn(connectedObj);
        MString name = connectedFn.name();
        setResult(name);
    }

    return redoIt();
}

MStatus GetMetaNodeConnectionCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus GetMetaNodeConnectionCmd::redoIt()
{
    return dgMod.doIt();
}

MStatus GetMetaNodeConnectionCmd::parseArgs(const MArgList & args)
{
    MStatus status;
    if( args.length() == 0 ) {
        return MS::kNotFound;
    }

    MArgDatabase argData(syntax(), args);

    //get the node name argument
    if (argData.isFlagSet(GetMetaNodeConnectionCmd::FileParam())) {
        MString tmp;
        status = argData.getFlagArgument(GetMetaNodeConnectionCmd::FileParam(), 0, tmp);
        if (!status) {
            status.perror("node flag parsing failed");
            return status;
        }
        this->m_metaNodeName = tmp;
    }
    //get the connection name argument
    if (argData.isFlagSet(GetMetaNodeConnectionCmd::FileParam2())) {
        MString tmp;
        status = argData.getFlagArgument(GetMetaNodeConnectionCmd::FileParam2(), 0, tmp);
        if (!status) {
            status.perror("connection flag parsing failed");
            return status;
        }
        this->m_connectionName = tmp;
    }

    return MS::kSuccess;
}


MSyntax GetMetaNodeConnectionCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(GetMetaNodeConnectionCmd::FileParam(), GetMetaNodeConnectionCmd::FileParamLong(), MSyntax::kString);
    syntax.addFlag(GetMetaNodeConnectionCmd::FileParam2(), GetMetaNodeConnectionCmd::FileParam2Long(), MSyntax::kString);

    return syntax;
}