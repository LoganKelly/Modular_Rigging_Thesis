/*************************************************************
* Summary: Creates a node used as the root of a metadata     *
*           node network.                                    *
*  Author: Logan Kelly                                       *
*    Date: 8/31/12                                           *
*************************************************************/

#ifndef _MetaRootNode
#define _MetaRootNode

#include "MetaDataNode.h"

class MetaRootNode : public MetaDataNode
{
public:
    MetaRootNode();
    virtual ~MetaRootNode();

    virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
    static  void*		creator();
	static  MStatus		initialize();

public:
    static MTypeId  id;

    static MObject xmlPath;
    static MObject geometry;
    static MObject topGroup;
    static MObject rigGroup;
    static MObject ctlGroup;
    static MObject noTransformGroup;
    static MObject skelLayer;
    static MObject ctlLayer;
    static MObject extrasLayer;

};

#endif