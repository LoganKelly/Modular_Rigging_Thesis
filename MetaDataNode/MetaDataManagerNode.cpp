/*************************************************************
* Summary: Defines a MetaDataManager node that is used       *
*          for tracking MetaRoot nodes in the scene and      *
*          updating their metadata networks.                 *
*  Author: Logan Kelly                                       *
*    Date: 9/18/12                                           *
*************************************************************/

#include "MetaDataManagerNode.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MGlobal.h>

MTypeId     MetaDataManagerNode::id( 0x00335 );

MObject     MetaDataManagerNode::metaRoots;

MetaDataManagerNode::MetaDataManagerNode() {}
MetaDataManagerNode::~MetaDataManagerNode() {}

MStatus MetaDataManagerNode::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus returnStatus;
 
	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if( plug == metaRoots )
	{
		
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

void* MetaDataManagerNode::creator()
{
	return new MetaDataManagerNode();
}

MStatus MetaDataManagerNode::initialize()
{
    MFnMessageAttribute mAttr;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;

    metaRoots = tAttr.create("metaRoots", "metaRoots", MFnData::kString); 

    addAttribute( metaRoots );

	return MS::kSuccess;

}