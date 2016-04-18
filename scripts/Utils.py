'''
Created on Oct 25, 2011

Description: Utility functions for creating nodes and other useful functions for rigging.

@author: Logan Kelly
'''

from pymel.all import *
import xml.etree.ElementTree as xml
import maya.mel as mel
from pymel.core.runtime import PointConstraint
from pymel.core.runtime import ConvertSelectionToVertices
import rig101wireControllers
from pymel.core.general import connectAttr
reload(rig101wireControllers)

def enum(**enums):
    return type('Enum', (), enums)

Colors = enum(BLACK='black',DARKGRAY='darkGray',LIGHTGRAY='lightGray',DARKRED='darkRed',
              DARKBLUE='darkBlue',LIGHTBLUE='lightBlue',DARKGREEN='darkGreen',DARKPURPLE='darkPurple',
              LIGHTPURPLE='lightPurple',LIGHTBROWN='lightBrown',DARKBROWN='darkBrown',RED='red',
              LIGHTRED='lightRed',LIGHTGREEN='lightGreen',BLUE='blue',WHITE='white',YELLOW='yellow',
              AQUABLUE='aquaBlue',AQUAGREEN='aquaGreen',PALERED='paleRed',PALERED2='paleRed2',PALEYELLOW='paleYellow',
              DARKAQUAGREEN='darkAquaGreen',PALEBROWN='paleBrown',MUSTARD='mustard',LEAFGREEN='leafGreen',
              DARKAQUAGREEN2='darkAquaGreen2',DARKAQUABLUE='darkAquaBlue',DARKPALEBLUE='darkPaleBlue',
              DARKPALEPURPLE='darkPalePurple',DARKDEEPPURPLE='darkDeepPurple')

#Generates an incremental list of node names in the form of prefix_NameNum_GEO
def genNameList(Name,Min,Max):
    list = [None,]
    for i in range(Min,(Max+1)):
        if i == Min:
            list[0] = self.prefix+Name+str(i)+'_GEO'
            continue
        list.append(self.prefix+Name+str(i)+'_GEO')
    return list

#Makes an empty home null group above the provided control
def makeHomeNull(control):
    select(cl=True)
    null = group(em=True, name=control.name()[:-3]+'GRP')
    parent(null, control)
    null.setTranslation((0.0,0.0,0.0))
    null.setRotation((0.0,0.0,0.0))
    null.setScale((1.0,1.0,1.0))
    parent(null,world=True)
    parent(control,null)
    null = Transform(null)
    return null

#Makes a display layer, if it doesn't already exist            
def makeDisplayLayer(layerName):        
    if objExists(layerName) == False:            
        return createDisplayLayer(n=layerName,number=1,empty=True)
    else:
        return PyNode(layerName)
    

#parent constrain a piece of geometry to a node
def constrainGeo(geoObject,parentObject):
    geoObject.tx.unlock()
    geoObject.ty.unlock()
    geoObject.tz.unlock()
    geoObject.rx.unlock()
    geoObject.ry.unlock()
    geoObject.rz.unlock()
    parentConstraint(parentObject,geoObject,mo=True,w=1)
    
#Create a blending utility node for two channel values    
def createBlendNode(inputOne,inputTwo,output):
    myNode = shadingNode('blendTwoAttr',asUtility=True)
    connectAttr(inputOne,myNode.name()+'.input[0]',f=True)
    connectAttr(inputTwo,myNode.name()+'.input[1]',f=True)
    disconnectAttr(inputOne, output)
    connectAttr(myNode+'.output',output,f=True)        
    return myNode

def createPlusMinusAvgNode(inputOne,inputTwo,output):
    myNode = shadingNode('plusMinusAverage',asUtility=True)
    connectAttr(inputOne,myNode.name()+'.input1D[0]',f=True)
    connectAttr(inputTwo,myNode.name()+'.input1D[1]',f=True)
    connectAttr(myNode+'.output1D',output,f=True)
    return myNode

