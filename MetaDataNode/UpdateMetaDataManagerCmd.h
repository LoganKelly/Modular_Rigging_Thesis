/***********************************************************
* Summary: Updates all of the meta data node networks      *
*           connected to the MetaDataManager based upon    *
*          their associated XML rig definitions.           *
*  Author: Logan Kelly                                     *
*    Date: 9/19/12                                         *
***********************************************************/

#ifndef _UpdateMetaDataManagerCmd
#define _UpdateMetaDataManagerCmd

#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include "XmlGuide.h"

class UpdateMetaDataManagerCmd : public MPxCommand
{
public:
    virtual MStatus doIt ( const MArgList& );
    virtual MStatus undoIt();
    virtual MStatus redoIt();
    virtual MStatus parseArgs(const MArgList& );
    virtual bool isUndoable() const { return true; }

    static void *creator() { return new UpdateMetaDataManagerCmd; }
    static MSyntax newSyntax();

    //name of a meta-root node in the scene, i.e. MRN_test
    static const char* FileParam() { return "-n"; }
    static const char* FileParamLong() { return "-name"; }
    //force update of the rig by ignoring version numbers
    static const char* ForceParam() { return "-f"; }
    static const char* ForceParamLong() { return "-force"; }
    //replacement xml file path
    static const char* XMLParam() { return "-r"; }
    static const char* XMLParamLong() { return "-replace"; }
    //preserve global position of keys on all controller objects
    static const char* GlobalPosParam() { return "-g"; }
    static const char* GlobalPosParamLong() { return "-globalPos"; }

private:
    virtual bool checkXmlFileVersion(float version);
    virtual MStatus updateHeaderInfo(MObject rootNode);
    virtual MStatus updateGeoNodes(MObject rootNode);
    MDGModifier dgMod;
    XmlGuide* m_xmlGuide;
    MString m_rootNodeName;
    MString m_alternateXMLPath; //path to the alternate xml file
    bool m_forceUpdate;
    bool m_alternateXML; //use an alternate xml file for the update
    bool m_globalPos; //fix keys to global position of controller

};

#endif