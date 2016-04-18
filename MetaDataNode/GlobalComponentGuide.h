/************************************************************
* Summary: Guide for loading global type component from     *
*          an xml guide.                                    *
*  Author: Logan Kelly                                      *
*    Date: 10/16/12                                         *
************************************************************/

#ifndef _GlobalComponentGuide
#define _GlobalComponentGuide

#include "ComponentGuide.h"

class GlobalComponentGuide : public ComponentGuide
{
public:
    GlobalComponentGuide(rapidxml::xml_node<>* compNode=NULL);
    ~GlobalComponentGuide();
    MString getColor() {return m_color;};
    void setColor(MString col) {m_color = col;};
    MString getIcon() {return m_icon;};
    void setIcon(MString icon) {m_icon = icon;};

private:
    MStatus readExtraAttribsFromXml(rapidxml::xml_node<>* compNode);
    MString m_color;
    MString m_icon;



};

#endif //_GlobalComponentGuide