/***************************************************************
* Summary: Rig class which contains all the guide information  *
*          and functions necessary for building a rig from     *
*          components.                                         *
*  Author: Logan Kelly                                         *
*    Date: 10/18/12                                            *
***************************************************************/

#ifndef _Rig
#define _Rig

#include <maya/MObject.h>
#include <maya/MDGModifier.h>
#include <maya/MString.h>
#include "RigIdManager.h"
#include "XMlGuide.h"
#include "GlobalComponent.h"
#include "HipComponent.h"
#include "SpineComponent.h"

typedef boost::shared_ptr<RigIdManager> RigIdManagerPtr;
typedef boost::shared_ptr<XmlGuide> XmlGuidePtr;
typedef boost::shared_ptr<ComponentGuide> ComponentGuidePtr;
typedef boost::shared_ptr<Component> ComponentPtr;
typedef boost::shared_ptr<GlobalComponent> GlobalComponentPtr;

class Rig
{
public:
    Rig(MString xmlPath = "", MObject metaRootNodeObj = MObject());
    ~Rig();
    MStatus load(MDGModifier & dgMod); //loads the rig into the scene based upon the guide information
    MStatus update(bool forceUpdate = false, bool globalPos = false); //updates a rig already loaded into the scene based upon its guide information
    MStatus updateGeoNodes(); //updates the geometry nodes connected to a given metaRoot node
    MStatus removeGeoNodes(); //removes the geometry nodes connected to the metaRoot node
    MStatus updateNodeNames(); //update the names of all the nodes attached to the metadata network of this rig
    MStatus remove(MDGModifier & dgMod); //removes the rig from the scene
    MString getName() {return m_name;};
    ComponentPtr createComponent(ComponentGuidePtr guide, ComponentPtr parentComp);
    ComponentPtr recursiveComponentCreate(ComponentGuidePtr guide, ComponentPtr parentComp); //recursively creates components based upon guide and child guide objects
    MObject recursiveLoadComponents(ComponentPtr comp, MDGModifier & dgMod); //load every rig component into the scene
    void recursiveUpdateComponents(MObject metaNodeObj, ComponentPtr comp, MDGModifier & dgMod); //update every rig component in the scene
    void recursiveRemoveComponents(MObject metaNodeObj, ComponentPtr comp, MDGModifier & dgMod);
    void recursiveGetComponentIds(ComponentPtr comp);
    void recursiveGetMetaDataIds(MObject metaNodeObj);

private:
    void readXml(MString xmlPath);
    void createComponentsFromXML();
    XmlGuidePtr m_pXmlGuide; //contains information loaded from xml file
    MObject m_metaRootNodeObj; //the corresponding meta root node for this rig in the DG
    //group nodes
    MObject m_topGroupObj; //top group node used for organization in the scene hierarchy
    MObject m_rigGroupObj; //rig group node used for organization in the scene hierarchy
    MObject m_ctlGroupObj; //control group node used for organization in the scene hierarchy
    MObject m_noTransformGroupObj; //noTransform group node used for organization in the scene hierarchy
    //display layer nodes
    MObject m_skelLayerObj;
    MObject m_ctlLayerObj;
    MObject m_extrasLayerObj;
    MString m_name;
    ComponentPtr m_pRootComponent; //the root component of the rig, usually the global component
    RigIdManagerPtr m_pRigIdManager;

};

#endif //_Rig