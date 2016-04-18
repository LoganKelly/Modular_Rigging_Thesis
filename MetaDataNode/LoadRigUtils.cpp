/************************************************************
* Summary: Utility functions for performing common tasks    *
*          necessary for loading a rig in the scene.        *       
*  Author: Logan Kelly                                      *
*    Date: 10/15/12                                         *
************************************************************/

#include <maya/MFnMessageAttribute.h>
#include <maya/MDagModifier.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MEulerRotation.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MMatrix.h>
#include "LoadRigUtils.h"
#include "MyErrorChecking.h"
#include "MetaDataManagerNode.h"
#include "MDHipNode.h"
#include "MDSpineNode.h"

MStatus lrutils::loadGeoReference(MString geoFilePath, MString geoName, MString & name, MObject & geoObj) {
    MStatus status = MS::kFailure;

    MString projPath = MGlobal::executeCommandStringResult(MString("workspace -q -rd;"),false,false);
    MString relativePath = geoFilePath.substring(2,geoFilePath.numChars() - 1);

    //assemble the full file path of the geometry file
    MString fullGeoPath = projPath + relativePath;

    //load the geometry file as a reference into the current scene
    //check to see if the referenced file has already been used
    MStringArray refNodeList;
    status = MFileIO::getReferences(refNodeList, true);
    MyCheckStatus(status, "getReferences failed");
    int numReferences = 0;
    for(unsigned int i = 0; i < refNodeList.length(); i++) {
        MString tmp = refNodeList[i];
        string tmp1 = tmp.asChar();
        string tmp2 = fullGeoPath.asChar();
        if(std::string::npos != tmp1.find(tmp2))
            numReferences++;
    }
    string str (geoFilePath.asChar());
    string key ("/");
    size_t found = str.rfind(key);
    string fileName = str.substr(found+1,str.length()-found-4);
    string fileNamespace;
    if(numReferences > 0) {
        stringstream tmp;
        tmp << fileName << (numReferences+1);
        fileNamespace = tmp.str();
    } else { fileNamespace = fileName; }

    {
        stringstream tmp;
        tmp << "file -r -type \"mayaAscii\" -gl -loadReferenceDepth \"all\" -namespace \"" << fileNamespace.c_str() << "\" -options \"v=0\" \"" << fullGeoPath.asChar() << "\";";
        MString referenceCommand = MString(tmp.str().c_str());
        MGlobal::executeCommand(referenceCommand);
    }
    
    //get the referenced geometry transform node and add the metaParent
    //attribute to it
    MSelectionList selection;
    if(numReferences > 0) {
        name += (boost::lexical_cast<string>(numReferences+1)).c_str();
    }
    stringstream geoRefName;
    geoRefName << fileNamespace << ":" << geoName;
    MString mGeoRefName = MString(geoRefName.str().c_str());
    status = selection.add( mGeoRefName, true );
    MyCheckStatusReturn(status, "add geoRefName "+mGeoRefName+" to selection failed.");

    if(selection.length() )
        selection.getDependNode(0, geoObj);

    MFnTransform transformFn;
    transformFn.setObject(geoObj);
    MFnMessageAttribute mAttr;
    MObject transformAttr = mAttr.create("metaParent", "metaParent");
    transformFn.addAttribute(transformAttr);

    if( !geoObj.isNull() )
        status = MS::kSuccess;

    return status;
}

MStatus lrutils::getObjFromName(MString name, MObject & obj) {
    MStatus status;

    MSelectionList selection;
    status = selection.add( name, true );
    MyCheckStatusReturn(status, "add node \""+name+"\" to selection failed.");

    if(selection.length() ) {
        selection.getDependNode(0, obj);
        status = MS::kSuccess;
    }

    return status;
}

MStatus lrutils::setLocation(MObject obj, MVectorArray location, MFnTransform& transformFn, bool translate, bool rotation, bool scale) {
    MStatus status = MS::kFailure;

    status = transformFn.setObject(obj);
    MyCheckStatusReturn(status, "invalid MObject provided for MFnTransform.setObject()");

    if( status == MS::kSuccess ) {
        if(translate) {
            MVector vTranslation = location[0] ;
            //stringstream text; text << "(" << vTranslation.x << ", " << vTranslation.y << ", " << vTranslation.z << ")";
            //MGlobal::displayInfo( text.str().c_str() );
            status = transformFn.setTranslation(vTranslation, MSpace::kTransform);
            stringstream text; text << "MFnTransform.setTranslation() failed, status code [" << status.errorString().asChar() << "]";
            MyCheckStatusReturn(status, text.str().c_str() ); 
            vTranslation = transformFn.getTranslation(MSpace::kWorld);
            //text.clear(); text << "(" << vTranslation.x << ", " << vTranslation.y << ", " << vTranslation.z << ")";
            //MGlobal::displayInfo( text.str().c_str() );
        }
        if(rotation) {
            MVector vRotation = location[1]*3.141592/180.0;
            MEulerRotation eRotation = MEulerRotation(vRotation);
            status = transformFn.setRotation(eRotation);
        }
        if(scale) {
            MVector vScale = location[2];
            double* scale = new double[3];
            vScale.get(scale);
            transformFn.setScale(scale);
            //make the scale of the controller the identity
            MGlobal::executeCommand("select -r "+transformFn.name()+";");
            MGlobal::executeCommand("makeIdentity -s true -apply true;");
        }
    }

    return status;
}

