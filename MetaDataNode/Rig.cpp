/***************************************************************
* Summary: Rig class which contains all the guide information  *
*          and functions necessary for building a rig from     *
*          components.                                         *
*  Author: Logan Kelly                                         *
*    Date: 10/18/12                                            *
***************************************************************/

#include <maya/MGlobal.h>
#include <maya/MItDependencyNodes.h>
#include "MetaDataManagerNode.h"
#include "Rig.h"
#include "MyErrorChecking.h"
#include "LoadRigUtils.h"
#include <sstream>

Rig::Rig(MString xmlPath, MObject metaRootNodeObj) {
    if(xmlPath.length() > 0) {
        this->readXml(xmlPath);
        this->m_metaRootNodeObj = metaRootNodeObj;
        if (!this->m_metaRootNodeObj.isNull()) {
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_topGroupObj, "topGroup");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_rigGroupObj, "rigGroup");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_ctlGroupObj, "ctlGroup");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_noTransformGroupObj, "noTransformGroup");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_skelLayerObj, "skelLayer");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_ctlLayerObj, "ctlLayer");
            lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_extrasLayerObj, "extrasLayer");
        }
        this->createComponentsFromXML();
    }
}

Rig::~Rig() {

}

void Rig::readXml(MString xmlPath) {
    this->m_pXmlGuide.reset( new XmlGuide(xmlPath, true));
    this->m_pXmlGuide->getName(this->m_name);
}

ComponentPtr Rig::createComponent(ComponentGuidePtr guide, ComponentPtr parentComp) {
    MString type = guide->getType();
    ComponentPtr component;
    MString rigName = this->m_name;
    if(type == MString("global") ) {
        GlobalComponentGuidePtr globalGuide = boost::shared_dynamic_cast<GlobalComponentGuide>(guide);
        component.reset( new GlobalComponent(globalGuide,rigName, parentComp) );
        return component;
    }
    else if(type == MString("hip") ) {
        HipComponentGuidePtr hipGuide = boost::shared_dynamic_cast<HipComponentGuide>(guide);
        component.reset( new HipComponent(hipGuide,rigName, parentComp) );
        return component;
    }
    else if(type == MString("spine") ) {
        SpineComponentGuidePtr spineGuide = boost::shared_dynamic_cast<SpineComponentGuide>(guide);
        component.reset( new SpineComponent(spineGuide, rigName, parentComp) );
        return component;
    }
    else {
        stringstream msg; msg << "component guide type " << type.asChar() << " is invalid";
        MGlobal::displayError(msg.str().c_str());
        return ComponentPtr();
    }

}

ComponentPtr Rig::recursiveComponentCreate(ComponentGuidePtr guide, ComponentPtr parentComp) {
    ComponentPtr comp = createComponent(guide, parentComp);
    //iterate through child component guides and call recursive create for them
    unsigned int numGuides = guide->getNumChildComps();
    for(unsigned int i = 0; i < numGuides; i++) {
        ComponentGuidePtr childCompGuide = guide->getChildCompGuide(i);
        ComponentPtr childComp = this->recursiveComponentCreate(childCompGuide,comp);
        comp->addChildComp(childComp);
    }

    return comp;
}

void Rig::createComponentsFromXML() {
    ComponentGuidePtr rootGuide = this->m_pXmlGuide->getRootComponent();
    if(rootGuide) {
        MString type = rootGuide->getType();
        this->m_pRootComponent = recursiveComponentCreate(rootGuide,ComponentPtr());
    }
}