#create a set driven key for two attributes and min and max values, reset driver to default value at end
def createSetDrivenKey(driven,driver,drivenMinVal,drivenMaxVal,driverMinVal,driverMaxVal,driverDefVal,drivenDefVal,inTangent,midTangent,outTangent, preInfinity, postInfinity):  
    setAttr(driver,driverMinVal)
    setAttr(driven,drivenMinVal)
    setDrivenKeyframe(driven,cd=driver,itt=inTangent,ott=outTangent)
    setAttr(driver,driverMaxVal)
    setAttr(driven,drivenMaxVal)
    setDrivenKeyframe(driven,cd=driver,itt=inTangent,ott=outTangent)
    setAttr(driver,driverDefVal)
    setAttr(driven,drivenDefVal)
    setDrivenKeyframe(driven,cd=driver,itt=midTangent,ott=midTangent)
    setDrivenNode = (PyNode(driver).connections())[-1]
    setDrivenNode.setPreInfinityType(preInfinity)
    setDrivenNode.setPostInfinityType(postInfinity)

#create a distanceBetween utility node with two transform node translations as inputs
def createDistanceBetween(nodeOne,nodeTwo):
    distNode = shadingNode('distanceBetween',asUtility=True)
    connectAttr(nodeOne+'.translate',distNode+'.point1',f=True)
    connectAttr(nodeTwo+'.translate',distNode+'.point2',f=True)
    return distNode

#connect the translation and rotation values of two nodes
def connectAttributes(nodeOne,nodeTwo,x,y,z,rotX,rotY,rotZ):
    if x:
        nodeOne.tx.connect(nodeTwo+'.translateX')
    if y:
        nodeOne.ty.connect(nodeTwo+'.translateY')
    if z:
        nodeOne.tz.connect(nodeTwo+'.translateZ')
    if rotX:
        nodeOne.rx.connect(nodeTwo+'.rotateX')
    if rotY:
        nodeOne.ry.connect(nodeTwo+'.rotateY')
    if rotZ:
        nodeOne.rz.connect(nodeTwo+'.rotateZ')

#create a condition utility node that performs an operation on two input values
#(only the first output value is used from the node instead of the whole tuple)
def createCondition(inputOne,inputOneType,inputTwo,inputTwoType,conditionType,outputTrue,outputTrueType,outputFalse,outputFalseType):
    conditionNode = shadingNode('condition',asUtility=True)
    if inputOneType == 'value':
        conditionNode.setAttr('firstTerm',inputOne)
    elif inputOneType == 'connection':
        connectAttr(inputOne,conditionNode+'.firstTerm')
    if inputTwoType == 'value':
        conditionNode.setAttr('secondTerm',inputTwo)
    elif inputTwoType == 'connection':
        connectAttr(inputTwo,conditionNode+'.secondTerm')
    if conditionType == '=':
        conditionNode.setAttr('operation', 0)
    elif conditionType == '!=':
        conditionNode.setAttr('operation', 1)
    elif conditionType == '>':
        conditionNode.setAttr('operation', 2)
    elif conditionType == '>=':
        conditionNode.setAttr('operation', 3)
    elif conditionType == '<':
        conditionNode.setAttr('operation', 4)
    elif conditionType == '<=':
        conditionNode.setAttr('operation', 5)
    if outputTrueType == 'value':
        conditionNode.setAttr('colorIfTrueR',outputTrue)
    elif outputTrueType == 'connection':
        connectAttr(outputTrue,conditionNode+'.colorIfTrueR')
    if outputFalseType == 'value':
        conditionNode.setAttr('colorIfFalseR',outputFalse)
    elif outputFalseType == 'connection':
        connectAttr(outputFalse,conditionNode+'.colorIfFalseR')
    return conditionNode

