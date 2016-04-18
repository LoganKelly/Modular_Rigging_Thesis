/************************************************************
* Summary: Returns the name or dag path of a node connected *
*          to a meta data node. Finds the connection based  *
*          upon the connection name provided.               *
*  Author: Logan Kelly                                      *
*    Date: 4/9/13                                           *
************************************************************/

#include "GetMetaChildByIdCmd.h"
#include "LoadRigUtils.h"
#include "MyErrorChecking.h"
#include <maya/MFn.h>
#include <maya/MFnDagNode.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>

MStatus GetMetaChildByIdCmd::doIt ( const MArgList &args )
{
    MStatus status;

    MStatus paramStatus = parseArgs(args);

    MObject metaNodeObj;
    status = lrutils::getObjFromName(this->m_metaNodeName, metaNodeObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");

    MObject metaChildNodeObj;
    status = lrutils::getMetaChildByRigId(metaNodeObj, this->m_id, metaChildNodeObj);
    MyCheckStatus(status, "lrutils::GetMetaChildByRigId() failed");

    MFnDependencyNode metaChildNodeFn(metaChildNodeObj);
    MString name = metaChildNodeFn.name();
    setResult(name);

    return redoIt();
}

MStatus GetMetaChildByIdCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus GetMetaChildByIdCmd::redoIt()
{
    return dgMod.doIt();
}

MStatus GetMetaChildByIdCmd::parseArgs(const MArgList & args)
{
    MStatus status;
    if( args.length() == 0 ) {
        return MS::kNotFound;
    }

    MArgDatabase argData(syntax(), args);

    //get the node name argument
    if (argData.isFlagSet(GetMetaChildByIdCmd::FileParam())) {
        MString tmp;
        status = argData.getFlagArgument(GetMetaChildByIdCmd::FileParam(), 0, tmp);
        if (!status) {
            status.perror("node flag parsing failed");
            return status;
        }
        this->m_metaNodeName = tmp;
    }
    //get the connection name argument
    if (argData.isFlagSet(GetMetaChildByIdCmd::FileParam2())) {
        MString tmp;
        status = argData.getFlagArgument(GetMetaChildByIdCmd::FileParam2(), 0, tmp);
        if (!status) {
            status.perror("connection flag parsing failed");
            return status;
        }
        this->m_id = tmp;
    }

    return MS::kSuccess;
}


MSyntax GetMetaChildByIdCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(GetMetaChildByIdCmd::FileParam(), GetMetaChildByIdCmd::FileParamLong(), MSyntax::kString);
    syntax.addFlag(GetMetaChildByIdCmd::FileParam2(), GetMetaChildByIdCmd::FileParam2Long(), MSyntax::kString);

    return syntax;
}