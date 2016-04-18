# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'MetaDataManagerGUI.ui'
#
# Created: Thu May 09 01:07:55 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_MetaDataManager(object):
    def setupUi(self, MetaDataManager):
        MetaDataManager.setObjectName(_fromUtf8("MetaDataManager"))
        MetaDataManager.resize(419, 300)
        self.addRigBtn = QtGui.QPushButton(MetaDataManager)
        self.addRigBtn.setGeometry(QtCore.QRect(10, 220, 75, 23))
        self.addRigBtn.setObjectName(_fromUtf8("addRigBtn"))
        self.removeSelectedBtn = QtGui.QPushButton(MetaDataManager)
        self.removeSelectedBtn.setEnabled(False)
        self.removeSelectedBtn.setGeometry(QtCore.QRect(200, 220, 91, 23))
        self.removeSelectedBtn.setObjectName(_fromUtf8("removeSelectedBtn"))
        self.enterRigEditBtn = QtGui.QPushButton(MetaDataManager)
        self.enterRigEditBtn.setEnabled(False)
        self.enterRigEditBtn.setGeometry(QtCore.QRect(300, 220, 111, 23))
        self.enterRigEditBtn.setObjectName(_fromUtf8("enterRigEditBtn"))
        self.VersionLbl = QtGui.QLabel(MetaDataManager)
        self.VersionLbl.setGeometry(QtCore.QRect(260, 10, 91, 16))
        self.VersionLbl.setObjectName(_fromUtf8("VersionLbl"))
        self.XMLFileLbl = QtGui.QLabel(MetaDataManager)
        self.XMLFileLbl.setGeometry(QtCore.QRect(260, 40, 46, 13))
        self.XMLFileLbl.setObjectName(_fromUtf8("XMLFileLbl"))
        self.rigList = QtGui.QListWidget(MetaDataManager)
        self.rigList.setGeometry(QtCore.QRect(10, 10, 241, 191))
        self.rigList.setObjectName(_fromUtf8("rigList"))
        self.updateAllBtn = QtGui.QPushButton(MetaDataManager)
        self.updateAllBtn.setGeometry(QtCore.QRect(10, 250, 75, 23))
        self.updateAllBtn.setObjectName(_fromUtf8("updateAllBtn"))
        self.updateSelectedBtn = QtGui.QPushButton(MetaDataManager)
        self.updateSelectedBtn.setEnabled(False)
        self.updateSelectedBtn.setGeometry(QtCore.QRect(200, 250, 91, 23))
        self.updateSelectedBtn.setObjectName(_fromUtf8("updateSelectedBtn"))
        self.xmlText = QtGui.QLabel(MetaDataManager)
        self.xmlText.setGeometry(QtCore.QRect(310, 40, 71, 111))
        self.xmlText.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.xmlText.setWordWrap(True)
        self.xmlText.setObjectName(_fromUtf8("xmlText"))
        self.exitRigEditBtn = QtGui.QPushButton(MetaDataManager)
        self.exitRigEditBtn.setEnabled(False)
        self.exitRigEditBtn.setGeometry(QtCore.QRect(300, 250, 111, 23))
        self.exitRigEditBtn.setObjectName(_fromUtf8("exitRigEditBtn"))
        self.createRigBtn = QtGui.QPushButton(MetaDataManager)
        self.createRigBtn.setGeometry(QtCore.QRect(90, 220, 101, 23))
        self.createRigBtn.setObjectName(_fromUtf8("createRigBtn"))
        self.openNodeBtn = QtGui.QPushButton(MetaDataManager)
        self.openNodeBtn.setEnabled(False)
        self.openNodeBtn.setGeometry(QtCore.QRect(90, 250, 101, 23))
        self.openNodeBtn.setObjectName(_fromUtf8("openNodeBtn"))
        self.versionComboBox = QtGui.QComboBox(MetaDataManager)
        self.versionComboBox.setGeometry(QtCore.QRect(310, 10, 81, 22))
        self.versionComboBox.setObjectName(_fromUtf8("versionComboBox"))
        self.globalPosCheckBox = QtGui.QCheckBox(MetaDataManager)
        self.globalPosCheckBox.setGeometry(QtCore.QRect(260, 60, 141, 31))
        self.globalPosCheckBox.setObjectName(_fromUtf8("globalPosCheckBox"))

        self.retranslateUi(MetaDataManager)
        QtCore.QMetaObject.connectSlotsByName(MetaDataManager)

    def retranslateUi(self, MetaDataManager):
        MetaDataManager.setWindowTitle(QtGui.QApplication.translate("MetaDataManager", "Rig Manager", None, QtGui.QApplication.UnicodeUTF8))
        self.addRigBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Add Rig", None, QtGui.QApplication.UnicodeUTF8))
        self.removeSelectedBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Remove Selected", None, QtGui.QApplication.UnicodeUTF8))
        self.enterRigEditBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Enter Rig Edit Mode", None, QtGui.QApplication.UnicodeUTF8))
        self.VersionLbl.setText(QtGui.QApplication.translate("MetaDataManager", " Version: ", None, QtGui.QApplication.UnicodeUTF8))
        self.XMLFileLbl.setText(QtGui.QApplication.translate("MetaDataManager", "XML File:", None, QtGui.QApplication.UnicodeUTF8))
        self.updateAllBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Update All", None, QtGui.QApplication.UnicodeUTF8))
        self.updateSelectedBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Update Selected", None, QtGui.QApplication.UnicodeUTF8))
        self.xmlText.setText(QtGui.QApplication.translate("MetaDataManager", "None selected", None, QtGui.QApplication.UnicodeUTF8))
        self.exitRigEditBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Exit Rig Edit Mode", None, QtGui.QApplication.UnicodeUTF8))
        self.createRigBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Create Rig", None, QtGui.QApplication.UnicodeUTF8))
        self.openNodeBtn.setText(QtGui.QApplication.translate("MetaDataManager", "Open Node Editor", None, QtGui.QApplication.UnicodeUTF8))
        self.globalPosCheckBox.setText(QtGui.QApplication.translate("MetaDataManager", "Maintain global position \n"
"of keys on update", None, QtGui.QApplication.UnicodeUTF8))