MStatus Rig::load(MDGModifier & dgMod) {
    MStatus status = MS::kFailure;

   //get info from the xml file  
    MString m_name;
    status = m_pXmlGuide->getName(m_name);
    MyCheckStatus(status, "getName failed");
    float version;
    status = m_pXmlGuide->getVersion(version);
    MyCheckStatus(status, "getVersion failed");
    MString geoFilePath;
    status = m_pXmlGuide->getGeoFilePath(geoFilePath);
    //MyCheckStatus(status, "getGeoFilePath failed");
    MString geoName;
    status = m_pXmlGuide->getGeoName(geoName);
    //MyCheckStatus(status, "getGeoName failed");

    //create groups used for scene organization
    status = lrutils::makeGroup(m_topGroupObj, this->m_name);
    MyCheckStatusReturn(status, "lrutils::makeGroup failed");
    MFnTransform topGroupFn;
    topGroupFn.setObject(m_topGroupObj);
    MFnMessageAttribute mAttr;
    MObject transformAttr = mAttr.create("metaParent", "metaParent");
    topGroupFn.addAttribute(transformAttr);
    MString rigGroupName = this->m_name+"_Rig";
    status = lrutils::makeGroup(m_rigGroupObj, rigGroupName);
    MFnTransform rigGroupFn; rigGroupFn.setObject( m_rigGroupObj );
    rigGroupFn.addAttribute( mAttr.create("metaParent", "metaParent") );
    MString ctlGroupName = this->m_name+"_Control";
    status = lrutils::makeGroup(m_ctlGroupObj, ctlGroupName);
    MFnTransform ctlGroupFn; ctlGroupFn.setObject( m_ctlGroupObj );
    ctlGroupFn.addAttribute( mAttr.create("metaParent", "metaParent") );
    MString noTransformName = this->m_name+"_NoTransform";
    status = lrutils::makeGroup(m_noTransformGroupObj, noTransformName);
    MFnTransform noTransformGroupFn; noTransformGroupFn.setObject( m_noTransformGroupObj );
    noTransformGroupFn.addAttribute( mAttr.create("metaParent", "metaParent") );
    topGroupFn.addChild(m_rigGroupObj);
    topGroupFn.addChild(m_ctlGroupObj);
    topGroupFn.addChild(m_noTransformGroupObj);

    //create display layers used to organize the rig
    //skeleton layer
    status = MGlobal::executeCommand( "python(\"layer = Utils.makeDisplayLayer('"+this->m_name+"_Skeleton_LYR')\");" );
    status = MGlobal::executeCommand( "python(\"layer.setAttr('visibility',False)\");" );
    MCommandResult res;
    status = MGlobal::executeCommand( MString("python(\"layer.name()\");"), res );
    MString sResult;
    res.getResult(sResult);
    lrutils::getObjFromName(sResult, m_skelLayerObj);
    MFnDependencyNode skelLayerFn( m_skelLayerObj );
    skelLayerFn.addAttribute( mAttr.create("metaParent", "metaParent") );
    //controllers layer
    status = MGlobal::executeCommand( "python(\"layer = Utils.makeDisplayLayer('"+this->m_name+"_Controllers_LYR')\");" );
    status = MGlobal::executeCommand( MString("python(\"layer.name()\");"), res );
    res.getResult(sResult);
    lrutils::getObjFromName(sResult, m_ctlLayerObj);
    MFnDependencyNode ctlLayerFn( m_ctlLayerObj );
    ctlLayerFn.addAttribute( mAttr.create("metaParent", "metaParent") );
    //extras layer
    status = MGlobal::executeCommand( "python(\"layer = Utils.makeDisplayLayer('"+this->m_name+"_ExtraStuff_DONOTUNHIDE_LYR')\");" );
    status = MGlobal::executeCommand( "python(\"layer.setAttr('visibility',False)\");" );
    status = MGlobal::executeCommand( MString("python(\"layer.name()\");"), res );
    res.getResult(sResult);
    lrutils::getObjFromName(sResult, m_extrasLayerObj);
    MFnDependencyNode extrasLayerFn( m_extrasLayerObj );
    extrasLayerFn.addAttribute( mAttr.create("metaParent", "metaParent") );


    //load referenced geometry into scene
    MObject geoObj;
    MFnTransform transformFn;
    if(geoFilePath != "" && geoName != "") {
        lrutils::loadGeoReference(geoFilePath, geoName, m_name, geoObj);
        transformFn.setObject(geoObj);
        //noTransformGroupFn.setObject(m_noTransformGroupObj);
        //noTransformGroupFn.addChild(geoObj);
        rigGroupFn.addChild(geoObj);
    }
    
    //find the MetaDataManager node if it exists
    MObject metaDataManagerNodeObj;
    bool MDMexists = false;
    for( MItDependencyNodes nodeIt(MFn::kPluginDependNode);
        !nodeIt.isDone(); nodeIt.next() ) {
        MFnDependencyNode nodeFn( nodeIt.item() );
        if(nodeFn.typeId() == MetaDataManagerNode::id) {
            MDMexists = true;
            metaDataManagerNodeObj = nodeIt.thisNode();
            break;
        }
    }
    //the manager node wasn't found, so make it
    if(!MDMexists) {
        metaDataManagerNodeObj = dgMod.createNode( "MetaDataManagerNode", &status );
        MyCheckStatusReturn(status, "createNode failed");
    }    
    MFnDependencyNode depManagerNodeFn( metaDataManagerNodeObj );
    
    //create and set up root node
    MObject metaRootNodeObj = dgMod.createNode( "MetaRootNode", &status );
    MyCheckStatusReturn(status, "createNode failed");
    MFnDependencyNode depRootNodeFn( metaRootNodeObj );
    MString rootNodeName = "MRN_";
    rootNodeName += m_name;
    MString xmlPath;
    status = this->m_pXmlGuide->getFilePath(xmlPath);
    MyCheckStatusReturn(status, "get xml file path failed");
    status = dgMod.newPlugValueString( depRootNodeFn.findPlug("xmlPath"), xmlPath );
    dgMod.renameNode(metaRootNodeObj, rootNodeName);
    MyCheckStatusReturn(status, "newPlugValueString failed");
    status = dgMod.newPlugValueFloat( depRootNodeFn.findPlug("version"), version );

    //create meta data network connections
    status = dgMod.connect( depManagerNodeFn.findPlug("metaRoots"), depRootNodeFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    if(transformFn.object() != MObject::kNullObj) {
        status = dgMod.connect( depRootNodeFn.findPlug("geometry"), transformFn.findPlug("metaParent") );
        MyCheckStatusReturn(status, "connect failed");
    }
    status = dgMod.connect( depRootNodeFn.findPlug("topGroup"), topGroupFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("rigGroup"), rigGroupFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("ctlGroup"), ctlGroupFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("noTransformGroup"), noTransformGroupFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("skelLayer"), skelLayerFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("ctlLayer"), ctlLayerFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    status = dgMod.connect( depRootNodeFn.findPlug("extrasLayer"), extrasLayerFn.findPlug("metaParent") );
    MyCheckStatusReturn(status, "connect failed");
    //the rig connections must be made before the rest of the components are loaded
    dgMod.doIt();
    
    //load the rest of the components needed for the rig
    if(m_pRootComponent) {
        MObject metaNodeObj = this->recursiveLoadComponents(m_pRootComponent, dgMod);
        MFnDependencyNode depNodeFn( metaNodeObj );
        status = dgMod.connect( depRootNodeFn.findPlug("metaChildren"), depNodeFn.findPlug("metaParent") );
    }
    //MyCheckStatusReturn(status, "connect failed");

    return status;   
}

MStatus Rig::update(bool forceUpdate, bool globalPos) {
    this->m_pRigIdManager.reset( new RigIdManager() );

    MStatus status = MS::kFailure;
    if(this->m_metaRootNodeObj.isNull())
        MyCheckStatusReturn(status, "Rig->m_metaRootNodeObj has not been set to a valid MObject.");

    MFnDependencyNode metaRootNodeFn(this->m_metaRootNodeObj);
    MDGModifier dgMod;

    //update the xml path attribute value
    MString xmlPath;
    status = this->m_pXmlGuide->getFilePath(xmlPath);
    MyCheckStatusReturn(status, "get xml file path failed");
    status = dgMod.newPlugValueString( metaRootNodeFn.findPlug("xmlPath"), xmlPath );

    status = this->updateNodeNames();
    status = this->updateGeoNodes();

    //update the rest of the components of the rig
    //MObject metaRootCompObj;
    //lrutils::getMetaChildByName(this->m_metaRootNodeObj, this->m_pRootComponent->getCompGuide()->getName(), metaRootCompObj);
    if(!this->m_pRootComponent) {
        return status;
    }
    this->recursiveGetComponentIds(this->m_pRootComponent);
    //get the first component of the rig
    MObject firstMetaChildObj;
    status = lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, firstMetaChildObj, MString("metaChildren"));
    if(status == MS::kFailure) {
        return status;
    }
    MyCheckStatus(status, "lrutils::getMetaNodeConnection failed");
    if(!firstMetaChildObj.isNull()) {
        this->recursiveGetMetaDataIds(firstMetaChildObj);
    }
    MString idManagerString = this->m_pRigIdManager->toString();
    //MGlobal::displayInfo(this->m_pRigIdManager->toString());
    //this->recursiveUpdateComponents(metaRootCompObj, this->m_pRootComponent, dgMod);
    this->m_pRigIdManager->updateComponents(dgMod,forceUpdate,globalPos);


    return status;
}

MStatus Rig::updateGeoNodes() {
    MStatus status = MS::kFailure;
    MDGModifier dgMod;

    MFnDependencyNode metaRootNodeFn( this->m_metaRootNodeObj );

    MPlug geometryPlug = metaRootNodeFn.findPlug(MString("geometry"),true,&status);
    MyCheckStatusReturn(status,"findPlug geometry failed");
    MPlugArray geometryPlugs;
    geometryPlug.connectedTo(geometryPlugs,false,true,&status);
    MyCheckStatusReturn(status, "MPlug.connectedTo failed");
    for(unsigned int j = 0; j < geometryPlugs.length(); j++) {
        MObject geoObj = geometryPlugs[j].node(&status);
        MFnTransform geoObjFn( geoObj );
        MyCheckStatusReturn(status, "MPlug.node() failed");

        //find the reference namespace
        string str ( geoObjFn.name().asChar() );
        string key (":");
        size_t found = str.rfind(key);
        string sNamespace = str.substr(0,found);
        sNamespace.append("RN" );

        //store the original transformation matrix, name of geometry transform node, and parent object of the geometry
        MTransformationMatrix origGeoTransMat = geoObjFn.transformation();
        MString origGeoName = geoObjFn.name();
        MObject parentObj = geoObjFn.parent(0);
        MFnTransform parentFn( parentObj );

        //remove the reference node, and load the reference file from the new path
        {
            stringstream tmp;
            tmp << "file -removeReference -referenceNode \"" << sNamespace.c_str() << "\";";
            MString referenceCommand = MString(tmp.str().c_str());
            MGlobal::executeCommand(referenceCommand);

            //get info from the xml file      
            MString geoFilePath;
            status = m_pXmlGuide->getGeoFilePath(geoFilePath);
            MyCheckStatus(status, "getGeoFilePath failed");
            MString name;
            status = m_pXmlGuide->getName(name);
            MyCheckStatus(status, "getName failed");
            MString geoName;
            status = m_pXmlGuide->getGeoName(geoName);
            MyCheckStatus(status, "getGeoName failed");

            MObject geoObj;
            lrutils::loadGeoReference(geoFilePath,geoName,name, geoObj);
            m_pXmlGuide->setName(name);
            parentFn.addChild(geoObj);
            MFnTransform transformFn(geoObj);
            transformFn.set(origGeoTransMat);

            status = dgMod.connect( metaRootNodeFn.findPlug("geometry"), transformFn.findPlug("metaParent") );
            MyCheckStatusReturn(status, "connect failed");
            dgMod.doIt();
        }

    }
    //if there is no geometry loaded but xml tag is present, load it into the scene
    if(geometryPlugs.length() == 0) {
        //get info from the xml file      
        MString geoFilePath;
        status = lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, m_rigGroupObj, MString("rigGroup"));
        MFnTransform rigGroupFn; 
        rigGroupFn.setObject( m_rigGroupObj );
        status = m_pXmlGuide->getGeoFilePath(geoFilePath);
        //MyCheckStatus(status, "getGeoFilePath failed");
        MString name;
        status = m_pXmlGuide->getName(name);
        //MyCheckStatus(status, "getName failed");
        MString geoName;
        status = m_pXmlGuide->getGeoName(geoName);
        //MyCheckStatus(status, "getGeoName failed");

        //load referenced geometry into scene
        MObject geoObj;
        MFnTransform transformFn;
        if(geoFilePath != "" && geoName != "") {
            lrutils::loadGeoReference(geoFilePath, geoName, name, geoObj);
            transformFn.setObject(geoObj);
            rigGroupFn.addChild(geoObj);

            if(transformFn.object() != MObject::kNullObj) {
                status = dgMod.connect( metaRootNodeFn.findPlug("geometry"), transformFn.findPlug("metaParent") );
                MyCheckStatusReturn(status, "connect failed");
                dgMod.doIt();
            }
        }
    }

    return status;
}

