/***************************************************************
* Summary: Class for implementing a hip component of a rig.    *
*          It provides all the functionality to load, update,  *
*           or remove a hip component.                         *
*  Author: Logan Kelly                                         *
*    Date: 10/29/12                                            *
***************************************************************/

#include "HipComponent.h"
#include "MyErrorChecking.h"
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "LoadRigUtils.h"
#include <maya/MFnMessageAttribute.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <map>

using namespace std;

HipComponent::HipComponent(HipComponentGuidePtr hCompGuide, MString rigName, boost::shared_ptr<Component> parentComp) : Component(boost::dynamic_pointer_cast<ComponentGuide>(hCompGuide), rigName, parentComp) {

}

HipComponent::~HipComponent() {

}

MObject HipComponent::loadComponent(MDGModifier & dgMod) {
    MStatus status = MS::kFailure;
    this->m_metaDataNode = dgMod.createNode( "MDHipNode", &status );
    MyCheckStatus(status, "createNode failed");
    dgMod.doIt();

    MString metaNodeName = "MHN_";
    metaNodeName += this->m_rigName + "_";
    metaNodeName += this->m_pCompGuide->getName();
    dgMod.renameNode(this->m_metaDataNode, metaNodeName);

    MFnDependencyNode depMetaDataNodeFn(this->m_metaDataNode);
    status = dgMod.newPlugValueFloat( depMetaDataNodeFn.findPlug("version"), this->m_pCompGuide->getVersion() );
    MyCheckStatus(status, "newPlugValueFloat() failed");
    status = dgMod.newPlugValueString( depMetaDataNodeFn.findPlug("rigId"), this->m_pCompGuide->getRigId() );
    MyCheckStatus(status, "newPlugValueString() failed");

    HipComponentGuidePtr hipGuide = boost::dynamic_pointer_cast<HipComponentGuide>(this->m_pCompGuide);
    MString ctlColor = hipGuide->getColor();
    MString ctlIcon = hipGuide->getIcon();

    status = MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + ctlIcon + "')\");" );
    status = MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + ctlColor + "')\");" );
    MCommandResult res;
    status = MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), res );
    int resType = res.resultType();
    if( resType == MCommandResult::kString ) {
        MString sResult;
        res.getResult(sResult);
        MObject ctlObj;
        status = lrutils::getObjFromName(sResult, ctlObj);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");

        MVectorArray ctlLocation = this->m_pCompGuide->getLocation(0);
        MFnTransform transformFn( ctlObj );
        lrutils::setLocation(ctlObj, ctlLocation, MFnTransform::MFnTransform(), false, false, true);

        MString ctlName = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_CTL";
        dgMod.renameNode(ctlObj, ctlName);
        dgMod.doIt();

        //add the metaParent attribute to the controller
        MFnMessageAttribute mAttr;
        MObject transformAttr = mAttr.create("metaParent", "metaParent");
        transformFn.addAttribute(transformAttr);
        //connect the controller's metaParent to the MDHip node
        status = dgMod.connect( depMetaDataNodeFn.findPlug("controller"), transformFn.findPlug("metaParent") );

        MObject ctlGroupObj;
        lrutils::makeHomeNull(ctlObj, MFnTransform(), ctlGroupObj);
        lrutils::setLocation(ctlGroupObj, ctlLocation, MFnTransform::MFnTransform(), true, true, false);
        MFnTransform ctlGroupFn( ctlGroupObj );
        //add the metaParent attribute to the controller group
        ctlGroupFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        //connect the controller group's metaParent to the MDHip node
        status = dgMod.connect( depMetaDataNodeFn.findPlug("controllerGroup"), ctlGroupFn.findPlug("metaParent") );
        MyCheckStatus(status, "connect failed");
        //parent the control group object under the metadata parent's controller
        MObject metaParentObj = this->m_pParentComp->getMetaDataNode();
        if( !metaParentObj.isNull() ) {
            MObject metaParentControllerObj;
            status = lrutils::getMetaNodeConnection(metaParentObj, metaParentControllerObj, "controller");
            MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            MFnTransform metaParentControllerFn( metaParentControllerObj );
            MString metaParentControllerPath = metaParentControllerFn.fullPathName();
            MFnTransform ctlGroupFn( ctlGroupObj);
            MString ctlGroupPath = ctlGroupFn.fullPathName();
            MGlobal::executeCommand("parent " + ctlGroupPath + " " + metaParentControllerPath + ";");
        }

        //get the metaRoot node of this rig
        MObject metaRootObj;
        status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
        //add controller to controller display layer
        MObject controlLayerObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, controlLayerObj, "ctlLayer");
        MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        MFnDependencyNode controlLayerFn(controlLayerObj);
        MString controlLayerName = controlLayerFn.name();
        MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+controlLayerName+" "+ctlGroupFn.name()+";");

        //create the hip joint
        MString prefix = this->m_rigName + "_" + this->m_pCompGuide->getName() + "_BIND";
        this->m_hipJointObj = lrutils::buildSkeletonFromGuide(this->m_pCompGuide->getLocations(), prefix).at(0);
        //add metaparent attribute to the joint
        MFnTransform hipJointFn( this->m_hipJointObj );
        MObject jointAttr = mAttr.create("metaParent", "metaParent");
        hipJointFn.addAttribute(jointAttr);
        //connect the metaparent attribute to the MDHip node
        status = dgMod.connect( depMetaDataNodeFn.findPlug("hipJoint"), hipJointFn.findPlug("metaParent") );
        MyCheckStatus(status, "connect failed");

        //add the hip joint to the skeleton display layer
        MObject skelLayerObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, skelLayerObj, "skelLayer");
        MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        MFnDependencyNode skelLayerFn(skelLayerObj);
        MString skelLayerName = skelLayerFn.name();
        MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+skelLayerName+" "+hipJointFn.name()+";");

        //check parent component for joints
        MObject metaParentJointObj;
        if( !metaParentObj.isNull() ) {
            status = lrutils::getMetaNodeConnection(metaParentObj, metaParentJointObj, "hipJoint");
            //MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        }
        if( !metaParentJointObj.isNull() ) {
            MFnTransform metaParentJointFn( metaParentJointObj );
            MString metaParentJointPath = metaParentJointFn.fullPathName();
            MFnTransform hipJointFn( this->m_hipJointObj );
            MString hipJointPath = hipJointFn.fullPathName();
            MGlobal::executeCommand("parent " + hipJointPath + " " + metaParentJointPath + ";");
        } else {
            //if no joints, move the hip joint to beneath the rig group
            MObject rigGroupObj;
            status = lrutils::getMetaNodeConnection(metaRootObj, rigGroupObj, "rigGroup");
            MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            MFnTransform rigGroupFn( rigGroupObj );
            MString rigGroupPath = rigGroupFn.fullPathName();
            MFnTransform hipJointFn( this->m_hipJointObj );
            MString hipJointPath = hipJointFn.fullPathName();
            MGlobal::executeCommand("parent " + hipJointPath + " " + rigGroupPath + ";");
        }

        //create parent constraint from the hip controller to the hip joint
        MGlobal::executeCommand("parentConstraint -mo "+transformFn.name()+" "+hipJointFn.name()+";", res);
        //connect the parent constraint object to the component's metadata node
        MStringArray sResults;
        res.getResult(sResults);
        status = lrutils::getObjFromName(sResults[0], this->m_hipJointParentConstraint);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnTransform hipJointParentConstraintFn( this->m_hipJointParentConstraint);
        hipJointParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("hipJointParentConstraint"), hipJointParentConstraintFn.findPlug("metaParent"));


    }

    return this->m_metaDataNode;
}

