#include <StdInc.h>
#include "Node.h"
#include "NodeKindRegistrar.h"
#include "Modifier.h"
#include <Compiler.h>
#include <Common/Alloc.h>

using namespace Nest;

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
    res->properties = node->properties;

    // Clone each node in the children vector
    size_t size = node->children.size();
    res->children.resize(size, nullptr);
    for ( size_t i=0; i<size; ++i )
    {
        res->children[i] = Nest_cloneNode(node->children[i]);
    }
    return res;
}

void Nest_initNode(Node* node, int nodeKind)
{
    node->nodeKind = nodeKind;
    node->nodeError = 0;
    node->nodeSemanticallyChecked = 0;
    node->computeTypeStarted = 0;
    node->semanticCheckStarted = 0;
    node->context = nullptr;
    node->childrenContext = nullptr;
    node->type = nullptr;
    node->explanation = nullptr;
}
void Nest_initCopyNode(Node* node, const Node* srcNode)
{
    node->nodeKind = srcNode->nodeKind;
    node->location = srcNode->location;
    node->referredNodes = srcNode->referredNodes;
    node->properties = srcNode->properties;

    // Clone each node in the children vector
    size_t size = srcNode->children.size();
    node->children.resize(size, nullptr);
    for ( size_t i=0; i<size; ++i )
    {
        node->children[i] = Nest_cloneNode(srcNode->children[i]);
    }
}


const char* Nest_toString(const Node* node)
{
    return Nest_getToStringFun(node->nodeKind)(node);
}

const char* Nest_nodeKindName(const Node* node)
{
    return Nest_getNodeKindName(node->nodeKind);
}


void Nest_setProperty(Node* node, const char* name, int val, bool passToExpl)
{
    node->properties[name] = Property(propInt, PropertyValue(val), passToExpl);
}
void Nest_setProperty(Node* node, const char* name, string val, bool passToExpl)
{
    node->properties[name] = Property(propString, PropertyValue(move(val)), passToExpl);
}
void Nest_setProperty(Node* node, const char* name, Node* val, bool passToExpl)
{
    node->properties[name] = Property(propNode, PropertyValue(val), passToExpl);
}
void Nest_setProperty(Node* node, const char* name, TypeRef val, bool passToExpl)
{
    node->properties[name] = Property(propType, PropertyValue(val), passToExpl);
}

bool Nest_hasProperty(const Node* node, const char* name)
{
    return node->properties.find(name) != node->properties.end();
}
const int* Nest_getPropertyInt(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propInt )
        return nullptr;
    return &it->second.value_.intValue_;
}
const string* Nest_getPropertyString(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propString )
        return nullptr;
    return it->second.value_.stringValue_;
}
Node*const* Nest_getPropertyNode(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propNode )
        return nullptr;
    return &it->second.value_.nodeValue_;
}
const TypeRef* Nest_getPropertyType(const Node* node, const char* name)
{
    auto it = node->properties.find(name);
    if ( it == node->properties.end() || it->second.kind_ != propType )
        return nullptr;
    return &it->second.value_.typeValue_;
}

int Nest_getCheckPropertyInt(const Node* node, const char* name)
{
    const int* res = Nest_getPropertyInt(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "Node of kind %1% does not have integer property %2%") % Nest_nodeKindName(node) % name;
    return *res;
}
const string& Nest_getCheckPropertyString(const Node* node, const char* name)
{
    const string* res = Nest_getPropertyString(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "Node of kind %1% does not have string property %2%") % Nest_nodeKindName(node) % name;
    return *res;
}
Node* Nest_getCheckPropertyNode(const Node* node, const char* name)
{
    Node*const* res = Nest_getPropertyNode(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "Node of kind %1% does not have Node property %2%") % Nest_nodeKindName(node) % name;
    return *res;
}
TypeRef Nest_getCheckPropertyType(const Node* node, const char* name)
{
    const TypeRef* res = Nest_getPropertyType(node, name);
    if ( !res )
        REP_INTERNAL(node->location, "Node of kind %1% does not have Type property %2%") % Nest_nodeKindName(node) % name;
    return *res;
}


void Nest_setContext(Node* node, CompilationContext* context)
{
    if ( context == node->context )
        return;
    ASSERT(context);
    node->context = context;

    if ( node->type )
        Nest_clearCompilationState(node);

    for ( Modifier* mod: node->modifiers )
        if ( mod->modifierType == modTypeBeforeSetContext )
            mod->modifierFun(mod, node);

    Nest_getSetContextForChildrenFun(node->nodeKind)(node);

    for ( Modifier* mod: node->modifiers )
        if ( mod->modifierType == modTypeAfterSetContext )
            mod->modifierFun(mod, node);
}

