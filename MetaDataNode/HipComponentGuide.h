/************************************************************
* Summary: Guide for loading hip type component from xml.   *
*  Author: Logan Kelly                                      *
*    Date: 1/23/13                                          *
************************************************************/

#ifndef _HipComponentGuide
#define _HipComponentGuide

#include "ComponentGuide.h"

class HipComponentGuide : public ComponentGuide
{
public:
    HipComponentGuide(rapidxml::xml_node<>* compNode=NULL);
    ~HipComponentGuide();
    MString getColor() {return m_color;};
    void setColor(MString col) {m_color = col;};
    MString getIcon() {return m_icon;};
    void setIcon(MString icon) {m_icon = icon;};
    

private:
    MStatus readExtraAttribsFromXml(rapidxml::xml_node<>* compNode);
    MString m_color;
    MString m_icon;

};

#endif //_HipComponentGuide