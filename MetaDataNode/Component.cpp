/***************************************************************
* Summary: Abstract base class for all component classes of    *
*          various types that derive from it.  It provides the *
*          basic functionality necessary to load, update, or   *
*          remove a rig component.                             *
*  Author: Logan Kelly                                         *
*    Date: 10/19/12                                            *
***************************************************************/

#include "Component.h"
#include "LoadRigUtils.h"
#include <maya/MGlobal.h>
#include <maya/MPlug.h>

Component::Component(ComponentGuidePtr compGuide,MString rigName,boost::shared_ptr<Component> parentComp) {
    if (compGuide != NULL) {
        this->m_pCompGuide = compGuide;
    }
    if (rigName.length() > 0) {
        this->m_rigName = rigName;
    }
    if (parentComp != NULL) {
        this->m_pParentComp = parentComp;
    }
}

Component::~Component() {

}

void Component::addChildComp(boost::shared_ptr<Component> comp) {
    this->m_vpChildComps.push_back(comp);
}

MStatus Component::setMetaDataNode(MObject metaDataNode) {
    MStatus status = MS::kFailure;
    
    if(!metaDataNode.isNull()) {
        this->m_metaDataNode = metaDataNode;
        status = MS::kSuccess;
    }

    return status;
}

void Component::updateMetaParentNode(MDGModifier & dgMod) {
    //disconnect the old metaParent, connects the new metaParent of the parent component
    MObject newParentObj = this->getMetaParentNode();
    MFnDependencyNode newParentFn( newParentObj );
    MObject oldParentObj; 
    lrutils::getMetaParent( this->m_metaDataNode, oldParentObj);
    MFnDependencyNode oldParentFn( oldParentObj );

    MFnDependencyNode metaDataNodeFn( this->m_metaDataNode );

    dgMod.disconnect(oldParentFn.findPlug("metaChildren"), metaDataNodeFn.findPlug("metaParent"));
    dgMod.connect(newParentFn.findPlug("metaChildren"), metaDataNodeFn.findPlug("metaParent"));
    dgMod.doIt();
}

MObject Component::getMetaParentNode() {
    if(m_pParentComp) {
        return m_pParentComp->getMetaDataNode();
    } else {
        return MObject::kNullObj;
    }
}