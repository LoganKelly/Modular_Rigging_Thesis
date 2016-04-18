/***********************************************************
* Summary: Returns a list of all metaRoot nodes connected  *
*          to the Meta Data Manager, if the manager exists *
*  Author: Logan Kelly                                     *
*    Date: 9/27/12                                         *
***********************************************************/

#include "MetaDataManagerNode.h"
#include "ListRootsCmd.h"
#include <maya/MItDependencyNodes.h>
#include <maya/MFn.h>
#include <maya/MPlugArray.h>
#include "MyErrorChecking.h"

MStatus ListRootsCmd::doIt ( const MArgList &args )
{
    MStatus stat;
    bool managerExists = false;

    for( MItDependencyNodes nodeIt(MFn::kPluginDependNode);
        !nodeIt.isDone(); nodeIt.next() ) {
        MFnDependencyNode nodeFn( nodeIt.item() );
        if(nodeFn.typeId() == MetaDataManagerNode::id) {
            //Get the plug for the list of metaRoots
            MPlug metaRootsPlug = nodeFn.findPlug(MString("metaRoots"),true,&stat);
            MyCheckStatusReturn(stat,"findPlug failed");
            //MGlobal::displayInfo( metaRootsPlug.name() );
            
            //follow its connection to all the plugs on connected metaRoots
            MPlugArray rootPlugs;
            metaRootsPlug.connectedTo(rootPlugs,false,true,&stat);
            MyCheckStatusReturn(stat,"MPlug.connectedTo failed");
            
            MStringArray result;

            for(unsigned int i = 0; i < rootPlugs.length(); i++) {
                MObject rootNodeObj = rootPlugs[i].node(&stat);
                MyCheckStatusReturn(stat,"MPlug.node() failed");
                MFnDependencyNode rootNodeFn( rootNodeObj );

                //get the name of the metaRoot node and append it to the results
                MString name = rootNodeFn.name();
                result.append(name);
            }

            setResult(result);
            managerExists = true;

        }
    }

    if(!managerExists) {
        MGlobal::displayInfo("MetaDataManager node does not exist.\n");        
    }

    return redoIt();
}

MStatus ListRootsCmd::undoIt()
{
    return dgMod.undoIt();
}

MStatus ListRootsCmd::redoIt()
{
    return dgMod.doIt();
}