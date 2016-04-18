/************************************************************
* Summary: Returns the name or dag path of a node connected *
*          to a meta data node. Finds the connection based  *
*          upon the connection name provided.               *
*  Author: Logan Kelly                                      *
*    Date: 4/9/13                                           *
************************************************************/

#ifndef _GetMetaNodeConnectionCmd
#define _GetMetaNodeConnectionCmd

#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

class GetMetaNodeConnectionCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual MStatus parseArgs(const MArgList& );
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new GetMetaNodeConnectionCmd; }
    static MSyntax newSyntax();

    static const char* FileParam() { return "-n"; }
    static const char* FileParamLong() { return "-node"; }

    static const char* FileParam2() { return "-c"; }
    static const char* FileParam2Long() { return "-connection"; }

private:
    MDGModifier dgMod;
    MString m_metaNodeName;
    MString m_connectionName;

};

#endif