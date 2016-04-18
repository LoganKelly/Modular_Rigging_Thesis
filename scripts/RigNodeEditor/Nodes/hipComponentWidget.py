# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'hipComponentWidget.ui'
#
# Created: Mon Apr 01 23:08:11 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_hipComponentWidget(object):
    def setupUi(self, hipComponentWidget):
        hipComponentWidget.setObjectName(_fromUtf8("hipComponentWidget"))
        hipComponentWidget.resize(206, 270)
        self.gridLayout = QtGui.QGridLayout(hipComponentWidget)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout.addItem(spacerItem, 6, 0, 1, 2)
        self.nameLabel = QtGui.QLabel(hipComponentWidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.nameLabel.sizePolicy().hasHeightForWidth())
        self.nameLabel.setSizePolicy(sizePolicy)
        self.nameLabel.setMaximumSize(QtCore.QSize(40, 16777215))
        self.nameLabel.setObjectName(_fromUtf8("nameLabel"))
        self.gridLayout.addWidget(self.nameLabel, 3, 0, 1, 1)
        self.name = QtGui.QLineEdit(hipComponentWidget)
        self.name.setObjectName(_fromUtf8("name"))
        self.gridLayout.addWidget(self.name, 3, 1, 1, 1)
        self.colorLabel = QtGui.QLabel(hipComponentWidget)
        self.colorLabel.setObjectName(_fromUtf8("colorLabel"))
        self.gridLayout.addWidget(self.colorLabel, 4, 0, 1, 1)
        self.colorComboBox = QtGui.QComboBox(hipComponentWidget)
        self.colorComboBox.setObjectName(_fromUtf8("colorComboBox"))
        self.gridLayout.addWidget(self.colorComboBox, 4, 1, 1, 1)
        self.iconLabel = QtGui.QLabel(hipComponentWidget)
        self.iconLabel.setObjectName(_fromUtf8("iconLabel"))
        self.gridLayout.addWidget(self.iconLabel, 5, 0, 1, 1)
        self.iconComboBox = QtGui.QComboBox(hipComponentWidget)
        self.iconComboBox.setObjectName(_fromUtf8("iconComboBox"))
        self.gridLayout.addWidget(self.iconComboBox, 5, 1, 1, 1)
        self.versionLabel = QtGui.QLabel(hipComponentWidget)
        self.versionLabel.setObjectName(_fromUtf8("versionLabel"))
        self.gridLayout.addWidget(self.versionLabel, 0, 0, 1, 1)
        self.version = QtGui.QLabel(hipComponentWidget)
        self.version.setText(_fromUtf8(""))
        self.version.setObjectName(_fromUtf8("version"))
        self.gridLayout.addWidget(self.version, 0, 1, 1, 1)

        self.retranslateUi(hipComponentWidget)
        QtCore.QMetaObject.connectSlotsByName(hipComponentWidget)

    def retranslateUi(self, hipComponentWidget):
        hipComponentWidget.setWindowTitle(QtGui.QApplication.translate("hipComponentWidget", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.nameLabel.setText(QtGui.QApplication.translate("hipComponentWidget", "Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.colorLabel.setText(QtGui.QApplication.translate("hipComponentWidget", "Color:", None, QtGui.QApplication.UnicodeUTF8))
        self.iconLabel.setText(QtGui.QApplication.translate("hipComponentWidget", "Icon:", None, QtGui.QApplication.UnicodeUTF8))
        self.versionLabel.setText(QtGui.QApplication.translate("hipComponentWidget", "Version:", None, QtGui.QApplication.UnicodeUTF8))

