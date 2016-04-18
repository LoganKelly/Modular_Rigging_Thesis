/*************************************************************
* Summary: Creates a node used as the root of a metadata     *
*           node network.                                    *
*  Author: Logan Kelly                                       *
*    Date: 8/31/12                                           *
*************************************************************/

#include "MetaRootNode.h"

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     MetaRootNode::id( 0x00334 );

MObject     MetaRootNode::xmlPath;
MObject     MetaRootNode::geometry;
MObject     MetaRootNode::topGroup;
MObject     MetaRootNode::rigGroup;
MObject     MetaRootNode::ctlGroup;
MObject     MetaRootNode::noTransformGroup;
MObject     MetaRootNode::skelLayer;
MObject     MetaRootNode::ctlLayer;
MObject     MetaRootNode::extrasLayer;

MetaRootNode::MetaRootNode() {}
MetaRootNode::~MetaRootNode() {}

MStatus MetaRootNode::compute( const MPlug& plug, MDataBlock& data )
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

void* MetaRootNode::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new MetaRootNode();
}

MStatus MetaRootNode::initialize()
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
    xmlPath = tAttr.create("xmlPath", "xmlPath", MFnData::kString);
    geometry = tAttr.create("geometry", "geo", MFnData::kString);
    topGroup = tAttr.create("topGroup", "topGrp", MFnData::kString);
    rigGroup = tAttr.create("rigGroup", "rigGrp", MFnData::kString);
    ctlGroup = tAttr.create("ctlGroup", "ctlGrp", MFnData::kString);
    noTransformGroup = tAttr.create("noTransformGroup", "noXformGrp", MFnData::kString);
    skelLayer = tAttr.create("skelLayer", "skelLyr", MFnData::kString);
    ctlLayer = tAttr.create("ctlLayer", "ctlLyr", MFnData::kString);
    extrasLayer = tAttr.create("extrasLayer", "xtrasLyr", MFnData::kString);

    addAttribute( metaParent );
    addAttribute( metaChildren );
    addAttribute( version );
    addAttribute( xmlPath );
    addAttribute( geometry );
    addAttribute( topGroup );
    addAttribute( rigGroup );
    addAttribute( ctlGroup );
    addAttribute( noTransformGroup );
    addAttribute( skelLayer );
    addAttribute( ctlLayer );
    addAttribute( extrasLayer );

	return MS::kSuccess;

}