MStatus Rig::updateNodeNames() {
    MStatus status = MS::kFailure;

    MString xmlName = this->m_name;
    
    MFnDependencyNode metaRootNodeFn(this->m_metaRootNodeObj);
    MString metaNodeName = metaRootNodeFn.name();
    MString rigName = metaNodeName.substring(4,metaNodeName.numChars()-1);
    //if the names match, skip the update
    if( xmlName == rigName ) {
        status = MS::kSuccess;
        return status;
    }
    else {
        //update the metaRoot node name
        status = lrutils::stringReplaceAll(metaNodeName, rigName, xmlName);
        metaRootNodeFn.setName(metaNodeName);
        //update the topGroup node name
        MFnDependencyNode nodeFn( this->m_topGroupObj );
        MString nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the rigGroup node name
        nodeFn.setObject( this->m_rigGroupObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the ctlGroup node name
        nodeFn.setObject( this->m_ctlGroupObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the noTransformGroup node name
        nodeFn.setObject( this->m_noTransformGroupObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the skelLayer node name
        nodeFn.setObject( this->m_skelLayerObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the ctlLayer node name
        nodeFn.setObject( this->m_ctlLayerObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);
        //update the extrasLayer node name
        nodeFn.setObject( this->m_extrasLayerObj );
        nodeName = nodeFn.name();
        status = lrutils::stringReplaceAll(nodeName, rigName, xmlName);
        nodeFn.setName(nodeName);

        
        status = MS::kSuccess;
    }


    return status;
}

MObject Rig::recursiveLoadComponents(ComponentPtr comp, MDGModifier & dgMod) {
    MStatus status = MS::kFailure;

    MObject metaNodeObj = comp->loadComponent(dgMod);
    MFnDependencyNode metaNodeFn( metaNodeObj );
    for(unsigned int i = 0; i < comp->getNumChildComps(); i++) {
        ComponentPtr childComp = comp->getChildComp(i);
        MObject childMetaNodeObj = this->recursiveLoadComponents(childComp, dgMod);
        MFnDependencyNode childMetaNodeFn( childMetaNodeObj );
        status = dgMod.connect( metaNodeFn.findPlug("metaChildren"), childMetaNodeFn.findPlug("metaParent") );
        MyCheckStatus(status, "connect failed");
    }

    return metaNodeObj;
}

void Rig::recursiveUpdateComponents(MObject metaNodeObj, ComponentPtr comp, MDGModifier & dgMod) {
    MStatus status = MS::kFailure;

    for(unsigned int i = 0; i < comp->getNumChildComps(); i++) {
        ComponentPtr childComp = comp->getChildComp(i);
        //MString childCompName = (childComp->getCompGuide())->getName();
        MString childRigId = (childComp->getCompGuide())->getRigId();
        MObject metaChildNodeObj;
        //status = lrutils::getMetaChildByName(metaNodeObj, childCompName, metaChildNodeObj);
        status = lrutils::getMetaChildByRigId(metaNodeObj, childRigId, metaChildNodeObj);
        //if the metaChildNodeObj is found, then proceed with the update
        if( !metaChildNodeObj.isNull() ) {
            this->recursiveUpdateComponents(metaChildNodeObj, childComp, dgMod);
        }
        //if the metaChildNodeObj is not found, then the component is either new or has changed its
        //position in the xml structure from a previous configuration
        //else {

        //}
    }
    comp->setMetaDataNode(metaNodeObj);
    comp->updateComponent(dgMod);
}

void Rig::recursiveGetComponentIds(ComponentPtr comp) {
    for(unsigned int i = 0; i < comp->getNumChildComps(); i++) {
        ComponentPtr childComp = comp->getChildComp(i);
        if( childComp ) {
            this->recursiveGetComponentIds(childComp);
        }
    }
    ComponentGuidePtr guide = comp->getCompGuide();
    MString id = guide->getRigId();
    this->m_pRigIdManager->addId(id, comp, MObject::kNullObj);    
}

void Rig::recursiveGetMetaDataIds(MObject metaNodeObj) {
    MStatus status = MS::kFailure;

    MFnDependencyNode metaNodeFn( metaNodeObj );
    MPlug metaChildrenPlug = metaNodeFn.findPlug( "metaChildren", true, &status );
    MyCheckStatus(status, "MFnDependencyNode.findPlug() failed");

    //follow the plug connection to the connected plug on the other object
    MPlugArray connectedChildPlugs;
    metaChildrenPlug.connectedTo(connectedChildPlugs,false,true,&status);
    MyCheckStatus(status,"MPlug.connectedTo() failed");

    for (unsigned int i = 0; i < connectedChildPlugs.length(); i++) {
        MPlug connectedPlug = connectedChildPlugs[i];
        MObject connectedNodeObj = connectedPlug.node(&status);
        MyCheckStatus(status, "MPlug.node() failed");

        this->recursiveGetMetaDataIds(connectedNodeObj);
    }
    //get the rigId number held in the rigId attribute
    MString metaNodeName = metaNodeFn.name();
    MPlug rigIdPlug = metaNodeFn.findPlug(MString("rigId"),true,&status);
    MyCheckStatus(status,"findPlug failed");
    MString metaId;
    rigIdPlug.getValue(metaId);
    this->m_pRigIdManager->addId(metaId, ComponentPtr(), metaNodeObj );
}

MStatus Rig::remove(MDGModifier &dgMod) {
    MStatus status = MS::kFailure;
    if(this->m_metaRootNodeObj.isNull())
        MyCheckStatusReturn(status, "Rig->m_metaRootNodeObj has not been set to a valid MObject.");

    //remove the components of the rig
    MObject metaRootCompObj;
    lrutils::getMetaChildByName(this->m_metaRootNodeObj, this->m_pRootComponent->getCompGuide()->getName(), metaRootCompObj);
    this->recursiveRemoveComponents(metaRootCompObj, this->m_pRootComponent, dgMod);

    //remove the geometry nodes
    this->removeGeoNodes();

    //delete the top level rig nodes
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_noTransformGroupObj, "noTransformGroup" );
    MGlobal::deleteNode( this->m_noTransformGroupObj );
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_ctlGroupObj, "ctlGroup" );
    MGlobal::deleteNode( this->m_ctlGroupObj );
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_rigGroupObj, "rigGroup" );
    MGlobal::deleteNode( this->m_rigGroupObj );
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_topGroupObj, "topGroup" );
    MGlobal::deleteNode( this->m_topGroupObj );

    //delete the display layers for the rig
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_ctlLayerObj, "ctlLayer" );
    MGlobal::deleteNode( this->m_ctlLayerObj );
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_skelLayerObj, "skelLayer" );
    MGlobal::deleteNode( this->m_skelLayerObj );
    lrutils::getMetaNodeConnection(this->m_metaRootNodeObj, this->m_extrasLayerObj, "extrasLayer" );
    MGlobal::deleteNode( this->m_extrasLayerObj );

    return status;
}

