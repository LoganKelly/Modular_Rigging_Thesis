/**************************************************************
* Summary: Manages a dictionary of all rig components indexed *
*          by id. Entries into the dictionary are composed of *
*          (id, component pointer, metadata node pointer)     *
*  Author: Logan Kelly                                        *
*    Date: 2/5/13                                             *
**************************************************************/

#ifndef _RigIdManager
#define _RigIdManager

#include <map>
#include <string>
#include <boost/lexical_cast.hpp>
#include "MetaDataNode.h"
#include "Component.h"

typedef boost::shared_ptr<Component> ComponentPtr;

class RigIdManager
{
public:
    RigIdManager();
    ~RigIdManager();
    void addId( MString id, ComponentPtr pComp, MObject metaNodeObj );
    bool checkIdExists( MString id );
    ComponentPtr getComponent( MString id );
    MObject getMetaDataNode( MString id );
    MString toString();
    //updates all components listed within the rigIdManager
    void updateComponents(MDGModifier & dgMod, bool forceUpdate = false, bool globalPos = false);
    
private:
    struct CompNodes
    {
        ComponentPtr pComponent;
        MObject metaNodeObj;
    };
    typedef boost::shared_ptr<CompNodes> CompNodesPtr;
    std::map<std::string, CompNodesPtr> m_IdDictionary;

    CompNodesPtr getCompNodes( MString id );
};

#endif