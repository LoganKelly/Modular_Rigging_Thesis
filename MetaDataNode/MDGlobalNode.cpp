/*************************************************************
* Summary: Creates a MDGlobalNode which is a meta data       *
*          node corresponding to a global component.         *
*  Author: Logan Kelly                                       *
*    Date: 11/7/12                                           *
*************************************************************/

#include "MDGlobalNode.h"

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     MDGlobalNode::id( 0x00336 );

MObject     MDGlobalNode::controller;
MObject     MDGlobalNode::controllerGrp;
MObject     MDGlobalNode::rigParentConstraint;
MObject     MDGlobalNode::rigScaleConstraint;
MObject     MDGlobalNode::noTransformScaleConstraint;

MDGlobalNode::MDGlobalNode() {}
MDGlobalNode::~MDGlobalNode() {}

MStatus MDGlobalNode::compute( const MPlug& plug, MDataBlock& data ) {
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

void* MDGlobalNode::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new MDGlobalNode();
}

MStatus MDGlobalNode::initialize()
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
    rigParentConstraint = mAttr.create("rigParentConstraint", "rigPrntCnstrnt");
    rigScaleConstraint = mAttr.create("rigScaleConstraint", "rigSclCnstrnt");
    noTransformScaleConstraint = mAttr.create("noTransformScaleConstraint", "noTransformSclCnstrnt");

    addAttribute( metaParent );
    addAttribute( metaChildren );
    addAttribute( version );
    addAttribute( rigId );
    addAttribute( controller );
    addAttribute( controllerGrp );
    addAttribute( rigParentConstraint );
    addAttribute( rigScaleConstraint );
    addAttribute( noTransformScaleConstraint );

	return MS::kSuccess;

}