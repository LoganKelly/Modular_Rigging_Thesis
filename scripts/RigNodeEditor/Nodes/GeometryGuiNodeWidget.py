'''
GeometryGuiNodeWidget creates the widgets necessary 
for the GeometryGuiNode in the channel box area on the right
of the interface. It also hooks up those widgets to the
rig definition authoring.
Author: Logan Kelly
Date: 3/20/13
'''
from geometryWidget import *
from AbstractAttrObject import *
from BaseAttrObject import *
import maya.OpenMayaUI as apiUI
import sip
import maya.cmds as cmds
from PyQt4.QtGui import *
from PyQt4.QtCore import *

class GeometryGuiNodeWidget(AbstractAttrObject, Ui_geometryWidget):
    widgetsUpdated = QtCore.pyqtSignal()

    def __init__(self, parent = None):
        super(GeometryGuiNodeWidget, self).__init__(parent)
        BaseAttrObject.__init__(self)
        self.setupUi(self)
        self.connectSignals()

    def connectSignals(self):
        self.findGeoFileButton.clicked.connect(self.addGeoFile)
        
    def addGeoFile(self):
        fileDialog = QFileDialog(self.getMayaWindow())
        fileDialog.fileSelected.connect(self.updateFileName)
        fileDialog.show()
        
    def updateFileName(self,fileName):
        modelFile = open(fileName)
        modelFileTextLines = modelFile.readlines()
        modelFile.close()
        self.geoNameList.clear()
        for line in modelFileTextLines:
            if 'createNode transform -n' in line:
                modelName = line.split('"')[1]
                item = QListWidgetItem(self.geoNameList)
                item.setText(modelName)
        projectDir = cmds.workspace(q=True,rd=True)
        found = fileName.compare(projectDir)
        if found is not 0:
            fileName = "./"+fileName[len(projectDir):]
        self.fileLocation.setText(fileName)
        #self.widgetsUpdated.emit()

    def mousePressEvent(self, event):
        super(GeometryGuiNodeWidget, self).mousePressEvent(event)

    def getMayaWindow(self):
        """
        Get the main Maya window as a QtGui.QMainWindow instance
        @return: QtGui.QMainWindow instance of the top level Maya windows
        """
        ptr = apiUI.MQtUtil.mainWindow()
        if ptr is not None:
            return sip.wrapinstance(long(ptr), QObject)
        
    def setScriptJobs(self):
        """
        If a property is changed in Maya, the changes will be reflected in the GUI
        with these functions.
        """
        pass

    def setupWidget(self):
        #self.fileLocation.returnPressed.connect(self.widgetsUpdated.emit)
        self.geoNameList.currentItemChanged.connect(self.widgetsUpdated.emit)
        #self.geoName.returnPressed.connect(self.widgetsUpdated.emit)