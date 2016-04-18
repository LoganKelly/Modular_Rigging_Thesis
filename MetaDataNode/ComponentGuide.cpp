/************************************************************
* Summary: Base class for loading a guide for a component   *
*          from a rapidXML node containing its data.        *
*  Author: Logan Kelly                                      *
*    Date: 10/16/12                                         *
************************************************************/

#include <maya/MGlobal.h>
#include <sstream>
#include "ComponentGuide.h"
#include <fstream>
#include <string>
#include <vector>
#include "MyErrorChecking.h"

using namespace std;
using namespace rapidxml;

ComponentGuide::ComponentGuide(xml_node<> *compNode) {
    if(compNode != NULL) {
        this->readAttribsFromXml(compNode);
    }
}

ComponentGuide::~ComponentGuide() {

}

MStatus ComponentGuide::readAttribsFromXml(xml_node<>* compNode) {
    MStatus status = MS::kSuccess;

    //get component attribute values common to all component types
    string name = compNode->first_attribute("name")->value();
    this->m_name = MString(name.c_str());
    string type = compNode->first_attribute("type")->value();
    this->m_type = MString(type.c_str());
    string version = compNode->first_attribute("version")->value();
    this->m_version = (float)::atof(version.c_str());
    string rigId = compNode->first_attribute("rigId")->value();
    this->m_rigId = MString(rigId.c_str());

    //get the information from lowResGeo nodes (if any)
    xml_node<>* lowResGeoNode = compNode->first_node("lowResGeo");
    while( lowResGeoNode != NULL ) {
        MStringArray geoAttribs;
        string geoName = lowResGeoNode->first_attribute("name")->value();
        geoAttribs.append(MString(geoName.c_str()));
        string geoJoint = lowResGeoNode->first_attribute("joint")->value();
        geoAttribs.append(MString(geoJoint.c_str()));
        this->m_vLowResGeoAttribs.push_back(geoAttribs);
        lowResGeoNode = lowResGeoNode->next_sibling("lowResGeo");        
    }

    //get the information from the location nodes (if any)
    xml_node<>* locNode = compNode->first_node("location");
    status = this->readLocations(locNode);

    return status;
}

MStatus ComponentGuide::readLocations(rapidxml::xml_node<>* locNode) {
    MStatus status = MS::kFailure;

    while( locNode != NULL ) {
        MVector location;
        double xPos = this->getLocAttrib(locNode, MString("localX"));
        double yPos = this->getLocAttrib(locNode, MString("localY"));
        double zPos = this->getLocAttrib(locNode, MString("localZ"));
        location = MVector(xPos,yPos,zPos);

        MVector rotation;
        double xRotation = this->getLocAttrib(locNode, MString("rotateX"));
        double yRotation = this->getLocAttrib(locNode, MString("rotateY"));
        double zRotation = this->getLocAttrib(locNode, MString("rotateZ"));
        rotation = MVector(xRotation,yRotation,zRotation);

        MVector scale;
        double xScale = this->getLocAttrib(locNode, MString("scaleX"));
        double yScale = this->getLocAttrib(locNode, MString("scaleY"));
        double zScale = this->getLocAttrib(locNode, MString("scaleZ"));
        scale = MVector(xScale,yScale,zScale);
        
        MVectorArray locData;
        locData.append(location); locData.append(rotation); locData.append(scale);

        this->m_vLocations.push_back(locData);

        locNode = locNode->first_node("location");
    }

    status = MS::kSuccess;
    return status;
}

double ComponentGuide::getLocAttrib(rapidxml::xml_node<>* locNode, MString attName) {
    double attrib = 0.0;
    if(locNode->first_attribute(attName.asChar())) {
        string sAttrib = locNode->first_attribute(attName.asChar())->value();
        attrib = (double)::atof(sAttrib.c_str());
    }
    return attrib;
}

MString ComponentGuide::getType() {
    return this->m_type;
}

MStatus ComponentGuide::addChildCompGuide(boost::shared_ptr<ComponentGuide> compGuide) {
    if(compGuide != boost::shared_ptr<ComponentGuide>()) {
        this->m_vpChildCompGuides.push_back(compGuide);
        return MS::kSuccess;
    }
    return MS::kFailure;
}