# create a multiplyDivide utility node that multiplies, divides, or raises one value to the power of the other
# for two input values and produces an output (uses only one value not the whole tuple)
def createMultiplyDivide(inputOne,inputOneType,inputTwo,inputTwoType,operationType):
    mathNode = shadingNode('multiplyDivide',asUtility=True)
    if inputOneType == 'value':
        mathNode.setAttr('input1X',inputOne)
    elif inputOneType == 'connection':
        connectAttr(inputOne,mathNode+'.input1X')
    if inputTwoType == 'value':
        mathNode.setAttr('input2X',inputTwo)
    elif inputTwoType == 'connection':
        connectAttr(inputTwo,mathNode+'.input2X')
    if operationType == '*':
        mathNode.setAttr('operation',1)
    elif operationType == '/':
        mathNode.setAttr('operation',2)
    elif operationType == '^':
        mathNode.setAttr('operation',3)
    return mathNode

#Lock and hide channels on a node
def lockHideAttrs(node,x,y,z,rotX,rotY,rotZ,scaleX,scaleY,scaleZ,vis):
    if x == True:
        node.translateX.lock()
        node.setAttr('translateX',keyable=False)
    if y == True:
        node.translateY.lock()
        node.setAttr('translateY',keyable=False)
    if z == True:
        node.translateZ.lock()
        node.setAttr('translateZ',keyable=False)
    if rotX == True:
        node.rotateX.lock()
        node.setAttr('rotateX',keyable=False)
    if rotY == True:
        node.rotateY.lock()
        node.setAttr('rotateY',keyable=False)
    if rotZ == True:
        node.rotateZ.lock()
        node.setAttr('rotateZ',keyable=False)
    if scaleX == True:
        node.scaleX.lock()
        node.setAttr('scaleX',keyable=False)
    if scaleY == True:
        node.scaleY.lock()
        node.setAttr('scaleY',keyable=False)
    if scaleZ == True:
        node.scaleZ.lock()
        node.setAttr('scaleZ',keyable=False)
    if vis == True:
        node.visibility.lock()
        node.setAttr('visibility',keyable=False)
    return
#Hide channels on a node
def hideAttrs(node,x,y,z,rotX,rotY,rotZ,scaleX,scaleY,scaleZ,vis):
    if x == True:
        node.setAttr('translateX',keyable=False)
    if y == True:
        node.setAttr('translateY',keyable=False)
    if z == True:
        node.setAttr('translateZ',keyable=False)
    if rotX == True:
        node.setAttr('rotateX',keyable=False)
    if rotY == True:
        node.setAttr('rotateY',keyable=False)
    if rotZ == True:
        node.setAttr('rotateZ',keyable=False)
    if scaleX == True:
        node.setAttr('scaleX',keyable=False)
    if scaleY == True:
        node.setAttr('scaleY',keyable=False)
    if scaleZ == True:
        node.setAttr('scaleZ',keyable=False)
    if vis == True:
        node.setAttr('visibility',keyable=False)
    return

#Write out geometry skin weights as an xml file
def writeGeoWeights(geo,skinCluster,scriptPath):
    vertEndIndex = geo.numVertices()-1
    rootElem = xml.Element('skinWeights')
    rootElem.set('skinCluster',skinCluster)
    rootElem.set('geometry',geo)
    num = 0
    for influence in skinCluster.getWeightedInfluence():
        elem = xml.Element('joint')
        elem.set('name',str(influence))
        rootElem.append(elem)
    for point in geo.vtx[0:vertEndIndex]:
        percentages = skinPercent( skinCluster, point, query=True, value=True )
        elem = xml.Element('point')
        elem.set('index',str(num))
        numTwo = 0
        for percentage in percentages:
            elem.set('wVal'+str(numTwo),str(percentage))
            numTwo += 1
        rootElem.append(elem)
        num += 1
    tree = xml.ElementTree(rootElem)
    file = open(scriptPath+'/'+geo.name()+'_Weights.xml','w')
    tree.write(file)
    file.close()

