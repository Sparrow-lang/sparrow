#pragma once

namespace SprFrontend
{
    /// Iterates recursively over all the nodes in the given node list, calling the given 'fun' function
    /// If a node has an explanation, we will use the explanation instead the actual node
    /// If a NodeList node is found as a child of the given node list, this will recursively call itself to gather the
    /// children from that node list too. The given function is not called for node-list nodes.
    template <typename F>
    inline void forEachNodeInNodeList(Node* nodeList, F fun)
    {
        if ( nodeList )
        {
            ASSERT( nodeList->nodeKind == Feather::nkFeatherNodeList );
            for ( Node* n: nodeList->children )
            {
                if ( !n )
                    continue;

                Node* nn = Nest::explanation(n);

                if ( nn->nodeKind == Feather::nkFeatherNodeList )
                    forEachNodeInNodeList(nn, fun);
                else
                    fun((DynNode*) n);
            }
        }
    }
}
