testBuild = False

import MetaDataManagerGUI
reload(MetaDataManagerGUI)

import maya.cmds as cmds
if testBuild == False:
    from rig101wireControllers import rig101
    import Utils
    reload(Utils)


#from MetaDataManager import *
import MetaDataManager as mdm
reload(mdm)
#from RigNodeEditor.RigNodeEditor import *
if testBuild == False:
    import RigNodeEditor.RigNodeEditor as rne
else:
    import RigNodeEditor as rne
reload(rne)

pluginFileName = 'MetaDataNode.mll'

pluginFilePath = 'E:\\Thesis_SVN\\MetaDataNode\\MetaDataNode\\Debug\\' + pluginFileName

def removePlugin():
    pluginLoaded = cmds.pluginInfo( pluginFilePath, query=True, loaded=True )
    pluginUnloadOk = cmds.pluginInfo( pluginFilePath, query=True, unloadOk=True )
    if pluginLoaded and not pluginUnloadOk:
        cmds.file(f=True,new=True)
    cmds.unloadPlugin(pluginFileName)

def setPlugin():
    cmds.loadPlugin( pluginFilePath )
    
if testBuild == True:
    setPlugin()