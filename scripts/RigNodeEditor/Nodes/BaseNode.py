# BaseNode is the base class for drawing all GUI nodes
# within the node-based rig editor. It extends the 
# QGraphicsSvgItem class which is used for displaying
# Scaleable Vector Graphics (SVG) files within a 
# QGraphicsScene widget. QGraphicsScene adds
# QGraphicsItem objects (which QGraphicsSvgItem is
# subclassed from) to it for drawing using the
# addItem() method.
# Author: Logan Kelly
#   Date: 2/24/13

from PyQt4 import QtGui, QtCore, QtSvg
import maya.mel as mel
import maya.cmds as cmds
import globals

class BaseNode(object):
    
    def __init__(self, newNode = "", **kwargs):
        self.displayText = newNode.split(".")[0]
        self.dictKey = ""
        #self.displayText = newNode.split(".")[0]
        self.imagePath = ""
        self.description = "Foo"
        self.listWidgetName = ""
        self.nodeType = ""
        self.nodeColor = ""
        self.widgetMenu = None
        self._widgetMenuObj = None
        self.childNodes = []
        self.metaNodeName = None
        self.xmlElem = None
        self.originalVersion = 1.0
        self.saveVersion = 1.0
        self.updateVersion = 1.0
        self.previousScale = [1.0,1.0,1.0]
        
        for key in kwargs:
            if hasattr(self, key):
                setattr(self, key, kwargs[key])
                
    def getWidgetMenu(self):
        if self._widgetMenuObj is None and self.widgetMenu:
            self._widgetMenuObj = self.widgetMenu()
        return self._widgetMenuObj
    
    def getXMLStart(self):
        pass
    
    def getXMLEnd(self):
        pass
    
    def getXMLElem(self,forSave):
        pass
    
    def incrementVersion(self):
        if self.updateVersion is not None:
            self.updateVersion += 0.1
        else:
            self.updateVersion = 1.0
    #implemented in derived classes
    def setupWidgetFromElem(self,elem):
        pass
    
    def getControllerXform(self):
        controllerName = None
        controllerGroup = None
        if self.metaNodeName is not None:
            controllerName = mel.eval('getMetaNodeConnection -n "'+self.metaNodeName+'" -c "controller";')
        self.translation = None
        self.rotation = None
        self.scale = None
        if controllerName is not None:
            self.translation = mel.eval('xform -worldSpace -query -translation '+controllerName)
            self.rotation = mel.eval('xform -worldSpace -query -rotation '+controllerName)
            self.scale = mel.eval('xform -query -r -scale '+controllerName)
            self.scale = [self.previousScale[0]*self.scale[0],self.previousScale[1]*self.scale[1],self.previousScale[2]*self.scale[2]]
            self.previousScale = self.scale
        else:
            self.translation = [0,0,0]
            self.rotation = [0,0,0]
            self.scale = [1,1,1]
        
    def controllerTranslateUpdate(self):
        globals.currentScriptJobNum = self.controllerJobNums[0]
        self.getWidgetMenu().widgetsUpdated.emit()
        if self.metaNodeName is not None:
            controllerName = mel.eval('getMetaNodeConnection -n "'+self.metaNodeName+'" -c "controller";')
            cmds.select(controllerName, r=True)
            mel.eval('MoveTool;')
            
    def controllerRotateUpdate(self):
        globals.currentScriptJobNum = self.controllerJobNums[1]
        self.getWidgetMenu().widgetsUpdated.emit()
        if self.metaNodeName is not None:
            controllerName = mel.eval('getMetaNodeConnection -n "'+self.metaNodeName+'" -c "controller";')
            cmds.select(controllerName, r=True)
            mel.eval('RotateTool;')
            
    def controllerScaleUpdate(self):
        globals.currentScriptJobNum = self.controllerJobNums[2]
        self.getWidgetMenu().widgetsUpdated.emit()
        if self.metaNodeName is not None:
            controllerName = mel.eval('getMetaNodeConnection -n "'+self.metaNodeName+'" -c "controller";')
            cmds.select(controllerName, r=True)
            mel.eval('ScaleTool;')
            
    def getVersionFromElem(self,elem):
        self.updateVersion = float(elem.get('version'))
        self.originalVersion = self.updateVersion
        self.saveVersion = self.originalVersion
        
    def getSaveVersion(self):
        if self.updateVersion > self.originalVersion:
            self.saveVersion += 0.1
            self.originalVersion = self.saveVersion
        else:
            self.saveVersion = self.originalVersion
        return self.saveVersion