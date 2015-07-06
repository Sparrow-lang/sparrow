#include <StdInc.h>
#include "Node.h"
#include "Type.h"
#include "CompilationContext.h"
#include "Modifier.h"
#include <Compiler.h>
#include <Common/Diagnostic.h>
#include <Common/NodeAllocator.h>
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;
using namespace Nest::Common::Ser;

BasicNode* Nest::createNode(int nodeKind)
{
    ASSERT(nodeKind > 0);

    // TODO (nodes): Make sure this will return an already zeroed memory
    void* p = theCompiler().nodeAllocator().alloc(sizeof(BasicNode));
    BasicNode* res = new (p) BasicNode();
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

BasicNode* Nest::cloneNode(BasicNode* node)
{
    ASSERT(node);
    BasicNode* res = createNode(node->nodeKind);

    res->location = node->location;
    res->referredNodes = node->referredNodes;
    res->properties = node->properties;

    // Clone each node in the children vector
    size_t size = node->children.size();
    res->children.resize(size, nullptr);
    for ( size_t i=0; i<size; ++i )
    {
        Node* n = node->children[i];
        if ( n )
            res->children[i] = n->clone();
    }
    return res;
}


Node::Node(int nodeKind, const Location& location, NodeVector children, NodeVector referredNodes)
    : basicNode_(createNode(nodeKind))
    , location_(basicNode_->location)
    , children_(basicNode_->children)
    , referredNodes_(basicNode_->referredNodes)
    , properties_(basicNode_->properties)
    , context_(basicNode_->context)
    , childrenContext_(basicNode_->childrenContext)
    , type_(basicNode_->type)
    , explanation_(basicNode_->explanation)
    , modifiers_(basicNode_->modifiers)
{
    basicNode_->location = location;
    basicNode_->children = move(children);
    basicNode_->referredNodes = move(referredNodes);
    // TODO (nodes): associate the Dynamic node with the basic node
}

Node::Node(const Node& other)
    : basicNode_(cloneNode(other.basicNode_))
    , location_(basicNode_->location)
    , children_(basicNode_->children)
    , referredNodes_(basicNode_->referredNodes)
    , properties_(basicNode_->properties)
    , context_(basicNode_->context)
    , childrenContext_(basicNode_->childrenContext)
    , type_(basicNode_->type)
    , explanation_(basicNode_->explanation)
    , modifiers_(basicNode_->modifiers)
{
}

void* Node::operator new(size_t size)
{
    return theCompiler().nodeAllocator().alloc(size);
}
void Node::operator delete(void* ptr)
{
    theCompiler().nodeAllocator().free(ptr);
}

string Node::toString() const
{
    ostringstream oss;
    dump(oss);
    return oss.str();
}


void Node::dump(ostream& os) const
{
    if ( basicNode_->explanation && 0 != strcmp(basicNode_->explanation->nodeKindName(), "Feather.Nop") )
        os << basicNode_->explanation;
    else
    {
        const string* name = getPropertyString("name");
        if ( name )
            os << *name;
        else
        {
            os << nodeKindName() << "(";
            for ( size_t i=0; i<basicNode_->children.size(); ++i )
            {
                if ( i > 0 )
                    os << ", ";
                os << basicNode_->children[i];
            }

            os << ")";
        }
    }
}

void Node::dumpWithType(ostream& os) const
{
    dump(os);
    os << " : " << basicNode_->type;
}


const Location& Node::location() const
{
    return basicNode_->location;
}

void Node::setLocation(const Location& loc)
{
    basicNode_->location = loc;
}

void Node::setProperty(const char* name, int val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propInt, PropertyValue(val), passToExpl);
}
void Node::setProperty(const char* name, string val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propString, PropertyValue(move(val)), passToExpl);
}
void Node::setProperty(const char* name, Node* val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propNode, PropertyValue(val), passToExpl);
}
void Node::setProperty(const char* name, TypeRef val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propType, PropertyValue(val), passToExpl);
}

