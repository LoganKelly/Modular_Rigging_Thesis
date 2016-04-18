/************************************************************
* Summary: Class for loading an xml file and accessing its  *
*           data. Uses RapidXml for the file parsing.       *
*  Author: Logan Kelly                                      *
*    Date: 10/9/12                                          *
************************************************************/

#ifndef _XmlGuide
#define _XmlGuide

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <boost/shared_ptr.hpp>
#include <rapidxml.hpp>
#include "GlobalComponentGuide.h"
#include "HipComponentGuide.h"
#include "SpineComponentGuide.h"

class XmlGuide
{
public:
    XmlGuide(MString filePath = "", bool bFullPath = false);
    ~XmlGuide();

    MStatus loadXmlFile(MString filePath, bool bFullPath);
    MStatus getName(MString & name);
    MStatus setName(MString name);
    MStatus getVersion(float & version);
    MStatus getGeoFilePath(MString & filePath);
    MStatus getGeoName(MString & name);
    MStatus getFilePath(MString & path);
    boost::shared_ptr<ComponentGuide> getRootComponent();
    //recursively create component guides from xml nodes
    boost::shared_ptr<ComponentGuide> recursiveGuideCreate(rapidxml::xml_node<>* compNode, boost::shared_ptr<ComponentGuide> parentGuide);
    //create a component guide from an xml node
    boost::shared_ptr<ComponentGuide> createGuide(rapidxml::xml_node<>* compNode, boost::shared_ptr<ComponentGuide> parentGuide);

private:
    MString m_name;
    float m_version;
    MString m_filePath; //full path to the xml file for this guide
    MString m_geoFilePath; //local path to the geometry asset file
    MString m_geoName; //name of the object within the geometry asset file used in the rig
    //rapidxml::xml_document<> m_xmlDoc;
    boost::shared_ptr<ComponentGuide> m_pChildComponent; //root component of the rig (typically the global component)



};

#endif