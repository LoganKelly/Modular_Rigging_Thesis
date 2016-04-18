# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'geometryWidget.ui'
#
# Created: Mon Apr 22 22:54:17 2013
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_geometryWidget(object):
    def setupUi(self, geometryWidget):
        geometryWidget.setObjectName(_fromUtf8("geometryWidget"))
        geometryWidget.resize(207, 270)
        self.gridLayout = QtGui.QGridLayout(geometryWidget)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.geoFileLocationLabel = QtGui.QLabel(geometryWidget)
        self.geoFileLocationLabel.setObjectName(_fromUtf8("geoFileLocationLabel"))
        self.gridLayout.addWidget(self.geoFileLocationLabel, 0, 0, 1, 2)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout.addItem(spacerItem, 5, 0, 1, 2)
        self.geoNameLabel = QtGui.QLabel(geometryWidget)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.geoNameLabel.sizePolicy().hasHeightForWidth())
        self.geoNameLabel.setSizePolicy(sizePolicy)
        self.geoNameLabel.setMaximumSize(QtCore.QSize(40, 16777215))
        self.geoNameLabel.setAlignment(QtCore.Qt.AlignLeading|QtCore.Qt.AlignLeft|QtCore.Qt.AlignTop)
        self.geoNameLabel.setObjectName(_fromUtf8("geoNameLabel"))
        self.gridLayout.addWidget(self.geoNameLabel, 4, 0, 1, 1)
        self.fileLocation = QtGui.QLineEdit(geometryWidget)
        self.fileLocation.setObjectName(_fromUtf8("fileLocation"))
        self.gridLayout.addWidget(self.fileLocation, 1, 0, 1, 2)
        self.findGeoFileButton = QtGui.QPushButton(geometryWidget)
        self.findGeoFileButton.setObjectName(_fromUtf8("findGeoFileButton"))
        self.gridLayout.addWidget(self.findGeoFileButton, 1, 2, 1, 1)
        self.geoNameList = QtGui.QListWidget(geometryWidget)
        self.geoNameList.setObjectName(_fromUtf8("geoNameList"))
        self.gridLayout.addWidget(self.geoNameList, 4, 1, 1, 2)

        self.retranslateUi(geometryWidget)
        QtCore.QMetaObject.connectSlotsByName(geometryWidget)

    def retranslateUi(self, geometryWidget):
        geometryWidget.setWindowTitle(QtGui.QApplication.translate("geometryWidget", "Form", None, QtGui.QApplication.UnicodeUTF8))
        self.geoFileLocationLabel.setText(QtGui.QApplication.translate("geometryWidget", "Geometry file location:", None, QtGui.QApplication.UnicodeUTF8))
        self.geoNameLabel.setText(QtGui.QApplication.translate("geometryWidget", "Name:", None, QtGui.QApplication.UnicodeUTF8))
        self.findGeoFileButton.setText(QtGui.QApplication.translate("geometryWidget", "Find...", None, QtGui.QApplication.UnicodeUTF8))

