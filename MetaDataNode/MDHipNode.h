/*************************************************************
* Summary: Creates a MDHipNode which is a meta data       *
*          node corresponding to a global component.         *
*  Author: Logan Kelly                                       *
*    Date: 11/7/12                                           *
*************************************************************/

#ifndef _MDHipNode
#define _MDHipNode

#include "MetaDataNode.h"

class MDHipNode : public MetaDataNode {
public:
    MDHipNode();
    virtual ~MDHipNode();

    virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
    static  void*		creator();
	static  MStatus		initialize();

public:
    static MTypeId  id;

    static MObject controller; //controller curve object
    static MObject controllerGrp; //controller group object
    static MObject hipJoint; //hip joint object
    static MObject hipJointParentConstraint; //parent constraint from controller to hip joint
};

#endif //_MDHipNode