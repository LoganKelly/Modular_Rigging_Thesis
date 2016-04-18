/***************************************************************
* Summary: Class for implementing a hip component of a rig.    *
*          It provides all the functionality to load, update,  *
*           or remove a hip component.                         *
*  Author: Logan Kelly                                         *
*    Date: 1/23/13                                             *
***************************************************************/

#ifndef _HipComponent
#define _HipComponent

#include <boost/shared_ptr.hpp>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>
#include <maya/MCommandResult.h>
#include <rapidxml.hpp>
#include <vector>
#include "HipComponentGuide.h"
#include "Component.h"

class HipComponent : public Component
{
public:
    HipComponent(HipComponentGuidePtr hCompGuide=HipComponentGuidePtr(), MString rigName=MString::MString(), boost::shared_ptr<Component> parentComp = boost::shared_ptr<Component>());
    ~HipComponent();
    MObject loadComponent(MDGModifier & dgMod);
    void updateComponent(MDGModifier & dgMod, bool forceUpdate = false, bool globalPos = false);
    void removeComponent(MDGModifier & dgMod);

private:
    MObject m_ctlObj;
    MObject m_ctlGroupObj;
    MObject m_hipJointObj;
    MObject m_hipJointParentConstraint;

};

#endif //_HipComponent