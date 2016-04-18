# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'rigWidget.ui'
#
# Created: Tue Apr 02 22:45:22 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_rigWidget(object):
    def setupUi(self, rigWidget):
        rigWidget.setObjectName(_fromUtf8("rigWidget"))
        rigWidget.resize(206, 270)
        self.gridLayout = QtGui.QGridLayout(rigWidget)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.rigNameLabel = QtGui.QLabel(rigWidget)
        self.rigNameLabel.setObjectName(_fromUtf8("rigNameLabel"))
        self.gridLayout.addWidget(self.rigNameLabel, 1, 0, 1, 1)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout.addItem(spacerItem, 6, 0, 1, 2)
        self.versionLabel = QtGui.QLabel(rigWidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.versionLabel.sizePolicy().hasHeightForWidth())
        self.versionLabel.setSizePolicy(sizePolicy)
        self.versionLabel.setMaximumSize(QtCore.QSize(40, 16777215))
        self.versionLabel.setObjectName(_fromUtf8("versionLabel"))
        self.gridLayout.addWidget(self.versionLabel, 0, 0, 1, 1)
        self.versionNumLabel = QtGui.QLabel(rigWidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.versionNumLabel.sizePolicy().hasHeightForWidth())
        self.versionNumLabel.setSizePolicy(sizePolicy)
        self.versionNumLabel.setObjectName(_fromUtf8("versionNumLabel"))
        self.gridLayout.addWidget(self.versionNumLabel, 0, 1, 1, 1)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem1, 2, 3, 1, 1)
        self.rigName = QtGui.QLineEdit(rigWidget)
        self.rigName.setObjectName(_fromUtf8("rigName"))
        self.gridLayout.addWidget(self.rigName, 1, 1, 1, 1)

        self.retranslateUi(rigWidget)
        QtCore.QMetaObject.connectSlotsByName(rigWidget)

    def retranslateUi(self, rigWidget):
        rigWidget.setWindowTitle(QtGui.QApplication.translate("rigWidget", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.rigNameLabel.setText(QtGui.QApplication.translate("rigWidget", "Rig name: ", None, QtGui.QApplication.UnicodeUTF8))
        self.versionLabel.setText(QtGui.QApplication.translate("rigWidget", "Version:", None, QtGui.QApplication.UnicodeUTF8))
        self.versionNumLabel.setText(QtGui.QApplication.translate("rigWidget", "TextLabel", None, QtGui.QApplication.UnicodeUTF8))

