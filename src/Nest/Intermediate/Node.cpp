#include <StdInc.h>
#include "Node.h"
#include "Type.h"
#include "CompilationContext.h"
#include "Modifier.h"
#include <Compiler.h>
#include <Common/Diagnostic.h>
#include <Common/NodeAllocator.h>

using namespace Nest;

Node::Node(const Location& location, NodeVector children, NodeVector referredNodes)
    : location_(location)
    , children_(move(children))
    , referredNodes_(move(referredNodes))
    , context_(nullptr)
    , childrenContext_(nullptr)
    , type_(nullptr)
    , explanation_(nullptr)
{
    memset(&flags_, 0, sizeof(flags_));
}

Node::Node(const Node& other)
    : location_(other.location_)
    , children_()
    , referredNodes_(other.referredNodes_)
    , properties_(other.properties_)
    , context_(nullptr)
    , childrenContext_(nullptr)
    , type_(nullptr)
    , explanation_(nullptr)
{
    memset(&flags_, 0, sizeof(flags_));

    // Clone the children vector
    size_t size = other.children_.size();
    children_.resize(size, nullptr);
    for ( size_t i=0; i<size; ++i )
    {
        Node* n = other.children_[i];
        if ( n )
            children_[i] = n->clone();
    }
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
    if ( explanation_ && 0 != strcmp(explanation_->nodeKindName(), "Feather.Nop") )
        os << explanation_;
    else
    {
        const string* name = getPropertyString("name");
        if ( name )
            os << *name;
        else
        {
            os << nodeKindName() << "(";
            for ( size_t i=0; i<children_.size(); ++i )
            {
                if ( i > 0 )
                    os << ", ";
                os << children_[i];
            }

            os << ")";
        }
    }
}

void Node::dumpWithType(ostream& os) const
{
    dump(os);
    os << " : " << type_;
}


const Location& Node::location() const
{
    return location_;
}

void Node::setLocation(const Location& loc)
{
    location_ = loc;
}

void Node::setProperty(const char* name, int val)
{
    properties_[name] = PropertyValue(val);
}
void Node::setProperty(const char* name, string val)
{
    properties_[name] = PropertyValue(move(val));
}
void Node::setProperty(const char* name, Node* val)
{
    properties_[name] = PropertyValue(val);
}
void Node::setProperty(const char* name, Type* val)
{
    properties_[name] = PropertyValue(val);
}

bool Node::hasProperty(const char* name) const
{
    return properties_.find(name) != properties_.end();
}
const int* Node::getPropertyInt(const char* name) const
{
    auto it = properties_.find(name);
    return it == properties_.end() ? nullptr : &it->second.intValue_;
}
const string* Node::getPropertyString(const char* name) const
{
    auto it = properties_.find(name);
    return it == properties_.end() ? nullptr : it->second.stringValue_;
}
Node*const* Node::getPropertyNode(const char* name) const
{
    auto it = properties_.find(name);
    return it == properties_.end() ? nullptr : &it->second.nodeValue_;
}
Type*const* Node::getPropertyType(const char* name) const
{
    auto it = properties_.find(name);
    return it == properties_.end() ? nullptr : &it->second.typeValue_;
}

int Node::getCheckPropertyInt(const char* name) const
{
    const int* res = getPropertyInt(name);
    if ( !res )
        REP_INTERNAL(location_, "Node of kind %1% does not have integer property %2%") % nodeKindName() % name;
    return *res;
}
const string& Node::getCheckPropertyString(const char* name) const
{
    const string* res = getPropertyString(name);
    if ( !res )
        REP_INTERNAL(location_, "Node of kind %1% does not have string property %2%") % nodeKindName() % name;
    return *res;
}
Node* Node::getCheckPropertyNode(const char* name) const
{
    Node*const* res = getPropertyNode(name);
    if ( !res )
        REP_INTERNAL(location_, "Node of kind %1% does not have Node property %2%") % nodeKindName() % name;
    return *res;
}
Type* Node::getCheckPropertyType(const char* name) const
{
    Type*const* res = getPropertyType(name);
    if ( !res )
        REP_INTERNAL(location_, "Node of kind %1% does not have Type property %2%") % nodeKindName() % name;
    return *res;
}


