'''
AbstractAttrObject extends the BaseAttrObject
to provide additional functionality for communicating
with other nodes and updating a node in Maya.
Author: Logan Kelly
Date: 3/8/13
'''

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from BaseAttrObject import *

class AbstractAttrObject(QWidget, BaseAttrObject):

    dataReady = pyqtSignal(object)

    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        BaseAttrObject.__init__(self)
        self._connectedTo = None

    def deleteData(self, data):
        """
        Abstract. This is the exact opposite of receiveData(). When a node is deleted this function
          will be called. So whatever data was obtained needs to be cleaned up and deleted.

        IMPORTANT: MUST ADD "self._connectedTo.getWidgetMenu().dataReady.disconnect(self.deleteData)"
         TO THE END OF EACH OF THESE FUNCTIONS YOU OVERLOAD OR EVERYTHING WILL GO TO SHIT.
        """
        pass

    def justConnected(self):
        """
        Abstract. This function is called WHEN TWO NODES ARE CONNECTED TOGETHER.
        Example: When I connect an ObjectUtility node into my Instancer, I will use this function to
         call another function that adds that object to the instancer list.
        """
        pass

    def packageData(self):
        """
        Abstract. Use this function to put all variables in a dictionary that you want the downstream nodes
         to access. MUST be in a dictionary.
        """
        pass

    def receiveData(self, data):
        """
        Abstract. receivedData will have whatever data the upstreamNode sends it. Then it's
         up to the widget to process and do stuff with that data.
        """
        pass

    def receiveFrom(self, otherObject, delete):
        """
        Connect this object to an upstream object and receive data from its output. You can specify
         whether or not the item is being deleted or created with the "delete" flag. This is used in
         "deleteNode()" in nodeModule.py under CategoryNode.
        """

        # _connectedTo is the other node. UtilNode/AttrNode/CategoryNode
        if self._connectedTo:
            try:
                self._connectedTo.getWidgetMenu().dataReady.disconnect(self.receiveData)
            except Exception, e:
                print "Trying to disconnect from delete function ERROR: %s" %str(e)

        if delete:
            otherObject.getWidgetMenu().dataReady.connect(self.deleteData)
        else:
            otherObject.getWidgetMenu().dataReady.connect(self.receiveData)
        self._connectedTo = otherObject

    def sendData(self, data):
        print "sendData() function called by: ", self
        """ Send data to downstream objects """
        self.dataReady.emit(data)
        print "EMITTED DATA"

    def setupWidget(self):
        """
        Abstract. This function is called AS SOON AS THE NODE IS CREATED IN THE SCENE, regardless of
         if it is connected to another node.
        """
        pass

    def updateMayaValues(self):
        """
        Abstract. The purpose of this function is to update all the widget's values in Maya when a new particle emitter is connected.
        Basically, call all the changeFunctions() in this. If you need to do something else, like create a Instancer node
         (example the "instancer" node), call those functions here.
        This function is called in nodeModule.py in def updateAll() in class ConnectionsBase()
        """
        pass