MStatus lrutils::setParentConstraintOffset(MObject constraintObj, MTransformationMatrix transform) {
    MStatus status = MS::kFailure;

    MFnTransform constraintFn;
    status = constraintFn.setObject( constraintObj );
    MyCheckStatusReturn(status, "invalid MObject provided for MFnTransform.setObject()");
    MString constraintName = constraintFn.name();

    if ( status = MS::kSuccess ) {
        MVector vTranslation = transform.getTranslation(MSpace::kTransform);
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetTranslateX\" "+vTranslation.x+";");
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetTranslateY\" "+vTranslation.y+";");
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetTranslateZ\" "+vTranslation.z+";");
        double* rotation = new double[3];
        MTransformationMatrix::RotationOrder rotOrder = MTransformationMatrix::kXYZ;
        transform.getRotation(rotation,rotOrder);
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetRotateX\" "+rotation[0]+";");
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetRotateY\" "+rotation[1]+";");
        MGlobal::executeCommand( "setAttr \""+constraintName+".target[0].targetOffsetRotateZ\" "+rotation[2]+";");
    }

    return status;
}

MStatus lrutils::makeHomeNull(MObject obj, MFnTransform& transformFn, MObject& groupObj) {
    MStatus status = MS::kFailure;
    
    status = transformFn.setObject(obj);
    MyCheckStatusReturn(status, "invalid MObject provided for MFnTransform.setObject()");

    if( status == MS::kSuccess ) {
        MDagModifier dagMod;
        groupObj = dagMod.createNode( "transform", MObject::kNullObj, &status );
        MyCheckStatusReturn(status, "MDagModifier.createNode() failed");
        dagMod.doIt();

        MFnTransform groupFn;
        groupFn.setObject(groupObj);
        groupFn.set(transformFn.transformation());
        groupFn.addChild(obj);
        
        MString groupName = transformFn.name();
        groupName = groupName.substring(0, groupName.numChars() - 4);
        groupName += "GRP";
        groupFn.setName(groupName);
        
    }

    return status;
}

MStatus lrutils::makeGroup(MObject & obj, MString name) {
    MStatus status = MS::kFailure;

    MDagModifier dagMod;
    MObject groupObj = dagMod.createNode( "transform", MObject::kNullObj, &status );
    MyCheckStatusReturn(status, "MDagModifier.createNode() failed");
    dagMod.doIt();

    MFnTransform groupFn;
    groupFn.setObject(groupObj);
    MString groupName = name;
    groupName += "_GRP";
    groupFn.setName(groupName);

    obj = groupObj;

    return status;
}

MStatus lrutils::getMetaRootByName(MObject & obj, MString name) {
    MStatus status = MS::kFailure;

    for( MItDependencyNodes nodeIt(MFn::kPluginDependNode);
        !nodeIt.isDone(); nodeIt.next() ) {
        MFnDependencyNode nodeFn( nodeIt.item() );
        if(nodeFn.typeId() == MetaDataManagerNode::id) {
            //Get the plug for the list of metaRoots
            MPlug metaRootsPlug = nodeFn.findPlug(MString("metaRoots"),true,&status);
            MyCheckStatusReturn(status,"findPlug failed");
            //MGlobal::displayInfo( metaRootsPlug.name() );
            
            //follow its connection to all the plugs on connected metaRoots
            MPlugArray rootPlugs;
            metaRootsPlug.connectedTo(rootPlugs,false,true,&status);
            MyCheckStatusReturn(status,"MPlug.connectedTo failed");
            unsigned int numRootPlugs = rootPlugs.length();

            for(unsigned int i = 0; i < numRootPlugs; i++) {
                MObject rootNodeObj = rootPlugs[i].node(&status);
                MyCheckStatusReturn(status,"MPlug.node() failed");
                MFnDependencyNode rootNodeFn( rootNodeObj );

                //get the name of the metaRoot node and compare it to the search string
                MString rootName = rootNodeFn.name();
                rootName = rootName.substring(4,rootName.numChars());
                if( rootName == name ) {
                    obj = rootNodeObj;
                    status = MS::kSuccess;
                    break;
                }
            }
        }
    }

    if(status == MS::kFailure) {
        MGlobal::displayInfo("MetaDataManager node does not exist.\n");        
    }

    return status;
}