MStatus Rig::removeGeoNodes() {
    MStatus status = MS::kFailure;

    MFnDependencyNode metaRootNodeFn( this->m_metaRootNodeObj );

    MPlug geometryPlug = metaRootNodeFn.findPlug( "geometry", true, &status );
    MyCheckStatusReturn(status,"findPlug geometry failed");
    MPlugArray geometryPlugs;
    geometryPlug.connectedTo(geometryPlugs,false,true,&status);
    MyCheckStatusReturn(status, "MPlug.connectedTo failed");
    for(unsigned int j = 0; j < geometryPlugs.length(); j++) {
        MObject geoObj = geometryPlugs[j].node(&status);
        MFnTransform geoObjFn( geoObj );
        MyCheckStatusReturn(status, "MPlug.node() failed");

        //find the reference namespace
        string str ( geoObjFn.name().asChar() );
        string key (":");
        size_t found = str.rfind(key);
        string sNamespace = str.substr(0,found);
        sNamespace.append("RN" );

        //remove the reference node
        {
            stringstream tmp;
            tmp << "file -removeReference -referenceNode \"" << sNamespace.c_str() << "\";";
            MString referenceCommand = MString(tmp.str().c_str());
            MGlobal::executeCommand(referenceCommand);
        }
    }

    return status;
}

void Rig::recursiveRemoveComponents(MObject metaNodeObj, ComponentPtr comp, MDGModifier & dgMod) {
    for( unsigned int i = 0; i < comp->getNumChildComps(); i++) {
        ComponentPtr childComp = comp->getChildComp(i);
        MString childCompName = (childComp->getCompGuide())->getName();
        MObject metaChildNodeObj;
        lrutils::getMetaChildByName(metaNodeObj, childCompName, metaChildNodeObj);
        this->recursiveRemoveComponents(metaChildNodeObj, childComp, dgMod);
    }
    comp->setMetaDataNode(metaNodeObj);
    comp->removeComponent(dgMod);
}