# NodeListItem inherits from BaseNode and
# QListWidgetItem. NodeListItems are used
# for displaying node types in the list
# below the node network view.
# NodeListItems can be dragged and dropped
# into the node network view to create an instance
# of that node type.
# Author: Logan Kelly
#   Date: 2/28/13
from PyQt4 import QtGui, QtCore, QtSvg
from BaseNode import *

class NodeListItem(BaseNode, QtGui.QListWidgetItem):
    
    def __init__(self, *args, **kwargs):
        
        # Grab the args as a list so we can edit it
        args = list(args)
        otherNode = None
        # Check to see if the first argument is of type NodeBase
        #if args and isinstance(args[0], BaseNode):
        if args:
            otherNode = args.pop(0)
        
        # Separately initiate the inherited classes
        QtGui.QListWidgetItem.__init__(self, *args, **kwargs)
        BaseNode.__init__(self)
        
        # Set all necessary attributes that come from the base class if they're present
        self.displayText = otherNode.displayText if otherNode else ""
        self.dictKey = otherNode.dictKey if otherNode else ""
        self.imagePath = otherNode.imagePath if otherNode else ""
        self.description = otherNode.description if otherNode else ""
        self.nodeColor = otherNode.nodeColor if otherNode else ""
        self.listWidgetName = otherNode.listWidgetName if otherNode else None
        
        self.setIcon(QtGui.QIcon(self.imagePath))
        self.setText(self.displayText)