MStatus lrutils::getMetaNodeConnection(MObject metaNodeObj, MObject & connectedObj, MString name) {
    MStatus status = MS::kFailure;
    MFnDependencyNode metaNodeFn( metaNodeObj );
    MPlug metaNodePlug = metaNodeFn.findPlug( name, true, &status );
    if( status != MS::kSuccess ) {
        return status;
    }
    //MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() '"+name+"' failed");

    if (metaNodePlug.isSource()) {
        //follow the plug connection to the connected plug on the other object
        MPlugArray connectedPlugs;
        metaNodePlug.connectedTo(connectedPlugs,false,true,&status);
        MyCheckStatusReturn(status,"MPlug.connectedTo() failed");
        MPlug connectedPlug = connectedPlugs[0];

        //get the connected object
        connectedObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");
        status = MS::kSuccess;
    }
    if (metaNodePlug.isDestination()) {
        MPlugArray connectedPlugs;
        metaNodePlug.connectedTo(connectedPlugs,true,false,&status);
        MyCheckStatusReturn(status,"MPlug.connectedTo() failed");
        MPlug connectedPlug = connectedPlugs[0];

        connectedObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");
        status = MS::kSuccess;
    }

    return status;
}

MStatus lrutils::stringReplaceAll(MString &source, MString search, MString replace) {
    MStatus status = MS::kFailure;

    //part of the string that has already been searched
    MString sourceProcessed;
    //part of the string that has not been searched yet
    MString sourceUnprocessed = source;
    
    int index = 0;
    int index2 = 0;
    //if the search and replace strings are the same, don't bother to change the string
    if( search == replace )
        return MS::kSuccess;
    for(;;) {
        index = sourceUnprocessed.indexW( search );
        if (index == -1 || sourceUnprocessed.numChars() == 0) {
            //if the search string is not found, end and add the rest of the string to
            //the part already processed
            sourceProcessed += sourceUnprocessed;
            break;
        }
        MString subString1 = source.substringW(0,index-1);
        index2 = index + search.numChars();
        MString subString2;
        if (index2 < (int)(source.numChars() - 1) ) {
            subString2 = source.substringW(index2,source.numChars()-1);
        }
        sourceProcessed += subString1 + replace;
        sourceUnprocessed = subString2;
        status = MS::kSuccess;
    }
    source = sourceProcessed;

    return status;
}

MStatus lrutils::getMetaChildByName(MObject metaNodeObj, MString name, MObject& metaChildObj) {
    MStatus status = MS::kFailure;

    MFnDependencyNode metaNodeFn( metaNodeObj );
    MPlug metaChildrenPlug = metaNodeFn.findPlug( "metaChildren", true, &status );
    MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

    //follow the plug connection to the connected plug on the other object
    MPlugArray connectedChildPlugs;
    metaChildrenPlug.connectedTo(connectedChildPlugs,false,true,&status);
    MyCheckStatusReturn(status,"MPlug.connectedTo() failed");

    for (unsigned int i = 0; i < connectedChildPlugs.length(); i++) {
        MPlug connectedPlug = connectedChildPlugs[i];
        MObject connectedNodeObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");

        MFnDependencyNode connectedNodeFn( connectedNodeObj );
        MString connectedNodeName = connectedNodeFn.name();
        int index = connectedNodeName.indexW( name );
        if( index != -1 ) {
            metaChildObj = connectedNodeObj;
            status = MS::kSuccess;
            break;
        }
    }

    return status;
}

MStatus lrutils::getMetaChildByRigId(MObject metaNodeObj, MString rigId, MObject& metaChildObj) {
    MStatus status = MS::kFailure;

    MFnDependencyNode metaNodeFn( metaNodeObj );
    MPlug metaChildrenPlug = metaNodeFn.findPlug( "metaChildren", true, &status );
    MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

    //follow the plug connection to the connected plug on the other object
    MPlugArray connectedChildPlugs;
    metaChildrenPlug.connectedTo(connectedChildPlugs,false,true,&status);
    MyCheckStatusReturn(status,"MPlug.connectedTo() failed");

    for (unsigned int i = 0; i < connectedChildPlugs.length(); i++) {
        MPlug connectedPlug = connectedChildPlugs[i];
        MObject connectedNodeObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");

        MFnDependencyNode connectedNodeFn( connectedNodeObj );

        //get the rigId number held in the rigId attribute
        MPlug rigIdPlug = connectedNodeFn.findPlug(MString("rigId"),true,&status);
        MyCheckStatusReturn(status,"findPlug failed");
        MString childRigId;
        rigIdPlug.getValue(childRigId);
        //if rigId is in childRigId then return the object
        if( childRigId.indexW(rigId) != -1 ) {
            metaChildObj = connectedNodeObj;
            status = MS::kSuccess;
            break;
        } 
    }

    return status;
}

