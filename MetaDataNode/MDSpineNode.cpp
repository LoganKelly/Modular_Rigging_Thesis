/*************************************************************
* Summary: Creates a MDSpineNode which is a meta data        *
*          node corresponding to a spine component.          *
*  Author: Logan Kelly                                       *
*    Date: 5/12/13                                           *
*************************************************************/

#include "MDSpineNode.h"

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     MDSpineNode::id( 0x00338 );

MObject     MDSpineNode::FKControllers;
MObject     MDSpineNode::FKControllerGrps;
MObject     MDSpineNode::FKJoints;
MObject     MDSpineNode::BindJoints;
MObject     MDSpineNode::FKJointParentConstraints;
MObject     MDSpineNode::HipJointCopy;
MObject     MDSpineNode::SplineIKHandle;
MObject     MDSpineNode::SplineIKEndEffector;
MObject     MDSpineNode::SplineIKCurve;
MObject     MDSpineNode::ShoulderControl;
MObject     MDSpineNode::ShoulderControlGroup;
MObject     MDSpineNode::MathNodes;
MObject     MDSpineNode::SplineIKClusters;
MObject     MDSpineNode::ShoulderControlConstraint;
MObject     MDSpineNode::HipControlConstraint;
MObject     MDSpineNode::MiddleClusterConstraint;
MObject     MDSpineNode::KinematicType;

MDSpineNode::MDSpineNode() {}
MDSpineNode::~MDSpineNode() {}

MStatus MDSpineNode::compute( const MPlug& plug, MDataBlock& data ) {
	MStatus returnStatus;
 
	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if( plug == metaChildren )
	{
		
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

void* MDSpineNode::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new MDSpineNode();
}

MStatus MDSpineNode::initialize()
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//		
{
    MFnMessageAttribute mAttr;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;

    metaParent = mAttr.create("metaParent", "metaParent");
    metaChildren = tAttr.create("metaChildren", "metaChildren", MFnData::kString);  
    version = nAttr.create("version","version",MFnNumericData::kFloat,1);
    rigId = tAttr.create("rigId", "rigId", MFnData::kString);
    FKControllers = mAttr.create("FKControllers", "FKControllers");
    FKControllerGrps = mAttr.create("FKControllerGroups", "FKControllerGrps");
    FKJoints = mAttr.create("FKJoints", "FKJoints");
    BindJoints = mAttr.create("BindJoints", "BindJoints");
    FKJointParentConstraints = mAttr.create("FKJointParentConstraints", "FKJointPrntCnstrnts");
    HipJointCopy = mAttr.create("HipJointCopy", "HipJointCopy");
    SplineIKHandle = mAttr.create("SplineIKHandle", "SplineIKHandle");
    SplineIKEndEffector = mAttr.create("SplineIKEndEffector","SplineIKEndEffector");
    SplineIKCurve = mAttr.create("SplineIKCurve","SplineIKCurve");
    ShoulderControl = mAttr.create("ShoulderControl","ShoulderCtl");
    ShoulderControlGroup = mAttr.create("ShoulderControlGroup","ShoulderCtlGrp");
    MathNodes = mAttr.create("MathNodes","MathNodes");
    SplineIKClusters = mAttr.create("SplineIKClusters","SplineIKClusters");
    ShoulderControlConstraint = mAttr.create("ShoulderControlConstraint","ShoulderControlConstraint");
    HipControlConstraint = mAttr.create("HipControlConstraint","HipControlConstraint");
    MiddleClusterConstraint = mAttr.create("MiddleClusterConstraint","MiddleClusterConstraint");
    KinematicType = tAttr.create("KinematicType","KinematicType", MFnData::kString);

    addAttribute( metaParent );
    addAttribute( metaChildren );
    addAttribute( version );
    addAttribute( rigId );
    addAttribute( FKControllers );
    addAttribute( FKControllerGrps );
    addAttribute( FKJoints );
    addAttribute( BindJoints );
    addAttribute( FKJointParentConstraints );
    addAttribute( HipJointCopy );
    addAttribute( SplineIKHandle );
    addAttribute( SplineIKEndEffector );
    addAttribute( SplineIKCurve );
    addAttribute( ShoulderControl );
    addAttribute( ShoulderControlGroup );
    addAttribute( MathNodes );
    addAttribute( SplineIKClusters );
    addAttribute( ShoulderControlConstraint );
    addAttribute( HipControlConstraint );
    addAttribute( MiddleClusterConstraint );
    addAttribute( KinematicType );

	return MS::kSuccess;

}