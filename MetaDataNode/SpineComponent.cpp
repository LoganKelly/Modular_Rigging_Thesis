/***************************************************************
* Summary: Class for implementing a hip component of a rig.    *
*          It provides all the functionality to load, update,  *
*           or remove a hip component.                         *
*  Author: Logan Kelly                                         *
*    Date: 5/13/13                                             *
***************************************************************/

#include "MDHipNode.h"
#include "MDSpineNode.h"
#include "SpineComponent.h"
#include "MyErrorChecking.h"
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "LoadRigUtils.h"
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <map>

using namespace std;

SpineComponent::SpineComponent(SpineComponentGuidePtr hCompGuide, MString rigName, boost::shared_ptr<Component> parentComp) : Component(boost::dynamic_pointer_cast<ComponentGuide>(hCompGuide), rigName, parentComp) {

}

SpineComponent::~SpineComponent() {

}

MObject SpineComponent::loadComponent(MDGModifier & dgMod) {
    MStatus status = MS::kFailure;
    //get the metaRoot node of this rig
    MObject metaRootObj;
    status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
    //get the extras layer from the meta root
    MObject extrasLayerObj;
    status = lrutils::getMetaNodeConnection(metaRootObj, extrasLayerObj, "extrasLayer");
    MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
    MFnDependencyNode extrasLayerFn(extrasLayerObj);
    MString extrasLayerName = extrasLayerFn.name();

    //used for creating metaParent attributes for objects
    MFnMessageAttribute mAttr;
    SpineComponentGuidePtr spineGuide = boost::dynamic_pointer_cast<SpineComponentGuide>(this->m_pCompGuide);
    //get the meta data node's parent
    MObject metaDataParentNode = this->m_pParentComp->getMetaDataNode();
    MFnDependencyNode metaDataParentFn( metaDataParentNode );

    this->m_metaDataNode = dgMod.createNode( "MDSpineNode", &status );
    MyCheckStatus(status, "createNode failed");
    dgMod.doIt();

    MString metaNodeName = "MSN_";
    metaNodeName += this->m_rigName + "_";
    metaNodeName += this->m_pCompGuide->getName();
    dgMod.renameNode(this->m_metaDataNode, metaNodeName);

    //set the meta data node's attributes
    MFnDependencyNode depMetaDataNodeFn(this->m_metaDataNode);
    status = dgMod.newPlugValueFloat( depMetaDataNodeFn.findPlug("version"), this->m_pCompGuide->getVersion() );
    MyCheckStatus(status, "newPlugValueFloat() failed");
    status = dgMod.newPlugValueString( depMetaDataNodeFn.findPlug("rigId"), this->m_pCompGuide->getRigId() );
    MyCheckStatus(status, "newPlugValueString() failed");
    status = dgMod.newPlugValueString( depMetaDataNodeFn.findPlug("KinematicType"), spineGuide->getKinematicType() );
    MyCheckStatus(status, "newPlugValueString() failed");
    dgMod.doIt();


    if(spineGuide->getKinematicType() == "splineIK" || spineGuide->getKinematicType() == "stretchySplineIK") {
        //create the spine's joints
        //create the spline IK joints that are also used for binding
        MString prefix = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_splineIK_BIND";
        MPlug metaDataPlug = depMetaDataNodeFn.findPlug("BindJoints");
        MObject metaDataParentNode = this->m_pParentComp->getMetaDataNode();
        MObject parentJoint;
        status = lrutils::getJointByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentJoint);
        MyCheckStatus(status, "lrutils::getJointByNum() failed");
        m_vBindJointObjs = lrutils::buildSkeletonFromGuide(this->m_pCompGuide->getLocations(), prefix, metaDataPlug, parentJoint);
        MGlobal::executeCommand("select -cl;");

        MFnDependencyNode metaDataParentFn( metaDataParentNode );
        if(metaDataParentFn.typeId() == MDHipNode::id) {
            //create the hip joint copy
            MObject rigGroupObj;
            status = lrutils::getMetaNodeConnection(metaRootObj, rigGroupObj, "rigGroup");
            MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            prefix = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_HipJointCopy";
            this->m_hipJointCopyObj = lrutils::createJointFromLocation(this->m_pParentComp->getCompGuide()->getLocation(0),prefix,0,rigGroupObj);
            MFnTransform hipJointCopyFn( this->m_hipJointCopyObj );
            MObject jointAttr = mAttr.create("metaParent", "metaParent");
            hipJointCopyFn.addAttribute(jointAttr);
            //connect the metaparent attribute to the MDSpine node
            metaDataPlug = depMetaDataNodeFn.findPlug("HipJointCopy");
            status = dgMod.connect( metaDataPlug, hipJointCopyFn.findPlug("metaParent") );
            MyCheckStatus(status, "connect failed");
            dgMod.doIt();
            MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ extrasLayerName+" "+ hipJointCopyFn.name()+";");
            MGlobal::executeCommand("select -cl;");
        }
        //create the FK joints
        prefix = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_FK";
        metaDataPlug = depMetaDataNodeFn.findPlug("FKJoints");
        metaDataParentNode = this->m_pParentComp->getMetaDataNode();
        status = lrutils::getJointByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentJoint);
        MyCheckStatus(status, "lrutils::getJointByNum() failed");
        if(metaDataParentFn.typeId() == MDHipNode::id) {
            m_vFKJointObjs = lrutils::buildSkeletonFromGuide(this->m_pCompGuide->getLocations(), prefix, metaDataPlug, this->m_hipJointCopyObj);
        } else {
            m_vFKJointObjs = lrutils::buildSkeletonFromGuide(this->m_pCompGuide->getLocations(), prefix, metaDataPlug);
        }
        MGlobal::executeCommand("select -cl;");
        //orient the hip copy joint chain if it exists
        if(!this->m_hipJointCopyObj.isNull()) {
            MFnTransform hipJointCopyFn( this->m_hipJointCopyObj );
            MGlobal::executeCommand("joint -e -zso -oj \"xyz\" -sao \"yup\" -ch " + hipJointCopyFn.fullPathName() + ";");
        }
        dgMod.doIt();
        this->buildIKSpline(parentJoint,m_vBindJointObjs.back(),m_vFKJointObjs.front(),m_vFKJointObjs.back());
    }
    else if (spineGuide->getKinematicType() == "FK") {
        //create the spine's joints
        //create the spline IK joints that are also used for binding
        MString prefix = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_FK_BIND";
        MPlug metaDataPlug = depMetaDataNodeFn.findPlug("BindJoints");
        MObject metaDataParentNode = this->m_pParentComp->getMetaDataNode();
        MObject parentJoint;
        status = lrutils::getJointByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentJoint);
        MyCheckStatus(status, "lrutils::getJointByNum() failed");
        m_vBindJointObjs = lrutils::buildSkeletonFromGuide(this->m_pCompGuide->getLocations(), prefix, metaDataPlug, parentJoint);
        MGlobal::executeCommand("select -cl;");
        //create the FK controller objects for the FK joint chain
        prefix = this->m_rigName + "_" + this->m_pCompGuide->getName();
        //get the controllers layer from the meta root
        MObject ctlLayerObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, ctlLayerObj, "ctlLayer");
        MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        MFnDependencyNode ctlLayerFn(ctlLayerObj);
        MString ctlLayerName = ctlLayerFn.name();
        MGlobal::executeCommand("select -cl;");
        //get the hip controller object to parent FK controllers to
        if(metaDataParentFn.typeId() == MDHipNode::id) {
            MObject hipControllerObj;
            status = lrutils::getMetaNodeConnection(metaDataParentNode, hipControllerObj, "controller");
            MFnTransform hipControllerFn(hipControllerObj);
            lrutils::buildFKControls(this->m_vFKCtlObjs, this->m_vFKCtlGroupObjs, this->m_pCompGuide->getLocations(), this->m_vBindJointObjs, prefix, spineGuide->getFKIcon(), spineGuide->getColor(), this->m_metaDataNode,hipControllerObj, ctlLayerName);
        } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
            MObject parentControllerObj;
            MPlug kinematicTypePlug = metaDataParentFn.findPlug( "KinematicType" );
            MString kinematicType;
            kinematicTypePlug.getValue(kinematicType);
            if(kinematicType == "FK") {
                status = lrutils::getFKControlByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentControllerObj);
                MyCheckStatus(status, "lrutils::getFKControlByNum() failed");
            } else if (kinematicType == "splineIK" || kinematicType == "stretchySplineIK") {
                status = lrutils::getMetaNodeConnection(metaDataParentNode, parentControllerObj, "ShoulderControl");
                MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            }
            lrutils::buildFKControls(this->m_vFKCtlObjs, this->m_vFKCtlGroupObjs, this->m_pCompGuide->getLocations(), this->m_vBindJointObjs, prefix, spineGuide->getFKIcon(), spineGuide->getColor(), this->m_metaDataNode, parentControllerObj, ctlLayerName);
        }
    }

    dgMod.doIt();

    return this->m_metaDataNode;
}