MStatus lrutils::getNumMetaChildren(MObject metaNodeObj, int & num) {
    MStatus status = MS::kFailure;    

    if( !metaNodeObj.isNull() ) {
        MFnDependencyNode metaNodeFn( metaNodeObj );
        MPlug metaChildrenPlug = metaNodeFn.findPlug( "metaChildren", true, &status );
        MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

        //follow the plug connection to the connected plug on the other object
        MPlugArray connectedChildPlugs;
        metaChildrenPlug.connectedTo(connectedChildPlugs,false,true,&status);
        MyCheckStatusReturn(status,"MPlug.connectedTo() failed");

        num = connectedChildPlugs.length();
    }

    return status;
}

MStatus lrutils::getMetaParent(MObject metaNodeObj, MObject & metaParentNodeObj) {
    MStatus status = MS::kFailure;

    MFnDependencyNode metaNodeFn( metaNodeObj );
    MPlug metaParentPlug = metaNodeFn.findPlug( "metaParent", true, &status );
    MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

    //follow the plug connection to the connected metaChildren plug on the parent object
    MPlugArray connectedPlugs;
    metaParentPlug.connectedTo(connectedPlugs,true,false,&status);
    MyCheckStatusReturn(status,"MPlug.connectedTo() failed");
    metaParentNodeObj = connectedPlugs[0].node(&status);
    MyCheckStatusReturn(status, "MPlug.node() failed");

    return status;
}


// In order for the new animation curve positions to match the controller's previous global position,
// first we need to get the controller's global transform matrix. The new local position of the controller
// will be the controller's global transform matrix multiplied by the inverse of the new parent group's
// global transform matrix.
// So if:
//  [A] = controller's global transformation matrix
//  [B] = parent group's global transformation matrix
//  [X] = controller's new local transformation matrix
// Then:
//  [X] = [A]inverse([B])
//
// The controller's new local transformation matrix needs to be calculated per keyframe, which is why
// a map of world position matrices for the controller are passed in as a parameter. The key for the map
// is the time at which the associated world matrix was stored.
MStatus lrutils::updateAnimCurves(MObject transformObj, std::map<double, MMatrix> ctlWorldMatrices, MMatrix ctlGroupMatrix) {
    MStatus status = MS::kFailure;
    MFnTransform transformFn( transformObj, &status );
    MyCheckStatusReturn(status, status.errorString() );

    //get all of the connections from the transform node
    MPlugArray transformConnections;
    status = transformFn.getConnections( transformConnections );
    MyCheckStatusReturn(status, status.errorString() );
    for(unsigned int i = 0; i < transformConnections.length(); i++) {
        //get all of the plugs this plug is connected to as a destination
        MPlugArray connectedPlugs;
        transformConnections[i].connectedTo(connectedPlugs,true,false,&status);
        MyCheckStatusReturn(status, status.errorString() );
        MString plugName = transformConnections[i].partialName(false,false,false,false,false,true);
        for(unsigned int j = 0; j < connectedPlugs.length(); j++) {
            MObject connectedObj = connectedPlugs[j].node();
            MFn::Type animType = connectedObj.apiType();
            //if the connected object is an animCurveT[L,A], then transform it
            if( (animType == MFn::kAnimCurveTimeToDistance) || (animType == MFn::kAnimCurveTimeToAngular) ) {
                MFnAnimCurve animCurve(connectedObj, &status);
                MyCheckStatusReturn(status, status.errorString() );
                //iterate through every key in the curve
                for(unsigned int k = 0; k < animCurve.numKeys(); k++) {
                    double keyTime = animCurve.time(k, &status).value();
                    MyCheckStatusReturn( status, status.errorString() );
                    double keyVal = animCurve.value(k, &status);
                    MyCheckStatusReturn( status, status.errorString() );

                    float tangentX, tangentY;
                    status = animCurve.getTangent(k, tangentX, tangentY, false);
                    MyCheckStatusReturn(status, status.errorString() );
                    //stringstream tmp;
                    //tmp << "out tangent = (" << tangentX << "," << tangentY << ")";
                    //MGlobal::displayInfo(tmp.str().c_str());

                    //calculate the controller's local transformation matrix
                    //first retrieve the corresponding world matrix for this key time
                    MMatrix ctlWorldMatrix = ctlWorldMatrices[keyTime];
                        //if the transformation matrices are the same, don't update the curve
                    //if( ctlWorldMatrix == ctlGroupMatrix) {
                    //    continue;
                    //}
                    MTransformationMatrix ctlGroupTransMatrix(ctlGroupMatrix);
                    MMatrix ctlGroupMatrixInverse = ctlGroupTransMatrix.asMatrixInverse();
                    MMatrix newCtlLocalMatrix = ctlWorldMatrix * ctlGroupMatrixInverse;
                    MTransformationMatrix newCtlLocalTransMatrix(newCtlLocalMatrix);

                    //determine if the curve is for translation data
                    if(animType == MFn::kAnimCurveTimeToDistance) {
                        MVector vTransform = newCtlLocalTransMatrix.translation(MSpace::kTransform);
                        if( plugName.indexW( MString("X") ) != -1 ) {
                            keyVal = vTransform.x;
                        } else if ( plugName.indexW( MString("Y") ) != -1 ) {
                            keyVal = vTransform.y;
                        } else if ( plugName.indexW( MString("Z") ) != -1 ) {
                            keyVal = vTransform.z;
                        }
                    //if the curve is for rotation data
                    } else if (animType == MFn::kAnimCurveTimeToAngular) {
                        double* rotation = new double[3];
                        MTransformationMatrix::RotationOrder rotOrder = MTransformationMatrix::kXYZ;
                        newCtlLocalTransMatrix.getRotation(rotation,rotOrder);
                        if( plugName.indexW( MString("X") ) != -1 ) {
                            keyVal = rotation[0];
                        } else if ( plugName.indexW( MString("Y") ) != -1 ) {
                            keyVal = rotation[1];
                        } else if ( plugName.indexW( MString("Z") ) != -1 ) {
                            keyVal = rotation[2];
                        }                        
                    }
                    animCurve.setValue(k, keyVal);
                    status = MS::kSuccess;
                }
            }
        }
    }
    return status;
}

