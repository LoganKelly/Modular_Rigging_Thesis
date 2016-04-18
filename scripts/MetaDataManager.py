#Meta Data Manager window for loading, unloading, and updating meta data rig definitions
#Uses PyQT to create the window using a .ui file

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4 import uic
#from hornePyQT import *
import maya.cmds as cmds
import pymel.all as pm
import maya.mel as mel
from MetaDataManagerGUI import Ui_MetaDataManager
import RigNodeEditor.RigNodeEditor as rne

import maya.OpenMayaUI as apiUI
import sip
import os
import glob
import re
import shutil
import xml.etree.ElementTree as xml
import RigNodeEditor.Nodes.globals as globals
reload(globals)

#If you put the .ui file for this example elsewhere, just change this path.
#MetaDataManager_form, MetaDataManager_base = uic.loadUiType('E:\\Thesis_SVN\\MetaDataNode\\MetaDataNode\\Debug\\MetaDataManager.ui')
class MetaDataManager(QMainWindow, Ui_MetaDataManager):
    m_DGNodes = None #list of rig root nodes attached to the manager by DG name
    
    rigEditSig = pyqtSignal(bool)
    
    def __init__(self, parent=None):
        #The update string is used to either call the newest
        #rig definition without forcing the update, or any
        #other version of the rig definition and forcing it
        self.updateString = ""
        
        if parent is None:
            parent = self.getMayaWindow()
        super(MetaDataManager, self).__init__(parent)
        self.setupUi(self)
        
        self.rigEditSig.connect(self.enterRigEditMode)

        #The names "addRigBtn" and "removeRigBtn"
        #come from the "objectName" attribute in Qt Designer
        #the attributes to access them are automatically created
        #for us when we call setupUi()
        #Designer ensures that the names are unique for us.
        self.addRigBtn.clicked.connect(self.addRig)
        self.updateAllBtn.pressed.connect(self.update)
        self.updateSelectedBtn.clicked.connect(self.updateSelected)
        self.removeSelectedBtn.clicked.connect(self.removeSelected)
        self.rigList.currentItemChanged.connect(self.updateRigInfo)
        self.enterRigEditBtn.clicked.connect(self.enterRigEditBtnClicked)
        self.exitRigEditBtn.clicked.connect(self.exitRigEditMode)
        self.openNodeBtn.clicked.connect(self.openNodeWindow)
        self.createRigBtn.clicked.connect(self.createRigBtnClicked)
        self.versionComboBox.activated.connect(self.forceRigUpdate)
        self.globalPosCheckBox.stateChanged.connect(self.forceRigUpdate)
        
        metaRoots = mel.eval("listMetaRoots;")
        if metaRoots is not None:
            for root in metaRoots:
                self.updateRigListNode(root)
    def addRig(self):
        """
        Add a new item to the end of the listWidget
        """
        projectDir = cmds.workspace(q=True,rd=True)
        fileDialog = QFileDialog(self.getMayaWindow())
        fileDialog.fileSelected.connect(self.updateRigList)
        fileDialog.show()
        
    def enterRigEditBtnClicked(self):
        self.rigEditSig.emit(False)
        
    def createRigBtnClicked(self):
        self.rigEditSig.emit(True)
        
    def enterRigEditMode(self,*args):
        self.versionComboBox.setEnabled(False)
        newRig = args[0]
        global g_rigEditModeFileName
        global xmlPath
        global rootNode
        projectDir = cmds.workspace(q=True,rd=True)
        #remove the HUD item at (2,0) if one is present
        cmds.headsUpDisplay(rp=(2,2))
        #add our HUD item for the rig edit mode
        cmds.headsUpDisplay("",s=2,b=2,ba="center",lfs="large",dw=50,l="Rig Edit Mode")
        
        tempDir = mel.eval("getenv TEMP;")
        #get the path to the XML file and save it for later use
        if newRig is False:
            current = self.rigList.currentItem()
            rootNode = "MRN_"+current.text()
            xmlPath = cmds.getAttr(str(rootNode+".xmlPath"))
            #write over temporary rig file
            shutil.copy(xmlPath, tempDir+"/rigTemp.xml")
            xmlPath = tempDir+"/rigTemp.xml"
        else:
            rootNode = ""
            xmlPath = ""
            
        
        #save the scene to a file, and load a new, empty scene
        
        
        g_rigEditModeFileName = cmds.file(q=True,sceneName=True)
        if g_rigEditModeFileName == "":
            cmds.file(rn=tempDir+"\\rigModeTemp.ma")
        cmds.file(f=True,save=True,type="mayaAscii")
        cmds.file(f=True,new=True)
        
        #load the rig file into the current scene
        if newRig is False:
            commandString = "loadRig -p \""+xmlPath+"\";"
            rigName = mel.eval(commandString)
        
        
        self.refreshListWidget()
        self.addRigBtn.setEnabled(False)
        self.updateAllBtn.setEnabled(False)
        self.enterRigEditBtn.setEnabled(False)
        self.rigList.setEnabled(False)
        self.createRigBtn.setEnabled(False)
        self.openNodeBtn.setEnabled(True)
        self.exitRigEditBtn.setEnabled(True)
        
    def exitRigEditMode(self):
        global g_rigEditModeFileName
        globals.noUpdate = True
        cmds.headsUpDisplay(rp=(2,2))
        
        tempDir = mel.eval("getenv TEMP;")
        if g_rigEditModeFileName == "":
            cmds.file(tempDir+"\\rigModeTemp.ma",f=True,open=True)
        else:
            cmds.file(g_rigEditModeFileName,f=True,open=True)
        
        self.refreshListWidget()
        self.addRigBtn.setEnabled(True)
        self.updateAllBtn.setEnabled(True)
        self.rigList.setEnabled(True)
        self.createRigBtn.setEnabled(True)
        self.openNodeBtn.setEnabled(False)
        self.exitRigEditBtn.setEnabled(False)
        self.versionComboBox.setEnabled(True)
        if "Latest" in str(self.versionComboBox.currentText()):
            self.forceRigUpdate()
        
    def removeRig(self):
        """
        Remove the selected item from the listWidget
        """
        currentRow = self.rigList.currentRow()
        
        if currentRow is not None:
            self.rigList.takeItem(currentRow)
    
    def updateRigList(self,fileName):
        global rootNode
        #bring the window to the front
        self.activateWindow()
        #build the actual meta data network and rig in the scene
        commandString = "loadRig -p \""+fileName+"\";";
        rigName = mel.eval(commandString)
        rootNode = rigName
        item = QListWidgetItem(self.rigList)
        item.setText(rigName)
        self.refreshListWidget()
        
    """
    add a new item to the list of rigs by providing a node name
    """
    def updateRigListNode(self,nodeName):
        #remove the prefix
        name = nodeName[4:]
        item = QListWidgetItem(self.rigList)
        item.setText(name)
        
    def updateAllRigs(self):
        mel.eval("updateMetaDataManager;")
        
    def update(self):
        if globals.noUpdate == True:
            globals.noUpdate = False
            self.refreshListWidget()
            return
        #update all of the metadata networks in the scene
        mel.eval("updateMetaDataManager;")
        self.refreshListWidget()
                
    def updateRigInfo(self,current,previous):
        global rootNode
        
        #don't update if we are in rig edit mode
        if rootNode is not None:
            xmlPath = cmds.getAttr(str(rootNode+".xmlPath"))
        if "rigTemp.xml" in xmlPath:
            return
        if current is not None:
            rootNode = "MRN_"+current.text()
            version = cmds.getAttr(str(rootNode+".version"))
            #self.versionTxt.setText( "%.3f" % version )
            xmlText = cmds.getAttr(str(rootNode+".xmlPath"))
            tmp = xmlText.split('/')
            xmlFileName = tmp[-1]
            self.xmlText.setText( xmlFileName )
            self.populateVersionBox(current.text())
            #enable disabled buttons when a rig is selected
            self.removeSelectedBtn.setEnabled(True)
            self.updateSelectedBtn.setEnabled(True)
            self.enterRigEditBtn.setEnabled(True)
        else:
            self.removeSelectedBtn.setEnabled(False)
            self.updateSelectedBtn.setEnabled(False)
            self.enterRigEditBtn.setEnabled(False)
            #self.versionTxt.setText("None selected")
            self.xmlText.setText("None selected")
            
    def populateVersionBox(self,xmlName):
        global rootNode
        currentItemText = ""
        if self.versionComboBox.count() > 0:
            currentItemText = self.versionComboBox.currentText()
        self.versionComboBox.clear()
        projPath = cmds.workspace(q=True,rd=True)
        rootNode = "MRN_"+xmlName
        xmlFilePath = cmds.getAttr(str(rootNode+".xmlPath"))
        #get the base rig name, i.e. $PROJDIR/rigDefinitions/test/test_1_1.xml would be test
        xmlFileName = xmlFilePath.split("/")[-1].split(".")[-2].split("_")[0]
        xmlDirPath = projPath + "rigDefinitions/" + xmlFileName + "/"
        rigVersions = []
        if os.path.isdir(xmlDirPath):
            #glob returns a list of files matching the regular expression
            for file in glob.glob(xmlDirPath+"*.xml"):
                tree = xml.ElementTree()
                tree.parse(file)
                rig = tree.getroot()
                version = float(rig.get('version'))
                rigVersions.append(version)
        latestXmlFile = projPath + "rigDefinitions/" + xmlFileName + ".xml"
        if os.path.isfile(latestXmlFile):
            tree = xml.ElementTree()
            tree.parse(latestXmlFile)
            rig = tree.getroot()
            version = float(rig.get('version'))
            rigVersions.append(version)
        self.versionComboBox.addItem("Latest("+str(rigVersions[-1])+")")
        rigVersions.pop()
        for version in reversed(rigVersions):
            self.versionComboBox.addItem(str(version))
        #set the version combo box to the previously selected value
        for i in range(self.versionComboBox.count()):
            itemText = self.versionComboBox.itemText(i)
            if currentItemText == itemText:
                self.versionComboBox.setCurrentIndex(i)

    def updateSelected(self):
        current = self.rigList.currentItem()
        if current is not None:
            rigName = current.text()
            name = "MRN_" + rigName
            #disable refresh until the rig update is complete
            cmds.refresh(su=True)
            mel.eval(self.updateString)
            cmds.select(cl=True)
            #enable refresh after update
            cmds.refresh(su=False)
            self.refreshListWidget()
            self.populateVersionBox(rigName)
            self.updateSelectedBtn.setStyleSheet("")
            
    def removeSelected(self):
        current = self.rigList.currentItem()
        if current is not None:
            name = "MRN_" + current.text()
            mel.eval("removeRig -n \""+name+"\";")
            self.refreshListWidget()
            
    def refreshListWidget(self):
        currentName = ""
        if self.rigList:
            item = self.rigList.currentItem()
            if item:
                currentName = item.text()
        self.rigList.clear()
        metaRoots = mel.eval("listMetaRoots;")
        if metaRoots is not None:
            for root in metaRoots:
                self.updateRigListNode(root)
        #if the previously selected rig is still in the list,
        #re-select it
        for i in range(self.rigList.count()):
            item = self.rigList.item(i)
            if currentName == item.text():
                self.rigList.setCurrentRow(i)
                
    def getMayaWindow(self):
        """
        Get the main Maya window as a QtGui.QMainWindow instance
        @return: QtGui.QMainWindow instance of the top level Maya windows
        """
        ptr = apiUI.MQtUtil.mainWindow()
        if ptr is not None:
            return sip.wrapinstance(long(ptr), QObject)
        
    def openNodeWindow(self):
        global xmlPath
        global RNEwindow
        RNEwindow = rne.RigNodeEditor()
        RNEwindow.SetManagerWindow(self)
        if xmlPath is not "" and rootNode is not "":
            RNEwindow.setupRigNodes(xmlPath,rootNode)
        RNEwindow.show()
        RNEwindow.activateWindow()
        RNEwindow.update()
        
    def forceRigUpdate(self):
        global rootNode
        print "rootNode = "+rootNode
        if rootNode is None or rootNode == "":
            return
        current = self.rigList.currentItem()
        rootNode = "MRN_"+current.text()
        xmlPath = cmds.getAttr(str(rootNode+".xmlPath"))
        
        #Find which version is the latest
        versions = []
        for i in range(self.versionComboBox.count()):
            versionText = self.versionComboBox.itemText(i)
            if "Latest" not in versionText:
                versions.append(float(versionText))
        versions.sort()
        latestVersion = versions[-1]
        #get only the filename, and remove all digits and underscores
        #the resulting filename should be the name of the xml file
        #in the rigDefinitions folder
        fileName = re.sub('[\d_]', '',xmlPath.split("/")[-1])
        projPath = cmds.workspace(q=True,rd=True)
        versionBoxValue = self.versionComboBox.currentText().__str__()
        
        globalPosString = ""
        if self.globalPosCheckBox.isChecked() == True:
            globalPosString = " -g"
        
        if "Latest" in versionBoxValue:
            latestFilePath = projPath+"rigDefinitions/"+fileName
            self.updateString = "updateMetaDataManager -n \""+rootNode+"\" -r \""+latestFilePath+"\" -f"+globalPosString+";"
        else:
            baseRigName = re.sub('\d','',current.text().__str__())
            version = re.sub('\.','_',versionBoxValue)
            versionFilePath = projPath+"rigDefinitions/"+baseRigName+"/"+baseRigName+"_"+version+".xml"
            print "versionFilePath = "+versionFilePath
            self.updateString = "updateMetaDataManager -n \""+rootNode+"\" -r \""+versionFilePath+"\" -f"+globalPosString+";"
        
        self.updateSelectedBtn.setStyleSheet("QPushButton{background:red }")
            
#MDMwindow = MetaDataManager()
#MDMwindow.show()