# These functions are used to make creating a PyQT window in Maya
# easier to do. PyQT windows can use a .ui file created in QT Designer
# to do visual window layout.
# From this blog article:
# http://nathanhorne.com/?p=451

__author__ = 'Nathan Horne'

#Source code for some common Maya/PyQt functions we will be using
import sip
sip.setapi('QString', 2)
sip.setapi('QVariant', 2)
from PyQt4 import QtGui, QtCore
import maya.OpenMayaUI as apiUI

def getMayaWindow():
    """
    Get the main Maya window as a QtGui.QMainWindow instance
    @return: QtGui.QMainWindow instance of the top level Maya windows
    """
    ptr = apiUI.MQtUtil.mainWindow()
    if ptr is not None:
        return sip.wrapinstance(long(ptr), QtCore.QObject)

def toQtObject(mayaName):
    """
    Convert a Maya ui path to a Qt object
    @param mayaName: Maya UI Path to convert (Ex: "scriptEditorPanel1Window|TearOffPane|scriptEditorPanel1|testButton" )
    @return: PyQt representation of that object
    """
    ptr = apiUI.MQtUtil.findControl(mayaName)
    if ptr is None:
        ptr = apiUI.MQtUtil.findLayout(mayaName)
    if ptr is None:
        ptr = apiUI.MQtUtil.findMenuItem(mayaName)
    if ptr is not None:
        return sip.wrapinstance(long(ptr), QtCore.QObject)