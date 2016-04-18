'''
RigGuiNodeWidget creates the widgets necessary 
for the RigGuiNode in the channel box area on the right
of the interface. It also hooks up those widgets to the
rig definition authoring.
Author: Logan Kelly
Date: 3/8/13
'''
from PyQt4 import QtCore
from rigWidget import *
from AbstractAttrObject import *
from BaseAttrObject import *

class RigGuiNodeWidget(AbstractAttrObject, Ui_rigWidget):
    widgetsUpdated = QtCore.pyqtSignal()

    def __init__(self, parent = None):
        super(RigGuiNodeWidget, self).__init__(parent)
        BaseAttrObject.__init__(self)
        self.setupUi(self)
        self.connectSignals()
        self.versionNumLabel.setText("1.0")

    def connectSignals(self):
        pass

    def mousePressEvent(self, event):
        super(RigGuiNodeWidget, self).mousePressEvent(event)

    def setScriptJobs(self):
        """
        If a property is changed in Maya, the changes will be reflected in the GUI
        with these functions.
        """
        pass

    def setupWidget(self):
        self.rigName.returnPressed.connect(self.widgetsUpdated.emit)
        