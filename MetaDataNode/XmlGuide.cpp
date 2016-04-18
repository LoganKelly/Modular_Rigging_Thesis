/************************************************************
* Summary: Class for loading an xml file and accessing its  *
*           data. Uses RapidXml for the file parsing.       *
*  Author: Logan Kelly                                      *
*    Date: 10/9/12                                          *
************************************************************/

#include <maya/MGlobal.h>
#include <sstream>
#include "XmlGuide.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "MyErrorChecking.h"

using namespace std;
using namespace rapidxml;

XmlGuide ::XmlGuide(MString filePath, bool bFullPath) {
    if(filePath.length() > 0) {
        MStatus status = this->loadXmlFile(filePath, bFullPath);    
        MyCheckStatus(status, "loadXmlFile failed");
    }
}

XmlGuide ::~XmlGuide() {

}

MStatus XmlGuide ::loadXmlFile(MString filePath, bool bFullPath) {
    MStatus success = MStatus::kFailure;

    MString projPath = MGlobal::executeCommandStringResult(MString("workspace -q -rd;"),false,false);
    MString relativePath = filePath.substring(2,filePath.numChars() - 1);

    //assemble the full file path of the xml file if necessary
    MString fullPath;
    if(bFullPath) {
        fullPath = filePath;
    } else {    
        MString projPath = MGlobal::executeCommandStringResult(MString("workspace -q -rd;"),false,false);
        MString relativePath = filePath.substring(2,filePath.numChars() - 1);
        fullPath = projPath + relativePath;
    }
    this->m_filePath = fullPath;

    //open the xml file and read its contents into a string
    ifstream xmlFile;
    xmlFile.open(fullPath.asWChar());
    //file open failed
    if(!xmlFile) {
        xmlFile.close();
        MyCheckStatusReturn(success, "Could not open the input xml file: "+fullPath);
    }
    string xmlContent;
    xmlContent.assign( (istreambuf_iterator<char>(xmlFile)),
                       (istreambuf_iterator<char>()       ) );
    xmlFile.close();

    //make a safe-to-modify copy of xmlContent
    vector<char> xmlContentCopy(xmlContent.begin(),xmlContent.end());
    xmlContentCopy.push_back('\0');

    //parse the xml contents with RapidXML and get the name
    //Good example for parsing an xml file with RapidXML:
    //http://www.ffuts.org/blog/quick-notes-on-how-to-use-rapidxml/
    xml_document<> doc;
    try {
        doc.parse<parse_declaration_node | parse_no_data_nodes>(&xmlContentCopy[0]);
    }
    catch (rapidxml::parse_error e) {
        MyCheckStatusReturn(success,"Could not parse the xml file.");
    }
    string xmlNameString = doc.first_node()->first_attribute("name")->value();

    m_name = MString( xmlNameString.c_str() );

    //get the rig version
    string versionString = doc.first_node()->first_attribute("version")->value();
    float version = (float)::atof(versionString.c_str());
    m_version = version;

    //get geo element info (file path and name)
    xml_node<>* geoNode = doc.first_node()->first_node("geo");
    if( geoNode != NULL) {
        //file path
        string geoFilePath = geoNode->first_attribute("file")->value();
        m_geoFilePath = geoFilePath.c_str();
        //name
        string geoName = geoNode->first_attribute("name")->value();
        m_geoName = geoName.c_str();
    }

    if (m_name.length() > 0 && versionString.length() > 0 ) {
        success = MStatus::kSuccess;
    }

    //create the guides for all components
    xml_node<>* rootComponentNode = doc.first_node()->first_node("component");
    if(rootComponentNode != NULL) {
        this->m_pChildComponent = this->recursiveGuideCreate(rootComponentNode, boost::shared_ptr<ComponentGuide>());
    }
    //this->m_pChildComponent.reset( new ComponentGuide(rootComponentNode) );

    return success;
}

boost::shared_ptr<ComponentGuide> XmlGuide::recursiveGuideCreate(xml_node<>* compNode, boost::shared_ptr<ComponentGuide> parentGuide) {
    boost::shared_ptr<ComponentGuide> compGuide = createGuide(compNode, parentGuide);
    //iterate through all child xml component nodes and call recursive create for them
    xml_node<>* childCompNode = compNode->first_node("component");
    while( childCompNode != NULL ) {
        boost::shared_ptr<ComponentGuide> childCompGuide = this->recursiveGuideCreate(childCompNode, compGuide);
        childCompNode = childCompNode->next_sibling("component");
    }
    return compGuide;
}

boost::shared_ptr<ComponentGuide> XmlGuide::createGuide(xml_node<>* compNode, boost::shared_ptr<ComponentGuide> parentGuide) {
    xml_node<>* componentNode = compNode;
    boost::shared_ptr<ComponentGuide> compGuide;
    if(componentNode != NULL) {
        string sCompType = componentNode->first_attribute("type")->value();
        MString componentType = MString(sCompType.c_str());
        if( componentType == MString("global") ) {
            compGuide.reset( new GlobalComponentGuide(componentNode) );
        }
        else if ( componentType == MString("hip") ) {
            compGuide.reset( new HipComponentGuide(componentNode) );
        }
        else if ( componentType == MString("spine") ) {
            compGuide.reset( new SpineComponentGuide(componentNode) );
        }
        else {
            stringstream msg; msg << "component type " << componentType.asChar() << " is invalid";
            MGlobal::displayError(msg.str().c_str());
        }
        if( parentGuide != boost::shared_ptr<ComponentGuide>() )
            parentGuide->addChildCompGuide(compGuide);
    }
    return compGuide;
}

MStatus XmlGuide::getName(MString & name) {
    MStatus success = MStatus::kFailure;

    if(m_name.length() > 0) {
        name = m_name;
        success = MStatus::kSuccess;
    } 

    return success;
}

MStatus XmlGuide::setName(MString name) {
    MStatus status = MS::kFailure;

    if(name.length() > 0) {
        m_name = name;
        status = MS::kSuccess;
    }

    return status;
}

MStatus XmlGuide::getVersion(float & version) {
    MStatus success = MS::kFailure;

    if(m_version != 0.0f) {
        version = m_version;
        success = MS::kSuccess;
    }

    return success;
}

MStatus XmlGuide::getFilePath(MString& path) {
    MStatus success = MS::kFailure;

    if(m_filePath.length() > 0) {
        path = m_filePath;
        success = MS::kSuccess;
    }

    return success;
}

MStatus XmlGuide::getGeoFilePath(MString & filePath) {
    MStatus success = MS::kFailure;

    if(m_geoFilePath.length() > 0) {
        filePath = m_geoFilePath;
        success = MStatus::kSuccess;
    }

    return success;
}

MStatus XmlGuide::getGeoName(MString &name) {
    MStatus success = MS::kFailure;

    if(m_geoName.length() > 0 ) {
        name = m_geoName;
        success = MS::kSuccess;
    }

    return success;
}

boost::shared_ptr<ComponentGuide> XmlGuide::getRootComponent() {
    return this->m_pChildComponent;
}