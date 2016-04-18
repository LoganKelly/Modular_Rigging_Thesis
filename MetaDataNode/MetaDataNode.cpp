/*************************************************************
* Summary: Base MetaDataNode class used for creating         *
* persistent metadata connections between rig components and *
* the meta-root node.                                        *
*  Author: Logan Kelly                                       *
*    Date: 8/31/12                                           *
*************************************************************/

#include "MetaDataNode.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MGlobal.h>

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     MetaDataNode::id( 0x00333 );

MObject     MetaDataNode::metaParent;
MObject     MetaDataNode::metaChildren;
MObject     MetaDataNode::version;
MObject     MetaDataNode::rigId;

MetaDataNode::MetaDataNode() {}
MetaDataNode::~MetaDataNode() {}

MStatus MetaDataNode::compute( const MPlug& plug, MDataBlock& data )
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
//
{
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

void* MetaDataNode::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new MetaDataNode();
}

MStatus MetaDataNode::initialize()
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

    addAttribute( metaParent );
    addAttribute( metaChildren );
    addAttribute( version );
    addAttribute( rigId );

	return MS::kSuccess;

}

