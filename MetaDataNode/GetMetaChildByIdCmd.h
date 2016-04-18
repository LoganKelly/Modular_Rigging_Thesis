/************************************************************
* Summary: Returns a meta child node by the id number       *
*          provided.                                        *
*  Author: Logan Kelly                                      *
*    Date: 4/9/13                                           *
************************************************************/

#ifndef _GetMetaChildByIdCmd
#define _GetMetaChildByIdCmd

#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

class GetMetaChildByIdCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual MStatus parseArgs(const MArgList& );
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new GetMetaChildByIdCmd; }
    static MSyntax newSyntax();

    static const char* FileParam() { return "-n"; }
    static const char* FileParamLong() { return "-node"; }

    static const char* FileParam2() { return "-rid"; }
    static const char* FileParam2Long() { return "-rigIdentifier"; }

private:
    MDGModifier dgMod;
    MString m_metaNodeName;
    MString m_id;

};

#endif