/***************************************************************
* Summary: Class for implementing a global component of a rig. *
*          It provides all the functionality to load, update,  *
*           or remove a global component.                      *
*  Author: Logan Kelly                                         *
*    Date: 10/29/12                                            *
***************************************************************/

#include "GlobalComponent.h"
#include "MyErrorChecking.h"
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "LoadRigUtils.h"
#include <maya/MFnMessageAttribute.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>

using namespace std;

GlobalComponent::GlobalComponent(GlobalComponentGuidePtr gCompGuide, MString rigName, boost::shared_ptr<Component> parentComp) : Component(boost::dynamic_pointer_cast<ComponentGuide>(gCompGuide), rigName, parentComp) {

}

GlobalComponent::~GlobalComponent() {

}

MObject GlobalComponent::loadComponent(MDGModifier & dgMod) {
    MStatus status = MS::kFailure;
    this->m_metaDataNode = dgMod.createNode( "MDGlobalNode", &status );
    MyCheckStatus(status, "createNode failed");

    MString metaNodeName = "MGN_";
    metaNodeName += this->m_rigName + "_";
    metaNodeName += this->m_pCompGuide->getName();
    dgMod.renameNode(this->m_metaDataNode, metaNodeName);

    MFnDependencyNode depMetaDataNodeFn(this->m_metaDataNode);
    status = dgMod.newPlugValueFloat( depMetaDataNodeFn.findPlug("version"), this->m_pCompGuide->getVersion() );
    MyCheckStatus(status, "newPlugValueFloat() failed");
    status = dgMod.newPlugValueString( depMetaDataNodeFn.findPlug("rigId"), this->m_pCompGuide->getRigId() );
    MyCheckStatus(status, "newPlugValueInt() failed");

    GlobalComponentGuidePtr globalGuide = boost::dynamic_pointer_cast<GlobalComponentGuide>(this->m_pCompGuide);
    MString ctlColor = globalGuide->getColor();
    MString ctlIcon = globalGuide->getIcon();

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
        //connect the controller's metaParent to the MDGlobal node
        status = dgMod.connect( depMetaDataNodeFn.findPlug("controller"), transformFn.findPlug("metaParent") );

        MObject ctlGroupObj;
        lrutils::makeHomeNull(ctlObj, MFnTransform(), ctlGroupObj);
        lrutils::setLocation(ctlGroupObj, ctlLocation, MFnTransform::MFnTransform(), true, true, false);
        MFnTransform ctlGroupFn( ctlGroupObj );
        //add the metaParent attribute to the controller group
        ctlGroupFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        //connect the controller group's metaParent to the MDGlobal node
        status = dgMod.connect( depMetaDataNodeFn.findPlug("controllerGroup"), ctlGroupFn.findPlug("metaParent") );
        MyCheckStatus(status, "connect failed"); 
        MObject metaRootObj;
        status = lrutils::getMetaRootByName(metaRootObj, this->m_rigName);
        MyCheckStatus(status, "lrutils::getMetaRootByName() failed");
        MObject rigCtlGroupObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, rigCtlGroupObj, "ctlGroup");
        MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        MFnTransform rigCtlGroupFn( rigCtlGroupObj );
        rigCtlGroupFn.addChild( ctlGroupObj );

        //add controller to controller display layer
        MObject controlLayerObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, controlLayerObj, "ctlLayer");
        MyCheckStatus(status, "lrutils::getMetaNodeConnection() failed");
        MFnDependencyNode controlLayerFn(controlLayerObj);
        MString controlLayerName = controlLayerFn.name();
        MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+controlLayerName+" "+rigCtlGroupFn.name()+";");
        //create parent constraints from the global controller to the rig group
        MObject rigRigGroupObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, rigRigGroupObj, "rigGroup");
        MFnTransform rigRigGroupFn( rigRigGroupObj );
        MGlobal::executeCommand("parentConstraint -mo "+transformFn.name()+" "+rigRigGroupFn.name()+";", res);
        //connect the parent constraint object to the component's metadata node
        MStringArray sResults;
        res.getResult(sResults);
        status = lrutils::getObjFromName(sResults[0], this->m_rigParentConstraint);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnTransform rigParentConstraintFn( this->m_rigParentConstraint);
        rigParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("rigParentConstraint"), rigParentConstraintFn.findPlug("metaParent"));
        //create the scale constraint from the global controller to the rig group
        MGlobal::executeCommand("scaleConstraint -mo "+transformFn.name()+" "+rigRigGroupFn.name()+";", res);
        //connect the scale constraint object to the component's metadata node
        res.getResult(sResults);
        status = lrutils::getObjFromName(sResults[0], this->m_rigScaleConstraint);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnTransform rigScaleConstraintFn( this->m_rigScaleConstraint );
        rigScaleConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("rigScaleConstraint"), rigScaleConstraintFn.findPlug("metaParent"));
        //create scale constraint from the global controller to the noTransform group
        MObject rigNoTransformGroupObj;
        status = lrutils::getMetaNodeConnection(metaRootObj, rigNoTransformGroupObj, "noTransformGroup");
        MFnTransform rigNoTransformGroupFn( rigNoTransformGroupObj );
        MGlobal::executeCommand("scaleConstraint -mo "+transformFn.name()+" "+rigNoTransformGroupFn.name()+";", res);
        //connect the scale constraint object to the component's metadata node
        res.getResult(sResults);
        status = lrutils::getObjFromName(sResults[0], this->m_noTransformScaleConstraint);
        MyCheckStatus(status, "lrutils::getObjFromName() failed");
        MFnTransform noTransformScaleConstraintFn( this->m_noTransformScaleConstraint);
        noTransformScaleConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
        status = dgMod.connect( depMetaDataNodeFn.findPlug("noTransformScaleConstraint"), noTransformScaleConstraintFn.findPlug("metaParent"));
    }

    return this->m_metaDataNode;
}

