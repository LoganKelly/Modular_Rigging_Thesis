/************************************************************
* Summary: Creates a MetaData rig network from the selected *
*          rig definition xml file.                         *
*  Author: Logan Kelly                                      *
*    Date: 10/9/12                                          *
************************************************************/

#ifndef _LoadRigCmd
#define _LoadRigCmd

#include <maya/MStatus.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <string>
#include "Rig.h"
using namespace std;

class LoadRigCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual MStatus parseArgs(const MArgList& );
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new LoadRigCmd; }
    static MSyntax newSyntax();

    static const char* FileParam() { return "-p"; }
    static const char* FileParamLong() { return "-path"; }

private:
    MDGModifier dgMod;
    MString m_xmlPath;


};

#endif