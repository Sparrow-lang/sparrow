#include <StdInc.h>
#include "Node.h"
#include "DynNode.h"
#include <Compiler.h>
#include <Common/NodeAllocator.h>

using namespace Nest;

Node* Nest::createNode(int nodeKind)
{
    ASSERT(nodeKind > 0);

    // TODO (nodes): Make sure this will return an already zeroed memory
    void* p = theCompiler().nodeAllocator().alloc(sizeof(Node));
    Node* res = new (p) Node();
    res->nodeKind = nodeKind;
    res->nodeError = 0;
    res->nodeSemanticallyChecked = 0;
    res->computeTypeStarted = 0;
    res->semanticCheckStarted = 0;
    res->context = nullptr;
    res->childrenContext = nullptr;
    res->type = nullptr;
    res->explanation = nullptr;
    return res;
}

Node* Nest::cloneNode(Node* node)
{
    ASSERT(node);
    Node* res = createNode(node->nodeKind);

    res->location = node->location;
    res->referredNodes = node->referredNodes;
    res->properties = node->properties;

    // Clone each node in the children vector
    size_t size = node->children.size();
    res->children.resize(size, nullptr);
    for ( size_t i=0; i<size; ++i )
    {
        DynNode* n = node->children[i];
        if ( n )
            res->children[i] = n->clone();
    }
    return res;
}