void SpineComponent::buildIKSpline(MObject IKStartJoint, MObject IKEndJoint, MObject FKIndexStart, MObject FKIndexEnd) {
    MDGModifier dgMod;
    MStatus status;
    MFnTransform IKStartJointFn(IKStartJoint);
    MFnTransform IKEndJointFn(IKEndJoint);
    MFnTransform FKIndexStartFn(FKIndexStart);
    MFnTransform FKIndexEndFn(FKIndexEnd);
    //get the metaRoot node of this rig
    MObject metaRootObj;
    status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
    //get the group nodes of this rig
    MObject rigGroupObj;
    status = lrutils::getMetaNodeConnection(metaRootObj, rigGroupObj, "rigGroup");
    MFnTransform rigGroupFn(rigGroupObj);
    MObject ctlGroupObj;
    status = lrutils::getMetaNodeConnection(metaRootObj,ctlGroupObj, "ctlGroup");
    MFnTransform ctlGroupFn(ctlGroupObj);
    //get the extras layer from the meta root
    MObject extrasLayerObj;
    status = lrutils::getMetaNodeConnection(metaRootObj, extrasLayerObj, "extrasLayer");
    MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
    MFnDependencyNode extrasLayerFn(extrasLayerObj);
    MString extrasLayerName = extrasLayerFn.name();
    //get the controllers layer from the meta root
    MObject ctlLayerObj;
    status = lrutils::getMetaNodeConnection(metaRootObj, ctlLayerObj, "ctlLayer");
    MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
    MFnDependencyNode ctlLayerFn(ctlLayerObj);
    MString ctlLayerName = ctlLayerFn.name();
    MGlobal::executeCommand("select -cl;");
    //used for creating metaParent attributes for objects
    MFnMessageAttribute mAttr;
    MFnDependencyNode depMetaDataNodeFn(this->m_metaDataNode);
    //get the meta data node's parent
    MObject metaDataParentNode = this->m_pParentComp->getMetaDataNode();
    MFnDependencyNode metaDataParentFn( metaDataParentNode );
    //get the guide for this component (as a spine guide type)
    SpineComponentGuidePtr spineGuide = boost::dynamic_pointer_cast<SpineComponentGuide>(this->m_pCompGuide);

    //create the IK spline solver
    MString handleName = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_HDL";
    MStringArray result;
    MGlobal::executeCommand("ikHandle -n \""+handleName+"\" -sj \""+IKStartJointFn.fullPathName()+"\" -ee \""+IKEndJointFn.fullPathName()+"\" -sol \"ikSplineSolver\" -ns 2 -pcv false;",result);
    //get the MObjects for all the IK solver components (handle, end effector, curve)
    status = lrutils::getObjFromName(result[0], m_SplineIKHandleObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    status = lrutils::getObjFromName(result[1], m_SplineIKEndEffectorObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    status = lrutils::getObjFromName(result[2], m_SplineIKCurveObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    //rename the end effector and curve objects
    dgMod.renameNode(m_SplineIKEndEffectorObj, this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_EFF");
    dgMod.renameNode(m_SplineIKCurveObj, this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_CRV");
    dgMod.doIt();

    //set parents for handle and curve
    MFnTransform splineIKHandleFn(m_SplineIKHandleObj);
    MGlobal::executeCommand("parent " + splineIKHandleFn.fullPathName() + " " + rigGroupFn.fullPathName() + ";");
    MFnTransform splineIKCurveFn(m_SplineIKCurveObj);
    MGlobal::executeCommand("parent " + splineIKCurveFn.fullPathName() + " " + ctlGroupFn.fullPathName() + ";");

    //set the display layers for handle and curve
    MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ extrasLayerName+" "+ splineIKHandleFn.fullPathName()+";");
    MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ extrasLayerName+" "+ splineIKCurveFn.fullPathName()+";");
    MGlobal::executeCommand("select -cl;");

    //add the metaParent attributes to the handle, end effector, and curve
    MObject handleAttr = mAttr.create("metaParent", "metaParent");
    splineIKHandleFn.addAttribute(handleAttr);
    MObject endEffectorAttr = mAttr.create("metaParent", "metaParent");
    MFnTransform splineIKEndEffectorFn(m_SplineIKEndEffectorObj);
    splineIKEndEffectorFn.addAttribute(endEffectorAttr);
    MObject curveAttr = mAttr.create("metaParent", "metaParent");
    splineIKCurveFn.addAttribute(curveAttr);
    //set up the meta data node connections to the handle, end effector, and curve
    MPlug metaDataPlug;
    metaDataPlug = depMetaDataNodeFn.findPlug("SplineIKHandle");
    status = dgMod.connect( metaDataPlug, splineIKHandleFn.findPlug("metaParent") );
    MyCheckStatus(status, "connect failed");
    metaDataPlug = depMetaDataNodeFn.findPlug("SplineIKEndEffector");
    status = dgMod.connect( metaDataPlug, splineIKEndEffectorFn.findPlug("metaParent") );
    MyCheckStatus(status, "connect failed");
    metaDataPlug = depMetaDataNodeFn.findPlug("SplineIKCurve");
    status = dgMod.connect( metaDataPlug, splineIKCurveFn.findPlug("metaParent") );
    MyCheckStatus(status, "connect failed");
    dgMod.doIt();
    //create the clusters to control the IK spline
    MFnNurbsCurve curveFn(splineIKCurveFn.child(0));
    for (int i = 0; i < curveFn.numCVs(); i++) {
        MString clusterName = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_"+i+"_CLS";
        MGlobal::executeCommand("select -r "+curveFn.fullPathName()+".cv["+i+"];");
        MStringArray result;
        MGlobal::executeCommand("cluster -n \""+clusterName+"\";",result);
        MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ extrasLayerName+" "+ result[1]+";");
        //rename the handle
        MObject clusterHandleObj;
        lrutils::getObjFromName(result[1],clusterHandleObj);
        MObject clusterObj;
        lrutils::getObjFromName(result[0],clusterObj);
        dgMod.renameNode(clusterHandleObj, this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_cluster"+i+"_HDL");
        dgMod.doIt();
        MFnTransform clusterHandleFn(clusterHandleObj);
        clusterHandleFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("SplineIKClusters"), clusterHandleFn.findPlug("metaParent"));
        dgMod.doIt();
        
        m_vSplineIKClusterObjs.push_back(clusterHandleObj);
    }

    if(spineGuide->getKinematicType() == "stretchySplineIK") {
        //create the curve info node to get the arc length of the spline
        MGlobal::executeCommand("arclen -ch true "+curveFn.fullPathName()+";",result);
        MString splineCurveInfoNodeName = result[0];
        MObject splineCurveInfoNodeObj;
        status = lrutils::getObjFromName(splineCurveInfoNodeName,splineCurveInfoNodeObj);
        MyCheckStatus(status,"lrutils::getObjFromName() failed");
        MFnDependencyNode splineCurveInfoNodeFn(splineCurveInfoNodeObj);
        splineCurveInfoNodeFn.setName(this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_Curve_INF");
        double arcLength;
        MGlobal::executeCommand("getAttr "+splineCurveInfoNodeFn.name()+".arcLength;",arcLength);
        //arc length needs to be normalized to the scaling of the rig group to avoid double length scale
        status = MGlobal::executeCommand( "python(\"mathNode = Utils.createMultiplyDivide('" + rigGroupFn.name() + ".scaleY','connection',"+arcLength+",'value','*')\");" );
        status = MGlobal::executeCommand( MString("python(\"mathNode.name()\");"), result );
        MObject arcLengthMultDivNodeObj;
        status = lrutils::getObjFromName(result[0], arcLengthMultDivNodeObj);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnDependencyNode arcLengthMultDivNodeFn(arcLengthMultDivNodeObj);
        //connect the math node's metaParent to the MDSpine node
        MObject arcLengthNodeAttr = mAttr.create("metaParent", "metaParent");
        arcLengthMultDivNodeFn.addAttribute(arcLengthNodeAttr);
        status = dgMod.connect( depMetaDataNodeFn.findPlug("MathNodes"), arcLengthMultDivNodeFn.findPlug("metaParent") );
        dgMod.doIt();
        arcLengthMultDivNodeFn.setName(this->m_rigName + "_" + this->m_pCompGuide->getName() + "_arcLength_UND");
        //use the arc length to calculate new scale values for the joints to make them reach the spline IK end effector
        status = MGlobal::executeCommand( "python(\"mathNode = Utils.createMultiplyDivide('" + splineCurveInfoNodeFn.name() + ".arcLength','connection','"+arcLengthMultDivNodeFn.name()+".outputX','connection','/')\");" );
        status = MGlobal::executeCommand( MString("python(\"mathNode.name()\");"), result );
        MObject stretchIKMultDivNodeObj;
        status = lrutils::getObjFromName(result[0], stretchIKMultDivNodeObj);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnDependencyNode stretchIKMultDivNodeFn(stretchIKMultDivNodeObj);
        stretchIKMultDivNodeFn.setName(this->m_rigName + "_" + this->m_pCompGuide->getName() + "_stretchIK_UND");
        //connect the math node's metaParent to the MDSpine node
        MObject stretchIKAttr = mAttr.create("metaParent", "metaParent");
        stretchIKMultDivNodeFn.addAttribute(stretchIKAttr);
        status = dgMod.connect( depMetaDataNodeFn.findPlug("MathNodes"), stretchIKMultDivNodeFn.findPlug("metaParent") );
        dgMod.doIt();
        //hook up the multiply/divide node to the joint scaleX attributes
        for(int i = 0; i < this->m_vBindJointObjs.size(); i++) {
            MObject jointObj = this->m_vBindJointObjs.at(i);
            MFnTransform jointFn(jointObj);
            MGlobal::executeCommand("connectAttr "+stretchIKMultDivNodeFn.name()+".outputX "+jointFn.fullPathName()+".scaleX;");
        }
        if(metaDataParentFn.typeId() == MDHipNode::id) {
            MObject hipJointObj;
            status = lrutils::getMetaNodeConnection(metaDataParentNode, hipJointObj, "hipJoint");
            MFnTransform hipJointFn(hipJointObj);
            MGlobal::executeCommand("connectAttr "+stretchIKMultDivNodeFn.name()+".outputX "+hipJointFn.fullPathName()+".scaleY;");
        } else {
            MObject parentJointObj;
            status = lrutils::getJointByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentJointObj);
            MyCheckStatus(status, "lrutils::getJointByNum() failed");
            MFnTransform parentJointFn(parentJointObj);
            MGlobal::executeCommand("connectAttr "+stretchIKMultDivNodeFn.name()+".outputX "+parentJointFn.fullPathName()+".scaleY;");
        }
    }

    MObject parentCtlObj; 
    MFnTransform parentCtlFn;
    MObject hipControllerObj;
    MFnTransform hipControllerFn;
    MFnTransform clusterFn;
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        //the first two clusters should be parented to the hip controller
        status = lrutils::getMetaNodeConnection(metaDataParentNode, hipControllerObj, "controller");
        hipControllerFn.setObject(hipControllerObj);
        clusterFn.setObject(m_vSplineIKClusterObjs.at(0));
        MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + hipControllerFn.fullPathName() + ";");
        clusterFn.setObject(m_vSplineIKClusterObjs.at(1));
        MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + hipControllerFn.fullPathName() + ";");
    } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
        //the first two clusters will be parented to the correct controller in the spine, depending on the kinematic type
        MPlug kinematicTypePlug = metaDataParentFn.findPlug( "KinematicType" );
        MString kinematicType;
        kinematicTypePlug.getValue(kinematicType);
        if( kinematicType == "FK" ) {
            lrutils::getFKControlByNum(metaDataParentNode, spineGuide->getParentJointNum(), parentCtlObj);
            parentCtlFn.setObject(parentCtlObj);
        } else if ( kinematicType == "splineIK" || kinematicType == "stretchySplineIK" ) {
            status = lrutils::getMetaNodeConnection(metaDataParentNode, parentCtlObj, "ShoulderControl");
            parentCtlFn.setObject(parentCtlObj);
            clusterFn.setObject(m_vSplineIKClusterObjs.at(0));
            MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + parentCtlFn.fullPathName() + ";");
            clusterFn.setObject(m_vSplineIKClusterObjs.at(1));
            MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + parentCtlFn.fullPathName() + ";");
        }

    }

    //create the shoulder control for the end of the spine joint chain
    MString ctlColor = spineGuide->getShoulderColor();
    MString ctlShoulderIcon = spineGuide->getShoulderIcon();
    status = MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + ctlShoulderIcon + "')\");" );
    status = MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + ctlColor + "')\");" );
    status = MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), result );
    //get the MObject for the controller
    MObject ctlObj;
    status = lrutils::getObjFromName(result[0], ctlObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    //set controller location
    MVectorArray ctlLocation = spineGuide->getShoulderLocation();
    MFnTransform ctlFn( ctlObj );
    lrutils::setLocation(ctlObj, ctlLocation, MFnTransform::MFnTransform(), false, false, true);
    //set controller name
    MString ctlName = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_shoulder_CTL";
    dgMod.renameNode(ctlObj, ctlName);
    //add the metaParent attribute to the controller
    MObject shoulderControlAttr = mAttr.create("metaParent", "metaParent");
    ctlFn.addAttribute(shoulderControlAttr);
    //connect the controller's metaParent to the MDSpine node
    status = dgMod.connect( depMetaDataNodeFn.findPlug("ShoulderControl"), ctlFn.findPlug("metaParent") );
    dgMod.doIt();

    //create the shoulder control null
    MObject controllerGroupObj;
    lrutils::makeHomeNull(ctlObj, MFnTransform(), controllerGroupObj);
    lrutils::setLocation(controllerGroupObj, ctlLocation, MFnTransform::MFnTransform(), true, true, false);
    MFnTransform controllerGroupFn( controllerGroupObj );
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        MGlobal::executeCommand("parent " + controllerGroupFn.fullPathName() + " " + hipControllerFn.fullPathName() + ";");
    } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
        MGlobal::executeCommand("parent " + controllerGroupFn.fullPathName() + " " + parentCtlFn.fullPathName() + ";");
    }
    //add the metaParent attribute to the controller group
    controllerGroupFn.addAttribute(mAttr.create("metaParent", "metaParent"));
    //connect the controller group's metaParent to the MDGlobal node
    status = dgMod.connect( depMetaDataNodeFn.findPlug("ShoulderControlGroup"), controllerGroupFn.findPlug("metaParent") );
    MyCheckStatus(status, "connect failed"); 
    dgMod.doIt();
    ctlGroupFn.addChild(controllerGroupObj);
    
    //parent the middle cluster under the rig group, and the last two clusters under the shoulder controller
    clusterFn.setObject(m_vSplineIKClusterObjs.at(2));
    MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + ctlGroupFn.fullPathName() + ";");
    clusterFn.setObject(m_vSplineIKClusterObjs.at(3));
    MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + ctlFn.fullPathName() + ";");
    clusterFn.setObject(m_vSplineIKClusterObjs.at(4));
    MGlobal::executeCommand("parent " + clusterFn.fullPathName() + " " + ctlFn.fullPathName() + ";");
    //the middle cluster is equally weighted to both the hip and shoulder controllers
    clusterFn.setObject(m_vSplineIKClusterObjs.at(2));
    MGlobal::executeCommand("parentConstraint -mo " + ctlFn.fullPathName() + " " + clusterFn.fullPathName() + ";",result);
    //add the meta data connection to the middle cluster constraint
    MObject middleClusterParentConstraintObj;
    status = lrutils::getObjFromName(result[0], middleClusterParentConstraintObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    MFnTransform middleClusterParentConstraintFn(middleClusterParentConstraintObj);
    middleClusterParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
    status = dgMod.connect( depMetaDataNodeFn.findPlug("MiddleClusterConstraint"), middleClusterParentConstraintFn.findPlug("metaParent"));
    dgMod.doIt();
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        MGlobal::executeCommand("parentConstraint -mo " + hipControllerFn.fullPathName() + " " + clusterFn.fullPathName() + ";");
    } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
        MGlobal::executeCommand("parentConstraint -mo " + parentCtlFn.fullPathName() + " " + clusterFn.fullPathName() + ";");
    }

    //the shoulder and hip controllers should be parent constrained to the fk chain
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        MObject hipControllerGrpObj;
        status = lrutils::getMetaNodeConnection(metaDataParentNode, hipControllerGrpObj, "controllerGroup");
        parentCtlFn.setObject(hipControllerGrpObj);
        MFnTransform hipJointCopyFn(m_hipJointCopyObj);
        MGlobal::executeCommand("parentConstraint -mo " + hipJointCopyFn.fullPathName() + " " + parentCtlFn.fullPathName() + ";",result);
        MObject hipCtlParentConstraintObj;
        status = lrutils::getObjFromName(result[0], hipCtlParentConstraintObj);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnTransform hipCtlParentConstraintFn(hipCtlParentConstraintObj);
        hipCtlParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("HipControlConstraint"), hipCtlParentConstraintFn.findPlug("metaParent"));
        dgMod.doIt();
    }

    MFnTransform lastFKJointFn(this->m_vFKJointObjs.back());
    MGlobal::executeCommand("parentConstraint -mo " + lastFKJointFn.fullPathName() + " " + controllerGroupFn.fullPathName() + ";",result);
    MObject shoulderCtlParentConstraintObj;
    status = lrutils::getObjFromName(result[0], shoulderCtlParentConstraintObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    MFnTransform shoulderCtlParentConstraintFn(shoulderCtlParentConstraintObj);
    shoulderCtlParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
    status = dgMod.connect( depMetaDataNodeFn.findPlug("ShoulderControlConstraint"), shoulderCtlParentConstraintFn.findPlug("metaParent"));
    dgMod.doIt();

    //create an expression to drive the twist attribute of the IK handle by the rotateX attributes of the FK controllers
    MString jointRotations = "";
    for(int i = 0; i < m_vFKJointObjs.size();i++) {
        MFnTransform jointFn(m_vFKJointObjs.at(0));
        jointRotations += " + "+jointFn.fullPathName() + ".rotateX";
    }
    MFnTransform joint1Fn(m_vFKJointObjs[0]);
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        MGlobal::executeCommand("expression -n \""+this->m_rigName + "_" + this->m_pCompGuide->getName() + "_splineIKTwist_EXP\" -s \""+splineIKHandleFn.fullPathName()+".twist = ("+hipControllerFn.fullPathName()+".rotateY"+jointRotations+")\"");
        MGlobal::executeCommand("select -cl;");
    } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
        MGlobal::executeCommand("expression -n \""+this->m_rigName + "_" + this->m_pCompGuide->getName() + "_splineIKTwist_EXP\" -s \""+splineIKHandleFn.fullPathName()+".twist = ("+parentCtlFn.fullPathName()+".rotateY"+jointRotations+")\"");
        MGlobal::executeCommand("select -cl;");
    }
    MString prefix = this->m_rigName + "_" + this->m_pCompGuide->getName();
    if(metaDataParentFn.typeId() == MDHipNode::id) {
        lrutils::buildFKControls(this->m_vFKCtlObjs, this->m_vFKCtlGroupObjs, this->m_pCompGuide->getLocations(), this->m_vFKJointObjs, prefix, spineGuide->getFKIcon(), spineGuide->getColor(), this->m_metaDataNode,hipControllerObj, ctlLayerName);
    } else if (metaDataParentFn.typeId() == MDSpineNode::id) {
        lrutils::buildFKControls(this->m_vFKCtlObjs, this->m_vFKCtlGroupObjs, this->m_pCompGuide->getLocations(), this->m_vFKJointObjs, prefix, spineGuide->getFKIcon(), spineGuide->getColor(), this->m_metaDataNode,parentCtlObj, ctlLayerName);
    }
}

