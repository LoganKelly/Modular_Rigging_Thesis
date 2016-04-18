/***********************************************************
* Summary: Returns a list of all metaRoot nodes connected  *
*          to the Meta Data Manager, if the manager exists *
*  Author: Logan Kelly                                     *
*    Date: 9/27/12                                         *
***********************************************************/

#ifndef _ListRootsCmd
#define _ListRootsCmd

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

class ListRootsCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new ListRootsCmd; }
    static MSyntax newSyntax();

private:
    MDGModifier dgMod;

};

#endif