/*************************************************************
* Summary: Creates a MDGlobalNode which is a meta data       *
*          node corresponding to a global component.         *
*  Author: Logan Kelly                                       *
*    Date: 11/7/12                                           *
*************************************************************/

#ifndef _MDGlobalNode
#define _MDGlobalNode

#include "MetaDataNode.h"

class MDGlobalNode : public MetaDataNode {
public:
    MDGlobalNode();
    virtual ~MDGlobalNode();

    virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
    static  void*		creator();
	static  MStatus		initialize();

public:
    static MTypeId  id;

    static MObject controller; //controller curve object
    static MObject controllerGrp; //controller group object
    static MObject rigParentConstraint; //parent constraint object for rig group
    static MObject rigScaleConstraint; //scale constraint object for rig group
    static MObject noTransformScaleConstraint; //scale constraint object for noTransform group
};

#endif //_MDGlobalNode