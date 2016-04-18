/************************************************************
* Summary: Removes a metadata rig network based upon the    *
*          name of the given metaroot node.                 *
*  Author: Logan Kelly                                      *
*    Date: 10/15/12                                         *
************************************************************/

#ifndef _RemoveRigCmd
#define _RemoveRigCmd

#include <maya/MStatus.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <string>
using namespace std;

class RemoveRigCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual MStatus parseArgs(const MArgList& );
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new RemoveRigCmd; }
    static MSyntax newSyntax();

    static const char* FileParam() { return "-n"; }
    static const char* FileParamLong() { return "-name"; }

private:
    virtual MStatus removeGeoNodes(MObject rootNode);

    MDGModifier dgMod;
    MString m_rootName;


};

#endif