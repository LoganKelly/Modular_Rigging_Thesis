/************************************************************
* Summary: Utility functions for performing common tasks    *
*          necessary for loading a rig in the scene.        *       
*  Author: Logan Kelly                                      *
*    Date: 10/15/12                                         *
************************************************************/

#ifndef _LoadRigUtils
#define _LoadRigUtils

#include <maya/MGlobal.h>
#include <maya/MFileIO.h>
#include <maya/MFnTransform.h>
#include <maya/MVectorArray.h>
#include <maya/MPlug.h>
#include <maya/MString.h>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>
#include <maya/MDagPath.h>
#include "ComponentGuide.h"

typedef boost::shared_ptr<ComponentGuide> ComponentGuidePtr;

using namespace std;

namespace lrutils {
    //loads a file reference into the scene that contains geometry to be rigged
    // geoFilePath = project relative path to location of reference file
    // geoName = name of the geometry node to rig in the reference file
    // name = name of this particular rig instance loaded from xml rig definition
    // geoObj = MObject instance of geometry transform node in scene after load is completed
    MStatus loadGeoReference(MString geoFilePath, MString geoName, MString & name, MObject & geoObj);
    MStatus getObjFromName(MString name, MObject & obj);
    //set an objects translation and scale based upon location information
    MStatus setLocation(MObject obj, MVectorArray location, MFnTransform& transformFn = MFnTransform::MFnTransform(), bool translate = true, bool rotation = true, bool scale = true);
    //sets a parent constraint's target offsets using a transformation matrix
    MStatus setParentConstraintOffset(MObject constraintObj, MTransformationMatrix transform);
    //creates a group node with the same transformation as the given MObject and parents that MObject to the group
    MStatus makeHomeNull(MObject obj, MFnTransform& transformFn = MFnTransform::MFnTransform(), MObject & groupObj = MObject());
    //creates and returns an empty group node
    MStatus makeGroup(MObject & obj, MString name);
    //find a metaRoot node by name, if it exists
    MStatus getMetaRootByName(MObject & obj, MString name);
    //find an object connected to a metadata node by an attribute name
    MStatus getMetaNodeConnection(MObject metaNodeObj, MObject & connectedObj, MString name);
    //get a metadata node's parent
    MStatus getMetaParent(MObject metaNodeObj, MObject & metaParentNodeObj);
    //get the number of metachildren from this metanode
    MStatus getNumMetaChildren(MObject metaNodeObj, int & num);
    //search for and replace every instance of a search string with a replacement string in a given source string
    MStatus stringReplaceAll(MString &source, MString search, MString replace);
    //find and return an MObject for a meta child node of a given meta data node by name
    MStatus getMetaChildByName(MObject metaNodeObj, MString name, MObject& metaChildObj);
    //find and return an MObject for a meta child node of a given meta data node by rigId
    MStatus getMetaChildByRigId(MObject metaNodeObj, MString rigId, MObject& metaChildObj);
    //update an object's attached animation curves by transforming the key data from one transform to another
    MStatus updateAnimCurves(MObject transformObj, std::map<double, MMatrix> ctlWorldMatrices, MMatrix ctlGroupMatrix);
    //retrieve a map of all of the world transformation matrices of a controller object for all of its keyframes
    MStatus getAllWorldTransforms(MObject ctlObj, std::map<double, MMatrix>& ctlWorldMatrices);
    //creates a skeletal joint chain from a given list of locations, and adds the prefix string to the joint names
    std::vector<MObject> buildSkeletonFromGuide(std::vector<MVectorArray> locations, MString prefix, MPlug metaDataPlug = MPlug::MPlug(), MObject metaParentJoint = MObject(), MString layerName = "");
    //creates FK controllers for a joint chain
    void buildFKControls(std::vector<MObject> vFKCtls, std::vector<MObject> vFKCtlGroups, std::vector<MVectorArray> locations, std::vector<MObject> joints, MString prefix, MString icon, MString color, MObject metaDataNode, MObject parentController = MObject(), MString layerName = "", bool createLastControl = false);
    //create a single joint from a location, with the given prefix, number, and sets the parent
    MObject createJointFromLocation(MVectorArray location, MString prefix, unsigned int num, MObject parent);
    //create a single FK controller with the given prefix, number, icon, color, and sets the parent.
    //Returns the MObject for the controller group
    void createFKCtlFromLocation(MVectorArray location, MObject joint, MString prefix, unsigned int num, MString icon, MString color, MObject parent, MObject& fkCtlObj, MObject& fkCtlGroupObj, MString layerName, MObject metaDataNode);
    //find a bind joint from a meta data node based upon its joint index number
    MStatus getJointByNum(MObject metaDataNode, unsigned int jointNum, MObject& jointObj);
    //find an FK controller from a meta data node based upon its index number
    MStatus getFKControlByNum(MObject metaDataNode, unsigned int fkNum, MObject& fkCtlObj);
    //delete all objects connected to a particular meta data node plug
    MStatus deleteMetaDataPlugConnections(MPlug metaDataPlug);
    //update the names of all objects connected to the given meta data node plug
    //searches and replaces the names of the objects for the rigName and compName
    MStatus updateMetaDataObjectNames(MPlug metaDataPlug, MString oldRigName, MString rigName, MString oldCompName, MString compName);
    //update a controller with a new controller shape and color
    MStatus updateControllerShapeColor(MObject oldControllerObj, MString shape, MString color, MVectorArray ctlLocation);
    //update all animation keys of a given controller object to preserve world space position
    MStatus updateAnimationKeys(MObject ctlObj, MVectorArray ctlLocation);
    //update the location of a controller's group, using world space locations
    MStatus updateControlGroupLocation(MObject ctlObj, MVectorArray ctlLocation);
}

#endif //_loadRigUtils