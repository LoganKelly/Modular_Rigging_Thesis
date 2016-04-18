/*************************************************************
* Summary: Creates a MDSpineNode which is a meta data        *
*          node corresponding to a spine component.          *
*  Author: Logan Kelly                                       *
*    Date: 5/12/13                                           *
*************************************************************/

#ifndef _MDSpineNode
#define _MDSpineNode

#include "MetaDataNode.h"

class MDSpineNode : public MetaDataNode {
public:
    MDSpineNode();
    virtual ~MDSpineNode();

    virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
    static  void*		creator();
	static  MStatus		initialize();

public:
    static MTypeId  id;

    static MObject FKControllers; //controller curve objects for FK
    static MObject FKControllerGrps; //controller group objects for FK
    static MObject FKJoints; //joint objects used for FK
    static MObject BindJoints; //joint objects used for binding to skin weights
    static MObject FKJointParentConstraints; //parent constraints from controllers to FK joints
    static MObject HipJointCopy; //copy of the hip joint
    static MObject SplineIKHandle; //ik handle for the spline IK system
    static MObject SplineIKEndEffector; //end effector for the spline IK system
    static MObject SplineIKCurve; //curve object for the spline IK system
    static MObject ShoulderControl;
    static MObject ShoulderControlGroup;
    static MObject ShoulderControlConstraint;
    static MObject HipControlConstraint;
    static MObject MiddleClusterConstraint;
    static MObject MathNodes; //math nodes used for calculating stretchy spline IK
    static MObject SplineIKClusters; //cluster objects used for controlling the curve CVs of the spline IK
    static MObject KinematicType;
};

#endif //_MDSpineNode