std::vector<MObject> lrutils::buildSkeletonFromGuide(std::vector<MVectorArray> locations, MString prefix, MPlug metaDataPlug, MObject metaParentJoint, MString layerName) {
    MDGModifier dgMod;
    MStatus status;
    std::vector<MObject> joints;
    MFnMessageAttribute mAttr;

    MObject parentJoint;
    unsigned int jointNum = 0;

    for (std::vector<MVectorArray>::iterator it = locations.begin(); it != locations.end(); ++it) {
        MVectorArray location = *it;
        MObject joint = createJointFromLocation(location, prefix, jointNum, parentJoint);
        joints.push_back(joint);

        MFnTransform jointFn( joint );
        //add metaParent attributes to joint
        if(!metaDataPlug.isNull()) {
            MObject jointAttr = mAttr.create("metaParent", "metaParent");
            jointFn.addAttribute(jointAttr);
            //connect the metaparent attribute to the MDSpine node
            status = dgMod.connect( metaDataPlug, jointFn.findPlug("metaParent") );
            MyCheckStatus(status, "connect failed");
            dgMod.doIt();
        }

        //if layer name is provided, add the joint to that display layer
        if(layerName != "") {
            MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ layerName+" "+ jointFn.name()+";");
        }

        parentJoint = joint;
        jointNum++;
    }
    //orient the joint chain
    MFnTransform topJointFn(joints.at(0));
    MGlobal::executeCommand("joint -e -zso -oj \"xyz\" -sao \"yup\" -ch " + topJointFn.fullPathName() + ";");

    //if meta parent joint is not null, parent first joint to it
    if(!metaParentJoint.isNull()) {
        MFnTransform firstJointFn(joints.at(0));

        MFnTransform metaParentJointFn(metaParentJoint);
        MGlobal::executeCommand("parent " + firstJointFn.fullPathName() + " " + metaParentJointFn.fullPathName() + ";");
        //orient the parent joint
        MGlobal::executeCommand("joint -e -zso -oj \"xyz\" -sao \"yup\" -ch " + metaParentJointFn.fullPathName() + ";");
    }

    return joints;
}

void lrutils::buildFKControls(std::vector<MObject> vFKCtls, std::vector<MObject> vFKCtlGroups, std::vector<MVectorArray> locations, std::vector<MObject> joints, MString prefix, MString icon, MString color, MObject metaDataNode, MObject parentController, MString layerName, bool createLastControl) {
    for(unsigned int i = 0; i < locations.size(); i++) {
        if(!createLastControl) {
            if(i == locations.size() - 1) {
                return;
            }
        }
        MVectorArray location = locations.at(i);
        MObject joint = joints.at(i);
        MObject fkCtlObj;
        MObject fkCtlGroupObj;
        createFKCtlFromLocation(location, joint, prefix, i, icon, color, parentController, fkCtlObj, fkCtlGroupObj, layerName, metaDataNode);
        parentController = fkCtlObj;
        vFKCtls.push_back(fkCtlObj);
        vFKCtlGroups.push_back(fkCtlGroupObj);
    }
}

