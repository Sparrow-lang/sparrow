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

namespace
{
    static unordered_map<const Node*, DynNode*> nodesAssoc;
}

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

    // Associate the basic node with this dynamic node
    nodesAssoc.insert(make_pair(basicNode_, this));
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
    // Associate the basic node with this dynamic node
    nodesAssoc.insert(make_pair(basicNode_, this));
}

DynNode* DynNode::fromNode(const Node* node)
{
    return nodesAssoc[node];
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
    Nest::setProperty(basicNode_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, string val, bool passToExpl)
{
    Nest::setProperty(basicNode_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, DynNode* val, bool passToExpl)
{
    Nest::setProperty(basicNode_, name, val, passToExpl);
}
void DynNode::setProperty(const char* name, TypeRef val, bool passToExpl)
{
    Nest::setProperty(basicNode_, name, val, passToExpl);
}

bool DynNode::hasProperty(const char* name) const
{
    return Nest::hasProperty(basicNode_, name);
}
const int* DynNode::getPropertyInt(const char* name) const
{
    return Nest::getPropertyInt(basicNode_, name);
}
const string* DynNode::getPropertyString(const char* name) const
{
    return Nest::getPropertyString(basicNode_, name);
}
DynNode*const* DynNode::getPropertyNode(const char* name) const
{
    return Nest::getPropertyNode(basicNode_, name);
}
const TypeRef* DynNode::getPropertyType(const char* name) const
{
    return Nest::getPropertyType(basicNode_, name);
}

int DynNode::getCheckPropertyInt(const char* name) const
{
    return Nest::getCheckPropertyInt(basicNode_, name);
}
const string& DynNode::getCheckPropertyString(const char* name) const
{
    return Nest::getCheckPropertyString(basicNode_, name);
}
DynNode* DynNode::getCheckPropertyNode(const char* name) const
{
    return Nest::getCheckPropertyNode(basicNode_, name);
}
TypeRef DynNode::getCheckPropertyType(const char* name) const
{
    return Nest::getCheckPropertyType(basicNode_, name);
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
    return Nest::explanation(basicNode_);
}

void DynNode::setChildrenContext(CompilationContext* childrenContext)
{
    basicNode_->childrenContext = childrenContext;
}



void DynNode::setContext(CompilationContext* context)
{
    Nest::setContext(basicNode_, context);
}

void DynNode::computeType()
{
    Nest::computeType(basicNode_);
}

void DynNode::semanticCheck()
{
    Nest::semanticCheck(basicNode_);
}

void DynNode::clearCompilationState()
{
    Nest::clearCompilationState(basicNode_);
}

void DynNode::addModifier(Modifier* mod)
{
    Nest::addModifier(basicNode_, mod);
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
    return Nest::childrenContext(basicNode_);
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
    Nest::setExplanation(basicNode_, explanation);
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
