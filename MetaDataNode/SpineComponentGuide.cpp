/************************************************************
* Summary: Guide for loading spine type component from      *
*          an xml guide.                                    *
*  Author: Logan Kelly                                      *
*    Date: 5/13/13                                          *
************************************************************/

#include <maya/MGlobal.h>
#include <sstream>
#include "SpineComponentGuide.h"
#include <fstream>
#include <string>
#include <vector>
#include "MyErrorChecking.h"

using namespace std;
using namespace rapidxml;

SpineComponentGuide::SpineComponentGuide(rapidxml::xml_node<>* compNode) : ComponentGuide(compNode) {
    if(compNode != NULL) {
        this->readExtraAttribsFromXml(compNode);
    }
}

SpineComponentGuide::~SpineComponentGuide() {

}

MStatus SpineComponentGuide::readExtraAttribsFromXml(xml_node<>* compNode) {
    MStatus status = MS::kSuccess;

    string color = compNode->first_attribute("color")->value();
    this->m_color = MString(color.c_str());
    string parentJointNum = compNode->first_attribute("parentJoint")->value();
    this->m_parentJointNum = (unsigned int)::atoi(parentJointNum.c_str());
    string kinematicType = compNode->first_attribute("kinematicType")->value();
    this->m_kinematicType = MString(kinematicType.c_str());
    string fkIcon = compNode->first_attribute("fkIcon")->value();
    this->m_fkIcon = MString(fkIcon.c_str());

    //get the shoulder controller attributes
    xml_node<>* shoulderControlNode = compNode->first_node("shoulderControl");
    string shoulderIcon = shoulderControlNode->first_attribute("icon")->value();
    this->m_shoulderIcon = MString(shoulderIcon.c_str());
    string shoulderColor = shoulderControlNode->first_attribute("color")->value();
    this->m_shoulderColor = MString(shoulderColor.c_str());
    MVector location;
    double xPos = this->getLocAttrib(shoulderControlNode, MString("localX"));
    double yPos = this->getLocAttrib(shoulderControlNode, MString("localY"));
    double zPos = this->getLocAttrib(shoulderControlNode, MString("localZ"));
    location = MVector(xPos,yPos,zPos);

    MVector rotation;
    double xRotation = this->getLocAttrib(shoulderControlNode, MString("rotateX"));
    double yRotation = this->getLocAttrib(shoulderControlNode, MString("rotateY"));
    double zRotation = this->getLocAttrib(shoulderControlNode, MString("rotateZ"));
    rotation = MVector(xRotation,yRotation,zRotation);

    MVector scale;
    double xScale = this->getLocAttrib(shoulderControlNode, MString("scaleX"));
    double yScale = this->getLocAttrib(shoulderControlNode, MString("scaleY"));
    double zScale = this->getLocAttrib(shoulderControlNode, MString("scaleZ"));
    scale = MVector(xScale,yScale,zScale);
    
    m_shoulderLocation.append(location); m_shoulderLocation.append(rotation); m_shoulderLocation.append(scale);

    return status;
}