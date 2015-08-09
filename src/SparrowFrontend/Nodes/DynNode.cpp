#include <StdInc.h>
#include "DynNode.h"
#include <Nest/Intermediate/NodeSer.h>
#include <Nest/Common/NodeAllocator.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Nest::Common::Ser;

DynNode::DynNode(int nodeKind, const Location& location, NodeVector children, NodeVector referredNodes)
{
    initNode(&data_, nodeKind);

    data_.location = location;
    data_.children = move(reinterpret_cast<NodeVector&>(children));
    data_.referredNodes = move(reinterpret_cast<NodeVector&>(referredNodes));
}

DynNode::DynNode(const DynNode& other)
{
    initCopyNode(&data_, &other.data_);
}

const DynNode* DynNode::fromNode(const Node* node)
{
    return reinterpret_cast<const DynNode*>(node);
}
DynNode* DynNode::fromNode(Node* node)
{
    return reinterpret_cast<DynNode*>(node);
}


void* DynNode::operator new(size_t size)
{
    return theCompiler().nodeAllocator().alloc(size);
}
void DynNode::operator delete(void* ptr)
{
    theCompiler().nodeAllocator().free(ptr);
}

DynNode* DynNode::clone() const
{
    return new DynNode(*this);
}


const char* DynNode::nodeKindName() const
{
    return getNodeKindName(data_.nodeKind);
}


string DynNode::toString() const
{
    return getToStringFun(data_.nodeKind)(&data_);
}


void DynNode::dump(ostream& os) const
{
    if ( data_.explanation && 0 != strcmp(Nest::nodeKindName(data_.explanation), "SprFrontend.Nop") )
        os << data_.explanation;
    else
    {
        const string* name = getPropertyString("name");
        if ( name )
            os << *name;
        else
        {
            os << nodeKindName() << "(";
            for ( size_t i=0; i<data_.children.size(); ++i )
            {
                if ( i > 0 )
                    os << ", ";
                os << data_.children[i];
            }

            os << ")";
        }
    }
}

void DynNode::dumpWithType(ostream& os) const
{
    dump(os);
    os << " : " << data_.type;
}


const Location& DynNode::location() const
{
    return data_.location;
}

void DynNode::setLocation(const Location& loc)
{
    data_.location = loc;
}

void DynNode::setProperty(const char* name, int val, bool passToExpl)
{
    Nest::setProperty(&data_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, string val, bool passToExpl)
{
    Nest::setProperty(&data_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, Node* val, bool passToExpl)
{
    Nest::setProperty(&data_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, TypeRef val, bool passToExpl)
{
    Nest::setProperty(&data_, name, val, passToExpl);
}

bool DynNode::hasProperty(const char* name) const
{
    return Nest::hasProperty(&data_, name);
}
const int* DynNode::getPropertyInt(const char* name) const
{
    return Nest::getPropertyInt(&data_, name);
}
const string* DynNode::getPropertyString(const char* name) const
{
    return Nest::getPropertyString(&data_, name);
}
Node*const* DynNode::getPropertyNode(const char* name) const
{
    return Nest::getPropertyNode(&data_, name);
}
const TypeRef* DynNode::getPropertyType(const char* name) const
{
    return Nest::getPropertyType(&data_, name);
}

int DynNode::getCheckPropertyInt(const char* name) const
{
    return Nest::getCheckPropertyInt(&data_, name);
}
const string& DynNode::getCheckPropertyString(const char* name) const
{
    return Nest::getCheckPropertyString(&data_, name);
}
Node* DynNode::getCheckPropertyNode(const char* name) const
{
    return Nest::getCheckPropertyNode(&data_, name);
}
TypeRef DynNode::getCheckPropertyType(const char* name) const
{
    return Nest::getCheckPropertyType(&data_, name);
}


TypeRef DynNode::type() const
{
    return data_.type;
}

bool DynNode::isExplained() const
{
    return data_.explanation != nullptr;
}

Node* DynNode::curExplanation()
{
    return data_.explanation;
}

Node* DynNode::explanation()
{
    return Nest::explanation(&data_);
}

void DynNode::setChildrenContext(CompilationContext* childrenContext)
{
    data_.childrenContext = childrenContext;
}



void DynNode::setContext(CompilationContext* context)
{
    Nest::setContext(&data_, context);
}

void DynNode::computeType()
{
    Nest::computeType(&data_);
}

void DynNode::semanticCheck()
{
    Nest::semanticCheck(&data_);
}

void DynNode::clearCompilationState()
{
    Nest::clearCompilationState(&data_);
}

void DynNode::addModifier(Modifier* mod)
{
    Nest::addModifier(&data_, mod);
}

bool DynNode::hasError() const
{
    return 0 != data_.nodeError;
}

bool DynNode::isSemanticallyChecked() const
{
    return data_.nodeSemanticallyChecked != 0;
}

CompilationContext* DynNode::context() const
{
    return data_.context;
}

CompilationContext* DynNode::childrenContext() const
{
    return Nest::childrenContext(&data_);
}


void DynNode::doSetContextForChildren()
{
    CompilationContext* childrenCtx = childrenContext();
    for ( Node* child: data_.children )
    {
        if ( child )
            Nest::setContext(child, childrenCtx);
    }
}

void DynNode::doComputeType()
{
    semanticCheck();
}

void DynNode::doSemanticCheck()
{
    REP_INTERNAL(data_.location, "Don't know how to semantic check a node of kind '%1%'") % nodeKindName();
}

void DynNode::setExplanation(Node* explanation)
{
    Nest::setExplanation(&data_, explanation);
}


void SprFrontend::save(const DynNode& obj, OutArchive& ar)
{
    Nest::save(*obj.node(), ar);
}

void SprFrontend::load(DynNode& obj, InArchive& ar)
{
    Nest::load(*obj.node(), ar);
}
