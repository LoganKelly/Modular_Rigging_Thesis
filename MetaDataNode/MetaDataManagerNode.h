/*************************************************************
* Summary: Defines a MetaDataManager node that is used       *
*          for tracking MetaRoot nodes in the scene and      *
*          updating their metadata networks.                 *
*  Author: Logan Kelly                                       *
*    Date: 9/18/12                                           *
*************************************************************/

#ifndef _MetaDataManagerNode
#define _MetaRootManagerNode

#include <maya/MPxNode.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MTypeId.h>

class MetaDataManagerNode : public MPxNode
{
public:
						MetaDataManagerNode();
	virtual				~MetaDataManagerNode(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:
    static  MObject     metaRoots;
    static  MTypeId		id;

};

#endif