# HipGuiNode extends the BaseNode class
# in order to provide functionality for drawing
# the interconnected nodes in the QGraphicsScene.
# Author: Logan Kelly
#   Date: 4/1/13

from PyQt4 import QtGui, QtCore, QtSvg
from BaseNode import *
from ConnectRigNode import *
import maya.mel as mel
import maya.cmds as cmds
import xml.etree.ElementTree as xml

class HipGuiNode(BaseNode,QtSvg.QGraphicsSvgItem):
    
    clickedSignal = QtCore.pyqtSignal(QtCore.QObject)
    nodeCreatedInScene = QtCore.pyqtSignal()
    widgetMenuChanged = QtCore.pyqtSignal()
    
    def __init__(self, *args, **kwargs):
        
        args = list(args)
        otherNode = None
        # Check to see if the first argument is of type BaseNode
        #if args and isinstance(args[0], BaseNode):
        if args:
            otherNode = args.pop(0)
        
        # Separately init the inherited class
        BaseNode.__init__(self)
        
        #print "isinstance(args[0], BaseNode) = "+ str(isinstance(args[0], BaseNode))
        #print "type(args[0]) == BaseNode = " + str(type(args[0]) == BaseNode)
        
        # Set all necessary attributes that come from the base class if they're present
        self.displayText = otherNode.displayText if otherNode else ""
        self.dictKey = otherNode.dictKey if otherNode else ""
        self.imagePath = otherNode.imagePath if otherNode else ""
        self.description = otherNode.description if otherNode else ""
        self.listWidgetName = otherNode.listWidgetName if otherNode else None
        self.nodeType = otherNode.nodeType if otherNode else ""
        self.nodeColor = otherNode.nodeColor if otherNode else ""
        self.rigId = None
        self.updateVersion = None
        
        # If the imagePath exists, insert it into args for QGraphicsSvgItem.__init__(QString filename)
        # This is where the base image (cat_node_base.svg or attr_base.svg) is actually being used
        if self.imagePath:
            args.insert(0, self.imagePath)
        
        QtSvg.QGraphicsSvgItem.__init__(self, self.imagePath, **kwargs)
        
        # Set the attributes and variables specific to the Category Node Item
        self.setFlags(QtGui.QGraphicsItem.ItemIsSelectable|QtGui.QGraphicsItem.ItemIsMovable|QtGui.QGraphicsItem.ItemIsFocusable)
        self.setCachingEnabled(False) # This added to get correct drawing of selection box around icon in viewport
        self.setAcceptHoverEvents(True)
        self.connectionPoints = [] # This is the amount of connection points that are on the node. Making it so you can have an arbitrary amount. Just have to make sure to append the connections to this list

        self.addNodeComponents()
        self.addText()
        self.nodeCreatedInScene.connect(self.doInitialMenuWidgetSetup)
        
    def mousePressEvent(self, event):
        super(HipGuiNode, self).mousePressEvent(event)
        self.clickedSignal.emit(self)
        
    def paint(self, painter, option, widget):

        selected = False

        # Bitwise operations. These 2 if statements are checking to see if the current state of the item is selected or has focus.
        # If it is either, it sets them to false. Then, we set OUR selected flag to True so we can make our own selection graphic.
        # In essence, this "catches" the selected state and turns it off so the default BAD dotted line doesn't appear around the icon.
        if option.state & QtGui.QStyle.State_HasFocus:
            option.state ^= QtGui.QStyle.State_HasFocus
            selected = True

        if option.state & QtGui.QStyle.State_Selected:
            option.state ^= QtGui.QStyle.State_Selected
            selected = True

        super(HipGuiNode, self).paint(painter, option, widget)

        # Since we turned off the default selection state, below we will make our own "selection" graphic.
        if selected:
            # Do special painting for selected state
            self.setElementId("hover")

        else:
            self.setElementId("regular")
            
    def addText(self):
        font = QtGui.QFont("SansSerif", 14)
        font.setStyleHint(QtGui.QFont.Helvetica)
        font.setStretch(100)
        self.displayText = QtGui.QGraphicsTextItem(self.displayText, self)
        self.displayText.setFont(font)
        self.displayText.setDefaultTextColor(QtGui.QColor(QtCore.Qt.white))
        self.displayText.setPos(self.boundingRect().center().x() - (self.displayText.boundingRect().width()/2) + 5, self.sceneBoundingRect().center().y()/2 + 5)
        
    def addNodeComponents(self):
        self.addConnections()
        
    def addConnections(self):
        self.outputConnection = ConnectRigNode(":/componentOutputConnection.svg", self)
        self.outputConnection.setPos(self.width()/2 - self.outputConnection.width()/2 + 6, self.height()-6.0)
        self.outputConnection.nodeType = self.nodeType
        self.outputConnection.isOutputConnection = True
        self.outputConnection.connectionName = "output"
        self.connectionPoints.append(self.outputConnection)
        
        self.inputConnection = ConnectRigNode(":/componentInputConnection.svg", self)
        self.inputConnection.setPos(self.width()/2 - self.outputConnection.width()/2 + 6, -self.inputConnection.height()+6.0)
        self.inputConnection.nodeType = self.nodeType
        self.inputConnection.isInputConnection = True
        self.inputConnection.connectionName = "input"
        self.connectionPoints.append(self.inputConnection)
        
    def width(self):
        return self.sceneBoundingRect().width()
    
    def height(self):
        return self.sceneBoundingRect().height()
    
    def doInitialMenuWidgetSetup(self):
        '''
        This function is to do whatever the node needs to do when the node is JUST CREATED
        '''
        self.getWidgetMenu().setupWidget()
        self.getWidgetMenu().widgetsUpdated.connect(self.incrementVersion)
        self.getWidgetMenu().widgetsUpdated.connect(self.widgetMenuChanged.emit)
        
    def deleteNode(self):
        for connectionPoint in self.connectionPoints:
            if connectionPoint.connectedLine:
                #if not connectionPoint.isInputConnection:
                    # This for loop is going through all of the connected lines in the OUTPUT connection. Also doing this in "deleteLine" in graphicsModule
                    #for connectedLine in connectionPoint.connectedLine:
                    #    connectedLine.getEndItem().getWidgetMenu().receiveFrom(connectedLine.getStartItem(), delete=True)
                    #    connectedLine.getStartItem().getWidgetMenu().sendData(connectedLine.getStartItem().getWidgetMenu().packageData())
                connectionPoint.clearLine()
                del connectionPoint.connectedLine[:]
        self.scene().removeItem(self)
        
    def setupWidgetFromElem(self,elem):
        self.xmlElem = elem
        self.getVersionFromElem(elem)
        #get attributes from xml element
        version = elem.get('version')
        name = elem.get('name')
        icon = elem.get('icon')
        color = elem.get('color')
        locElem = elem.find("location")
        self.previousScale = [float(locElem.get('scaleX')),float(locElem.get('scaleY')),float(locElem.get('scaleZ'))]
        #set the values of the widget fields
        widgetMenu = self.getWidgetMenu()
        widgetMenu.version.setText(version)
        widgetMenu.name.setText(name)
        colorIndex = widgetMenu.colorComboBox.findText(color,QtCore.Qt.MatchExactly)
        widgetMenu.colorComboBox.setCurrentIndex(colorIndex)
        iconIndex = widgetMenu.iconComboBox.findText(icon,QtCore.Qt.MatchExactly)
        widgetMenu.iconComboBox.setCurrentIndex(iconIndex)
        
    def getXMLStart(self):
        return "Hip XML Started, metaNodeName = "+self.metaNodeName+", rigId = "+self.rigId
    
    def getXMLEnd(self):
        return "Hip XML Ended"
    
    def getXMLElem(self,forSave):
        widgetMenu = self.getWidgetMenu()
        attributes = {}
        attributes['name'] = widgetMenu.name.text()
        attributes['type'] = "hip"
        attributes['color'] = widgetMenu.colorComboBox.currentText().__str__()
        attributes['icon'] = widgetMenu.iconComboBox.currentText().__str__()
        if forSave:
            attributes['version'] = str(self.getSaveVersion())
        else:
            attributes['version'] = str(self.updateVersion)
        attributes['rigId'] = str(self.rigId)
        guiPosition = self.pos()
        attributes['guiLocation'] = str(guiPosition.x())+","+str(guiPosition.y())
        updateElem = xml.Element('component',attributes)
        self.getControllerXform()
        locAttributes = {}
        locAttributes['localX'] = str(self.translation[0])
        locAttributes['localY'] = str(self.translation[1])
        locAttributes['localZ'] = str(self.translation[2])
        locAttributes['rotateX'] = str(self.rotation[0])
        locAttributes['rotateY'] = str(self.rotation[1])
        locAttributes['rotateZ'] = str(self.rotation[2])
        locAttributes['scaleX'] = str(self.scale[0])
        locAttributes['scaleY'] = str(self.scale[1])
        locAttributes['scaleZ'] = str(self.scale[2])
        locationElem = xml.Element('location',locAttributes)
        updateElem.append(locationElem)
        return updateElem
    
    def setupScriptJobs(self):
        if self.metaNodeName is not None and self.metaNodeName != "":
            self.controllerJobNums = []
            controllerName = mel.eval('getMetaNodeConnection -n "'+self.metaNodeName+'" -c "controller";')
            self.controllerJobNums.append( cmds.scriptJob( ro=True, ac=[controllerName+'.translate', self.controllerTranslateUpdate] ) )
            self.controllerJobNums.append( cmds.scriptJob( ro=True, ac=[controllerName+'.rotate', self.controllerRotateUpdate] ) )
            self.controllerJobNums.append( cmds.scriptJob( ro=True, ac=[controllerName+'.scale', self.controllerScaleUpdate] ) )
            return self.controllerJobNums