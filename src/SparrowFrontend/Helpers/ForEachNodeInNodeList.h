#pragma once

#include <Feather/Nodes/NodeList.h>

namespace SprFrontend
{
    /// Iterates recursively over all the nodes in the given node list, calling the given 'fun' function
    /// If a node has an explanation, we will use the explanation instead the actual node
    /// If a NodeList node is found as a child of the given node list, this will recursively call itself to gather the
    /// children from that node list too. The given function is not called for node-list nodes.
    template <typename F>
    inline void forEachNodeInNodeList(Feather::NodeList* nodeList, F fun)
    {
        if ( nodeList )
        {
            for ( DynNode* n: nodeList->children() )
            {
                if ( !n )
                    continue;

                DynNode* nn = n->explanation();
                NodeList* nl = nn->as<NodeList>();

                if ( nl )
                    forEachNodeInNodeList(nl, fun);
                else
                    fun(n);
            }
        }
    }
}
