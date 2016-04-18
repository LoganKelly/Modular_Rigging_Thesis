#SceneView inherits from QGraphicsScene, which is a Qt widget
#useful for displaying repositionable graphics items in a 2D
#layout
#The SceneView is used for displaying and editing the node
#tree for the currently active modular rig
# Author: Logan Kelly
#   Date: 2/24/13

from PyQt4 import QtGui, QtCore, QtSvg
import cPickle, weakref
import Nodes.RigGuiNode as rgn
reload(rgn)
import Nodes.GeometryGuiNode as geometrygn
reload(geometrygn)
import Nodes.GlobalGuiNode as globalgn
reload(globalgn)
import Nodes.HipGuiNode as hipgn
reload(hipgn)
import Nodes.BaseNode as bn
reload(bn)
import Nodes.NodeTypes as nt
reload(nt)
import Nodes.NodeList as nl
reload(nl)
from Nodes.ConnectRigNode import *
import Nodes
reload(Nodes)
import Nodes.LineClass as lc
reload(lc)
import maya.mel as mel
import uuid
import re

class SceneView(QtGui.QGraphicsScene):
    
    categoryItemClicked = QtCore.pyqtSignal(QtCore.QObject)
    rigChangedSignal = QtCore.pyqtSignal()
    
    def __init__(self, parent=None):
        super(SceneView, self).__init__(parent)
        
        self.sceneNodes = weakref.WeakValueDictionary() # Dict of all SVG nodes in the scene
        self.line = None
        self.previousMousePosition = None
        nt.createNodeTypes()

    def createNode(self, nodeToCreate, pos):
        name = nodeToCreate.displayText
        #nodeToCreate = nt.NodeTypes["rigNode"]
        if nodeToCreate.nodeType == 'rig':
            node = rgn.RigGuiNode(nodeToCreate)
        elif nodeToCreate.nodeType == 'geometry':
            node = geometrygn.GeometryGuiNode(nodeToCreate)
        elif nodeToCreate.nodeType == 'globalComponent':
            node = globalgn.GlobalGuiNode(nodeToCreate)
        elif nodeToCreate.nodeType == 'hipComponent':
            node = hipgn.HipGuiNode(nodeToCreate)
        node.setPos(pos)
        
        node.widgetMenu = nodeToCreate.widgetMenu
        node.displayText.setPlainText(name)
        
        # Here I am emitting a signal with another signal. The purpose is to get the category node into to my MainWindow "space" (So I can use the variable to change the QScrollArea)
        node.clickedSignal.connect(self.categoryItemClicked.emit)
        node.widgetMenuChanged.connect(self.rigChangedSignal.emit)
        node.nodeCreatedInScene.emit()
        
        self.sceneNodes[name] = node
        self.addItem(node)
        
        return node

    def dropEvent(self, event):
        if event.mimeData().hasFormat("application/x-imgname"):
            event.accept()
            data = event.mimeData().data("application/x-imgname")
            data = data.data()
            unPickleData = cPickle.loads(data)
            unPickleData = nl.NodeList.d[unPickleData]
            # If the node if a rig node, and a rig node already exists in the scene,
            # do not create a new rig node and display an error message
            rigExists = None
            for node in self.sceneNodes:
                if self.sceneNodes[node].nodeType == 'rig':
                    rigExists = True
            if rigExists and unPickleData.nodeType == 'rig':
                #cmds.error("Rig Node Editor: Two rig nodes can\'t exist in the node editor.")
                try:
                    mel.eval('error \"Rig Node Editor: Two rig nodes can\'t exist in the node editor.\";')
                except:
                    return
            # Create the node in the scene
            newNode = self.createNode(unPickleData, event.scenePos())
            #assign a rigId to the new node
            if newNode.nodeType == 'globalComponent' or newNode.nodeType == 'hipComponent':
                newNode.rigId = re.sub("-","",uuid.uuid4().__str__())
            if newNode.nodeType == 'rig' or newNode.nodeType == 'globalComponent' or newNode.nodeType == 'hipComponent':
                newNode.updateVersion = 1.0
            
    def dragMoveEvent(self, event):
        if event.mimeData().hasFormat("application/x-imgname"):
            event.accept()
            
    def mousePressEvent(self, event):
        item = self.itemAt(event.scenePos())
 
        if event.button() == QtCore.Qt.LeftButton and (isinstance(item, ConnectRigNode)):
            self.line = QtGui.QGraphicsLineItem(QtCore.QLineF(event.scenePos(), event.scenePos()))
            self.addItem(self.line)
            
 
        #if item is None:
        #    cmds.select(clear=True)
 
        super(SceneView, self).mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self.line:
            newLine = QtCore.QLineF(self.line.line().p1(), event.scenePos())
            self.line.setLine(newLine)
 
        super(SceneView, self).mouseMoveEvent(event)
        self.update()

    def mouseReleaseEvent(self, event):
 
        if self.line:
            startItems = self.items(self.line.line().p1())
            if len(startItems) and startItems[0] == self.line:
                startItems.pop(0)
            endItems = self.items(self.line.line().p2())
            if len(endItems) and endItems[0] == self.line:
                endItems.pop(0)
 
            self.removeItem(self.line)
            
            # If this is true a successful line was created
            if self.connectionTest(startItems, endItems):
                if len(startItems[0].connectedLine) is not 0:
                    #remove the start item node from the original end item's list of child nodes
                    originalEndNode = startItems[0].connectedLine[0].myEndItem.parentItem()
                    originalEndNode.childNodes[:] = [n for n in originalEndNode.childNodes if n is not startItems[0].parentItem()]
                    startItems[0].connectedLine[0].deleteLine()
                endItems[0].parentItem().childNodes.append(startItems[0].parentItem())
                # Creates a line that is basically of 0 length, just to put a line into the scene
                connectionLine = lc.LineClass(startItems[0], endItems[0], QtCore.QLineF(startItems[0].scenePos(), endItems[0].scenePos()))
                self.addItem(connectionLine)
                # Now use that previous line created and update its position, giving it the proper length and etc...
                connectionLine.updatePosition()
                # Sending the data downstream. The start item is the upstream node ALWAYS. The end item is the downstream node ALWAYS.
                #connectionLine.getEndItem().getWidgetMenu().receiveFrom(connectionLine.getStartItem(), delete=False)
                #connectionLine.getStartItem().getWidgetMenu().sendData(connectionLine.getStartItem().getWidgetMenu().packageData())
                # Emitting the "justConnected" signal (That is on all connection points)
                connectionLine.myEndItem.lineConnected.emit()
                connectionLine.myStartItem.lineConnected.emit()
                newNode = startItems[0].parentItem()
                if newNode.nodeType == 'rig' or newNode.nodeType == 'globalComponent' or newNode.nodeType == 'hipComponent':
                    newNode.incrementVersion()
                self.rigChangedSignal.emit()
            
        self.line = None

        super(SceneView, self).mouseReleaseEvent(event)
        
    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            for item in self.selectedItems():
                if isinstance(item, QtSvg.QGraphicsSvgItem):
                    item.deleteNode()
                    self.rigChangedSignal.emit()
                else:
                    try:
                        endNode = item.myEndItem.parentItem()
                        endNode.childNodes[:] = [n for n in endNode.childNodes if n is not item.myStartItem.parentItem()]
                        item.myStartItem.parentItem().metaNodeName = None
                        item.deleteLine()
                        self.rigChangedSignal.emit()
                    except AttributeError:
                        pass

        else:
            super(SceneView, self).keyPressEvent(event)
        
    def connectionTest(self, startItems, endItems):
        '''
        This is the big if statement that is checking
        to make sure that whatever nodes the user is trying to
        make a connection between is allowable.
        '''
        if startItems[0].isInputConnection:
            temp = startItems[0]
            startItems[0] = endItems[0]
            endItems[0] = temp

        try:
            if len(startItems) is not 0 and len(endItems) is not 0:
                if startItems[0] is not endItems[0]:
                    if (startItems[0].isInputConnection and not endItems[0].isInputConnection) or\
                       (endItems[0].isInputConnection and not startItems[0].isInputConnection):
                        if (startItems[0].parentItem().listWidgetName is endItems[0].parentItem().listWidgetName) or\
                            (startItems[0].connectionName is endItems[0].connectionName):
                            try:
                                if (startItems[0].parentItem().nodeType == 'geometry' and endItems[0].parentItem().nodeType == 'globalComponent'):
                                    mel.eval('error \"Rig Node Editor: A geometry node can\'t be connected to a global component node.\";')
                            except:
                                return False
                            try:
                                if (startItems[0].parentItem().nodeType == 'geometry' and endItems[0].parentItem().nodeType == 'hipComponent'):
                                    mel.eval('error \"Rig Node Editor: A geometry node can\'t be connected to a hip component node.\";')
                            except:
                                return False
                            return True
        except AttributeError:
            pass
        return False
    