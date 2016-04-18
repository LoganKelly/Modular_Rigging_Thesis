/*************************************************************
* Summary: Creates a MDHipNode which is a meta data       *
*          node corresponding to a global component.         *
*  Author: Logan Kelly                                       *
*    Date: 11/7/12                                           *
*************************************************************/

#include "MDHipNode.h"

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     MDHipNode::id( 0x00337 );

MObject     MDHipNode::controller;
MObject     MDHipNode::controllerGrp;
MObject     MDHipNode::hipJoint;
MObject     MDHipNode::hipJointParentConstraint;

MDHipNode::MDHipNode() {}
MDHipNode::~MDHipNode() {}

MStatus MDHipNode::compute( const MPlug& plug, MDataBlock& data ) {
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

void* MDHipNode::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new MDHipNode();
}

MStatus MDHipNode::initialize()
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
    controller = mAttr.create("controller", "controller");
    controllerGrp = mAttr.create("controllerGroup", "controllerGrp");
    hipJoint = mAttr.create("hipJoint", "hipJoint");
    hipJointParentConstraint = mAttr.create("hipJointParentConstraint", "hipJointPrntCnstrnt");

    addAttribute( metaParent );
    addAttribute( metaChildren );
    addAttribute( version );
    addAttribute( rigId );
    addAttribute( controller );
    addAttribute( controllerGrp );
    addAttribute( hipJoint );
    addAttribute( hipJointParentConstraint );

	return MS::kSuccess;

}