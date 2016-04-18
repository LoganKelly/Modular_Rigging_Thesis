/***************************************************************
* Summary: Class for implementing a spine component of a rig.  *
*          It provides all the functionality to load, update,  *
*           or remove a hip component.                         *
*  Author: Logan Kelly                                         *
*    Date: 5/13/13                                             *
***************************************************************/

#ifndef _SpineComponent
#define _SpineComponent

#include <boost/shared_ptr.hpp>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MVectorArray.h>
#include <maya/MCommandResult.h>
#include <rapidxml.hpp>
#include <vector>
#include "SpineComponentGuide.h"
#include "Component.h"

class SpineComponent : public Component
{
public:
    SpineComponent(SpineComponentGuidePtr hCompGuide=SpineComponentGuidePtr(), MString rigName=MString::MString(), boost::shared_ptr<Component> parentComp = boost::shared_ptr<Component>());
    ~SpineComponent();
    MObject loadComponent(MDGModifier & dgMod);
    void updateComponent(MDGModifier & dgMod, bool forceUpdate = false, bool globalPos = false);
    void removeComponent(MDGModifier & dgMod);
    //creates a IK spline setup
    void buildIKSpline(MObject IKStartJoint, MObject IKEndJoint, MObject FKIndexStart, MObject FKIndexEnd);

private:
    //FK chain objects
    std::vector<MObject> m_vFKCtlObjs;
    std::vector<MObject> m_vFKCtlGroupObjs;
    std::vector<MObject> m_vFKJointObjs;
    std::vector<MObject> m_vFKJointPrntCnstrntObjs;
    //IK chain objects
    std::vector<MObject> m_vSplineIKCtlObjs;
    std::vector<MObject> m_vSplineIKCtlGroupObjs;
    std::vector<MObject> m_vBindJointObjs;
    std::vector<MObject> m_vSplineIKJointPrntCnstrntObjs;
    std::vector<MObject> m_vSplineIKClusterObjs;
    MObject m_SplineIKHandleObj;
    MObject m_SplineIKEndEffectorObj;
    MObject m_SplineIKCurveObj;
    //Hip joint copy objects
    MObject m_hipJointCopyObj;

};

#endif //_SpineComponent