/***************************************************************
* Summary: Class for implementing a global component of a rig. *
*          It provides all the functionality to load, update,  *
*           or remove a global component.                      *
*  Author: Logan Kelly                                         *
*    Date: 10/29/12                                            *
***************************************************************/

#ifndef _GlobalComponent
#define _GlobalComponent

#include <boost/shared_ptr.hpp>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>
#include <maya/MCommandResult.h>
#include <rapidxml.hpp>
#include <vector>
#include "GlobalComponentGuide.h"
#include "Component.h"

class GlobalComponent : public Component
{
public:
    GlobalComponent(GlobalComponentGuidePtr gCompGuide=GlobalComponentGuidePtr(), MString rigName=MString::MString(), boost::shared_ptr<Component> parentComp = boost::shared_ptr<Component>());
    ~GlobalComponent();
    MObject loadComponent(MDGModifier & dgMod);
    void updateComponent(MDGModifier & dgMod, bool forceUpdate = false, bool globalPos = false);
    void removeComponent(MDGModifier & dgMod);

private:
    MObject m_ctlObj;
    MObject m_ctlGroupObj;
    MObject m_rigParentConstraint;
    MObject m_rigScaleConstraint;
    MObject m_noTransformScaleConstraint;

};

#endif //_GlobalComponent