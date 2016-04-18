'''
BaseAttrObject provides the basic objects
and methods necessary for creating a node widget
that can communicate with an object in a Maya scene(?)
and pass data to and from connected nodes
Author: Logan Kelly
Date: 3/8/13
'''

class BaseAttrObject(object):

    def __init__(self):
        self.dataToSend = {}
        self.receivedData = {}
        self.object = None

    def connectSignals(self):
        pass

    def getObject(self):
        try:
            return self.object
        except (TypeError, AttributeError), e:
            print "getObject() Error: %s" %str(e)

    def setObject(self, object):
        if object is not None:
            self.object = object.replace(' ', '_')
            self.object = object.lstrip('1234567890')
        else:
            self.object = None

    def setScriptJobs(self):
        pass

    def setupVariables(self):
        pass