/**************************************************************
* Summary: Manages a dictionary of all rig components indexed *
*          by id. Entries into the dictionary are composed of *
*          (id, component pointer, metadata node pointer)     *
*  Author: Logan Kelly                                        *
*    Date: 2/5/13                                             *
**************************************************************/

#include "RigIdManager.h"
#include <string>
#include <maya/MGlobal.h>
#include "GlobalComponent.h"
#include "HipComponent.h"
#include "LoadRigUtils.h"
#include "MyErrorChecking.h"

typedef boost::shared_ptr<HipComponent> HipComponentPtr;
typedef boost::shared_ptr<GlobalComponent> GlobalComponentPtr;

RigIdManager::RigIdManager() {

}

RigIdManager::~RigIdManager() {
    this->m_IdDictionary.clear();
}

void RigIdManager::addId( MString id, ComponentPtr pComp, MObject metaNodeObj ) {
    std::string idString (id.asChar());
    CompNodesPtr cp;
    //if id already exists, grab its existing content
    if (checkIdExists(id) ) {
        cp = this->getCompNodes(id);
    }
    //if it doesn't, make a new set of nodes
    else {
        cp.reset( new CompNodes() );
    }
    //if it currently doesn't have a component and the argument component isn't null,
    //add the new argument component
    if( !cp->pComponent && pComp ) {
        cp->pComponent = pComp;
    }
    //if it currently doesn't have a metadata node and the argument metadata node isn't null,
    //add the new argument metadata node
    if( cp->metaNodeObj.isNull() && !metaNodeObj.isNull() ) {
        cp->metaNodeObj = metaNodeObj;
    }
    this->m_IdDictionary[idString] = cp;
}

bool RigIdManager::checkIdExists(MString id) {
    std::string idString(id.asChar());
    int count = (int)this->m_IdDictionary.count(idString);

    if ( count > 0 ) 
        return true;
    else 
        return false;
}

ComponentPtr RigIdManager::getComponent( MString id ) {
    std::string idString(id.asChar());
    CompNodesPtr cp = this->m_IdDictionary[idString];
    return cp->pComponent;
}

MObject RigIdManager::getMetaDataNode( MString id ) {
    std::string idString(id.asChar());
    CompNodesPtr cp = this->m_IdDictionary[idString];
    return cp->metaNodeObj;
}

RigIdManager::CompNodesPtr RigIdManager::getCompNodes( MString id ) {
    std::string idString(id.asChar());
    CompNodesPtr cp = this->m_IdDictionary[idString];
    return cp;
}

MString RigIdManager::toString() {
    MString printMsg;
    std::map<std::string, RigIdManager::CompNodesPtr>::iterator itr;

    for( itr = this->m_IdDictionary.begin(); itr != this->m_IdDictionary.end(); itr++ ) {
        RigIdManager::CompNodesPtr compNodes( (*itr).second );
        ComponentPtr comp = compNodes->pComponent;
        MObject metaNodeObj = compNodes->metaNodeObj;
        MFnDependencyNode metaNodeFn(metaNodeObj);

        printMsg += "Id: " + MString((*itr).first.c_str()) + " CompName: ";
        MString compName;
        if( comp ) {
            compName = comp->getCompGuide()->getName();
        } else {
            compName = "NULL";
        }
        printMsg += compName + " MetaNodeName: ";
        MString metaName = metaNodeFn.name() + "\n\r";
        printMsg += metaName;
    }

    return printMsg;
}