void SpineComponent::updateComponent(MDGModifier & dgMod,bool forceUpdate, bool globalPos) {    
    MFnMessageAttribute mAttr;
    MStatus status;
    if( !this->m_metaDataNode.isNull() ) {
        //get the rig name
        MFnDependencyNode metaDataNodeFn( m_metaDataNode );
        MString metaNodeName = metaDataNodeFn.name();
        MStringArray nameArray;
        metaNodeName.split('_', nameArray);
        MString oldRigName = nameArray[1];
        MString newRigName = this->m_rigName;
        //get the controller name
        MString oldComponentName = nameArray[2];
        MString newComponentName = this->m_pCompGuide->getName();
        //update names of component objects
        if( oldRigName != newRigName || oldComponentName != newComponentName ) {
            //set the metadata node name
            lrutils::stringReplaceAll(metaNodeName, oldRigName, newRigName);
            lrutils::stringReplaceAll(metaNodeName, oldComponentName, newComponentName);
            metaDataNodeFn.setName(metaNodeName);
            //set FK controller names
            MPlug fkControllersPlug = metaDataNodeFn.findPlug( "FKControllers", true, &status );
            lrutils::updateMetaDataObjectNames(fkControllersPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set FK controller group names
            MPlug fkControllerGroupsPlug = metaDataNodeFn.findPlug( "FKControllerGroups", true, &status );
            lrutils::updateMetaDataObjectNames(fkControllerGroupsPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set FK joint names
            MPlug fkJointsPlug = metaDataNodeFn.findPlug( "FKJoints", true, &status );
            lrutils::updateMetaDataObjectNames(fkJointsPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set FK joint constraint names
            MPlug fkJointConstraintsPlug = metaDataNodeFn.findPlug( "FKJointParentConstraints", true, &status );
            lrutils::updateMetaDataObjectNames(fkJointConstraintsPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set bind joint names
            MPlug bindJointsPlug = metaDataNodeFn.findPlug( "BindJoints", true, &status );
            lrutils::updateMetaDataObjectNames(bindJointsPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set hip joints copy name
            MPlug hipJointCopyPlug = metaDataNodeFn.findPlug( "HipJointCopy", true, &status );
            lrutils::updateMetaDataObjectNames(hipJointCopyPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set spline IK handle name
            MPlug splineIKHandlePlug = metaDataNodeFn.findPlug( "SplineIKHandle", true, &status );
            lrutils::updateMetaDataObjectNames(splineIKHandlePlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set spline IK end effector name
            MPlug splineIKEndEffectorPlug = metaDataNodeFn.findPlug( "SplineIKEndEffector", true, &status );
            lrutils::updateMetaDataObjectNames(splineIKEndEffectorPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set spline IK curve name
            MPlug splineIKCurvePlug = metaDataNodeFn.findPlug( "SplineIKCurve", true, &status );
            lrutils::updateMetaDataObjectNames(splineIKCurvePlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set shoulder control name
            MPlug shoulderControlPlug = metaDataNodeFn.findPlug( "ShoulderControl", true, &status );
            lrutils::updateMetaDataObjectNames(shoulderControlPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set shoulder control group name
            MPlug shoulderControlGroupPlug = metaDataNodeFn.findPlug( "ShoulderControlGroup", true, &status );
            lrutils::updateMetaDataObjectNames(shoulderControlGroupPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set shoulder control constraint name
            MPlug shoulderControlConstraintPlug = metaDataNodeFn.findPlug( "ShoulderControlConstraint", true, &status );
            lrutils::updateMetaDataObjectNames(shoulderControlConstraintPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set math node names
            MPlug mathNodesPlug = metaDataNodeFn.findPlug( "MathNodes", true, &status );
            lrutils::updateMetaDataObjectNames(mathNodesPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set spine IK cluster names
            MPlug splineIKClustersPlug = metaDataNodeFn.findPlug( "SplineIKClusters", true, &status );
            lrutils::updateMetaDataObjectNames(splineIKClustersPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set hip control constraint name
            MPlug hipControlConstraintPlug = metaDataNodeFn.findPlug( "HipControlConstraint", true, &status );
            lrutils::updateMetaDataObjectNames(hipControlConstraintPlug, oldRigName, newRigName, oldComponentName, newComponentName);
            //set middle cluster constraint name
            MPlug middleClusterConstraintPlug = metaDataNodeFn.findPlug( "MiddleClusterConstraint", true, &status );
            lrutils::updateMetaDataObjectNames(middleClusterConstraintPlug, oldRigName, newRigName, oldComponentName, newComponentName);
        }
        //update component settings, if the version increment is raised
        //or force update is true
        MPlug versionPlug = metaDataNodeFn.findPlug( "version" );
        float nodeVersion; 
        versionPlug.getValue(nodeVersion);
        if( (this->m_pCompGuide->getVersion() > nodeVersion) || forceUpdate ) {
            SpineComponentGuidePtr spineGuide = boost::dynamic_pointer_cast<SpineComponentGuide>(this->m_pCompGuide);
            //get the metaRoot node of this rig
            MObject metaRootObj;
            status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
            //get the controllers layer from the meta root
            MObject ctlLayerObj;
            status = lrutils::getMetaNodeConnection(metaRootObj, ctlLayerObj, "ctlLayer");
            MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            MFnDependencyNode ctlLayerFn(ctlLayerObj);
            MString ctlLayerName = ctlLayerFn.name();

            versionPlug.setValue( this->m_pCompGuide->getVersion() );

            MPlug kTypePlug = metaDataNodeFn.findPlug( "KinematicType" );
            MString kinematicType;
            kTypePlug.getValue(kinematicType);

            if(kinematicType == "FK") {
                SpineComponentGuidePtr spineGuide = boost::dynamic_pointer_cast<SpineComponentGuide>(this->m_pCompGuide);
                MString ctlIcon = spineGuide->getFKIcon();
                MString ctlColor = spineGuide->getColor();

                MPlug fkControllersPlug = metaDataNodeFn.findPlug( "FKControllers", true, &status );
                MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");

                MPlugArray connectedFKCtlPlugs;
                fkControllersPlug.connectedTo(connectedFKCtlPlugs,false,true,&status);
                MyCheckStatus(status,"MPlug.connectedTo() failed");

                //update the fk controller colors,shapes, and locations
                for (unsigned int i = 0; i < connectedFKCtlPlugs.length(); i++) {
                    MPlug fkCtlPlug = connectedFKCtlPlugs[i];
                    //need to switch the first and second in the plug list if more than one
                    //is attached, because the are switched for some reason when plugs are
                    //connected. This is an unavoidable bug in maya api.
                    if( connectedFKCtlPlugs.length() > 1) {
                        if(i == 0) { 
                            fkCtlPlug = connectedFKCtlPlugs[1];
                        } else if(i == 1) { 
                            fkCtlPlug = connectedFKCtlPlugs[0];
                        }
                    }
                    MObject fkCtlObj = fkCtlPlug.node(&status);
                    MFnTransform fkCtlFn(fkCtlObj);
                    MyCheckStatus(status, "MPlug.node() failed");
                    MVectorArray ctlLocation = this->m_pCompGuide->getLocation(i);

                    lrutils::updateControllerShapeColor(fkCtlObj, ctlIcon, ctlColor, ctlLocation);

                    if(globalPos) {
                        lrutils::updateAnimationKeys(fkCtlObj,ctlLocation);
                    } else {
                        lrutils::updateControlGroupLocation(fkCtlObj, ctlLocation);
                    }
                }
                //update the number of fk joints and controllers
                MPlug fkJointsPlug = metaDataNodeFn.findPlug( "BindJoints", true, &status );
                MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
                MPlugArray connectedFKJointPlugs;
                fkJointsPlug.connectedTo(connectedFKJointPlugs,false,true,&status);
                MyCheckStatus(status,"MPlug.connectedTo() failed");
                unsigned int numLocations = this->m_pCompGuide->getNumLocations();
                unsigned int numJoints = connectedFKJointPlugs.length();
                //remove excess joints and controllers
                if( numLocations < numJoints) {
                    unsigned int numJointsToDelete = numJoints - numLocations;
                    for(unsigned int i = connectedFKJointPlugs.length() - 1; i > connectedFKJointPlugs.length() - 1 - numJointsToDelete; i--) {
                        MGlobal::deleteNode(connectedFKJointPlugs[i].node());
                        MGlobal::deleteNode(connectedFKCtlPlugs[i].node());
                        MGlobal::displayInfo("blah");
                    }
                    MGlobal::deleteNode(connectedFKCtlPlugs[connectedFKJointPlugs.length() - 1 - numJointsToDelete].node());
                //add additional joints and controllers
                } else if ( numLocations > numJoints) {
                    unsigned int numJointsToAdd = numLocations - numJoints;
                    MString prefixJoint = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_FK_BIND";
                    MString prefixCtl = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_FK";
                    for(unsigned int i = numLocations - 1 - numJointsToAdd; i < numLocations; i++) {
                        MVectorArray location = this->m_pCompGuide->getLocation(i);
                        MObject joint = lrutils::createJointFromLocation(location, prefixJoint, i, connectedFKJointPlugs[i-1].node());

                        MFnTransform jointFn( joint );
                        //add metaParent attributes to joint
                        MObject jointAttr = mAttr.create("metaParent", "metaParent");
                        jointFn.addAttribute(jointAttr);
                        //connect the metaparent attribute to the MDSpine node
                        status = dgMod.connect( metaDataNodeFn.findPlug( "BindJoints", true, &status ), jointFn.findPlug("metaParent") );
                        MyCheckStatus(status, "connect failed");
                        dgMod.doIt();

                        if(i != numLocations - 1) {
                            MObject ctlObj;
                            MObject ctlGroupObj;
                            lrutils::createFKCtlFromLocation(location, joint, prefixCtl, i, ctlIcon, ctlColor, connectedFKCtlPlugs[i-1].node(), ctlObj, ctlGroupObj, ctlLayerName, this->m_metaDataNode);
                        }
                    }
                }
                //delete fk constraints
                MPlug fkConstraintsPlug = metaDataNodeFn.findPlug( "FKJointParentConstraints", true, &status );
                MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
                lrutils::deleteMetaDataPlugConnections(fkConstraintsPlug);
                //re-orient the joint chain
                MFnTransform topJointFn(connectedFKJointPlugs[0].node());
                MGlobal::executeCommand("joint -e -zso -oj \"xyz\" -sao \"yup\" -ch " + topJointFn.fullPathName() + ";");
                //re-establish the constraints
                for(unsigned int i = 0; i < connectedFKCtlPlugs.length(); i++) {
                    MPlug fkCtlPlug = connectedFKCtlPlugs[i];
                    MPlug fkJointPlug = connectedFKJointPlugs[i];
                    //need to switch the first and second in the plug list if more than one
                    //is attached, because they are switched for some reason when plugs are
                    //connected. This is an unavoidable bug in maya api.
                    if( connectedFKCtlPlugs.length() > 1) {
                        if(i == 0) { 
                            fkCtlPlug = connectedFKCtlPlugs[1];
                            fkJointPlug = connectedFKJointPlugs[1];
                        } else if(i == 1) { 
                            fkCtlPlug = connectedFKCtlPlugs[0];
                            fkJointPlug = connectedFKJointPlugs[0];
                        }
                    }
                    MStringArray result;
                    MFnTransform fkCtlFn(fkCtlPlug.node());
                    MFnTransform fkJointFn(fkJointPlug.node());
                    MGlobal::executeCommand("parentConstraint -mo " + fkCtlFn.fullPathName() + " " + fkJointFn.fullPathName() + ";",result);
                    MObject jointParentConstraintObj;
                    status = lrutils::getObjFromName(result[0], jointParentConstraintObj);
                    MyCheckStatus(status, "lrutils::getObjFromName() failed");
                    MFnTransform jointParentConstraintFn(jointParentConstraintObj);
                    jointParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
                    status = dgMod.connect( metaDataNodeFn.findPlug("FKJointParentConstraints"), jointParentConstraintFn.findPlug("metaParent"));
                    dgMod.doIt();
                }

            }

        }
    }

}

void SpineComponent::removeComponent(MDGModifier & dgMod) {
    MStatus status;
    MFnDependencyNode metaNodeFn( this->m_metaDataNode );
    
    //delete shoulder constraint
    MPlug shoulderConstraintPlug = metaNodeFn.findPlug("ShoulderControlConstraint", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(shoulderConstraintPlug);

    //delete hip control constraint
    MPlug hipConstraintPlug = metaNodeFn.findPlug("HipControlConstraint", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(hipConstraintPlug);

    //delete middle cluster constraint
    MPlug middleClusterConstraintPlug = metaNodeFn.findPlug("MiddleClusterConstraint", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(middleClusterConstraintPlug);

    //delete shoulder control
    MPlug shoulderControllerPlug = metaNodeFn.findPlug("ShoulderControl", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(shoulderControllerPlug);

    //delete shoulder control group
    MPlug shoulderGroupPlug = metaNodeFn.findPlug("ShoulderControlGroup", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(shoulderGroupPlug);

    //delete math nodes
    MPlug mathNodesPlug = metaNodeFn.findPlug("MathNodes", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(mathNodesPlug);

    //delete clusters
    MPlug clustersPlug = metaNodeFn.findPlug("SplineIKClusters", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(clustersPlug);

    //delete spline IK curve
    MPlug splineIKCurvePlug = metaNodeFn.findPlug("SplineIKCurve", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(splineIKCurvePlug);

    //delete spline IK end effector
    MPlug splineIKEndEffectorPlug = metaNodeFn.findPlug("SplineIKEndEffector", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(splineIKEndEffectorPlug);

    //delete spline IK handle
    MPlug splineIKHandlePlug = metaNodeFn.findPlug("SplineIKHandle", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(splineIKHandlePlug);

    //delete fk constraints
    MPlug fkConstraintsPlug = metaNodeFn.findPlug( "FKJointParentConstraints", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(fkConstraintsPlug);

    //delete fk joints
    MPlug fkJointsPlug = metaNodeFn.findPlug( "FKJoints", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(fkJointsPlug);

    //delete fk controllers
    MPlug fkControllersPlug = metaNodeFn.findPlug( "FKControllers", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(fkControllersPlug);

    //delete fk controller groups
    MPlug fkControllerGroupsPlug = metaNodeFn.findPlug( "FKControllerGroups", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(fkControllerGroupsPlug);

    //delete hip joint copy
    MPlug hipJointCopyPlug = metaNodeFn.findPlug("HipJointCopy", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(hipJointCopyPlug);

    //delete bind joints
    MPlug bindJointsPlug = metaNodeFn.findPlug("BindJoints", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");
    lrutils::deleteMetaDataPlugConnections(bindJointsPlug);
}