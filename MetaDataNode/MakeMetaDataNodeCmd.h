/***********************************************************
* Summary: Creates a MetaDataNode DG Node and connects it  *
*          to the selected node.                           *
*  Author: Logan Kelly                                     *
*    Date: 8/31/12                                         *
***********************************************************/

#ifndef _MakeMetaDataNodeCmd
#define _MakeMetaDataNodeCmd

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

class MakeMetaDataNodeCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new MakeMetaDataNodeCmd; }
    static MSyntax newSyntax();

private:
    MDGModifier dgMod;

};

#endif