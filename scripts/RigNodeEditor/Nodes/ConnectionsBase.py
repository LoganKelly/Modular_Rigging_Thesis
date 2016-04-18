"""
ConnectionsBase is the base class
for all connections that are added
to a node. Connections allow nodes
to be hooked up to other nodes
to indicate a parent/child relationship.
Author: Logan Kelly
Date: 3/7/13
"""

from PyQt4 import QtGui, QtCore, QtSvg

class ConnectionsBase(QtSvg.QGraphicsSvgItem):

    lineConnected = QtCore.pyqtSignal()
    
    def __init__(self, *args, **kwargs):
        super(ConnectionsBase, self).__init__(*args, **kwargs)
        self.setAcceptHoverEvents(True)
        self.isInputConnection = False
        self.connectedLine = []
        self.setElementId("regular")
        self.nodeType = ""
        self.lineConnected.connect(self.updateAll)
        self.lineConnected.connect(self.lineJustConnected)
        
    def clearLine(self):
        '''
        The purpose of this function is to graphically clear all a line and destroy references to it from other objects
        '''
        # The [:] denotes a copy of the connectedLine list. Because within deleteLine() I'm removing elements from connectedLine. Without [:] everything goes to shit.
        for line in self.connectedLine[:]:
            line.deleteLine()

    def lineJustConnected(self):
        '''
        This signal is being emitted in the mouseReleaseEvent() in graphicsModule.py
        '''
        self.parentItem().getWidgetMenu().justConnected()
        
    def width(self):
        return self.sceneBoundingRect().width()

    def height(self):
        return self.sceneBoundingRect().height()

    def hoverMoveEvent(self,event):
        self.setElementId("hover")

    def hoverLeaveEvent(self, event):
        self.setElementId("regular")

    def mouseReleaseEvent(self, event):
        self.setElementId("regular")

    def updateAll(self):
        pass