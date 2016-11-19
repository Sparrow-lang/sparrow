#include "Nest/Api/Node.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Assert.h"
#include "Nest/Utils/Diagnostic.h"

#include <stdlib.h>
#include <string.h>

unsigned _myMinU(unsigned a, unsigned b) { return a<b ? a : b; }
unsigned _myMaxU(unsigned a, unsigned b) { return b<a ? a : b; }

char* _appendStr(char* dest, char* endOfStore, const char* src)
{
    unsigned toCopy = _myMinU(strlen(src), endOfStore-dest);
    memcpy(dest, src, toCopy);
    return dest + toCopy;
}

NodeProperties _cloneProperties(NodeProperties src) {
    unsigned size = src.end - src.begin;
    NodeProperties res = { 0, 0, 0 };
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
int _setExplanation(Node* node, Node* explanation)
{
    if ( explanation == node->explanation )
        return 1;

    node->explanation = explanation;

    if ( explanation == node )
        return 1;

    // Copy all the properties marked accordingly
    NodeProperty* p = node->properties.begin;
    for ( ; p!=node->properties.end; ++p )
        if ( p->passToExpl )
            Nest_setProperty(node->explanation, *p);

    // Try to semantically check the explanation
    int res = 1;
    if ( !explanation->nodeSemanticallyChecked )
    {
        Nest_setContext(node->explanation, node->context);
        res = NULL != Nest_semanticCheck(node->explanation);
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
    res->context = 0;
    res->childrenContext = 0;
    res->type = 0;
    res->explanation = 0;
    return res;
}

Node* Nest_cloneNode(Node* node)
{
    if ( !node )
        return 0;

    ASSERT(node);
    Node* res = Nest_createNode(node->nodeKind);

    res->location = node->location;
    res->referredNodes = node->referredNodes;
    res->properties = _cloneProperties(node->properties);

    // Clone each node in the children vector
    unsigned size = Nest_nodeArraySize(node->children);
    unsigned i;
    Nest_resizeNodeArray(&res->children, size);
    for ( i=0; i<size; ++i )
    {
        res->children.beginPtr[i] = Nest_cloneNode(node->children.beginPtr[i]);
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
    if ( node->nodeError || node->computeTypeStarted )
        return 0;

    if ( !node->context )
        Nest_reportFmt(node->location, diagInternalError, "No context associated with node %s", Nest_toString(node));

    node->computeTypeStarted = 1;

    _applyModifiers(node, modTypeBeforeComputeType);

    // Actually compute the type
    node->type = Nest_getComputeTypeFun(node->nodeKind)(node);
    if ( !node->type )
    {
        node->nodeError = 1;
        return 0;
    }

    _applyModifiers(node, modTypeAfterComputeType);

    return node->type;
}

Node* Nest_semanticCheck(Node* node)
{
    if ( node->nodeSemanticallyChecked )
        return node->explanation;
    if ( node->nodeError )
        return 0;

    if ( !node->context )
        Nest_reportFmt(node->location, diagInternalError, "No context associated with node %s", Nest_toString(node));

    // Check for recursive dependency
    if ( node->semanticCheckStarted ) {
        Nest_reportDiagnostic(node->location, diagError, "Recursive dependency detected while semantically checking the current node");
        return 0;
    }
    node->semanticCheckStarted = 1;

    _applyModifiers(node, modTypeBeforeSemanticCheck);

    // Actually do the semantic check
    Node* res = Nest_getSemanticCheckFun(node->nodeKind)(node);
    if ( !res )
    {
        node->nodeError = 1;
        return 0;
    }
    if ( !_setExplanation(node, res) )
    {
        node->nodeError = 1;
        return 0;
    }
    if ( !node->type )
        Nest_reportDiagnostic(node->location, diagInternalError, "Node semantically checked, but no actual types was generated");
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
    node->explanation = 0;
    node->type = 0;
    node->modifiers.endPtr = node->modifiers.beginPtr;

    Node** pp;
    for ( pp=node->children.beginPtr; pp!=node->children.endPtr; ++pp )
    {
        Node* p = *pp;
        if ( p )
            Nest_clearCompilationState(p);
    }
}

const char* Nest_defaultFunToString(const Node* node)
{
    const StringRef* name = Nest_getPropertyString(node, "name");
    if ( name && name->begin )
        return name->begin;

    if ( node->explanation && node != node->explanation )
        return Nest_toString(node->explanation);

    // First get the strings to be used
    // These may reserve string buffers on their own
    const char* nodeKindName = Nest_nodeKindName(node);
    static const size_t maxChildToReport = 10;
    const char* childStrings[10] = { 0 };
    unsigned i;
    unsigned numChildToReport = _myMinU(maxChildToReport, Nest_nodeArraySize(node->children));
    for ( i=0; i<numChildToReport; ++i )
    {
        Node* n = node->children.beginPtr[i];
        childStrings[i] = n ? Nest_toString(n) : "<null>";
    }

    // Now build the string for this
    const static unsigned maxSize = 1024;
    char* res = startString(maxSize+1);
    char* end = res;
    char* endOfStore = res + maxSize;
    end = _appendStr(end, endOfStore, nodeKindName);

    // Write all the string attributes
    if ( node->properties.begin != node->properties.end ) {
        end = _appendStr(end, endOfStore, "[");
        NodeProperty* p = node->properties.begin;
        for ( ; p != node->properties.end; ++p ) {
            if ( p->kind == propString ) {
                end = _appendStr(end, endOfStore, p->name.begin);
                end = _appendStr(end, endOfStore, "=");
                if ( p->value.stringValue.begin )
                    end = _appendStr(end, endOfStore, p->value.stringValue.begin);
            }
        }
        end = _appendStr(end, endOfStore, "]");        
    }

    // Write the children
    end = _appendStr(end, endOfStore, "(");
    for ( i=0; i<numChildToReport; ++i )
    {
        if ( i > 0 )
            end = _appendStr(end, endOfStore, ", ");
        Node* n = node->children.beginPtr[i];
        if ( n )
            end = _appendStr(end, endOfStore, childStrings[i]);
    }
    if ( numChildToReport < Nest_nodeArraySize(node->children) )
        end = _appendStr(end, endOfStore, ", ...");
    end = _appendStr(end, endOfStore, ")");
    endString(end-res+1);
    return res;
}

void Nest_defaultFunSetContextForChildren(Node* node)
{
    CompilationContext* childrenCtx = Nest_childrenContext(node);
    Node** pp;
    for ( pp=node->children.beginPtr; pp!=node->children.endPtr; ++pp )
    {
        Node* child = *pp;
        if ( child )
            Nest_setContext(child, childrenCtx);
    }
}

TypeRef Nest_defaultFunComputeType(Node* node)
{
    Nest_semanticCheck(node);
    return node->type;
}