bool Node::hasProperty(const char* name) const
{
    return basicNode_->properties.find(name) != basicNode_->properties.end();
}
const int* Node::getPropertyInt(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propInt )
        return nullptr;
    return &it->second.value_.intValue_;
}
const string* Node::getPropertyString(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propString )
        return nullptr;
    return it->second.value_.stringValue_;
}
Node*const* Node::getPropertyNode(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propNode )
        return nullptr;
    return &it->second.value_.nodeValue_;
}
const TypeRef* Node::getPropertyType(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propType )
        return nullptr;
    return &it->second.value_.typeValue_;
}

int Node::getCheckPropertyInt(const char* name) const
{
    const int* res = getPropertyInt(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "Node of kind %1% does not have integer property %2%") % nodeKindName() % name;
    return *res;
}
const string& Node::getCheckPropertyString(const char* name) const
{
    const string* res = getPropertyString(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "Node of kind %1% does not have string property %2%") % nodeKindName() % name;
    return *res;
}
Node* Node::getCheckPropertyNode(const char* name) const
{
    Node*const* res = getPropertyNode(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "Node of kind %1% does not have Node property %2%") % nodeKindName() % name;
    return *res;
}
TypeRef Node::getCheckPropertyType(const char* name) const
{
    const TypeRef* res = getPropertyType(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "Node of kind %1% does not have Type property %2%") % nodeKindName() % name;
    return *res;
}


TypeRef Node::type() const
{
    return basicNode_->type;
}

bool Node::isExplained() const
{
    return basicNode_->explanation != nullptr;
}

Node* Node::curExplanation()
{
    return basicNode_->explanation;
}

Node* Node::explanation()
{
    return basicNode_->explanation ? basicNode_->explanation->explanation() : this;
}

void Node::setChildrenContext(CompilationContext* childrenContext)
{
    basicNode_->childrenContext = childrenContext;
}



void Node::setContext(CompilationContext* context)
{
    if ( context == basicNode_->context )
        return;
    ASSERT(context);
    basicNode_->context = context;

    if ( basicNode_->type )
        clearCompilationState();

    for ( Modifier* mod: modifiers_ )
        mod->beforeSetContext(this);

    doSetContextForChildren();

    for ( Modifier* mod: modifiers_ )
        mod->afterSetContext(this);
}

void Node::computeType()
{
    if ( basicNode_->type )
        return;
    if ( basicNode_->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !basicNode_->context )
            REP_INTERNAL(basicNode_->location, "No context associated with node (%1%)") % toString();

        // Check for recursive dependency
        if ( basicNode_->computeTypeStarted )
            REP_ERROR(basicNode_->location, "Recursive dependency detected while computing the type of the current node");
        basicNode_->computeTypeStarted = 1;

        for ( Modifier* mod: modifiers_ )
            mod->beforeComputeType(this);

        // Actually compute the type
        doComputeType();
        if ( !basicNode_->type )
            REP_INTERNAL(basicNode_->location, "Type computed sucessfully, but no actual type was generated");

        for ( Modifier* mod: boost::adaptors::reverse(modifiers_) )
            mod->afterComputeType(this);
    }
    catch (const Nest::Common::CompilationError&)
    {
        basicNode_->nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        basicNode_->nodeError = 1;
        REP_INTERNAL(basicNode_->location, "Exception thrown during computeType(): %1%") % e.what();
    }
}

void Node::semanticCheck()
{
    if ( basicNode_->nodeSemanticallyChecked )
        return;
    if ( basicNode_->nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !basicNode_->context )
            REP_INTERNAL(basicNode_->location, "No context associated with node (%1%)") % toString();

        // Check for recursive dependency
        if ( basicNode_->semanticCheckStarted )
            REP_ERROR(basicNode_->location, "Recursive dependency detected while semantically checking the current node");
        basicNode_->semanticCheckStarted = 1;

        for ( Modifier* mod: modifiers_ )
            mod->beforeSemanticCheck(this);

        // Actually do the semantic check
        doSemanticCheck();
        if ( !basicNode_->type )
            REP_INTERNAL(basicNode_->location, "Node semantically checked, but no actual type was generated");
        basicNode_->nodeSemanticallyChecked = 1;

        for ( Modifier* mod: boost::adaptors::reverse(modifiers_) )
            mod->afterSemanticCheck(this);
    }
    catch (const Nest::Common::CompilationError& e)
    {
        if ( e.what() ) {}      // Avoid warning about not using e
        basicNode_->nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        basicNode_->nodeError = 1;
        REP_INTERNAL(basicNode_->location, "Exception thrown during semanticCheck(): %1%") % e.what();
    }
}

