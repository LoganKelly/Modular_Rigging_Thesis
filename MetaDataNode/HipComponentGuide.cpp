/************************************************************
* Summary: Guide for loading hip type component from        *
*          an xml guide.                                    *
*  Author: Logan Kelly                                      *
*    Date: 1/23/12                                          *
************************************************************/

#include <maya/MGlobal.h>
#include <sstream>
#include "HipComponentGuide.h"
#include <fstream>
#include <string>
#include <vector>
#include "MyErrorChecking.h"

using namespace std;
using namespace rapidxml;

HipComponentGuide::HipComponentGuide(rapidxml::xml_node<>* compNode) : ComponentGuide(compNode) {
    if(compNode != NULL) {
        this->readExtraAttribsFromXml(compNode);
    }
}

HipComponentGuide::~HipComponentGuide() {

}

MStatus HipComponentGuide::readExtraAttribsFromXml(xml_node<>* compNode) {
    MStatus status = MS::kSuccess;

    string color = compNode->first_attribute("color")->value();
    this->m_color = MString(color.c_str());
    string icon = compNode->first_attribute("icon")->value();
    this->m_icon = MString(icon.c_str());

    return status;
}