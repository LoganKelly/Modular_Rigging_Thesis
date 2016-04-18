/*********************************************************************
* Summary: Abstract base class for loading a guide for a component   *
*          from a rapidXML node containing its data.                 *
*  Author: Logan Kelly                                               *
*    Date: 10/16/12                                                  *
*********************************************************************/

#ifndef _ComponentGuide
#define _ComponentGuide

#include <boost/shared_ptr.hpp>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>
#include <rapidxml.hpp>
#include <vector>

class ComponentGuide
{
public:
    ComponentGuide(rapidxml::xml_node<>* compNode=NULL);
    virtual ~ComponentGuide();
    MString getType();
    unsigned int getNumChildComps() {return  (unsigned int)m_vpChildCompGuides.size();} ;
    boost::shared_ptr<ComponentGuide> getChildCompGuide(unsigned int idx) {return m_vpChildCompGuides.at(idx);};
    MString getName() {return m_name;};
    MVectorArray getLocation(unsigned int i) {return m_vLocations.at(i);};
    unsigned int getNumLocations() {return (unsigned int)m_vLocations.size();};
    std::vector<MVectorArray> getLocations() {return m_vLocations;};
    float getVersion() {return m_version;};
    MString getRigId() {return m_rigId;};
    MStatus addChildCompGuide(boost::shared_ptr<ComponentGuide> compGuide);

protected:
    //read attributes common to all component nodes
    MStatus readAttribsFromXml(rapidxml::xml_node<>* compNode);
    //read attributes unique to specific component types
    virtual MStatus readExtraAttribsFromXml(rapidxml::xml_node<>* compNode) = 0;
    //read attributes for locations given a top level location node
    MStatus readLocations(rapidxml::xml_node<>* locNode);
    //get the specified attribute from the given location node
    //returns 0 if not found
    double getLocAttrib(rapidxml::xml_node<>* locNode, MString attName);


    MString m_type;
    MString m_name;
    float m_version;
    MString m_rigId;
    //guides for components that are children of this component
    std::vector<boost::shared_ptr<ComponentGuide>> m_vpChildCompGuides;
    std::vector<MStringArray> m_vLowResGeoAttribs;
    //locations of interest used for building this component
    //for each MVectorArray corresponding to a location:
    //MVector[0] = local translation in x, y, and z
    //MVector[1] = local rotation in x, y, and z
    //MVector[2] = scale in x, y, and z
    std::vector<MVectorArray> m_vLocations;


};

#endif