void lrutils::createFKCtlFromLocation(MVectorArray location, MObject joint, MString prefix, unsigned int num, MString icon, MString color, MObject parent, MObject& fkCtlObj, MObject& fkCtlGroupObj, MString layerName, MObject metaDataNode) {
    MStatus status;
    MDGModifier dgMod;
    //used for holding results from executed commands
    MStringArray result;
    MFnDependencyNode depMetaDataNodeFn(metaDataNode);
    //used for creating metaParent attributes for objects
    MFnMessageAttribute mAttr;
    MFnTransform jointFn(joint);
    
    //create the control object and set its color
    status = MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + icon + "')\");" );
    status = MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + color + "')\");" );
    status = MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), result );
    //get the MObject for the controller
    status = lrutils::getObjFromName(result[0], fkCtlObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    MFnTransform fkCtlFn( fkCtlObj );
    lrutils::setLocation(fkCtlObj, location, MFnTransform::MFnTransform(), false, false, true);
    //set controller name
    MString fkCtlName = prefix + "_FK_"+num+"_CTL";
    dgMod.renameNode(fkCtlObj, fkCtlName);
    //add the metaParent attribute to the controller
    MObject controlAttr = mAttr.create("metaParent", "metaParent");
    fkCtlFn.addAttribute(controlAttr);
    //connect the controller's metaParent to the MDSpine node
    status = dgMod.connect( depMetaDataNodeFn.findPlug("FKControllers"), fkCtlFn.findPlug("metaParent") );
    dgMod.doIt();
    
    //create the fk control null
    lrutils::makeHomeNull(fkCtlObj, MFnTransform(), fkCtlGroupObj);
    lrutils::setLocation(fkCtlGroupObj, location, MFnTransform::MFnTransform(), true, true, false);
    MFnTransform fkCtlGroupFn( fkCtlGroupObj );
    if(!parent.isNull()) {
        MFnTransform parentFn(parent);
        MGlobal::executeCommand("parent " + fkCtlGroupFn.fullPathName() + " " + parentFn.fullPathName() + ";");
    }
    //add the metaParent attribute to the controller group
    fkCtlGroupFn.addAttribute(mAttr.create("metaParent", "metaParent"));
    //connect the controller group's metaParent to the MDGlobal node
    status = dgMod.connect( depMetaDataNodeFn.findPlug("FKControllerGroups"), fkCtlGroupFn.findPlug("metaParent") );
    MyCheckStatus(status, "connect failed"); 
    dgMod.doIt();

    MGlobal::executeCommand("parentConstraint -mo " + fkCtlFn.fullPathName() + " " + jointFn.fullPathName() + ";",result);
    MObject jointParentConstraintObj;
    status = lrutils::getObjFromName(result[0], jointParentConstraintObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    MFnTransform jointParentConstraintFn(jointParentConstraintObj);
    jointParentConstraintFn.addAttribute(mAttr.create("metaParent", "metaParent"));
    status = dgMod.connect( depMetaDataNodeFn.findPlug("FKJointParentConstraints"), jointParentConstraintFn.findPlug("metaParent"));
    dgMod.doIt();

    //set the display layers for the controller and controller group
    MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ layerName+" "+ fkCtlFn.fullPathName()+";");
    MGlobal::executeCommand("editDisplayLayerMembers -noRecurse "+ layerName+" "+ fkCtlGroupFn.fullPathName()+";");
    MGlobal::executeCommand("select -cl;");
}

MObject lrutils::createJointFromLocation(MVectorArray location, MString prefix, unsigned int num, MObject parent) {
    MStatus status = MS::kFailure;
    MObject jointObj;

    //make joint object
    MDagModifier dagMod;
    jointObj = dagMod.createNode( "joint", MObject::kNullObj, &status );
    MyCheckStatus(status, "MDagModifier.createNode() failed");
    dagMod.doIt();
    //set position
    lrutils::setLocation(jointObj, location, MFnTransform::MFnTransform(), true, false, false);
    //set name        
    MString jointName = prefix + "_Skel" + boost::lexical_cast<string>(num).c_str() + "_JNT";
    dagMod.renameNode(jointObj, jointName);
    dagMod.doIt();
    MFnTransform jointFn(jointObj);
    
    //parent the joint to its parent, if not null
    if(parent != MObject::kNullObj) {
        MFnTransform parentFn(parent);
        MGlobal::executeCommand("parent " + jointFn.fullPathName() + " " + parentFn.fullPathName() + ";");
    }

    return jointObj;
}

MStatus lrutils::getAllWorldTransforms(MObject ctlObj, std::map<double, MMatrix>& ctlWorldMatrices) {
    MStatus status = MS::kFailure;
    
    MFnTransform ctlFn( ctlObj );
    MGlobal::select(ctlObj, MGlobal::kReplaceList);
    double nextKeyTime = 0;
    double prevKeyTime = -1;
    double currentTime;
    MGlobal::executeCommand("currentTime -q;",currentTime);
    MGlobal::viewFrame(1);
    while(nextKeyTime > prevKeyTime) {
        prevKeyTime = nextKeyTime;
        MGlobal::executeCommand("findKeyframe -timeSlider -which next;",nextKeyTime);
        MGlobal::viewFrame(nextKeyTime);
        MDagPath path;
        status = ctlFn.getPath(path);
        MyCheckStatus(status, "MFnDagNode.getPath() failed");
        MMatrix ctlWorldMatrix = path.inclusiveMatrix(&status);
        MyCheckStatus(status, "MDagPath.inclusiveMatrix() failed");
        ctlWorldMatrices.insert(std::pair<double, MMatrix>(nextKeyTime,ctlWorldMatrix));
    }
    MGlobal::viewFrame(currentTime);

    status = MS::kSuccess;
    return status;
}

