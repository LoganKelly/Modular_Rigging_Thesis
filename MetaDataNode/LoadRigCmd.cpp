/************************************************************
* Summary: Creates a MetaData rig network from the selected *
*          rig definition xml file.                         *
*  Author: Logan Kelly                                      *
*    Date: 10/9/12                                          *
************************************************************/

#include <maya/MGlobal.h>
#include "LoadRigCmd.h"
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MFileIO.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MSelectionList.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMessageAttribute.h>
#include "MyErrorChecking.h"
#include "Rig.h"
#include "LoadRigUtils.h"
#include <boost/lexical_cast.hpp>
#include <sstream>

MStatus LoadRigCmd::doIt ( const MArgList &args )
{
    MStatus status;

    status = parseArgs(args);

    if(MS::kSuccess != status )
        return status;

    Rig* aRig = new Rig(m_xmlPath);
    aRig->load(dgMod); 
    setResult("MRN_"+aRig->getName());
    delete aRig;

    //find the script node if it exists
    bool SNexists = false;
    for( MItDependencyNodes nodeIt(MFn::kScript);
        !nodeIt.isDone(); nodeIt.next() ) {
        MFnDependencyNode nodeFn( nodeIt.item() );
        if(nodeFn.name() == MString("updateMDM")) {
            SNexists = true;
            break;
        }
    }

    if(!SNexists) {
        status = dgMod.commandToExecute( MString("scriptNode -stp \"mel\" -beforeScript \"InitMetaDataUI();\" -st 1 -n \"updateMDM\"; ") );
        MyCheckStatusReturn(status, "scriptNode command failed");
    }
    
    return redoIt();
}

MStatus LoadRigCmd::parseArgs(const MArgList & args )
{
    MStatus status;
    if( args.length() == 0 ) {
        status = MStatus::kFailure;
        status.perror("loadRig command requires path parameter\n");
        MGlobal::displayError("loadRig command requires path parameter\n");
        return status;
    }

    MArgDatabase argData(syntax(), args);

    if (argData.isFlagSet(LoadRigCmd::FileParam())) {
        MString tmp;
        status = argData.getFlagArgument(LoadRigCmd::FileParam(), 0, tmp);
        if (!status) {
            status.perror("path flag parsing failed");
            return status;
        }
        this->m_xmlPath = tmp;
    }

    return MS::kSuccess;
}

MStatus LoadRigCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus LoadRigCmd::redoIt()
{
    return dgMod.doIt();
}

MSyntax LoadRigCmd::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(LoadRigCmd::FileParam(), LoadRigCmd::FileParamLong(), MSyntax::kString);

    return syntax;
}