#set a display color for the given controller
def setControllerColor(controller,color):
    controllerShape = controller.getShape()
    controllerShape.setAttr('overrideEnabled', 1)
    if color == Colors.BLACK:
        controllerShape.setAttr('overrideColor',1)
    elif color == Colors.DARKGRAY:
        controllerShape.setAttr('overrideColor', 2 )
    elif color == Colors.LIGHTGRAY:
        controllerShape.setAttr('overrideColor', 3 )
    elif color == Colors.DARKRED:
        controllerShape.setAttr('overrideColor', 4 )
    elif color == Colors.DARKBLUE:
        controllerShape.setAttr('overrideColor', 5 )
    elif color == Colors.LIGHTBLUE:
        controllerShape.setAttr('overrideColor', 6 )
    elif color == Colors.DARKGREEN:
        controllerShape.setAttr('overrideColor', 7 )
    elif color == Colors.DARKPURPLE:
        controllerShape.setAttr('overrideColor', 8 )
    elif color == Colors.LIGHTPURPLE:
        controllerShape.setAttr('overrideColor', 9 )
    elif color == Colors.LIGHTBROWN:
        controllerShape.setAttr('overrideColor', 10 )
    elif color == Colors.DARKBROWN:
        controllerShape.setAttr('overrideColor', 11 )
    elif color == Colors.RED:
        controllerShape.setAttr('overrideColor', 12 )
    elif color == Colors.LIGHTRED:
        controllerShape.setAttr('overrideColor', 13 )
    elif color == Colors.LIGHTGREEN:
        controllerShape.setAttr('overrideColor', 14 )
    elif color == Colors.BLUE:
        controllerShape.setAttr('overrideColor', 15 )
    elif color == Colors.WHITE:
        controllerShape.setAttr('overrideColor', 16 )
    elif color == Colors.YELLOW:
        controllerShape.setAttr('overrideColor', 17 )
    elif color == Colors.AQUABLUE:
        controllerShape.setAttr('overrideColor', 18 )
    elif color == Colors.AQUAGREEN:
        controllerShape.setAttr('overrideColor', 19 )
    elif color == Colors.PALERED:
        controllerShape.setAttr('overrideColor', 20 )
    elif color == Colors.PALERED2:
        controllerShape.setAttr('overrideColor', 21 )
    elif color == Colors.PALEYELLOW:
        controllerShape.setAttr('overrideColor', 22 )
    elif color == Colors.DARKAQUAGREEN:
        controllerShape.setAttr('overrideColor', 23 )
    elif color == Colors.PALEBROWN:
        controllerShape.setAttr('overrideColor', 24 )
    elif color == Colors.MUSTARD:
        controllerShape.setAttr('overrideColor', 25 )
    elif color == Colors.LEAFGREEN:
        controllerShape.setAttr('overrideColor', 26 )
    elif color == Colors.DARKAQUAGREEN2:
        controllerShape.setAttr('overrideColor', 27 )
    elif color == Colors.DARKAQUABLUE:
        controllerShape.setAttr('overrideColor', 28 )
    elif color == Colors.DARKPALEBLUE:
        controllerShape.setAttr('overrideColor', 29 )
    elif color == Colors.DARKPALEPURPLE:
        controllerShape.setAttr('overrideColor', 30 )
    elif color == Colors.DARKDEEPPURPLE:
        controllerShape.setAttr('overrideColor', 31 )