void RigIdManager::updateComponents(MDGModifier & dgMod, bool forceUpdate, bool globalPos) {
    std::map<std::string, RigIdManager::CompNodesPtr>::iterator itr;

    //pre-processing stuff, the metadata nodes need to be set before
    //updating can proceed correctly
    for( itr = this->m_IdDictionary.begin(); itr != this->m_IdDictionary.end(); itr++ ) {
        RigIdManager::CompNodesPtr compNodes( (*itr).second );
        ComponentPtr comp = compNodes->pComponent;
        MObject metaNodeObj = compNodes->metaNodeObj;
        MFnDependencyNode metaNodeFn(metaNodeObj);

        //if both the metadata node and the component exist, add the meta data node to the component object
        if( !metaNodeObj.isNull() && comp ) {
            comp->setMetaDataNode(metaNodeObj);
        }
    }

    //load new nodes before updating existing nodes
    //this will allow existing nodes to be updated and parented
    //under nodes that have just been added
    for( itr = this->m_IdDictionary.begin(); itr != this->m_IdDictionary.end(); itr++ ) {
        RigIdManager::CompNodesPtr compNodes( (*itr).second );
        ComponentPtr comp = compNodes->pComponent;
        MObject metaNodeObj = compNodes->metaNodeObj;
        MFnDependencyNode metaNodeFn(metaNodeObj);

        //if only the component exists but not the metadata node, add the component to the scene
        if ( metaNodeObj.isNull() && comp ) {
            MStatus status;
            MObject metaNodeObj = comp->loadComponent(dgMod);
            MFnDependencyNode metaNodeFn( metaNodeObj );
            MString metaNodeName = metaNodeFn.name();
            MObject parentMetaNodeObj = comp->getMetaParentNode();
            //if the parent meta data node is found, connect the child meta data node to it
            if( !parentMetaNodeObj.isNull() ) {
                MFnDependencyNode parentMetaNodeFn( parentMetaNodeObj );
                MString parentMetaNodeName = parentMetaNodeFn.name();

                status = dgMod.connect( parentMetaNodeFn.findPlug("metaChildren"), metaNodeFn.findPlug("metaParent") );
                MyCheckStatus(status, "dgMod.connect() failed: " + status.errorString() );
                dgMod.doIt();
            }
            //if this is a global component, its metaParent attribute needs to be set to the meta root node
            GlobalComponentPtr globalComp = boost::dynamic_pointer_cast<GlobalComponent>(comp);
            if(globalComp) {
                MObject metaRootObj;
                lrutils::getObjFromName("MRN_"+comp->getRigName(),metaRootObj);
                MFnDependencyNode metaRootFn( metaRootObj );

                status = dgMod.connect( metaRootFn.findPlug("metaChildren"), metaNodeFn.findPlug("metaParent") );
                MyCheckStatus(status, "dgMod.connect() failed: " + status.errorString() );
                dgMod.doIt();
            }
        }
    }

    //update all nodes before proceeding with removing old ones
    //this will fix dependencies in cases where a node used to be parented under a node
    //that was just deleted
    for( itr = this->m_IdDictionary.begin(); itr != this->m_IdDictionary.end(); itr++ ) {
        RigIdManager::CompNodesPtr compNodes( (*itr).second );
        ComponentPtr comp = compNodes->pComponent;
        MObject metaNodeObj = compNodes->metaNodeObj;
        MFnDependencyNode metaNodeFn(metaNodeObj);

        //if both the metadata node and the component exist, update the component in the scene
        if( !metaNodeObj.isNull() && comp ) {
            comp->updateComponent(dgMod, forceUpdate, globalPos);
            dgMod.doIt();
        }
    }

    //nodes can now be removed since nodes that might have been dependent on them
    //are updated and moved to their new parent nodes
    for( itr = this->m_IdDictionary.begin(); itr != this->m_IdDictionary.end(); itr++ ) {
        RigIdManager::CompNodesPtr compNodes( (*itr).second );
        ComponentPtr comp = compNodes->pComponent;
        MObject metaNodeObj = compNodes->metaNodeObj;
        MFnDependencyNode metaNodeFn(metaNodeObj);

        //if only the metadata node exists, remove the component from the scene
        if( !metaNodeObj.isNull() && !comp ) {
            MString nodeType = metaNodeFn.typeName();
            if( nodeType == "MDGlobalNode" ) {
                GlobalComponentPtr gComp( new GlobalComponent() );
                gComp->setMetaDataNode(metaNodeObj);
                gComp->removeComponent(dgMod);
            } else if( nodeType == "MDHipNode" ) {
                HipComponentPtr hComp( new HipComponent() );
                hComp->setMetaDataNode(metaNodeObj);
                hComp->removeComponent(dgMod);
            }
            dgMod.doIt();
        }
    }    
}