void Nest_computeType(Node* node)
{
    if ( node->type )
        return;
    if ( node->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !node->context )
            REP_INTERNAL(node->location, "No context associated with node (%1%)") % Nest_toString(node);

        // Check for recursive dependency
        if ( node->computeTypeStarted )
            REP_ERROR(node->location, "Recursive dependency detected while computing the type of the current node");
        node->computeTypeStarted = 1;

        for ( Modifier* mod: node->modifiers )
            if ( mod->modifierType == modTypeBeforeComputeType )
                mod->modifierFun(mod, node);

        // Actually compute the type
        TypeRef res = Nest_getComputeTypeFun(node->nodeKind)(node);
        if ( !res )
            REP_INTERNAL(node->location, "Type computed successfully, but no actual type was generated");
        node->type = res;

        for ( Modifier* mod: boost::adaptors::reverse(node->modifiers) )
            if ( mod->modifierType == modTypeAfterComputeType )
                mod->modifierFun(mod, node);
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

void Nest_semanticCheck(Node* node)
{
    if ( node->nodeSemanticallyChecked )
        return;
    if ( node->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !node->context )
            REP_INTERNAL(node->location, "No context associated with node (%1%)") % Nest_toString(node);

        // Check for recursive dependency
        if ( node->semanticCheckStarted )
            REP_ERROR(node->location, "Recursive dependency detected while semantically checking the current node");
        node->semanticCheckStarted = 1;

        for ( Modifier* mod: node->modifiers )
            if ( mod->modifierType == modTypeBeforeSemanticCheck )
                mod->modifierFun(mod, node);

        // Actually do the semantic check
        Node* res = Nest_getSemanticCheckFun(node->nodeKind)(node);
        if ( !res )
            REP_INTERNAL(node->location, "Node semantically checked, but no actual explanation was generated");
        Nest_setExplanation(node, res);
        if ( !node->type )
            REP_INTERNAL(node->location, "Node semantically checked, but no actual types was generated");
        node->nodeSemanticallyChecked = 1;

        for ( Modifier* mod: boost::adaptors::reverse(node->modifiers) )
            if ( mod->modifierType == modTypeAfterSemanticCheck )
                mod->modifierFun(mod, node);
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

void Nest_clearCompilationState(Node* node)
{
    node->nodeError = 0;
    node->nodeSemanticallyChecked = 0;
    node->computeTypeStarted = 0;
    node->semanticCheckStarted = 0;
    node->explanation = nullptr;
    node->type = nullptr;
    node->modifiers.clear();

    for ( Node* p: node->children )
    {
        if ( p )
            Nest_clearCompilationState(p);
    }
}

void Nest_addModifier(Node* node, Modifier* mod)
{
    node->modifiers.push_back(mod);
}

CompilationContext* Nest_childrenContext(const Node* node)
{
    return node->childrenContext ? node->childrenContext : node->context;
}

void Nest_setExplanation(Node* node, Node* explanation)
{
    if ( explanation == node || explanation == node->explanation )
        return;

    node->explanation = explanation;

    // Copy all the properties marked accordingly
    for ( const auto& prop : node->properties )
        if ( prop.second.passToExpl_ )
            node->explanation->properties[prop.first] = prop.second;

    // Try to semantically check the explanation
    if ( !explanation->nodeSemanticallyChecked )
    {
        Nest_setContext(node->explanation, node->context);
        Nest_semanticCheck(node->explanation);
    }
    node->type = node->explanation->type;
}

Node* Nest_explanation(Node* node)
{
    return node && node->explanation && node->explanation != node ? Nest_explanation(node->explanation) : node;
}

Node* Nest_ofKind(Node* src, int desiredNodeKind)
{
    return src && src->nodeKind == desiredNodeKind ? src : nullptr;
}


const char* Nest_defaultFunToString(const Node* node)
{
    ostringstream os;
    if ( node->explanation && 0 != strcmp(Nest_nodeKindName(node->explanation), "Feather.Nop") )
        os << node->explanation;
    else
    {
        const string* name = Nest_getPropertyString(node, "name");
        if ( name )
            os << *name;
        else
        {
            os << Nest_nodeKindName(node) << "(";
            for ( size_t i=0; i<node->children.size(); ++i )
            {
                if ( i > 0 )
                    os << ", ";
                os << node->children[i];
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