def createRibbonSpine(prefix,numJoints,botCtlScale,midCtlScale,topCtlScale,fkCtlScale,ribbonRotY,ctlColor):
    rc101 = rig101wireControllers.rig101()
    bindJoints = []
    fkControls = []
    fkControlNulls = []
    controlJoints = []
    ribbonSpineGrp = group(n=prefix+'RibbonSpine_GRP')
    #create a nurbs plane with a number of divisions equal to the joints
    ribbonPlane = nurbsPlane(u=1,v=numJoints,d=3,w=1,lr=numJoints,name=prefix+'RibbonSplineIK_SRF')
    ribbonPlane[0].ry.set(ribbonRotY)
    ribbonPlane[0].setAttr('visibility',0)
    ribbonPlane[0].setParent(ribbonSpineGrp)
    rebuildSurface(dir=0,du=1,su=1)
    bakePartialHistory(ribbonPlane[0],prePostDeformers=True)
    makeIdentity(apply=True,r=True,s=True)
    mel.eval('createHair 1 '+str(numJoints)+' 10 0 0 0 0 5 0 2 1 1;')
    #remove unnecessary hair nodes
    delete('hairSystem1')
    delete('hairSystem1OutputCurves')
    follicleGrp = PyNode('hairSystem1Follicles')
    follicleGrp.setParent(ribbonSpineGrp)
    follicles = follicleGrp.getChildren()
    follicleGrp.rename(prefix+'RibbonSplineFollicles_GRP')
    #create the bind joints and parent them under the corresponding follicle node
    for i in range(numJoints):
        follicles[i].rename(prefix+'RibbonSplineFollicle'+str(i)+'_FLC')
        bindJoints.append( joint(a=True,p=follicles[i].getTranslation(space='world'),rad=0.5,n=prefix+'RibbonSpline_BIND_'+str(i)+'_JNT') )
        bindJoints[-1].setParent(follicles[i])
        fkControls.append(rc101.rig101WC2NailLong())
        fkControls[-1].rename(prefix+'RibbonSpine_FK'+str(i+1)+'_CTL')
        fkControls[-1].setTranslation(bindJoints[-1].getTranslation(space='world'))
        fkControls[-1].ry.set(ribbonRotY)
        fkControls[-1].setScale((fkCtlScale,fkCtlScale,fkCtlScale))
        setControllerColor(fkControls[-1], ctlColor)
        select(fkControls[-1],r=True)
        makeIdentity(apply=True,r=True,s=True)
        fkControlNulls.append(makeHomeNull(fkControls[-1]))
        fkControlNulls[-1].setParent(ribbonSpineGrp)
        #orientConstraint(fkControls[-1],bindJoints[-1],mo=True)
        #pointConstraint(fkControls[-1],bindJoints[-1],mo=True)
        parentConstraint(fkControls[-1],bindJoints[-1],mo=True)
        parentConstraint(follicles[i],bindJoints[-1],mo=True)
        parentConstraint(follicles[i],fkControlNulls[-1],mo=True)
        lockHideAttrs(fkControls[-1], False, False, False, False, False, False, True, True, True, True)
        select(cl=True)
    #create the control joints used by controllers to deform the ribbon spine
    controlJoints.append(joint(a=True,p=(0,-numJoints/2.0,0),rad=1.0,n=prefix+'RibbonSpline_bottomDriver0_JNT'))
    controlJoints[0].setAttr('visibility',0)
    controlJoints.append(joint(a=True,p=(0,-numJoints/2.0+numJoints/4.0,0),rad=1.0,n=prefix+'RibbonSpline_bottomDriver1_JNT'))
    controlJoints[1].setAttr('visibility',0)
    select(cl=True)
    controlJoints.append(joint(a=True,p=(0,0,0),rad=1.0,n=prefix+'RibbonSpline_middleDriver0_JNT'))
    controlJoints[2].setAttr('visibility',0)
    select(cl=True)
    controlJoints.append(joint(a=True,p=(0,numJoints/2.0,0),rad=1.0,n=prefix+'RibbonSpline_topDriver0_JNT'))
    controlJoints[3].setAttr('visibility',0)
    controlJoints.append(joint(a=True,p=(0,numJoints/2.0-numJoints/4.0,0),rad=1.0,n=prefix+'RibbonSpline_topDriver1_JNT'))
    controlJoints[4].setAttr('visibility',0)
    select(cl=True)
    #create bottom controller and locators
    bottomControl = circle(nr=(0,1,0),name=prefix+'RibbonSpine_Bottom_CTL')
    bottomControl[0].setTranslation(controlJoints[0].getTranslation(space='world'))
    bottomControl[0].setScale((botCtlScale,botCtlScale,botCtlScale))
    setControllerColor(bottomControl[0], ctlColor)
    makeIdentity(s=True,apply=True)
    lockHideAttrs(bottomControl[0], False, False, False, False, False, False, True, True, True, True)
    bottomControlNull = makeHomeNull(bottomControl[0])
    bottomControlNull.setParent(ribbonSpineGrp)
    bottomAim = spaceLocator(r=False,n=prefix+'RibbonSpine_bottomAim_LOC')
    bottomAim.setTranslation(bottomControl[0].getTranslation(space='world'))
    bottomAim.setParent(bottomControl[0])
    bottomAim.setAttr('visibility',0)
    bottomUp = spaceLocator(r=False,n=prefix+'RibbonSpine_bottomUp_LOC')
    bottomUp.setTranslation(bottomControl[0].getTranslation(space='world'))
    bottomUp.translateBy((1,0,0))
    bottomUp.setParent(bottomControl[0])
    bottomUp.setAttr('visibility',0)
    #create top controller and locators
    topControl = circle(nr=(0,1,0),name=prefix+'RibbonSpine_Top_CTL')
    topControl[0].setTranslation(controlJoints[3].getTranslation(space='world'))
    topControl[0].setScale((topCtlScale,topCtlScale,topCtlScale))
    setControllerColor(topControl[0], ctlColor)
    makeIdentity(s=True,apply=True)
    lockHideAttrs(topControl[0], False, False, False, False, False, False, True, True, True, True)
    topControlNull = makeHomeNull(topControl[0])
    topControlNull.setParent(ribbonSpineGrp)
    topAim = spaceLocator(r=False,n=prefix+'RibbonSpine_topAim_LOC')
    topAim.setTranslation(topControl[0].getTranslation(space='world'))
    topAim.setParent(topControl[0])
    topAim.setAttr('visibility',0)
    topUp = spaceLocator(r=False,n=prefix+'RibbonSpine_topUp_LOC')
    topUp.setTranslation(topControl[0].getTranslation(space='world'))
    topUp.translateBy((1,0,0))
    topUp.setParent(topControl[0])
    topUp.setAttr('visibility',0)
    #create middle controller and locators
    midControl = circle(nr=(0,1,0),name=prefix+'RibbonSpine_Mid_CTL')
    midControl[0].setTranslation(controlJoints[2].getTranslation(space='world'))
    midControl[0].setAttr('rotateY',keyable=False)
    midControl[0].setScale((midCtlScale,midCtlScale,midCtlScale))
    setControllerColor(midControl[0], ctlColor)
    makeIdentity(s=True,apply=True)
    lockHideAttrs(midControl[0], False, False, False, False, False, False, True, True, True, True)
    midControlNull = makeHomeNull(midControl[0])
    midControlNull.setParent(ribbonSpineGrp)
    #midAim = spaceLocator(r=False,n=prefix+'RibbonSpine_midAim_LOC')
    #midAim.setTranslation(midControl[0].getTranslation(space='world'))
    #midAim.setParent(midControl[0])
    #midAim.setAttr('visibility',0)
    #midUp = spaceLocator(r=False,n=prefix+'RibbonSpine_midUp_LOC')
    #midUp.setTranslation(midControl[0].getTranslation(space='world'))
    #midUp.translateBy((1,0,0))
    #midUp.setParent(midControl[0])
    #midUp.setAttr('visibility',0)
    #parent the control joints under their corresponding aim locators
    controlJoints[0].setParent(bottomAim)
    #controlJoints[2].setParent(midAim)
    controlJoints[2].setParent(midControl[0])
    controlJoints[3].setParent(topAim)
    #aim constrain the aim locators to the controller on the opposite side
    aimConstraint(topControl[0],bottomAim,aim=(0,1,0),upVector=(1,0,0),worldUpType='object',worldUpObject=bottomUp)
    aimConstraint(bottomControl[0],topAim,aim=(0,-1,0),upVector=(1,0,0),worldUpType='object',worldUpObject=topUp)
    #point constrain the bottom and top controllers to the middle control group
    pointConstraint(topControl[0],bottomControl[0],midControlNull,mo=True)
    #aimConstraint(topControl[0],midAim,aim=(0,1,0),upVector=(1,0,0),worldUpType='object',worldUpObject=midUp)
    #bind the ribbon plane to the control joints
    jointList = (controlJoints[0],controlJoints[2],controlJoints[3])
    ribbonSkin = skinCluster(jointList,ribbonPlane[0],mi=2,dr=4,tsb=True,rui=True,n=ribbonPlane[0]+'_skinCluster')
    #set skin percentages
    for i in range(numJoints+3):
        tVals = []
        for skinJoint in jointList:
            weight = None
            if numJoints == 5:
                if skinJoint == jointList[0]:
                    if i == 0:
                        weight = 1.0
                    elif i == 1:
                        weight = 0.9
                    elif i == 2:
                        weight = 0.6
                    elif i == 3:
                        weight = 0.2
                    else:
                        weight = 0.0
                elif skinJoint == jointList[1]:
                    if i == 1 or i == 6:
                        weight = 0.1
                    elif i == 2 or i == 5:
                        weight = 0.4
                    elif i == 3 or i == 4:
                        weight = 0.8
                    else:
                        weight = 0.0
                elif skinJoint == jointList[2]:
                    if i == 4:
                        weight = 0.2
                    elif i == 5:
                        weight = 0.6
                    elif i == 6:
                        weight = 0.9
                    elif i == 7:
                        weight = 1.0
                    else:
                        weight = 0.0
            if numJoints == 3:
                if skinJoint == jointList[0]:
                    if i == 0:
                        weight = 1.0
                    elif i == 1:
                        weight = 0.8
                    elif i == 2:
                        weight = 0.1
                    else:
                        weight = 0.0
                elif skinJoint == jointList[1]:
                    if i == 4 or i == 1:
                        weight = 0.2
                    elif i == 3 or i == 2:
                        weight = 0.9
                    else:
                        weight = 0.0
                elif skinJoint == jointList[2]:
                    if i == 3:
                        weight = 0.1
                    elif i == 4:
                        weight = 0.8
                    elif i == 5:
                        weight = 1.0
                    else:
                        weight = 0.0
            tVals.append((skinJoint,weight))
        skinPercent(ribbonSkin,ribbonPlane[0]+'.cv[0]['+str(i)+']',ribbonPlane[0]+'.cv[1]['+str(i)+']', transformValue=tVals)
    connectAttr(topControl[0]+'.rotate',controlJoints[3]+'.rotate',f=True)
    connectAttr(midControl[0]+'.rotate',controlJoints[2]+'.rotate',f=True)
    connectAttr(bottomControl[0]+'.rotate',controlJoints[0]+'.rotate',f=True)
    expression(s=midControl[0]+'.rotateY = ('+topControl[0]+'.rotateY + '+bottomControl[0]+'.rotateY) / 2',n=prefix+'RibbonSpineTwist_EXP')
    ikControls = [bottomControl[0],midControl[0],topControl[0]]
    return [bottomControlNull,topControlNull,ribbonSpineGrp,ikControls,midControlNull,fkControlNulls,follicleGrp,bindJoints]
#create a locator at the center of the selection
def makeSelectionLocator():
    x = 0
    y = 0
    z = 0
    ConvertSelectionToVertices()
    selectionLength = 0
    for selection in ls(sl=True):
        for vertex in selection:
            selectionLength += 1
            x += vertex.getPosition(space='world')[0]
            y += vertex.getPosition(space='world')[1]
            z += vertex.getPosition(space='world')[2]
    x = x / selectionLength
    y = y / selectionLength
    z = z / selectionLength
    selLocator = spaceLocator()
    selLocator.setTranslation((x,y,z))