MStatus lrutils::getJointByNum(MObject metaDataNode, unsigned int jointNum, MObject& jointObj) {
    MStatus status;

    if(!metaDataNode.isNull()) {
        MFnDependencyNode metaDataFn(metaDataNode);
        if(metaDataFn.typeId() == MDHipNode::id) {
            status = lrutils::getMetaNodeConnection(metaDataNode, jointObj, "hipJoint");
            MyCheckStatusReturn(status, "lrutils::getMetaNodeConnection() failed");
        } else if (metaDataFn.typeId() == MDSpineNode::id) {
            MPlug bindJointsPlug = metaDataFn.findPlug( "BindJoints", true, &status );
            MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

            //follow the plug connection to the connected plug on the other object
            MPlugArray connectedJointPlugs;
            bindJointsPlug.connectedTo(connectedJointPlugs,false,true,&status);
            MyCheckStatusReturn(status,"MPlug.connectedTo() failed");
            if( connectedJointPlugs.length() > 0) {
                //for some reason when more than one plug is connected in an MPlugArray,
                //the first object connected is in index 1 and the second object connected is
                //in index 2. The following code simply reverses the joint number we are
                //looking for since this is the case.
                if( connectedJointPlugs.length() > 1) {
                    if(jointNum == 1) {
                        jointNum = 0;
                    } else if (jointNum == 0) {
                        jointNum = 1;
                    }
                }
                MPlug jointPlug = connectedJointPlugs[jointNum];
                //MPlug jointPlug = bindJointsPlug.connectionByPhysicalIndex(0);
                jointObj = jointPlug.node(&status);
                MyCheckStatusReturn(status, "MPlug.node() failed");
            }
        }
    }

    return status;
}

MStatus lrutils::getFKControlByNum(MObject metaDataNode, unsigned int fkNum, MObject& fkCtlObj) {
    MStatus status;

    if(!metaDataNode.isNull()) {
        MFnDependencyNode metaDataFn(metaDataNode);
        MPlug fkControlsPlug = metaDataFn.findPlug( "FKControllers", true, &status );
        MyCheckStatusReturn(status, "MFnDependencyNode.findPlug() failed");

        //follow the plug connection to the connected plug on the other object
        MPlugArray connectedControlPlugs;
        fkControlsPlug.connectedTo(connectedControlPlugs,false,true,&status);
        MyCheckStatusReturn(status,"MPlug.connectedTo() failed");
        if( connectedControlPlugs.length() > 0) {
            //for some reason when more than one plug is connected in an MPlugArray,
            //the first object connected is in index 1 and the second object connected is
            //in index 2. The following code simply reverses the number we are
            //looking for since this is the case.
            if( connectedControlPlugs.length() > 1) {
                if(fkNum == 1) {
                    fkNum = 0;
                } else if (fkNum == 0) {
                    fkNum = 1;
                }
            }
            MPlug controlPlug = connectedControlPlugs[fkNum];
            //MPlug jointPlug = fkControlsPlug.connectionByPhysicalIndex(0);
            fkCtlObj = controlPlug.node(&status);
            MyCheckStatusReturn(status, "MPlug.node() failed");
        }
    }

    return status;
}

MStatus lrutils::deleteMetaDataPlugConnections(MPlug metaDataPlug) {
    MStatus status;
    //follow the plug connection to the connected plug on the other object
    MPlugArray connectedPlugs;
    metaDataPlug.connectedTo(connectedPlugs,false,true,&status);
    MyCheckStatusReturn(status,"MPlug.connectedTo() failed");

    for (unsigned int i = 0; i < connectedPlugs.length(); i++) {
        MPlug connectedPlug = connectedPlugs[i];
        MObject connectedNodeObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");
        MGlobal::deleteNode(connectedNodeObj);
    }
    return MS::kSuccess;
}

MStatus lrutils::updateMetaDataObjectNames(MPlug metaDataPlug, MString oldRigName, MString rigName, MString oldCompName, MString compName) {
    MStatus status;
    //follow the plug connection to the connected plug on the other object
    MPlugArray connectedPlugs;
    metaDataPlug.connectedTo(connectedPlugs,false,true,&status);
    MyCheckStatusReturn(status,"MPlug.connectedTo() failed");

    for (unsigned int i = 0; i < connectedPlugs.length(); i++) {
        MPlug connectedPlug = connectedPlugs[i];
        MObject connectedNodeObj = connectedPlug.node(&status);
        MyCheckStatusReturn(status, "MPlug.node() failed");
        MFnDependencyNode connectedNodeFn( connectedNodeObj );
        MString connectedNodeName = connectedNodeFn.name();
        //set rig namespace
        lrutils::stringReplaceAll(connectedNodeName, oldRigName, rigName);
        //set component namespace
        lrutils::stringReplaceAll(connectedNodeName, oldCompName, compName);
        connectedNodeFn.setName(connectedNodeName);
    }

    return MS::kSuccess;
}