void GlobalComponent::updateComponent(MDGModifier & dgMod,bool forceUpdate, bool globalPos) {
    MStatus status;
    if( !this->m_metaDataNode.isNull() ) {
        //get the rig name
        MFnDependencyNode metaDataNodeFn( m_metaDataNode );
        MString metaNodeName = metaDataNodeFn.name();
        MStringArray nameArray;
        metaNodeName.split('_', nameArray);
        MString rigName = nameArray[1];
        //get the controller name
        MString controllerName = nameArray[2];
        MString compXmlName = this->m_pCompGuide->getName();
        //update names of component objects
        if( rigName != this->m_rigName || controllerName != compXmlName ) {
            //set the metadata node name
            lrutils::stringReplaceAll(metaNodeName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(metaNodeName, controllerName, this->m_pCompGuide->getName());
            metaDataNodeFn.setName(metaNodeName);
            //set controller object name
            MObject globalCtlObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, globalCtlObj, "controller");
            MFnDependencyNode globalCtlFn( globalCtlObj );
            MString globalCtlName = globalCtlFn.name();
            lrutils::stringReplaceAll(globalCtlName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(globalCtlName, controllerName, this->m_pCompGuide->getName());
            globalCtlFn.setName(globalCtlName);
            //set controller group object name
            MObject globalCtlGroupObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, globalCtlGroupObj, "controllerGroup");
            MFnDependencyNode globalCtlGroupFn( globalCtlGroupObj );
            MString globalCtlGroupName = globalCtlGroupFn.name();
            lrutils::stringReplaceAll(globalCtlGroupName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(globalCtlGroupName, controllerName, this->m_pCompGuide->getName());
            globalCtlGroupFn.setName(globalCtlGroupName);
            //set rigParentConstraint object name
            MObject rigParentConstraintObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, rigParentConstraintObj, "rigParentConstraint");
            MFnDependencyNode rigParentConstraintFn( rigParentConstraintObj );
            MString rigParentConstraintName = rigParentConstraintFn.name();
            lrutils::stringReplaceAll(rigParentConstraintName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(rigParentConstraintName, controllerName, this->m_pCompGuide->getName());
            rigParentConstraintFn.setName(rigParentConstraintName);
            //set rigScaleConstraint object name
            MObject rigScaleConstraintObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, rigScaleConstraintObj, "rigScaleConstraint");
            MFnDependencyNode rigScaleConstraintFn( rigScaleConstraintObj );
            MString rigScaleConstraintName = rigScaleConstraintFn.name();
            lrutils::stringReplaceAll(rigScaleConstraintName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(rigScaleConstraintName, controllerName, this->m_pCompGuide->getName());
            rigScaleConstraintFn.setName(rigScaleConstraintName);
            //set noTransformScaleConstraint object name
            MObject noTransformScaleConstraintObj;
            lrutils::getMetaNodeConnection(this->m_metaDataNode, noTransformScaleConstraintObj, "noTransformScaleConstraint");
            MFnDependencyNode noTransformScaleConstraintFn( noTransformScaleConstraintObj );
            MString noTransformScaleConstraintName = noTransformScaleConstraintFn.name();
            lrutils::stringReplaceAll(noTransformScaleConstraintName, rigName, this->m_rigName);
            lrutils::stringReplaceAll(noTransformScaleConstraintName, controllerName, this->m_pCompGuide->getName());
            noTransformScaleConstraintFn.setName(noTransformScaleConstraintName); 
        }
        //update component settings, if the version increment is raised
        //or force update is true
        MPlug versionPlug = metaDataNodeFn.findPlug( "version" );
        float nodeVersion; 
        versionPlug.getValue(nodeVersion);
        if( (this->m_pCompGuide->getVersion() > nodeVersion) || forceUpdate ) {
            versionPlug.setValue( this->m_pCompGuide->getVersion() );
            //make a new controller object based upon the xml settings    
            GlobalComponentGuidePtr globalGuide = boost::dynamic_pointer_cast<GlobalComponentGuide>(this->m_pCompGuide);
            MString ctlColor = globalGuide->getColor();
            MString ctlIcon = globalGuide->getIcon();

            MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + ctlIcon + "')\");" );
            MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + ctlColor + "')\");" );
            MCommandResult res;
            MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), res );        
            MString sResult;
            res.getResult(sResult);
            MObject ctlObj;
            MStatus status = lrutils::getObjFromName(sResult, ctlObj);
            MyCheckStatus(status, "lrutils::getObjFromName() failed");
            //apply the scale of the controller location to the new shape
            MVectorArray ctlLocation = this->m_pCompGuide->getLocation(0);
            MFnTransform ctlFn( ctlObj );
            lrutils::setLocation(ctlObj, ctlLocation, MFnTransform::MFnTransform(), false, false, true);


            //get the global transforms of the controller for all keyframes and save them for later use
            MObject oldCtlObj;
            status = lrutils::getMetaNodeConnection( this->m_metaDataNode, oldCtlObj, "controller" );
            MyCheckStatus(status, "getMetaNodeConnection() failed");
            MFnTransform oldCtlFn( oldCtlObj );
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
            MObject oldCtlShapeObj; lrutils::getObjFromName(oldCtlShapePath, oldCtlShapeObj);
            //delete the old shape node
            MGlobal::deleteNode( oldCtlShapeObj );
            //get the new shape node
            MGlobal::executeCommand( "listRelatives -s -fullPath "+ctlFn.name()+";", sResults );
            MString ctlShapePath = sResults[0];
            MObject ctlShapeObj; lrutils::getObjFromName(ctlShapePath, ctlShapeObj);
            //instance the new shape node under the old controller node
            MString command = "parent -s -add " + ctlShapePath + " " + oldCtlFn.name() + ";";
            MGlobal::executeCommand( command );
            MFnDependencyNode ctlShapeFn( ctlShapeObj );
            ctlShapeFn.setName( oldCtlShapeName );
            //set the old controller group translation to the new location
            MObject oldCtlGroupObj;
            lrutils::getMetaNodeConnection( this->m_metaDataNode, oldCtlGroupObj, "controllerGroup" );
            MFnTransform oldCtlGroupFn( oldCtlGroupObj );
            //save the original old controller position
            MTransformationMatrix oldXForm = oldCtlGroupFn.transformation();
            lrutils::setLocation(oldCtlGroupObj, ctlLocation, oldCtlGroupFn, true, true, false);
            //compute the inverse transformation matrix of the old control group
            MTransformationMatrix oldCtlGrpXform = oldCtlGroupFn.transformation();
            MTransformationMatrix inverseXform = MTransformationMatrix(oldCtlGrpXform.asMatrixInverse());
            //set the target offset for the rigParentConstraint node
            lrutils::getMetaNodeConnection(this->m_metaDataNode, this->m_rigParentConstraint, "rigParentConstraint");
            lrutils::setParentConstraintOffset( this->m_rigParentConstraint, inverseXform );
            //delete the new controller transform
            MGlobal::deleteNode( ctlObj );
            
            //find the global transformation matrix of the controller group
            MDagPath groupPath;
            status = oldCtlGroupFn.getPath(groupPath);
            MyCheckStatus(status, "MFnDagNode.getPath() failed");
            MMatrix oldCtlGroupWorldMatrix = groupPath.inclusiveMatrix(&status);
            MyCheckStatus(status, "MDagPath.inclusiveMatrix() failed");
            if(globalPos) {
                //update the animation curves attached to the old controller
                lrutils::updateAnimCurves(oldCtlObj, oldCtlWorldMatrices, oldCtlGroupWorldMatrix);
            }
        }
    }
}

void GlobalComponent::removeComponent(MDGModifier & dgMod) {
    //delete constraints
    lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_noTransformScaleConstraint, "noTransformScaleConstraint" );
    MGlobal::deleteNode(this->m_noTransformScaleConstraint);
    lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_rigParentConstraint, "rigParentConstraint" );
    MGlobal::deleteNode(this->m_rigParentConstraint);
    lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_rigScaleConstraint, "rigScaleConstraint" );
    MGlobal::deleteNode(this->m_rigScaleConstraint);

    //delete controller and controller group
    lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_ctlObj, "controller" );
    MGlobal::deleteNode(this->m_ctlObj);
    lrutils::getMetaNodeConnection( this->m_metaDataNode, this->m_ctlGroupObj, "controllerGroup" );
    MGlobal::deleteNode(this->m_ctlGroupObj);

}