Type* Node::type() const
{
    return type_;
}

bool Node::isExplained() const
{
    return explanation_ != nullptr;
}

Node* Node::curExplanation()
{
    return explanation_;
}

Node* Node::explanation()
{
    return explanation_ ? explanation_->explanation() : this;
}

void Node::setChildrenContext(CompilationContext* childrenContext)
{
    childrenContext_ = childrenContext;
}



void Node::setContext(CompilationContext* context)
{
    if ( context == context_ )
        return;
    ASSERT(context);
    context_ = context;

    if ( type_ )
        clearCompilationState();

    for ( Modifier* mod: modifiers_ )
        mod->beforeSetContext(this);

    doSetContextForChildren();

    for ( Modifier* mod: modifiers_ )
        mod->afterSetContext(this);
}

void Node::computeType()
{
    if ( type_ )
        return;
    if ( flags_.nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !context_ )
            REP_INTERNAL(location_, "No context associated with node (%1%)") % toString();

        // Check for recursive dependency
        if ( flags_.computeTypeStarted )
            REP_ERROR(location_, "Recursive dependency detected while computing the type of the current node");
        flags_.computeTypeStarted = 1;

        for ( Modifier* mod: modifiers_ )
            mod->beforeComputeType(this);

        // Actually compute the type
        doComputeType();
        if ( !type_ )
            REP_INTERNAL(location_, "Type computed sucessfully, but no actual type was generated");

        for ( Modifier* mod: boost::adaptors::reverse(modifiers_) )
            mod->afterComputeType(this);
    }
    catch (const Nest::Common::CompilationError&)
    {
        flags_.nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        flags_.nodeError = 1;
        REP_INTERNAL(location_, "Exception thrown during computeType(): %1%") % e.what();
    }
}

void Node::semanticCheck()
{
    if ( flags_.nodeSemanticallyChecked )
        return;
    if ( flags_.nodeError )
        REP_ERROR_THROW("Already marked as having an error");

    try
    {
        if ( !context_ )
            REP_INTERNAL(location_, "No context associated with node (%1%)") % toString();

        // Check for recursive dependency
        if ( flags_.semanticCheckStarted )
            REP_ERROR(location_, "Recursive dependency detected while semantically checking the current node");
        flags_.semanticCheckStarted = 1;

        for ( Modifier* mod: modifiers_ )
            mod->beforeSemanticCheck(this);

        // Actually do the semantic check
        doSemanticCheck();
        if ( !type_ )
            REP_INTERNAL(location_, "Node semantically checked, but no actual type was generated");
        flags_.nodeSemanticallyChecked = 1;

        for ( Modifier* mod: boost::adaptors::reverse(modifiers_) )
            mod->afterSemanticCheck(this);
    }
    catch (const Nest::Common::CompilationError& e)
    {
        if ( e.what() ) {}      // Avoid warning about not using e
        flags_.nodeError = 1;
        throw;
    }
    catch (const exception& e)
    {
        flags_.nodeError = 1;
        REP_INTERNAL(location_, "Exception thrown during semanticCheck(): %1%") % e.what();
    }
}

void Node::clearCompilationState()
{
    memset(&flags_, 0, sizeof(flags_));
    explanation_ = nullptr;
    type_ = nullptr;
    modifiers_.clear();

    for ( Node* p: children_ )
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
    return 0 != flags_.nodeError;
}

bool Node::isSemanticallyChecked() const
{
    return flags_.nodeSemanticallyChecked != 0;
}

CompilationContext* Node::context() const
{
    return context_;
}

CompilationContext* Node::childrenContext() const
{
    return childrenContext_ ? childrenContext_ : context_;
}


void Node::doSetContextForChildren()
{
    CompilationContext* childrenCtx = childrenContext();
    for ( Node* child: children_ )
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
    REP_INTERNAL(location_, "Don't know how to semantic check a node of kind '%1%'") % nodeKindName();
}

void Node::setExplanation(Node* explanation)
{
    explanation_ = explanation;
    if ( !explanation->isSemanticallyChecked() )
    {
        explanation_->setContext(context_);
        explanation_->semanticCheck();
    }
    type_ = explanation_->type();
}