MStatus lrutils::updateControllerShapeColor(MObject oldControllerObj, MString shape, MString color, MVectorArray ctlLocation) {
    MStatus status;
    MFnTransform oldControllerFn(oldControllerObj);
    
    MGlobal::executeCommand( "python(\"control = rig101().rig101WCGetByName('" + shape + "')\");" );
    MGlobal::executeCommand( "python(\"Utils.setControllerColor(control, '" + color + "')\");" );
    MString sResult;
    MGlobal::executeCommand( MString("python(\"control.fullPath()\");"), sResult ); 
    MObject ctlObj;
    status = lrutils::getObjFromName(sResult, ctlObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    //apply the scale of the controller location to the new shape
    MFnTransform ctlFn( ctlObj );
    lrutils::setLocation(ctlObj, ctlLocation, MFnTransform::MFnTransform(), false, false, true);

    //get the shape node of the original controller object
    MStringArray sResults;
    MGlobal::executeCommand( "listRelatives -s -fullPath "+oldControllerFn.name()+";", sResults );
    MString oldCtlShapePath = sResults[0];
    MGlobal::executeCommand( "listRelatives -s -path "+oldControllerFn.name()+";", sResults );
    MString oldCtlShapeName = sResults[0];
    MObject oldCtlShapeObj; 
    status = lrutils::getObjFromName(oldCtlShapePath, oldCtlShapeObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    //delete the old shape node
    MGlobal::deleteNode( oldCtlShapeObj );
    //get the new shape node
    MGlobal::executeCommand( "listRelatives -s -fullPath "+ctlFn.name()+";", sResults );
    MString ctlShapePath = sResults[0];
    MObject ctlShapeObj; 
    status = lrutils::getObjFromName(ctlShapePath, ctlShapeObj);
    MyCheckStatus(status, "lrutils::getObjFromName() failed");
    //instance the new shape node under the old controller node
    MString command = "parent -s -add " + ctlShapePath + " " + oldControllerFn.name() + ";";
    MGlobal::executeCommand( command );
    MFnDependencyNode ctlShapeFn( ctlShapeObj );
    ctlShapeFn.setName( oldCtlShapeName );
    //delete the new controller transform
    MGlobal::deleteNode( ctlObj );

    return MS::kSuccess;
}

MStatus lrutils::updateAnimationKeys(MObject ctlObj, MVectorArray ctlLocation) {
    MStatus status;
    MFnTransform ctlFn(ctlObj);
    //get the controller's group object
    //save the original controller parent path
    MString ctlGroupParentPath;
    MStringArray result;
    MGlobal::executeCommand("listRelatives -p -f "+ctlFn.fullPathName()+";",result);
    ctlGroupParentPath = result[0];
    MObject ctlGroupObj;
    lrutils::getObjFromName(ctlGroupParentPath, ctlGroupObj);
    MFnTransform ctlGroupFn(ctlGroupObj);

    MyCheckStatus(status, "getMetaNodeConnection() failed");
    std::map<double, MMatrix> ctlWorldMatrices;
    status = lrutils::getAllWorldTransforms(ctlObj, ctlWorldMatrices);
    MyCheckStatus(status, "lrutils::getAllWorldTransforms() failed");

    //set the old controller group translation to the new location
    lrutils::updateControlGroupLocation(ctlObj, ctlLocation);

    //find the global transformation matrix of the controller group
    MDagPath groupPath;
    status = ctlGroupFn.getPath(groupPath);
    MyCheckStatus(status, "MFnDagNode.getPath() failed");
    MMatrix ctlGroupWorldMatrix = groupPath.inclusiveMatrix(&status);
    MyCheckStatus(status, "MDagPath.inclusiveMatrix() failed");
    
    //update the animation curves attached to the old controller
    lrutils::updateAnimCurves(ctlObj, ctlWorldMatrices, ctlGroupWorldMatrix);

    return MS::kSuccess;
}

MStatus lrutils::updateControlGroupLocation(MObject ctlObj, MVectorArray ctlLocation) {
    MFnTransform ctlFn(ctlObj);

    MString ctlGroupPath;
    MStringArray result;
    MGlobal::executeCommand("listRelatives -p -f "+ctlFn.fullPathName()+";",result);
    ctlGroupPath = result[0];
    MObject ctlGroupObj;
    lrutils::getObjFromName(ctlGroupPath, ctlGroupObj);
    MFnTransform ctlGroupFn(ctlGroupObj);
    MString ctlGroupParentPath;
    MGlobal::executeCommand("listRelatives -p -f "+ctlGroupFn.fullPathName()+";",result);
    ctlGroupParentPath = result[0];
    //move the controller group to world for absolute positioning
    MGlobal::executeCommand("parent -world " + ctlGroupFn.fullPathName() + ";");            
    lrutils::setLocation(ctlGroupObj, ctlLocation, ctlGroupFn, true, true, false);
    MGlobal::executeCommand("parent " + ctlGroupFn.fullPathName() + " " + ctlGroupParentPath + ";");
    
    return MS::kSuccess;
}