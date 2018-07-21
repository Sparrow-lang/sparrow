#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/PtrArray.h"

using Nest::NodeHandle;
using Nest::StringRef;
using Nest::TypeRef;
using Nest::Node;

Nest_NodeProperty* _findProperty(Nest_NodeProperties properties, StringRef name) {
    Nest_NodeProperty* p = properties.begin;
    for (; p != properties.end; ++p)
        if (p->name == name)
            return p;
    return nullptr;
}

void _setProperty(Nest_NodeProperties* properties, Nest_NodeProperty prop) {
    // Try to see if a property with the same name already exists
    Nest_NodeProperty* p = _findProperty(*properties, prop.name);
    if (p) {
        *p = prop;
    } else {
        Nest_addProperty(properties, prop);
    }
}

const char* Nest_toString(Node* node) {
    return NodeHandle(node).toString();
}

const char* Nest_toStringEx(Node* node) {
    return NodeHandle(node).toStringEx();
}

const char* Nest_nodeKindName(Node* node) {
    return NodeHandle(node).kindName();
}

Nest_NodeRange Nest_nodeChildren(Node* node) { return NodeHandle(node).children(); }

Node* Nest_getNodeChild(Node* node, unsigned int index) {
    return NodeHandle(node).children()[index];
}

Nest_NodeRange Nest_nodeReferredNodes(Node* node) { return NodeHandle(node).referredNodes(); }

Node* Nest_getReferredNode(Node* node, unsigned int index) {
    return NodeHandle(node).referredNodes()[index];
}

void Nest_nodeSetChildren(Node* node, Nest_NodeRange children) {
    NodeHandle(node).setChildren(children);
}

void Nest_nodeAddChild(Node* node, Node* child) { NodeHandle(node).addChild(child); }

void Nest_nodeAddChildren(Node* node, Nest_NodeRange children) {
    NodeHandle(node).addChildren(children);
}

void Nest_nodeSetReferredNodes(Node* node, Nest_NodeRange nodes) {
    NodeHandle(node).setReferredNodes(nodes);
}

void Nest_setProperty(Node* node, Nest_NodeProperty prop) { _setProperty(&node->properties, prop); }

void Nest_setPropertyInt(Node* node, const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 0, {0}};
    prop.value.intValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyString(Node* node, const char* name, Nest_StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 0, {0}};
    prop.value.stringValue = StringRef(val).dup();
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyNode(Node* node, const char* name, Node* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 0, {0}};
    prop.value.nodeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyType(Node* node, const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 0, {0}};
    prop.value.typeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyPtr(Node* node, const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 0, {0}};
    prop.value.ptrValue = val;
    _setProperty(&node->properties, prop);
}

void Nest_setPropertyExplInt(Node* node, const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 1, {0}};
    prop.value.intValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplString(Node* node, const char* name, StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 1, {0}};
    prop.value.stringValue = StringRef(val).dup();
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplNode(Node* node, const char* name, Node* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 1, {0}};
    prop.value.nodeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplType(Node* node, const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 1, {0}};
    prop.value.typeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplPtr(Node* node, const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 1, {0}};
    prop.value.ptrValue = val;
    _setProperty(&node->properties, prop);
}

int Nest_hasProperty(Node* node, const char* name) {
    return nullptr != _findProperty(node->properties, name);
}
const int* Nest_getPropertyInt(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propInt)
        return nullptr;
    return &p->value.intValue;
}
const Nest_StringRef* Nest_getPropertyString(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propString)
        return nullptr;
    return &p->value.stringValue;
}
Nest_StringRef Nest_getPropertyStringDeref(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propString)
        return StringRef{nullptr, nullptr};
    return p->value.stringValue;
}
Node* const* Nest_getPropertyNode(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propNode)
        return nullptr;
    return &p->value.nodeValue;
}
const TypeRef* Nest_getPropertyType(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propType)
        return nullptr;
    return &p->value.typeValue;
}
void* const* Nest_getPropertyPtr(Node* node, const char* name) {
    Nest_NodeProperty* p = _findProperty(node->properties, name);
    if (!p || p->kind != propPtr)
        return nullptr;
    return &p->value.ptrValue;
}

