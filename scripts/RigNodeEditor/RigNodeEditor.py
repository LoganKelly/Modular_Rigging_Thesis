#Rig Node editor for authoring rig XML definition files
#Uses PyQT to create the window using a .ui file

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4 import QtGui
from PyQt4 import uic
#from hornePyQT import *
import maya.cmds as cmds
import pymel.all as pm
import maya.mel as mel
import SceneView as sv
reload(sv)

import maya.OpenMayaUI as apiUI
import sip

import Nodes.NodeList as nl
reload(nl)
import Nodes.NodeTypes as nt
reload(nt)
import Nodes.LineClass as lc
reload(lc)

from designer.NodeWindowBasic import Ui_MainWindow
from designer import icons

import os
import shutil
import re
import xml.etree.ElementTree as xml
import Nodes.globals as globals

#If you put the .ui file for this example elsewhere, just change this path.
#RigNodeEditor_form, RigNodeEditor_base = uic.loadUiType('E:\\Thesis_SVN\\scripts\\RigNodeEditor\\designer\\NodeWindowBasic.ui')
#class RigNodeEditor(RigNodeEditor_form, RigNodeEditor_base):
class RigNodeEditor(QMainWindow, Ui_MainWindow):
    
    previousMousePosition = None
    rootNodeName = None
    
    def __init__(self, parent=None):
        self.scriptJobNumbers = []
        if parent is None:
            parent = self.getMayaWindow()
        super(RigNodeEditor, self).__init__(parent)
        self.setupUi(self)
        #Add the file save menu
        self.saveAction = QtGui.QAction(self.tr("S&ave"), self)
        self.saveAction.setShortcut(self.tr("Ctrl+S"))
        self.saveAction.setStatusTip(self.tr("Save the rig definition to an XML file"))
        self.connect(self.saveAction, SIGNAL("triggered()"),self.SaveXMLFile)
        self.menuFile.addAction(self.saveAction)
        
        #instantiate all of the types of nodes to
        #use in the NodeLists
        nt.createNodeTypes()
        
        #set up the scene view
        reload(sv)
        self.scene = sv.SceneView()
        self.nodeDropGraphicsView.setScene(self.scene)
        self.nodeDropGraphicsView.setSceneRect(0, 0, 6300, 5550)
        self.nodeDropGraphicsView.setBackgroundBrush(QBrush(QColor(60, 60, 60, 255), Qt.SolidPattern))
        self.nodeDropGraphicsView.setTransformationAnchor(0)
        
        # events for scene view
        #self.nodeDropGraphicsView.mouseMoveEvent = self.graphicsView_mouseMoveEvent
        #self.nodeDropGraphicsView.mouseReleaseEvent = self.graphicsView_mouseReleaseEvent
        self.nodeDropGraphicsView.wheelEvent = self.graphicsView_wheelEvent
        self.nodeDropGraphicsView.resizeEvent = self.graphicsView_resizeEvent
        
        # Tabs
        self.addTabs()
        
        # Connections
        self.makeConnections()
        
        #add the path for the xml file used for rig updating
        self.updateXmlPath = mel.eval("getenv TEMP;") + "/rigTemp.xml"
        
    def getMayaWindow(self):
        """
        Get the main Maya window as a QtGui.QMainWindow instance
        @return: QtGui.QMainWindow instance of the top level Maya windows
        """
        ptr = apiUI.MQtUtil.mainWindow()
        if ptr is not None:
            return sip.wrapinstance(long(ptr), QObject)
        
    def addTabs(self):
        self.nodesWindow.removeTab(0)
        
        self.rigNodeList = nl.NodeList()
        self.rigNodeList.listName = "rigNodeList"
        listIcon = QIcon(":/rigNodeList.svg")
        self.nodesWindow.insertTab(0, self.rigNodeList, listIcon, "Rig Nodes")
        
        self.rigNodeList.populateListWidget(nt.NodeTypes)
        
    def makeConnections(self):
        self.rigNodeList.itemClicked.connect(self.changeDescription)
        self.scene.categoryItemClicked.connect(self.setWidgetMenu)
        #self.scene.categoryItemClicked.connect(self.setWidgetMenu)
        self.scene.rigChangedSignal.connect(self.rigUpdate)
        
    def changeDescription(self, listItem):
        self.descriptionLabel.setText(listItem.description)
        
    def setWidgetMenu(self, item):
        self.nodeMenuArea.takeWidget()
        self.nodeMenuArea.setWidget(item.getWidgetMenu())
        self.nodeOptionsWindow.setTitle(item.displayText.toPlainText())
        #self.nodeOptionsWindow.setTitle(item.displayText)
        
    def graphicsView_wheelEvent(self, event):
        factor = 1.41 ** ((event.delta()*.5) / 240.0)
        self.nodeDropGraphicsView.scale(factor, factor)
        
    def graphicsView_resizeEvent(self, event):
        self.scene.setSceneRect(0, 0, self.nodeDropGraphicsView.width(), self.nodeDropGraphicsView.height())
        
    def graphicsView_mouseMoveEvent(self, event):
        if event.buttons() == Qt.MiddleButton:
            if self.previousMousePosition is None:
                self.previousMousePosition = event.globalPos()
            delta = (event.globalPos() - self.previousMousePosition)
            self.nodeDropGraphicsView.translate(delta.x(),delta.y())
            self.previousMousePosition = event.globalPos()
            
    def graphicsView_mouseReleaseEvent(self, event):
        if event.button() == Qt.MiddleButton:
            delta = QPoint(0,0)
        
    #adds a corresponding set of nodes to the scene view based upon an xml file
    def setupRigNodes(self,xmlPath,rootNode):
        self.saveXmlPath = xmlPath
        self.rootNodeName = rootNode
        #self.updateXmlPath = mel.eval('getAttr '+rootNode+'.xmlPath')
        tree = xml.ElementTree()
        tree.parse(self.updateXmlPath)
        rig = tree.getroot()
        self.recursiveNodeCreate(None, rig, rootNode)
        rigGuiNode = self.scene.sceneNodes["Rig"]
        self.recursiveSetupScriptJobs(rigGuiNode)
        
    #Creates all of the gui nodes and connections between the nodes from
    #a given rig xml definition file.
    def recursiveNodeCreate(self,parentNode,elem, metaNodeName):
        type = None
        if elem.tag == 'rig':
            type = nt.NodeTypes["rigNode"]
        elif elem.tag == 'geo':
            type = nt.NodeTypes["geometryNode"]
        elif elem.tag == 'component':
            if elem.get('type') == 'global':
                type = nt.NodeTypes["globalComponentNode"]
            elif elem.get('type') == 'hip':
                type = nt.NodeTypes["hipComponentNode"]
        else:
            return
        nodeLocStrings = elem.get('guiLocation').split(",")
        nodePos = QPointF(float(nodeLocStrings[0]),float(nodeLocStrings[1]))
        node = self.scene.createNode(type, nodePos)
        if elem.tag == 'rig' or elem.tag == 'component':
            node.metaNodeName = metaNodeName
        if elem.tag == 'component':
            node.rigId = str(elem.get('rigId'))
        if elem.tag == 'component' and (elem.get('type') == 'global' or elem.get('type') == 'hip'):
            controllerName = mel.eval('getMetaNodeConnection -n "'+node.metaNodeName+'" -c "controller";')
            node.previousScale = mel.eval('xform -query -r -scale '+controllerName)
        node.setupWidgetFromElem(elem)
        if parentNode != None:
            connectionLine = lc.LineClass(node.outputConnection, parentNode.inputConnection, QLineF(node.outputConnection.scenePos(), parentNode.inputConnection.scenePos()))
            connectionLine.updatePosition()
            self.scene.addItem(connectionLine)
        for childElem in elem:
            childMetaNodeName = None
            if childElem.tag == 'component':
                childId = str(childElem.get('rigId'))
                childMetaNodeName = mel.eval('getMetaChildById -n \"'+metaNodeName+'\" -rid \"'+childId+'\";')
            childNode = self.recursiveNodeCreate(node,childElem,childMetaNodeName)
            if childNode is not node:
                node.childNodes.append(childNode)
        return node
    # This function is called whenever the rig node configuration
    # changes or whenever one of the node widget menus is changed.
    # This function causes the rig in the scene to be updated.
    def rigUpdate(self):
        #disable refresh until the rig update is complete
        cmds.refresh(su=True)
        rigNodeFound = False
        try:
            rigGuiNode = self.scene.sceneNodes["Rig"]
            rigNodeFound = True
        except KeyError:
            rigNodeFound = False
        if rigNodeFound:
            #kill all script jobs created by controllers to avoid
            #an update loop which the rig is updated
            for jobNum in self.scriptJobNumbers:
                if jobNum != globals.currentScriptJobNum:
                    cmds.scriptJob(k=jobNum)
            self.scriptJobNumbers = []
            rigGuiNode.updateVersion += 0.1
            rootElem = self.recursiveGetXML(rigGuiNode)
            self.indent(rootElem)
            tree = xml.ElementTree(rootElem)
            file = open(self.updateXmlPath, 'w')
            tree.write(file)
            file.close()
            self.recursiveZeroOutControllers(rigGuiNode)
            if rigGuiNode.metaNodeName is not None and rigGuiNode.metaNodeName != "":
                self.rootNodeName = mel.eval("updateMetaDataManager -n \""+rigGuiNode.metaNodeName+"\";")
            else:
                self.rootNodeName = mel.eval("loadRig -p \""+self.updateXmlPath+"\";")
            cmds.select(cl=True)
            self.recursiveUpdateMetaNodes(rigGuiNode,self.rootNodeName)
            self.recursiveSetupScriptJobs(rigGuiNode)
        cmds.refresh(su=False)
        
    def recursiveUpdateMetaNodes(self,guiNode,metaNodeName):
        if guiNode.nodeType == 'rig' or guiNode.nodeType == 'globalComponent' or guiNode.nodeType == 'hipComponent':
            guiNode.metaNodeName = metaNodeName
        for childGuiNode in guiNode.childNodes:
            childMetaNodeName = None
            if childGuiNode is not None:
                if childGuiNode.nodeType == 'globalComponent' or childGuiNode.nodeType == 'hipComponent':
                    childMetaNodeName = mel.eval('getMetaChildById -n \"'+metaNodeName+'\" -rid \"'+str(childGuiNode.rigId)+'\";')
                self.recursiveUpdateMetaNodes(childGuiNode, childMetaNodeName)
                
    #sets up script jobs for all nodes that have controllers associated with their meta data nodes
    #this causes rig updates to happen when the controllers get moved in the 3d scene
    def recursiveSetupScriptJobs(self,guiNode):
        if guiNode.nodeType == 'globalComponent' or guiNode.nodeType == 'hipComponent':
            self.scriptJobNumbers.extend(guiNode.setupScriptJobs())
        for childGuiNode in guiNode.childNodes:
            if childGuiNode is not None:
                self.recursiveSetupScriptJobs(childGuiNode)
        
    def recursiveGetXML(self,node,forSave=False):
        if node is not None:
            nodeElem = node.getXMLElem(forSave)
            #xmlString = node.getXMLStart()+"\n"
            for childNode in node.childNodes:
                childElem = self.recursiveGetXML(childNode,forSave)
                if childElem is not None:
                    nodeElem.append(childElem)
                #xmlString += self.recursiveGetXML(childNode)
            #xmlString += node.getXMLEnd()+"\n"
            return nodeElem
        else:
            return None
        
    def recursiveZeroOutControllers(self,node):
        if node is not None:
            if node.nodeType == 'globalComponent' or node.nodeType == 'hipComponent':
                if node.metaNodeName is not None:
                    controllerName = mel.eval('getMetaNodeConnection -n "'+node.metaNodeName+'" -c "controller";')
                    cmds.setAttr(controllerName+'.tx',0)
                    cmds.setAttr(controllerName+'.ty',0)
                    cmds.setAttr(controllerName+'.tz',0)
                    cmds.setAttr(controllerName+'.rx',0)
                    cmds.setAttr(controllerName+'.ry',0)
                    cmds.setAttr(controllerName+'.rz',0)
                    cmds.setAttr(controllerName+'.sx',1)
                    cmds.setAttr(controllerName+'.sy',1)
                    cmds.setAttr(controllerName+'.sz',1)
            for childNode in node.childNodes:
                self.recursiveZeroOutControllers(childNode)
        
    def indent(self,elem, level=0):
        i = "\n" + level*"  "
        if len(elem):
            if not elem.text or not elem.text.strip():
                elem.text = i + "  "
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
            for elem in elem:
                self.indent(elem, level+1)
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
        else:
            if level and (not elem.tail or not elem.tail.strip()):
                elem.tail = i
                
    def SaveXMLFile(self):
        #fileDialog = QFileDialog(self.getMayaWindow())
        #fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        #fileDialog.fileSelected.connect(self.WriteXMLToFile)
        #fileDialog.show()
        
    #def WriteXMLToFile(self,fileName):
        rigNodeFound = False
        try:
            rigGuiNode = self.scene.sceneNodes["Rig"]
            rigNodeFound = True
        except KeyError:
            rigNodeFound = False
            try:
                mel.eval('error \"Rig Node Editor: No rig network exists to write to file.\";')
            except:
                return
        
        if rigNodeFound:
            rootElem = self.recursiveGetXML(rigGuiNode,True)
            rigName = rootElem.get('name')
            #define the most current rig definition file name and the rig definition folder
            #for older versions
            fileName = cmds.workspace(q=True,rd=True) + "rigDefinitions/" + rigName + ".xml"
            fileDirectory = cmds.workspace(q=True,rd=True) + "rigDefinitions/" + rigName + "/"
            currentFileExists = os.path.isfile(fileName)
            archiveFolderExists = os.path.isdir(fileDirectory)
            archiveFilesExist = False
            if archiveFolderExists:
                archiveFilesExist = len(os.listdir(fileDirectory)) > 0
            versionExists = False
            versionFile = None
            #check archive file names to see if version already exists
            if archiveFilesExist:
                saveVersion = rootElem.get('version')
                versionSearchString = re.sub('\.','_',saveVersion)
                for file in os.listdir(fileDirectory):
                    if versionSearchString in file:
                        versionExists = True
                        versionFile = file
            #check most recently saved file to see if version exists
            if currentFileExists:
                tree = xml.ElementTree()
                tree.parse(fileName)
                rigElem = tree.getroot()
                lastVersion = rigElem.get('version')
                saveVersion = rootElem.get('version')
                if lastVersion == saveVersion:
                    versionExists = True
                    versionFile = fileName
            self.FileSave(currentFileExists, archiveFolderExists, versionExists, versionFile, fileName, fileDirectory,rootElem)
            
    #depending on the boolean flags provided, save the xml file in different ways
    def FileSave(self,currentFileExists,archiveFolderExists,versionExists,versionFile,fileName,fileDirectory,rootElem):
        #create the archive folder if it doesn't exist
        if archiveFolderExists == False:
            os.makedirs(str(fileDirectory))
        #If the file doesn't exist,
        #save the current edits to fileName
        if currentFileExists == False:
            saveDialog = QMessageBox(self.getMayaWindow())
            saveDialog.setText("Create new rig definition file? \n"+fileName)
            saveDialog.addButton("Yes",QMessageBox.AcceptRole)
            saveDialog.addButton("No",QMessageBox.RejectRole)
            returnValue = saveDialog.exec_()
            if returnValue == QMessageBox.AcceptRole:
                self.SaveFile(fileName, rootElem)
            elif returnValue == QMessageBox.RejectRole:
                return
        #If the file does exist, and it isn't a currently saved version,
        # archive the most recently saved version, and save the current edits to fileName
        elif currentFileExists == True and versionExists == False:
            saveDialog = QMessageBox(self.getMayaWindow())
            saveDialog.setText("Save rig definition file? \n"+fileName)
            saveDialog.addButton("Yes",QMessageBox.AcceptRole)
            saveDialog.addButton("No",QMessageBox.RejectRole)
            returnValue = saveDialog.exec_()
            if returnValue == QMessageBox.AcceptRole:
                self.ArchiveFile(fileName, fileDirectory)
                self.SaveFile(fileName, rootElem)
            elif returnValue == QMessageBox.RejectRole:
                return
        elif currentFileExists == True and versionExists == True:
            version = rootElem.get('version')
            tree = xml.ElementTree()
            tree.parse(fileName)
            rigElem = tree.getroot()
            latestVersion = float(rigElem.get('version'))+0.1
            saveDialog = QMessageBox(self.getMayaWindow())
            saveDialog.setText("This version of the rig already exists: \n"+fileName+" ("+version+")")
            latestButton = saveDialog.addButton("Save to latest version ("+str(latestVersion)+")",QMessageBox.YesRole)
            acceptButton = saveDialog.addButton("Overwrite version ("+version+")",QMessageBox.AcceptRole)
            cancelButton = saveDialog.addButton("Cancel",QMessageBox.NoRole)
            saveDialog.exec_()
            if saveDialog.clickedButton() == latestButton:
                self.ArchiveFile(fileName, fileDirectory)
                rootElem.set('version',str(latestVersion))
                self.SaveFile(fileName, rootElem)
            elif saveDialog.clickedButton() == acceptButton:
                self.SaveFile(versionFile, rootElem)
            elif saveDialog.clickedButton() == cancelButton:
                return
        
    #archive fileName to fileDirectory
    def ArchiveFile(self,fileName,fileDirectory):
        oldTree = xml.ElementTree()
        oldTree.parse(fileName)
        oldRootElem = oldTree.getroot()
        oldRootVersion = oldRootElem.get('version')
        rigName = oldRootElem.get('name')
        oldRootVersion = "_"+re.sub('\.','_',oldRootVersion)
        shutil.copyfile(fileName, fileDirectory+rigName+oldRootVersion+".xml")
            
    #save the rig currently represented by the nodes in the editor to fileName
    def SaveFile(self,fileName,rootElem):
        self.indent(rootElem)
        tree = xml.ElementTree(rootElem)
        file = open(fileName, 'w')
        tree.write(file)
        file.close()
        
    def SetManagerWindow(self,window):
        self.MDMWindow = window