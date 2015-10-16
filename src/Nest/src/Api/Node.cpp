#include "Nest/src/StdInc.hpp"
#include "Nest/Api/Node.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"

NodeProperties _cloneProperties(NodeProperties src) {
    unsigned size = src.end - src.begin;
    NodeProperties res = { NULL, NULL, NULL };
    res.begin = (NodeProperty*) malloc(size*sizeof(NodeProperty));
    res.end = res.begin+size;
    res.endOfStore = res.end;
    NodeProperty* s = src.begin;
    NodeProperty* d = res.begin;
    for ( ; s!=src.end; ++s, ++d )
        *d = *s;
    return res;
}

void _applyModifiers(Node* node, ModifierType modType)
{
    Modifier** mod = node->modifiers.beginPtr;
    for ( ; mod != node->modifiers.endPtr; ++mod ) {
        if ( (*mod)->modifierType == modType )
            (*mod)->modifierFun(*mod, node);
    }
}

/// Set the explanation of this node.
/// makes sure it has the right context, compiles it, and set the type of the current node to be the type of the
/// explanation
bool _setExplanation(Node* node, Node* explanation)
{
    if ( explanation == node->explanation )
        return true;

    node->explanation = explanation;

    if ( explanation == node )
        return true;

    // Copy all the properties marked accordingly
    NodeProperty* p = node->properties.begin;
    for ( ; p!=node->properties.end; ++p )
        if ( p->passToExpl )
            Nest_setProperty(node->explanation, *p);

    // Try to semantically check the explanation
    bool res = true;
    if ( !explanation->nodeSemanticallyChecked )
    {
        Nest_setContext(node->explanation, node->context);
        res = Nest_semanticCheck(node->explanation);
    }
    node->type = node->explanation->type;
    return res;
}


Node* Nest_createNode(int nodeKind)
{
    ASSERT(nodeKind >= 0);

    Node* res = (Node*) alloc(sizeof(Node), allocNode);
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

Node* Nest_cloneNode(Node* node)
{
    if ( !node )
        return NULL;

    ASSERT(node);
    Node* res = Nest_createNode(node->nodeKind);

    res->location = node->location;
    res->referredNodes = node->referredNodes;
    res->properties = _cloneProperties(node->properties);

    // Clone each node in the children vector
    unsigned size = Nest_nodeArraySize(node->children);
    Nest_resizeNodeArray(&res->children, size);
    for ( size_t i=0; i<size; ++i )
    {
        at(res->children, i) = Nest_cloneNode(at(node->children, i));
    }
    return res;
}

void Nest_setContext(Node* node, CompilationContext* context)
{
    if ( context == node->context )
        return;
    ASSERT(context);
    node->context = context;

    if ( node->type )
        Nest_clearCompilationState(node);

    _applyModifiers(node, modTypeBeforeSetContext);

    Nest_getSetContextForChildrenFun(node->nodeKind)(node);

    _applyModifiers(node, modTypeAfterSetContext);
}

TypeRef Nest_computeType(Node* node)
{
    if ( node->type )
        return node->type;
    if ( node->nodeError )
        return NULL;

    if ( !node->context )
        REP_INTERNAL(node->location, "No context associated with node (%1%)") % Nest_toString(node);

    // Check for recursive dependency
    if ( node->computeTypeStarted )
        REP_ERROR_RET(nullptr, node->location, "Recursive dependency detected while computing the type of the current node");
    node->computeTypeStarted = 1;

    _applyModifiers(node, modTypeBeforeComputeType);

    // Actually compute the type
    node->type = Nest_getComputeTypeFun(node->nodeKind)(node);
    if ( !node->type )
    {
        node->nodeError = 1;
        return NULL;
    }

    _applyModifiers(node, modTypeAfterComputeType);

    return node->type;
}

Node* Nest_semanticCheck(Node* node)
{
    if ( node->nodeSemanticallyChecked )
        return node->explanation;
    if ( node->nodeError )
        return NULL;

    if ( !node->context )
        REP_INTERNAL(node->location, "No context associated with node (%1%)") % Nest_toString(node);

    // Check for recursive dependency
    if ( node->semanticCheckStarted )
        REP_ERROR_RET(nullptr, node->location, "Recursive dependency detected while semantically checking the current node");
    node->semanticCheckStarted = 1;

    _applyModifiers(node, modTypeBeforeSemanticCheck);

    // Actually do the semantic check
    Node* res = Nest_getSemanticCheckFun(node->nodeKind)(node);
    if ( !res )
    {
        node->nodeError = 1;
        return NULL;
    }
    if ( !_setExplanation(node, res) )
    {
        node->nodeError = 1;
        return NULL;
    }
    if ( !node->type )
        REP_INTERNAL(node->location, "Node semantically checked, but no actual types was generated");
    node->nodeSemanticallyChecked = 1;

    _applyModifiers(node, modTypeAfterSemanticCheck);

    return node->explanation;
}

void Nest_clearCompilationState(Node* node)
{
    node->nodeError = 0;
    node->nodeSemanticallyChecked = 0;
    node->computeTypeStarted = 0;
    node->semanticCheckStarted = 0;
    node->explanation = nullptr;
    node->type = nullptr;
    node->modifiers.endPtr = node->modifiers.beginPtr;

    for ( Node* p: node->children )
    {
        if ( p )
            Nest_clearCompilationState(p);
    }
}

const char* Nest_defaultFunToString(const Node* node)
{
    ostringstream os;
    if ( node->explanation && node != node->explanation )
        os << node->explanation;
    else
    {
        const StringRef* name = Nest_getPropertyString(node, "name");
        if ( name )
            os << name->begin;
        else
        {
            os << Nest_nodeKindName(node) << "(";
            for ( size_t i=0; i<Nest_nodeArraySize(node->children); ++i )
            {
                if ( i > 0 )
                    os << ", ";
                os << at(node->children, i);
            }

            os << ")";
        }
    }
    return dupString(os.str().c_str());
}

void Nest_defaultFunSetContextForChildren(Node* node)
{
    CompilationContext* childrenCtx = Nest_childrenContext(node);
    for ( Node* child: node->children )
    {
        if ( child )
            Nest_setContext(child, childrenCtx);
    }
}

TypeRef Nest_defaultFunComputeType(Node* node)
{
    Nest_semanticCheck(node);
    return node->type;
}

