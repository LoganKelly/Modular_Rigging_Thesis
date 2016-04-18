'''
ConnectRigNode is a connection port for a rig node.
Author: Logan Kelly
Date: 3/7/13
'''

import ConnectionsBase as cb

class ConnectRigNode(cb.ConnectionsBase):

    def __init__(self, *args, **kwargs):
        cb.ConnectionsBase.__init__(self,*args, **kwargs)

    def mousePressEvent(self, event):
        #print self.connectedLine
        pass