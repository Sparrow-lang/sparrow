#include <StdInc.h>
#include "Node.h"
#include "DynNode.h"
#include "Modifier.h"
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
        {
            res->children[i] = n->clone();
        }
    }
    return res;
}

bool Nest::isDynNode(Node* node)
{
    return node->nodeKind >= 100;
}

string Nest::toString(Node* node)
{
    return DynNode::fromNode(node)->toString();
}

string Nest::nodeKindName(const Node* node)
{
    // TODO (nodes): Obtain this from the node registrar
    return DynNode::fromNode(node)->nodeKindName();
}


void Nest::setProperty(Node* node, const char* name, int val, bool passToExpl)
{
    node->properties[name] = Property(propInt, PropertyValue(val), passToExpl);
}
void Nest::setProperty(Node* node, const char* name, string val, bool passToExpl)
{
    node->properties[name] = Property(propString, PropertyValue(move(val)), passToExpl);
}
void Nest::setProperty(Node* node, const char* name, DynNode* val, bool passToExpl)
{
    node->properties[name] = Property(propNode, PropertyValue(val), passToExpl);
}
void Nest::setProperty(Node* node, const char* name, TypeRef val, bool passToExpl)
{
    node->properties[name] = Property(propType, PropertyValue(val), passToExpl);
}

bool Nest::hasProperty(const Node* node, const char* name)
{
    return node->properties.find(name) != node->properties.end();
}
const int* Nest::getPropertyInt(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propInt )
        return nullptr;
    return &it->second.value_.intValue_;
}
const string* Nest::getPropertyString(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propString )
        return nullptr;
    return it->second.value_.stringValue_;
}
DynNode*const* Nest::getPropertyNode(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propNode )
        return nullptr;
    return &it->second.value_.nodeValue_;
}
const TypeRef* Nest::getPropertyType(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propType )
        return nullptr;
    return &it->second.value_.typeValue_;
}

int Nest::getCheckPropertyInt(const Node* node, const char* name)
{
    const int* res = getPropertyInt(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "DynNode of kind %1% does not have integer property %2%") % nodeKindName(node) % name;
    return *res;
}
const string& Nest::getCheckPropertyString(const Node* node, const char* name)
{
    const string* res = getPropertyString(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "DynNode of kind %1% does not have string property %2%") % nodeKindName(node) % name;
    return *res;
}
DynNode* Nest::getCheckPropertyNode(const Node* node, const char* name)
{
    DynNode*const* res = getPropertyNode(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "DynNode of kind %1% does not have DynNode property %2%") % nodeKindName(node) % name;
    return *res;
}
TypeRef Nest::getCheckPropertyType(const Node* node, const char* name)
{
    const TypeRef* res = getPropertyType(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "DynNode of kind %1% does not have Type property %2%") % nodeKindName(node) % name;
    return *res;
}


void Nest::setContext(Node* node, CompilationContext* context)
{
    DynNode* dynNode = DynNode::fromNode(node);

    if ( context == node->context )
        return;
    ASSERT(context);
    node->context = context;

    if ( node->type )
        clearCompilationState(node);

    for ( Modifier* mod: node->modifiers )
        mod->beforeSetContext(dynNode);

    dynNode->doSetContextForChildren();

    for ( Modifier* mod: node->modifiers )
        mod->afterSetContext(dynNode);
}

void Nest::computeType(Node* node)
{
    DynNode* dynNode = DynNode::fromNode(node);

    if ( node->type )
        return;
    if ( node->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !node->context )
            REP_INTERNAL(node->location, "No context associated with node (%1%)") % toString(node);

        // Check for recursive dependency
        if ( node->computeTypeStarted )
            REP_ERROR(node->location, "Recursive dependency detected while computing the type of the current node");
        node->computeTypeStarted = 1;

        for ( Modifier* mod: node->modifiers )
            mod->beforeComputeType(dynNode);

        // Actually compute the type
        DynNode::fromNode(node)->doComputeType();
        if ( !node->type )
            REP_INTERNAL(node->location, "Type computed sucessfully, but no actual type was generated");

        for ( Modifier* mod: boost::adaptors::reverse(node->modifiers) )
            mod->afterComputeType(dynNode);
    }
    catch (const Nest::Common::CompilationError&)
    {
        node->nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        node->nodeError = 1;
        REP_INTERNAL(node->location, "Exception thrown during computeType(): %1%") % e.what();
    }
}

void Nest::semanticCheck(Node* node)
{
    DynNode* dynNode = DynNode::fromNode(node);

    if ( node->nodeSemanticallyChecked )
        return;
    if ( node->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !node->context )
            REP_INTERNAL(node->location, "No context associated with node (%1%)") % toString(node);

        // Check for recursive dependency
        if ( node->semanticCheckStarted )
            REP_ERROR(node->location, "Recursive dependency detected while semantically checking the current node");
        node->semanticCheckStarted = 1;

        for ( Modifier* mod: node->modifiers )
            mod->beforeSemanticCheck(dynNode);

        // Actually do the semantic check
        DynNode::fromNode(node)->doSemanticCheck();
        if ( !node->type )
            REP_INTERNAL(node->location, "DynNode semantically checked, but no actual type was generated");
        node->nodeSemanticallyChecked = 1;

        for ( Modifier* mod: boost::adaptors::reverse(node->modifiers) )
            mod->afterSemanticCheck(dynNode);
    }
    catch (const Nest::Common::CompilationError& e)
    {
        if ( e.what() ) {}      // Avoid warning about not using e
        node->nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        node->nodeError = 1;
        REP_INTERNAL(node->location, "Exception thrown during semanticCheck(): %1%") % e.what();
    }
}

void Nest::clearCompilationState(Node* node)
{
    node->nodeError = 0;
    node->nodeSemanticallyChecked = 0;
    node->computeTypeStarted = 0;
    node->semanticCheckStarted = 0;
    node->explanation = nullptr;
    node->type = nullptr;
    node->modifiers.clear();

    for ( DynNode* p: node->children )
    {
        if ( p )
            clearCompilationState(p->node());
    }
}

void Nest::addModifier(Node* node, Modifier* mod)
{
    node->modifiers.push_back(mod);
}

CompilationContext* Nest::childrenContext(const Node* node)
{
    return node->childrenContext ? node->childrenContext : node->context;
}

void Nest::setExplanation(Node* node, DynNode* explanation)
{
    node->explanation = explanation;

    // Copy all the properties marked accordingly
    for ( const auto& prop : node->properties )
        if ( prop.second.passToExpl_ )
            node->explanation->basicNode_->properties[prop.first] = prop.second;

    // Try to semantically check the explanation
    if ( !explanation->isSemanticallyChecked() )
    {
        node->explanation->setContext(node->context);
        node->explanation->semanticCheck();
    }
    node->type = node->explanation->type();
}

DynNode* Nest::explanation(const Node* node)
{
    return node->explanation ? node->explanation->explanation() : DynNode::fromNode(node);
}

