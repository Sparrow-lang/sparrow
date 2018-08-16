#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/PtrArray.h"

using Nest::Node;
using Nest::NodeHandle;
using Nest::StringRef;
using Nest::TypeRef;

namespace Nest {

namespace {
Nest_NodeProperty* _findProperty(Nest_NodeProperties properties, StringRef name) {
    Nest_NodeProperty* p = properties.begin;
    for (; p != properties.end; ++p)
        if (p->name == name)
            return p;
    return nullptr;
}

} // namespace

NodeHandle NodeHandle::create(int nodeKind, const Location& loc) {
    Nest::Node* res = Nest_createNode(nodeKind);
    res->location = loc;
    return {res};
}

NodeHandle NodeHandle::clone() { return Nest_cloneNode(handle); }

void NodeHandle::setContext(CompilationContext* context) { Nest_setContext(handle, context); }

TypeRef NodeHandle::computeType() { return Nest_computeType(handle); }

NodeHandle NodeHandle::semanticCheck() { return Nest_semanticCheck(handle); }

void NodeHandle::clearCompilationStateSimple() { Nest_clearCompilationStateSimple(handle); }
void NodeHandle::clearCompilationState() { Nest_clearCompilationState(handle); }

const char* NodeHandle::toString() const { return Nest_getToStringFun(handle->nodeKind)(handle); }
const char* NodeHandle::toStringEx() const {
    ostringstream ss;
    ss << toString();
    if (handle->type)
        ss << ": " << handle->type;
    ss << '(';
    ss << Nest_getNodeKindName(handle->nodeKind);
    if (handle->explanation && handle->explanation != handle)
        ss << " -> " << Nest_getNodeKindName(handle->explanation->nodeKind);
    ss << ')';

    // If we have some properties, also write those
    if (handle->properties.end != handle->properties.begin) {
        ss << '[';
        Nest_NodeProperty* p = handle->properties.begin;
        for (; p != handle->properties.end; ++p) {
            if (p != handle->properties.begin)
                ss << ", ";
            ss << p->name << '=';
            switch (p->kind) {
            case propInt:
                ss << p->value.intValue;
                break;
            case propString:
                ss << p->value.stringValue;
                break;
            case propNode:
                ss << Nest_toStringEx(p->value.nodeValue);
                break;
            case propType:
                ss << p->value.typeValue;
                break;
            case propPtr:
                ss << p->value.ptrValue;
                break;
            }
        }
        ss << ']';
    }
    return strdup(ss.str().c_str());
}
const char* NodeHandle::kindName() const { return Nest_getNodeKindName(handle->nodeKind); }

NodeRange NodeHandle::children() const { return all(handle->children); }
NodeRangeM NodeHandle::childrenM() const { return allM(handle->children); }

NodeRange NodeHandle::referredNodes() const { return all(handle->referredNodes); }
NodeRangeM NodeHandle::referredNodesM() const { return allM(handle->referredNodes); }

void NodeHandle::setChildren(NodeRange children) {
    unsigned size = Nest_nodeRangeSize(children);
    Nest_resizeNodeArray(&handle->children, size);
    for (unsigned i = 0; i < size; ++i)
        at(handle->children, i) = at(children, i);
}
void NodeHandle::addChild(NodeHandle child) {
    Nest_appendNodeToArray(&handle->children, child.handle);
}
void NodeHandle::addChildren(NodeRange children) {
    Nest_appendNodesToArray(&handle->children, children);
}

void NodeHandle::setReferredNodes(NodeRange nodes) {
    unsigned size = Nest_nodeRangeSize(nodes);
    Nest_resizeNodeArray(&handle->referredNodes, size);
    for (unsigned i = 0; i < size; ++i)
        at(handle->referredNodes, i) = at(nodes, i);
}

void NodeHandle::setProperty(Nest_NodeProperty prop) { Nest_setProperty(handle, prop); }
void NodeHandle::setProperty(const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 0, {0}};
    prop.value.intValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setProperty(const char* name, StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 0, {0}};
    prop.value.stringValue = StringRef(val).dup();
    Nest_setProperty(handle, prop);
}
void NodeHandle::setProperty(const char* name, NodeHandle val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 0, {0}};
    prop.value.nodeValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setProperty(const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 0, {0}};
    prop.value.typeValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setProperty(const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 0, {0}};
    prop.value.ptrValue = val;
    Nest_setProperty(handle, prop);
}

void NodeHandle::setPropertyExpl(const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 1, {0}};
    prop.value.intValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setPropertyExpl(const char* name, StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 1, {0}};
    prop.value.stringValue = StringRef(val).dup();
    Nest_setProperty(handle, prop);
}
void NodeHandle::setPropertyExpl(const char* name, NodeHandle val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 1, {0}};
    prop.value.nodeValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setPropertyExpl(const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 1, {0}};
    prop.value.typeValue = val;
    Nest_setProperty(handle, prop);
}
void NodeHandle::setPropertyExpl(const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 1, {0}};
    prop.value.ptrValue = val;
    Nest_setProperty(handle, prop);
}

