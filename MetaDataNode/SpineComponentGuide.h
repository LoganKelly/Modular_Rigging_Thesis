/************************************************************
* Summary: Guide for loading spine type component from xml. *
*  Author: Logan Kelly                                      *
*    Date: 5/13/13                                          *
************************************************************/

#ifndef _SpineComponentGuide
#define _SpineComponentGuide

#include "ComponentGuide.h"

class SpineComponentGuide : public ComponentGuide
{
public:
    SpineComponentGuide(rapidxml::xml_node<>* compNode=NULL);
    ~SpineComponentGuide();
    unsigned int getParentJointNum() {return m_parentJointNum;};
    MString getShoulderIcon() {return m_shoulderIcon;};
    MString getShoulderColor() {return m_shoulderColor;};
    MVectorArray getShoulderLocation() {return m_shoulderLocation;};
    MString getColor() {return m_color;};
    void setColor(MString col) {m_color = col;};
    MString getFKIcon() {return m_fkIcon;};
    MString getKinematicType() {return m_kinematicType;};

private:
    MStatus readExtraAttribsFromXml(rapidxml::xml_node<>* compNode);
    unsigned int m_parentJointNum;
    MString m_kinematicType;
    MString m_shoulderIcon;
    MString m_shoulderColor;
    MString m_color;
    MVectorArray m_shoulderLocation;
    MString m_fkIcon;

};

#endif //_SpineComponentGuide