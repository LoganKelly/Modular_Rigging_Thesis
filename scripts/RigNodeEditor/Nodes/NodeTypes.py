# This module is used for creating
# a list of every type of node to 
# populate the node lists with
# and defines which particular list 
# that node should be contained in.
# Author: Logan Kelly
#   Date: 2/28/13

#this is the dictionary of node types
#that we will be using to build the node lists

from BaseNode import *
import RigGuiNodeWidget as rgnw
reload(rgnw)
import GeometryGuiNodeWidget as geognw
reload(geognw)
import GlobalGuiNodeWidget as ggnw
reload(ggnw)
import HipGuiNodeWidget as hgnw
reload(hgnw)

NodeTypes = {}

def createNodeTypes(force=False):
    
    if NodeTypes and not force:
        return NodeTypes
    
    """
     *************              NODES           *************

        Fill these in as necessary:
            self.dictKey = The dictionary key that you give it
            self.displayText = The text that is displayed in the GUI
            self.imagePath = The Image Path to the node icon
            self.description = Node description to show up in lower right of GUI
            self.listWidgetName = The name of the list widget this attribute belongs to
            self.widgetMenu = The menu that is displayed with this node. maya.cmds are integrated into those menus
    """
    
    """Rig Node"""
    
    NodeTypes["rigNode"] = BaseNode(
        dictKey = "rigNode",
        displayText = "Rig",
        imagePath = ":/rigNode.svg",
        description = "This is the rig node that all other nodes will hook into."\
                      " Without this node nothing will happen.",
        listWidgetName = "rigNodeList",
        nodeType = "rig",
        nodeColor = "blue",
        widgetMenu = rgnw.RigGuiNodeWidget,
    )
    
    #Geometry Node
    
    NodeTypes["geometryNode"] = BaseNode(
        dictKey = "geometryNode",
        displayText = "Geometry",
        imagePath = ":/geometryNode.svg",
        description = "The geometry node defines a piece of geometry that will be referenced"\
                        " from another file. Name is the name of the model in the reference file.",
        listWidgetName = "rigNodeList",
        nodeType = "geometry",
        nodeColor = "green",
        widgetMenu = geognw.GeometryGuiNodeWidget,
    )
    
    NodeTypes["globalComponentNode"] = BaseNode(
        dictKey = "globalComponentNode",
        displayText = "Global",
        imagePath = ":/componentNode.svg",
        description = "The global component node is used for a top level controller that moves"\
                        " an entire rig in global space.",
        listWidgetName = "rigNodeList",
        nodeType = "globalComponent",
        nodeColor = "red",
        widgetMenu = ggnw.GlobalGuiNodeWidget,
    )
    
    NodeTypes["hipComponentNode"] = BaseNode(
        dictKey = "hipComponentNode",
        displayText = "Hip",
        imagePath = ":/componentNode.svg",
        description = "The hip component node is used for creating a hip controller and joint.",
        listWidgetName = "rigNodeList",
        nodeType = "hipComponent",
        nodeColor = "red",
        widgetMenu = hgnw.HipGuiNodeWidget,
    )
        
    
    return NodeTypes