void HipComponent::updateComponent(MDGModifier & dgMod,bool forceUpdate, bool globalPos) {
    MStatus status;
    if( !this->m_metaDataNode.isNull() ) {
        //get the rig name
        MFnDependencyNode metaDataNodeFn( m_metaDataNode );
        MString metaNodeName = metaDataNodeFn.name();
        MStringArray nameArray;
        metaNodeName.split('_', nameArray);
        MString rigName = nameArray[1];
        //get the controller name
        MString componentName = nameArray[2];
        MString compXmlName = this->m_pCompGuide->getName();
        //update names of component objects
        if( rigName != this->m_rigName || componentName != compXmlName ) {
            //set the metadata node name
            lrutils::stringReplaceAll(metaNodeName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(metaNodeName, componentName, this->m_pCompGuide->getName());
            metaDataNodeFn.setName(metaNodeName);
            //set controller object name
            MObject hipCtlObj;
            MStatus status = lrutils::getMetaNodeConnection(this->m_metaDataNode, hipCtlObj, "controller");
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnDependencyNode hipCtlFn( hipCtlObj );
            MString hipCtlName = hipCtlFn.name();
            //set rig namespace
            lrutils::stringReplaceAll(hipCtlName, rigName, this->m_rigName);
            //set component namespace
            lrutils::stringReplaceAll(hipCtlName, componentName, this->m_pCompGuide->getName());
            hipCtlFn.setName(hipCtlName);
            //set controller group object name
            MObject hipCtlGroupObj;
            status = lrutils::getMetaNodeConnection(this->m_metaDataNode, hipCtlGroupObj, "controllerGroup");
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnDependencyNode hipCtlGroupFn( hipCtlGroupObj );
            MString hipCtlGroupName = hipCtlGroupFn.name();
            lrutils::stringReplaceAll(hipCtlGroupName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(hipCtlGroupName, componentName, this->m_pCompGuide->getName());
            hipCtlGroupFn.setName(hipCtlGroupName);
            //set hipJoint object name
            MObject hipJointObj;
            status = lrutils::getMetaNodeConnection(this->m_metaDataNode, hipJointObj, "hipJoint");
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnDependencyNode hipJointFn( hipJointObj );
            MString hipJointName = hipJointFn.name();
            lrutils::stringReplaceAll(hipJointName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(hipJointName, componentName, this->m_pCompGuide->getName());
            hipJointFn.setName(hipJointName);
            //set hipJointParentConstraint object name
            MObject hipJointParentConstraintObj;
            status = lrutils::getMetaNodeConnection(this->m_metaDataNode, hipJointParentConstraintObj, "hipJointParentConstraint");
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnDependencyNode hipJointParentConstraintFn( hipJointParentConstraintObj );
            MString hipJointParentConstraintName = hipJointParentConstraintFn.name();
            lrutils::stringReplaceAll(hipJointParentConstraintName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(hipJointParentConstraintName, componentName, this->m_pCompGuide->getName());
            hipJointParentConstraintFn.setName(hipJointParentConstraintName);
        }
        //update component settings, if the version increment is raised
        //or force update is true
        MPlug versionPlug = metaDataNodeFn.findPlug( "version" );
        float nodeVersion; 
        versionPlug.getValue(nodeVersion);
        if( (this->m_pCompGuide->getVersion() > nodeVersion) || forceUpdate ) {
            versionPlug.setValue( this->m_pCompGuide->getVersion() );
            //make a new controller object based upon the xml settings    
            HipComponentGuidePtr hipGuide = boost::dynamic_pointer_cast<HipComponentGuide>(this->m_pCompGuide);
            MString ctlColor = hipGuide->getColor();
            MString ctlIcon = hipGuide->getIcon();

            MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + ctlIcon + "')\");" );
            MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + ctlColor + "')\");" );
            MCommandResult res;
            MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), res );        
            MString sResult;
            res.getResult(sResult);
            MObject ctlObj;
            status = lrutils::getObjFromName(sResult, ctlObj);
            MyCheckStatus(status, "lrutils::getObjFromName() failed");
            //apply the scale of the controller location to the new shape
            MVectorArray ctlLocation = this->m_pCompGuide->getLocation(0);
            MFnTransform ctlFn( ctlObj );
            lrutils::setLocation(ctlObj, ctlLocation, MFnTransform::MFnTransform(), false, false, true);


            //get the global transforms of the controller for all keyframes and save them for later use
            MObject oldCtlObj;
            status = lrutils::getMetaNodeConnection( this->m_metaDataNode, oldCtlObj, "controller" );
            MFnTransform oldCtlFn(oldCtlObj);
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            std::map<double, MMatrix> oldCtlWorldMatrices;
            if(globalPos) {
                status = lrutils::getAllWorldTransforms(oldCtlObj, oldCtlWorldMatrices);
                MyCheckStatus(status, "lrutils::getAllWorldTransforms() failed");
            }

            //get the shape node of the original controller object
            MStringArray sResults;
            MGlobal::executeCommand( "listRelatives -s -fullPath "+oldCtlFn.name()+";", sResults );
            MString oldCtlShapePath = sResults[0];
            MGlobal::executeCommand( "listRelatives -s -path "+oldCtlFn.name()+";", sResults );
            MString oldCtlShapeName = sResults[0];
            MObject oldCtlShapeObj; 
            status = lrutils::getObjFromName(oldCtlShapePath, oldCtlShapeObj);
            MyCheckStatus(status, "lrutils::getObjFromName() failed");
            //delete the old shape node
            MGlobal::deleteNode( oldCtlShapeObj );
            //get the new shape node
            MGlobal::executeCommand( "listRelatives -s -fullPath "+ctlFn.name()+";", sResults );
            MString ctlShapePath = sResults[0];
            MObject ctlShapeObj; 
            status = lrutils::getObjFromName(ctlShapePath, ctlShapeObj);
            MyCheckStatus(status, "lrutils::getObjFromName() failed");
            //instance the new shape node under the old controller node
            MString command = "parent -s -add " + ctlShapePath + " " + oldCtlFn.name() + ";";
            MGlobal::executeCommand( command );
            MFnDependencyNode ctlShapeFn( ctlShapeObj );
            ctlShapeFn.setName( oldCtlShapeName );
            //set the old controller group translation to the new location
            MObject oldCtlGroupObj;
            status = lrutils::getMetaNodeConnection( this->m_metaDataNode, oldCtlGroupObj, "controllerGroup" );
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnTransform oldCtlGroupFn( oldCtlGroupObj );
            //save the original old controller position
            MTransformationMatrix oldXForm = oldCtlGroupFn.transformation();
            //move the controller group to world for absolute positioning
            MString oldCtlGroupPath = oldCtlGroupFn.fullPathName();
            MGlobal::executeCommand("parent -world " + oldCtlGroupPath + ";");            
            lrutils::setLocation(oldCtlGroupObj, ctlLocation, oldCtlGroupFn, true, true, false);
            //find the global transformation matrix of the controller group
            MDagPath groupPath;
            status = oldCtlGroupFn.getPath(groupPath);
            MyCheckStatus(status, "MFnDagNode.getPath() failed");
            MMatrix oldCtlGroupWorldMatrix = groupPath.inclusiveMatrix(&status);
            MyCheckStatus(status, "MDagPath.inclusiveMatrix() failed");
        
            MTransformationMatrix oldCtlGrpXform = oldCtlGroupFn.transformation();
            //delete the new controller transform
            MGlobal::deleteNode( ctlObj );
            if(globalPos) {
                //update the animation curves attached to the old controller
                lrutils::updateAnimCurves(oldCtlObj, oldCtlWorldMatrices, oldCtlGroupWorldMatrix);
            }

            //update the metadata node metaParent
            this->updateMetaParentNode(dgMod);

            //update controller parent
            MStatus status;
            MObject metaParentObj = this->m_pParentComp->getMetaDataNode();
            MFnDependencyNode metaParentFn( metaParentObj );
            MString metaParentName = metaParentFn.name();
            if( !metaParentObj.isNull() ) {
                MObject metaParentControllerObj;
                status = lrutils::getMetaNodeConnection(metaParentObj, metaParentControllerObj, "controller");
                MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
                MFnTransform metaParentControllerFn( metaParentControllerObj );
                MString metaParentControllerPath = metaParentControllerFn.fullPathName();
                MFnTransform oldCtlGroupFn( oldCtlGroupObj );
                MString oldCtlGroupPath = oldCtlGroupFn.fullPathName();
                MGlobal::executeCommand("parent " + oldCtlGroupPath + " " + metaParentControllerPath + ";");
            }
            
            //update joint parenting, check parent component for joints
            MObject metaParentJointObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, this->m_hipJointObj, "hipJoint");
            if( !metaParentObj.isNull() ) {
                status = lrutils::getMetaNodeConnection(metaParentObj, metaParentJointObj, "hipJoint");
                //MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
            }
            if( !metaParentJointObj.isNull() ) {
                MFnTransform metaParentJointFn( metaParentJointObj );
                MString metaParentJointPath = metaParentJointFn.fullPathName();
                MFnTransform hipJointFn( this->m_hipJointObj );
                MString hipJointPath = hipJointFn.fullPathName();
                MGlobal::executeCommand("parent " + hipJointPath + " " + metaParentJointPath + ";");
            } else {
                //if no joints, move the hip joint to beneath the rig group
                //get the metaRoot node of this rig
                MObject metaRootObj;
                status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
                MObject rigGroupObj;
                status = lrutils::getMetaNodeConnection(metaRootObj, rigGroupObj, "rigGroup");
                MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
                MFnTransform rigGroupFn( rigGroupObj );
                MString rigGroupPath = rigGroupFn.fullPathName();
                MFnTransform hipJointFn( this->m_hipJointObj );
                MString hipJointPath = hipJointFn.fullPathName();

                MGlobal::executeCommand("parent " + hipJointPath + " " + rigGroupPath + ";");
            }            
        }
    }
}

void HipComponent::removeComponent(MDGModifier & dgMod) {
    //delete constraints
    MStatus status = lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_hipJointParentConstraint, "hipJointParentConstraint" );
    MyCheckStatus(status, "getMetaNodeConnection() failed");
    MGlobal::deleteNode(this->m_hipJointParentConstraint);

    //delete joint
    status = lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_hipJointObj, "hipJoint" );
    MyCheckStatus(status, "getMetaNodeConnection() failed");
    MGlobal::deleteNode(this->m_hipJointObj);

    //delete controller and controller group
    status = lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_ctlObj, "controller" );
    MyCheckStatus(status, "getMetaNodeConnection() failed");
    MGlobal::deleteNode(this->m_ctlObj);
    status = lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_ctlGroupObj, "controllerGroup" );
    MyCheckStatus(status, "getMetaNodeConnection() failed");
    MGlobal::deleteNode(this->m_ctlGroupObj);

}