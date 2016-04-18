/*************************************************************
* Summary: Base MetaDataNode class used for creating         *
* persistent metadata connections between rig components and *
* the meta-root node.                                        *
*  Author: Logan Kelly                                       *
*    Date: 8/31/12                                           *
*************************************************************/

#ifndef _MetaDataNode
#define _MetaDataNode
//
// Copyright (C) 
// 
// File: MetaDataNodeNode.h
//
// Dependency Graph Node: MetaDataNode
//
// Author: Maya Plug-in Wizard 2.0
//

#include <maya/MPxNode.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MTypeId.h> 

 
class MetaDataNode : public MPxNode
{
public:
						MetaDataNode();
	virtual				~MetaDataNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
    static  MObject metaChildren;
    static  MObject metaParent;
    static  MObject version;
    static  MObject rigId;


	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static  MTypeId		id;
};

#endif