void Node::clearCompilationState()
{
    basicNode_->nodeError = 0;
    basicNode_->nodeSemanticallyChecked = 0;
    basicNode_->computeTypeStarted = 0;
    basicNode_->semanticCheckStarted = 0;
    basicNode_->explanation = nullptr;
    basicNode_->type = nullptr;
    modifiers_.clear();

    for ( Node* p: basicNode_->children )
    {
        if ( p )
            p->clearCompilationState();
    }
}

void Node::addModifier(Modifier* mod)
{
    modifiers_.push_back(mod);
}

bool Node::hasError() const
{
    return 0 != basicNode_->nodeError;
}

bool Node::isSemanticallyChecked() const
{
    return basicNode_->nodeSemanticallyChecked != 0;
}

CompilationContext* Node::context() const
{
    return basicNode_->context;
}

CompilationContext* Node::childrenContext() const
{
    return basicNode_->childrenContext ? basicNode_->childrenContext : basicNode_->context;
}


void Node::doSetContextForChildren()
{
    CompilationContext* childrenCtx = childrenContext();
    for ( Node* child: basicNode_->children )
    {
        if ( child )
            child->setContext(childrenCtx);
    }
}

void Node::doComputeType()
{
    semanticCheck();
}

void Node::doSemanticCheck()
{
    REP_INTERNAL(basicNode_->location, "Don't know how to semantic check a node of kind '%1%'") % nodeKindName();
}

void Node::setExplanation(Node* explanation)
{
    basicNode_->explanation = explanation;

    // Copy all the properties marked accordingly
    for ( const auto& prop : basicNode_->properties )
        if ( prop.second.passToExpl_ )
            basicNode_->explanation->basicNode_->properties[prop.first] = prop.second;

    // Try to semantically check the explanation
    if ( !explanation->isSemanticallyChecked() )
    {
        basicNode_->explanation->setContext(basicNode_->context);
        basicNode_->explanation->semanticCheck();
    }
    basicNode_->type = basicNode_->explanation->type();
}


void Nest::save(const Node& obj, OutArchive& ar)
{
    // TODO (nodes): Make serialization work
//    unsigned char flags = (unsigned char) *reinterpret_cast<const unsigned int*>(&obj.basicNode_->flags);
    ar.write("kind", obj.nodeKind());
    ar.write("kindName", obj.nodeKindName());
//    ar.write("flags", flags);
    ar.write("location", obj.basicNode_->location);
    ar.writeArray("children", obj.basicNode_->children, [] (OutArchive& ar, Node* child) {
        ar.write("", child);
    });
    ar.writeArray("referredNodes", obj.basicNode_->referredNodes, [] (OutArchive& ar, Node* node) {
        ar.write("", node);
    });
    ar.writeArray("properties", obj.basicNode_->properties, [] (OutArchive& ar, const PropertyVal& prop) {
        ar.write("", prop, [] (OutArchive& ar, const PropertyVal& prop) {
            ar.write("name", prop.first);
            ar.write("kind", (int) prop.second.kind_);
            ar.write("passToExpl", (unsigned char) prop.second.passToExpl_);
            switch ( prop.second.kind_ )
            {
            case propInt:
                ar.write("val", prop.second.value_.intValue_);
                break;
            case propString:
                ar.write("val", *prop.second.value_.stringValue_);
                break;
            case propNode:
                ar.write("val", prop.second.value_.nodeValue_);
                break;
            case propType:
                ar.write("val", prop.second.value_.typeValue_);
                break;
            }
        });
    });
    ar.write("type", obj.basicNode_->type);
    ar.write("explanation", obj.basicNode_->explanation);
}

void Nest::load(Node& obj, InArchive& ar)
{
    // TODO
}
