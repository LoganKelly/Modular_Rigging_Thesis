# NodeList inherits from QListWidget
# and is used for managing the list of 
# nodes below the node network view.
# NodeListItems can be added to NodeLists
# in order to build out the list.
# NodeLists also use cPickle in order to
# enable the drag/drop functionality of
# creating nodes in the QGraphicsScene
# from a NodeListItem.
# Author: Logan Kelly
#   Date: 2/28/13
from PyQt4 import QtGui, QtCore, QtSvg
import cPickle, weakref
import NodeListItem as nli
reload(nli)
import NodeTypes as nt
reload(nt)

class NodeList(QtGui.QListWidget):
    
    d = weakref.WeakValueDictionary()
    
    def __init__(self, *args, **kwargs):
        super(NodeList, self).__init__(*args, **kwargs)
        
        self.setLayout(QtGui.QHBoxLayout())
        self.setWrapping(True)
        self.setLayoutMode(QtGui.QListView.SinglePass)
        self.setDragEnabled(True)
        self.setSpacing(1.5)
        self.setGeometry(9, 9, 608, 193)

        self.listName = ""
        
    def populateListWidget(self, listItems):
        for key in sorted(listItems.iterkeys()):
            if listItems[key].nodeType == "rig" or listItems[key].nodeType == "geometry" or listItems[key].nodeType == "globalComponent" or listItems[key].nodeType == "hipComponent" and listItems[key].listWidgetName == self.listName:
                self.addItem(nli.NodeListItem(listItems[key]))
                
    def startDrag(self, event):
        # item is of type NodeListItem
        item = self.currentItem()
        # nodeData is the data (BaseNode type) that item was created with
        nodeData = nt.NodeTypes[item.dictKey]
        #nodeData = mayaNodesModule.MayaNodes[item.dictKey])
        
        i = id(nodeData)
        self.d[i] = nodeData
        pickleData = cPickle.dumps(i)
        
        data = QtCore.QByteArray.fromRawData(pickleData)
        
        mimeData = QtCore.QMimeData()
        mimeData.setData("application/x-imgname", data)
        
        drag = QtGui.QDrag(self)
        drag.setMimeData(mimeData)
        
        # Setting the icon that the mouse cursor displays
        icon = item.icon()
        pixmap = icon.pixmap(48, 48)
        drag.setPixmap(pixmap.scaled(pixmap.height()*.5, pixmap.width()*.5))
        # Actually starts the dragging
        drag.exec_()