#include <StdInc.h>
#include "DynNode.h"
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

DynNode::DynNode(int nodeKind, const Location& location, DynNodeVector children, DynNodeVector referredNodes)
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

DynNode::DynNode(const DynNode& other)
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

void* DynNode::operator new(size_t size)
{
    return theCompiler().nodeAllocator().alloc(size);
}
void DynNode::operator delete(void* ptr)
{
    theCompiler().nodeAllocator().free(ptr);
}

string DynNode::toString() const
{
    ostringstream oss;
    dump(oss);
    return oss.str();
}


void DynNode::dump(ostream& os) const
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

void DynNode::dumpWithType(ostream& os) const
{
    dump(os);
    os << " : " << basicNode_->type;
}


const Location& DynNode::location() const
{
    return basicNode_->location;
}

void DynNode::setLocation(const Location& loc)
{
    basicNode_->location = loc;
}

void DynNode::setProperty(const char* name, int val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propInt, PropertyValue(val), passToExpl);
}
void DynNode::setProperty(const char* name, string val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propString, PropertyValue(move(val)), passToExpl);
}
void DynNode::setProperty(const char* name, DynNode* val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propNode, PropertyValue(val), passToExpl);
}
void DynNode::setProperty(const char* name, TypeRef val, bool passToExpl)
{
    basicNode_->properties[name] = Property(propType, PropertyValue(val), passToExpl);
}

bool DynNode::hasProperty(const char* name) const
{
    return basicNode_->properties.find(name) != basicNode_->properties.end();
}
const int* DynNode::getPropertyInt(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propInt )
        return nullptr;
    return &it->second.value_.intValue_;
}
const string* DynNode::getPropertyString(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propString )
        return nullptr;
    return it->second.value_.stringValue_;
}
DynNode*const* DynNode::getPropertyNode(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propNode )
        return nullptr;
    return &it->second.value_.nodeValue_;
}
const TypeRef* DynNode::getPropertyType(const char* name) const
{
    auto it = basicNode_->properties.find(name);
    if ( it == basicNode_->properties.end() || it->second.kind_ != propType )
        return nullptr;
    return &it->second.value_.typeValue_;
}

int DynNode::getCheckPropertyInt(const char* name) const
{
    const int* res = getPropertyInt(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "DynNode of kind %1% does not have integer property %2%") % nodeKindName() % name;
    return *res;
}
const string& DynNode::getCheckPropertyString(const char* name) const
{
    const string* res = getPropertyString(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "DynNode of kind %1% does not have string property %2%") % nodeKindName() % name;
    return *res;
}
DynNode* DynNode::getCheckPropertyNode(const char* name) const
{
    DynNode*const* res = getPropertyNode(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "DynNode of kind %1% does not have DynNode property %2%") % nodeKindName() % name;
    return *res;
}
TypeRef DynNode::getCheckPropertyType(const char* name) const
{
    const TypeRef* res = getPropertyType(name);
    if ( !res )
        REP_INTERNAL(basicNode_->location, "DynNode of kind %1% does not have Type property %2%") % nodeKindName() % name;
    return *res;
}


TypeRef DynNode::type() const
{
    return basicNode_->type;
}

bool DynNode::isExplained() const
{
    return basicNode_->explanation != nullptr;
}

DynNode* DynNode::curExplanation()
{
    return basicNode_->explanation;
}

DynNode* DynNode::explanation()
{
    return basicNode_->explanation ? basicNode_->explanation->explanation() : this;
}

void DynNode::setChildrenContext(CompilationContext* childrenContext)
{
    basicNode_->childrenContext = childrenContext;
}



void DynNode::setContext(CompilationContext* context)
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

void DynNode::computeType()
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

void DynNode::semanticCheck()
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
            REP_INTERNAL(basicNode_->location, "DynNode semantically checked, but no actual type was generated");
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

void DynNode::clearCompilationState()
{
    basicNode_->nodeError = 0;
    basicNode_->nodeSemanticallyChecked = 0;
    basicNode_->computeTypeStarted = 0;
    basicNode_->semanticCheckStarted = 0;
    basicNode_->explanation = nullptr;
    basicNode_->type = nullptr;
    modifiers_.clear();

    for ( DynNode* p: basicNode_->children )
    {
        if ( p )
            p->clearCompilationState();
    }
}

void DynNode::addModifier(Modifier* mod)
{
    modifiers_.push_back(mod);
}

bool DynNode::hasError() const
{
    return 0 != basicNode_->nodeError;
}

bool DynNode::isSemanticallyChecked() const
{
    return basicNode_->nodeSemanticallyChecked != 0;
}

CompilationContext* DynNode::context() const
{
    return basicNode_->context;
}

CompilationContext* DynNode::childrenContext() const
{
    return basicNode_->childrenContext ? basicNode_->childrenContext : basicNode_->context;
}


void DynNode::doSetContextForChildren()
{
    CompilationContext* childrenCtx = childrenContext();
    for ( DynNode* child: basicNode_->children )
    {
        if ( child )
            child->setContext(childrenCtx);
    }
}

void DynNode::doComputeType()
{
    semanticCheck();
}

void DynNode::doSemanticCheck()
{
    REP_INTERNAL(basicNode_->location, "Don't know how to semantic check a node of kind '%1%'") % nodeKindName();
}

void DynNode::setExplanation(DynNode* explanation)
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


void Nest::save(const DynNode& obj, OutArchive& ar)
{
    // TODO (nodes): Make serialization work
//    unsigned char flags = (unsigned char) *reinterpret_cast<const unsigned int*>(&obj.basicNode_->flags);
    ar.write("kind", obj.nodeKind());
    ar.write("kindName", obj.nodeKindName());
//    ar.write("flags", flags);
    ar.write("location", obj.basicNode_->location);
    ar.writeArray("children", obj.basicNode_->children, [] (OutArchive& ar, DynNode* child) {
        ar.write("", child);
    });
    ar.writeArray("referredNodes", obj.basicNode_->referredNodes, [] (OutArchive& ar, DynNode* node) {
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

void Nest::load(DynNode& obj, InArchive& ar)
{
    // TODO
}