int Nest_getPropertyDefaultInt(Node* node, const char* name, int defaultVal) {
    const int* res = Nest_getPropertyInt(node, name);
    return res ? *res : defaultVal;
}
Nest_StringRef Nest_getPropertyDefaultString(Node* node, const char* name, StringRef defaultVal) {
    const Nest_StringRef* res = Nest_getPropertyString(node, name);
    return res ? *res : defaultVal;
}
Node* Nest_getPropertyDefaultNode(Node* node, const char* name, Node* defaultVal) {
    Node* const* res = Nest_getPropertyNode(node, name);
    return res ? *res : defaultVal;
}
TypeRef Nest_getPropertyDefaultType(Node* node, const char* name, TypeRef defaultVal) {
    const TypeRef* res = Nest_getPropertyType(node, name);
    return res ? *res : defaultVal;
}
void* Nest_getPropertyDefaultPtr(Node* node, const char* name, void* defaultVal) {
    void* const* res = Nest_getPropertyPtr(node, name);
    return res ? *res : defaultVal;
}

int Nest_getCheckPropertyInt(Node* node, const char* name) {
    const int* res = Nest_getPropertyInt(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have integer property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
Nest_StringRef Nest_getCheckPropertyString(Node* node, const char* name) {
    const Nest_StringRef* res = Nest_getPropertyString(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have string property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
Node* Nest_getCheckPropertyNode(Node* node, const char* name) {
    Node* const* res = Nest_getPropertyNode(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have Node property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
TypeRef Nest_getCheckPropertyType(Node* node, const char* name) {
    const TypeRef* res = Nest_getPropertyType(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have Type property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
void* Nest_getCheckPropertyPtr(Node* node, const char* name) {
    void* const* res = Nest_getPropertyPtr(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have Ptr property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}

void Nest_addModifier(Node* node, Nest_Modifier* mod) { NodeHandle(node).addModifier(mod); }

Nest_CompilationContext* Nest_childrenContext(Node* node) {
    return NodeHandle(node).childrenContext();
}

Node* Nest_explanation(Node* node) { return NodeHandle(node).explanation(); }

Node* Nest_ofKind(Node* src, int desiredNodeKind) {
    return src && src->nodeKind == desiredNodeKind ? src : nullptr;
}

Nest_NodeArray _fromPtrArray(NestUtils_PtrArray arr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    Nest_NodeArray res = {(Node**)arr.beginPtr, (Node**)arr.endPtr, (Node**)arr.endOfStorePtr};
    return res;
}
NestUtils_PtrArray _toPtrArray(Nest_NodeArray arr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    NestUtils_PtrArray res = {(void**)arr.beginPtr, (void**)arr.endPtr, (void**)arr.endOfStorePtr};
    return res;
}

Nest_NodeArray Nest_allocNodeArray(unsigned capacity) {
    return _fromPtrArray(NestUtils_allocPtrArray(capacity));
}

void Nest_freeNodeArray(Nest_NodeArray arr) { NestUtils_freePtrArray(_toPtrArray(arr)); }

void Nest_reserveNodeArray(Nest_NodeArray* arr, unsigned capacity) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_reservePtrArray(&arr2, capacity);
    *arr = _fromPtrArray(arr2);
}

void Nest_resizeNodeArray(Nest_NodeArray* arr, unsigned size) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_resizePtrArray(&arr2, size);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodeToArray(Nest_NodeArray* arr, Node* node) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_appendObjectToPtrArray(&arr2, node);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodesToArray(Nest_NodeArray* arr, Nest_NodeRange nodes) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    Nest_PtrRange objects = {(void**)nodes.beginPtr, (void**)nodes.endPtr};
    NestUtils_appendObjectsToPtrArray(&arr2, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_insertNodeIntoArray(Nest_NodeArray* arr, unsigned index, Node* node) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_insertObjectIntoPtrArray(&arr2, index, node);
    *arr = _fromPtrArray(arr2);
}
void Nest_insertNodesIntoArray(Nest_NodeArray* arr, unsigned index, Nest_NodeRange nodes) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    Nest_PtrRange objects = {(void**)nodes.beginPtr, (void**)nodes.endPtr};
    NestUtils_insertObjectsIntoPtrArray(&arr2, index, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_eraseNodeFromArray(Nest_NodeArray* arr, unsigned index) {
    NestUtils_PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_eraseFromPtrArray(&arr2, index);
    *arr = _fromPtrArray(arr2);
}

Nest_NodeRange Nest_getNodeRangeFromArray(Nest_NodeArray arr) {
    Nest_NodeRange res = {arr.beginPtr, arr.endPtr};
    return res;
}

unsigned Nest_nodeArraySize(Nest_NodeArray arr) { return arr.endPtr - arr.beginPtr; }
unsigned Nest_nodeArrayCapacity(Nest_NodeArray arr) { return arr.endOfStorePtr - arr.beginPtr; }

unsigned Nest_nodeRangeSize(Nest_NodeRange nodes) { return nodes.endPtr - nodes.beginPtr; }
unsigned Nest_nodeRangeSize(Nest_NodeRangeM nodes) { return nodes.endPtr - nodes.beginPtr; }

Nest_NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned count) {
    Nest_NodeRange res = {nodes, nodes + count};
    return res;
}

namespace Nest {

NodeHandle NodeHandle::create(int nodeKind) { return Nest_createNode(nodeKind); }

NodeHandle NodeHandle::clone(NodeHandle node) { return Nest_cloneNode(node.handle); }

void NodeHandle::setContext(Nest_CompilationContext* context) { Nest_setContext(handle, context); }

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

void NodeHandle::setProperty(Nest_NodeProperty prop) { _setProperty(&handle->properties, prop); }
void NodeHandle::setProperty(const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 0, {0}};
    prop.value.intValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setProperty(const char* name, StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 0, {0}};
    prop.value.stringValue = val.dup();
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setProperty(const char* name, NodeHandle val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 0, {0}};
    prop.value.nodeValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setProperty(const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 0, {0}};
    prop.value.typeValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setProperty(const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 0, {0}};
    prop.value.ptrValue = val;
    _setProperty(&handle->properties, prop);
}

void NodeHandle::setPropertyExpl(const char* name, int val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propInt, 1, {0}};
    prop.value.intValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setPropertyExpl(const char* name, StringRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propString, 1, {0}};
    prop.value.stringValue = val.dup();
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setPropertyExpl(const char* name, NodeHandle val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propNode, 1, {0}};
    prop.value.nodeValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setPropertyExpl(const char* name, TypeRef val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propType, 1, {0}};
    prop.value.typeValue = val;
    _setProperty(&handle->properties, prop);
}
void NodeHandle::setPropertyExpl(const char* name, void* val) {
    Nest_NodeProperty prop = {StringRef(name).dup(), propPtr, 1, {0}};
    prop.value.ptrValue = val;
    _setProperty(&handle->properties, prop);
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

Nest_CompilationContext* NodeHandle::childrenContext() const {
    return handle->childrenContext ? handle->childrenContext : handle->context;
}

NodeHandle NodeHandle::explanation() {
    return handle && handle->explanation && handle->explanation != handle
                   ? NodeHandle(handle->explanation).explanation()
                   : *this;
}

const char* NodeRange::toString() const {
    ostringstream ss;
    ss << '[';
    bool first = true;
    for (auto node : *this) {
        if (first)
            first = false;
        else
            ss << ", ";
        if (node)
            ss << node.toString();
        else
            ss << "null";
    }
    ss << ']';
    return strdup(ss.str().c_str());
}

const char* NodeRangeM::toString() const { return NodeRange(range).toString(); }

} // namespace Nest
