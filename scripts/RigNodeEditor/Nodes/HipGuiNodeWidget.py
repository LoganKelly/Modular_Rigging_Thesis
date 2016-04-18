'''
HipGuiNodeWidget creates the widgets necessary 
for the HipGuiNode in the channel box area on the right
of the interface. It also hooks up those widgets to the
rig definition authoring.
Author: Logan Kelly
Date: 4/1/13
'''
from hipComponentWidget import *
from AbstractAttrObject import *
from BaseAttrObject import *
import Utils
import rig101wireControllers

class HipGuiNodeWidget(AbstractAttrObject, Ui_hipComponentWidget):
    widgetsUpdated = QtCore.pyqtSignal()

    def __init__(self, parent = None):
        super(HipGuiNodeWidget, self).__init__(parent)
        BaseAttrObject.__init__(self)
        self.setupUi(self)
        self.colorComboBox.clear()
        colorList = []
        for k,v in vars(Utils.Colors).items():
            if not callable(k) and not k.startswith("__"):
                colorList.append(v)
        colorList.sort()
        for color in colorList:
            self.colorComboBox.addItem(color)
        self.iconComboBox.clear()
        iconList = []
        for k,v in vars(rig101wireControllers.ControllerShapes).items():
            if not callable(k) and not k.startswith("__"):
                iconList.append(v)
        iconList.sort()
        for icon in iconList:
            self.iconComboBox.addItem(icon)
        self.connectSignals()
        self.version.setText("1.0")

    def connectSignals(self):
        pass

    def mousePressEvent(self, event):
        super(HipGuiNodeWidget, self).mousePressEvent(event)

    def setScriptJobs(self):
        """
        If a property is changed in Maya, the changes will be reflected in the GUI
        with these functions.
        """
        pass

    def setupWidget(self):
        self.name.returnPressed.connect(self.widgetsUpdated.emit)
        self.colorComboBox.activated.connect(self.widgetsUpdated.emit)
        self.iconComboBox.activated.connect(self.widgetsUpdated.emit)