bool NodeHandle::hasProperty(const char* name) const {
    return nullptr != _findProperty(handle->properties, name);
}
const int* NodeHandle::getPropertyInt(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propInt)
        return nullptr;
    return &p->value.intValue;
}
const StringRef* NodeHandle::getPropertyString(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propString)
        return nullptr;
    return reinterpret_cast<const StringRef*>(&p->value.stringValue);
}
StringRef NodeHandle::getPropertyStringDeref(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propString)
        return StringRef{nullptr, nullptr};
    return p->value.stringValue;
}
const NodeHandle* NodeHandle::getPropertyNode(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propNode)
        return nullptr;
    return reinterpret_cast<const NodeHandle*>(&p->value.nodeValue);
}
const TypeRef* NodeHandle::getPropertyType(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propType)
        return nullptr;
    return &p->value.typeValue;
}
void* const* NodeHandle::getPropertyPtr(const char* name) const {
    Nest_NodeProperty* p = _findProperty(handle->properties, name);
    if (!p || p->kind != propPtr)
        return nullptr;
    return &p->value.ptrValue;
}

int NodeHandle::getPropertyDefaultInt(const char* name, int defaultVal) const {
    const int* res = getPropertyInt(name);
    return res ? *res : defaultVal;
}
StringRef NodeHandle::getPropertyDefaultString(const char* name, StringRef defaultVal) const {
    const StringRef* res = getPropertyString(name);
    return res ? *res : defaultVal;
}
NodeHandle NodeHandle::getPropertyDefaultNode(const char* name, NodeHandle defaultVal) const {
    const NodeHandle* res = getPropertyNode(name);
    return res ? *res : defaultVal;
}
TypeRef NodeHandle::getPropertyDefaultType(const char* name, TypeRef defaultVal) const {
    const TypeRef* res = getPropertyType(name);
    return res ? *res : defaultVal;
}
void* NodeHandle::getPropertyDefaultPtr(const char* name, void* defaultVal) const {
    void* const* res = getPropertyPtr(name);
    return res ? *res : defaultVal;
}

int NodeHandle::getCheckPropertyInt(const char* name) const {
    const int* res = getPropertyInt(name);
    if (!res)
        REP_INTERNAL(handle->location, "Node of kind %1% does not have integer property %2%") %
                Nest_nodeKindName(handle) % name;
    return *res;
}
StringRef NodeHandle::getCheckPropertyString(const char* name) const {
    const StringRef* res = getPropertyString(name);
    if (!res)
        REP_INTERNAL(handle->location, "Node of kind %1% does not have string property %2%") %
                Nest_nodeKindName(handle) % name;
    return *res;
}
NodeHandle NodeHandle::getCheckPropertyNode(const char* name) const {
    const NodeHandle* res = getPropertyNode(name);
    if (!res)
        REP_INTERNAL(handle->location, "Node of kind %1% does not have Node property %2%") %
                Nest_nodeKindName(handle) % name;
    return *res;
}
TypeRef NodeHandle::getCheckPropertyType(const char* name) const {
    const TypeRef* res = getPropertyType(name);
    if (!res)
        REP_INTERNAL(handle->location, "Node of kind %1% does not have Type property %2%") %
                Nest_nodeKindName(handle) % name;
    return *res;
}
void* NodeHandle::getCheckPropertyPtr(const char* name) const {
    void* const* res = getPropertyPtr(name);
    if (!res)
        REP_INTERNAL(handle->location, "Node of kind %1% does not have Ptr property %2%") %
                Nest_nodeKindName(handle) % name;
    return *res;
}

void NodeHandle::addModifier(Nest_Modifier* mod) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    NestUtils_appendObjectToPtrArray((NestUtils_PtrArray*)&handle->modifiers, mod);
}

CompilationContext* NodeHandle::childrenContext() const {
    return handle->childrenContext ? handle->childrenContext : handle->context;
}

NodeHandle NodeHandle::explanation() {
    return handle && handle->explanation && handle->explanation != handle
                   ? NodeHandle(handle->explanation).explanation()
                   : *this;
}
TypeRef NodeHandle::computeTypeImpl() {
    semanticCheck();
    return type();
}
void NodeHandle::setContextForChildrenImpl() {
    // Set the children context to all of the children
    CompilationContext* childrenCtx = childrenContext();
    for (auto child: children())
        if (child)
            child.setContext(childrenCtx);
}
const char* NodeHandle::toStringImpl() {
    return Nest_defaultFunToString(handle);
}

ostream& operator<<(ostream& os, NodeHandle n) {
    if (n)
        os << n.toString();
    else
        os << "<null>";
    return os;
}

} // namespace Nest
