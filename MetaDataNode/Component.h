/***************************************************************
* Summary: Abstract base class for all component classes of    *
*          various types that derive from it.  It provides the *
*          basic functionality necessary to load, update, or   *
*          remove a rig component.                             *
*  Author: Logan Kelly                                         *
*    Date: 10/19/12                                            *
***************************************************************/

#ifndef _Component
#define _Component

#include <boost/shared_ptr.hpp>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MVectorArray.h>
#include <maya/MDGModifier.h>
#include <rapidxml.hpp>
#include <vector>
#include "ComponentGuide.h"
#include "GlobalComponentGuide.h"
#include "HipComponentGuide.h"
#include "SpineComponentGuide.h"

typedef boost::shared_ptr<ComponentGuide> ComponentGuidePtr;
typedef boost::shared_ptr<GlobalComponentGuide> GlobalComponentGuidePtr;
typedef boost::shared_ptr<HipComponentGuide> HipComponentGuidePtr;
typedef boost::shared_ptr<SpineComponentGuide> SpineComponentGuidePtr;

class Component
{
public:
    Component(ComponentGuidePtr compGuide=ComponentGuidePtr(), MString rigName=MString::MString(), boost::shared_ptr<Component> parentComp = boost::shared_ptr<Component>());
    ~Component();
    void addChildComp(boost::shared_ptr<Component> comp);
    unsigned int getNumChildComps() {return (unsigned int)m_vpChildComps.size();};
    boost::shared_ptr<Component> getChildComp(unsigned int idx) {return m_vpChildComps.at(idx);};
    ComponentGuidePtr getCompGuide() {return m_pCompGuide;};
    virtual MObject loadComponent(MDGModifier & dgMod) = 0; //loads component into the scene, returns its corresponding metaDataNode DG object
    virtual void updateComponent(MDGModifier & dgMod,bool forceUpdate = false, bool globalPos = false) = 0; //updates the component currently in the scene based upon its guide information
    virtual void removeComponent(MDGModifier & dgMod) = 0; //removes the component from the scene
    MStatus setMetaDataNode(MObject metaDataNode);
    MObject getMetaDataNode() {return this->m_metaDataNode;};
    //returns the meta data node of the parent component
    MObject getMetaParentNode();
    //updates the metaParent of the component's meta data node
    void updateMetaParentNode(MDGModifier & dgMod);
    MString getRigName() {return m_rigName;};

protected:

    MString m_rigName;
    //components that are children of this component
    std::vector<boost::shared_ptr<Component>> m_vpChildComps;
    //guide that holds the information necessary for constructing this component
    ComponentGuidePtr m_pCompGuide;
    MObject m_metaDataNode; //the corresponding metaDataNode for this component in the DG
    boost::shared_ptr<Component> m_pParentComp; //parent component of this component (if present)


};

#endif //_Component