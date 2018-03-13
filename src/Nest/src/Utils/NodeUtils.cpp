#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/PtrArray.h"

NodeProperty* _findProperty(NodeProperties properties, StringRef name) {
    NodeProperty* p = properties.begin;
    for (; p != properties.end; ++p)
        if (p->name == name)
            return p;
    return nullptr;
}

void _setProperty(NodeProperties* properties, NodeProperty prop) {
    // Try to see if a property with the same name already exists
    NodeProperty* p = _findProperty(*properties, prop.name);
    if (p) {
        *p = prop;
    } else {
        Nest_addProperty(properties, prop);
    }
}

const char* Nest_toString(const Node* node) { return Nest_getToStringFun(node->nodeKind)(node); }

const char* Nest_toStringEx(const Node* node) {
    ostringstream ss;
    ss << Nest_toString(node);
    if (node->type)
        ss << ": " << node->type;
    ss << '(';
    ss << Nest_getNodeKindName(node->nodeKind);
    if (node->explanation && node->explanation != node)
        ss << " -> " << Nest_getNodeKindName(node->explanation->nodeKind);
    ss << ')';

    // If we have some properties, also write those
    if (node->properties.end != node->properties.begin) {
        ss << '[';
        NodeProperty* p = node->properties.begin;
        for (; p != node->properties.end; ++p) {
            if (p != node->properties.begin)
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
            }
        }
        ss << ']';
    }
    return strdup(ss.str().c_str());
}

const char* Nest_rangeToString(NodeRange nodes) {
    ostringstream ss;
    ss << '[';
    for (int i = 0; i < size(nodes); ++i) {
        Node* node = at(nodes, i);
        if (i > 0)
            ss << ", ";
        if (node)
            ss << Nest_toString(node);
        else
            ss << "null";
    }
    ss << ']';
    return strdup(ss.str().c_str());
}

const char* Nest_nodeKindName(const Node* node) { return Nest_getNodeKindName(node->nodeKind); }

NodeRange Nest_nodeChildren(Node* node) { return all(node->children); }

Node* Nest_getNodeChild(Node* node, unsigned int index) {
    NodeRange children = Nest_nodeChildren(node);
    ASSERT(index < Nest_nodeRangeSize(children));
    return children.beginPtr[index];
}

NodeRange Nest_nodeReferredNodes(Node* node) { return all(node->referredNodes); }

Node* Nest_getReferredNode(Node* node, unsigned int index) {
    NodeRange refNodes = Nest_nodeReferredNodes(node);
    ASSERT(index < Nest_nodeRangeSize(refNodes));
    return refNodes.beginPtr[index];
}

void Nest_nodeSetChildren(Node* node, NodeRange children) {
    unsigned size = Nest_nodeRangeSize(children);
    Nest_resizeNodeArray(&node->children, size);
    for (unsigned i = 0; i < size; ++i)
        at(node->children, i) = at(children, i);
}

void Nest_nodeAddChild(Node* node, Node* child) { Nest_appendNodeToArray(&node->children, child); }

void Nest_nodeAddChildren(Node* node, NodeRange children) {
    Nest_appendNodesToArray(&node->children, children);
}

void Nest_nodeSetReferredNodes(Node* node, NodeRange nodes) {
    unsigned size = Nest_nodeRangeSize(nodes);
    Nest_resizeNodeArray(&node->referredNodes, size);
    for (unsigned i = 0; i < size; ++i)
        at(node->referredNodes, i) = at(nodes, i);
}

void Nest_setProperty(Node* node, NodeProperty prop) { _setProperty(&node->properties, prop); }

void Nest_setPropertyInt(Node* node, const char* name, int val) {
    NodeProperty prop = {dupCStr(name), propInt, 0, {0}};
    prop.value.intValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyString(Node* node, const char* name, StringRef val) {
    NodeProperty prop = {dupCStr(name), propString, 0, {0}};
    prop.value.stringValue = dup(val);
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyNode(Node* node, const char* name, Node* val) {
    NodeProperty prop = {dupCStr(name), propNode, 0, {0}};
    prop.value.nodeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyType(Node* node, const char* name, TypeRef val) {
    NodeProperty prop = {dupCStr(name), propType, 0, {0}};
    prop.value.typeValue = val;
    _setProperty(&node->properties, prop);
}

void Nest_setPropertyExplInt(Node* node, const char* name, int val) {
    NodeProperty prop = {dupCStr(name), propInt, 1, {0}};
    prop.value.intValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplString(Node* node, const char* name, StringRef val) {
    NodeProperty prop = {dupCStr(name), propString, 1, {0}};
    prop.value.stringValue = dup(val);
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplNode(Node* node, const char* name, Node* val) {
    NodeProperty prop = {dupCStr(name), propNode, 1, {0}};
    prop.value.nodeValue = val;
    _setProperty(&node->properties, prop);
}
void Nest_setPropertyExplType(Node* node, const char* name, TypeRef val) {
    NodeProperty prop = {dupCStr(name), propType, 1, {0}};
    prop.value.typeValue = val;
    _setProperty(&node->properties, prop);
}

int Nest_hasProperty(const Node* node, const char* name) {
    return nullptr != _findProperty(node->properties, fromCStr(name));
}
const int* Nest_getPropertyInt(const Node* node, const char* name) {
    NodeProperty* p = _findProperty(node->properties, fromCStr(name));
    if (!p || p->kind != propInt)
        return nullptr;
    return &p->value.intValue;
}
const StringRef* Nest_getPropertyString(const Node* node, const char* name) {
    NodeProperty* p = _findProperty(node->properties, fromCStr(name));
    if (!p || p->kind != propString)
        return nullptr;
    return &p->value.stringValue;
}
StringRef Nest_getPropertyStringDeref(const Node* node, const char* name) {
    NodeProperty* p = _findProperty(node->properties, fromCStr(name));
    if (!p || p->kind != propString)
        return StringRef{nullptr, nullptr};
    return p->value.stringValue;
}
Node* const* Nest_getPropertyNode(const Node* node, const char* name) {
    NodeProperty* p = _findProperty(node->properties, fromCStr(name));
    if (!p || p->kind != propNode)
        return nullptr;
    return &p->value.nodeValue;
}
const TypeRef* Nest_getPropertyType(const Node* node, const char* name) {
    NodeProperty* p = _findProperty(node->properties, fromCStr(name));
    if (!p || p->kind != propType)
        return nullptr;
    return &p->value.typeValue;
}

int Nest_getPropertyDefaultInt(const Node* node, const char* name, int defaultVal) {
    const int* res = Nest_getPropertyInt(node, name);
    return res ? *res : defaultVal;
}
StringRef Nest_getPropertyDefaultString(const Node* node, const char* name, StringRef defaultVal) {
    const StringRef* res = Nest_getPropertyString(node, name);
    return res ? *res : defaultVal;
}
Node* Nest_getPropertyDefaultNode(const Node* node, const char* name, Node* defaultVal) {
    Node* const* res = Nest_getPropertyNode(node, name);
    return res ? *res : defaultVal;
}
TypeRef Nest_getPropertyDefaultType(const Node* node, const char* name, TypeRef defaultVal) {
    const TypeRef* res = Nest_getPropertyType(node, name);
    return res ? *res : defaultVal;
}

int Nest_getCheckPropertyInt(const Node* node, const char* name) {
    const int* res = Nest_getPropertyInt(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have integer property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
StringRef Nest_getCheckPropertyString(const Node* node, const char* name) {
    const StringRef* res = Nest_getPropertyString(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have string property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
Node* Nest_getCheckPropertyNode(const Node* node, const char* name) {
    Node* const* res = Nest_getPropertyNode(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have Node property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}
TypeRef Nest_getCheckPropertyType(const Node* node, const char* name) {
    const TypeRef* res = Nest_getPropertyType(node, name);
    if (!res)
        REP_INTERNAL(node->location, "Node of kind %1% does not have Type property %2%") %
                Nest_nodeKindName(node) % name;
    return *res;
}

void Nest_addModifier(Node* node, Modifier* mod) {
    NestUtils_appendObjectToPtrArray((PtrArray*)&node->modifiers, mod);
}

CompilationContext* Nest_childrenContext(const Node* node) {
    return node->childrenContext ? node->childrenContext : node->context;
}

Node* Nest_explanation(Node* node) {
    return node && node->explanation && node->explanation != node
                   ? Nest_explanation(node->explanation)
                   : node;
}

Node* Nest_ofKind(Node* src, int desiredNodeKind) {
    return src && src->nodeKind == desiredNodeKind ? src : nullptr;
}

NodeArray _fromPtrArray(PtrArray arr) {
    NodeArray res = {(Node**)arr.beginPtr, (Node**)arr.endPtr, (Node**)arr.endOfStorePtr};
    return res;
}
PtrArray _toPtrArray(NodeArray arr) {
    PtrArray res = {(void**)arr.beginPtr, (void**)arr.endPtr, (void**)arr.endOfStorePtr};
    return res;
}

NodeArray Nest_allocNodeArray(unsigned capacity) {
    return _fromPtrArray(NestUtils_allocPtrArray(capacity));
}

void Nest_freeNodeArray(NodeArray arr) { NestUtils_freePtrArray(_toPtrArray(arr)); }

void Nest_reserveNodeArray(NodeArray* arr, unsigned capacity) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_reservePtrArray(&arr2, capacity);
    *arr = _fromPtrArray(arr2);
}

void Nest_resizeNodeArray(NodeArray* arr, unsigned size) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_resizePtrArray(&arr2, size);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodeToArray(NodeArray* arr, Node* node) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_appendObjectToPtrArray(&arr2, node);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodesToArray(NodeArray* arr, NodeRange nodes) {
    PtrArray arr2 = _toPtrArray(*arr);
    PtrRange objects = {(void**)nodes.beginPtr, (void**)nodes.endPtr};
    NestUtils_appendObjectsToPtrArray(&arr2, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_insertNodeIntoArray(NodeArray* arr, unsigned index, Node* node) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_insertObjectIntoPtrArray(&arr2, index, node);
    *arr = _fromPtrArray(arr2);
}
void Nest_insertNodesIntoArray(NodeArray* arr, unsigned index, NodeRange nodes) {
    PtrArray arr2 = _toPtrArray(*arr);
    PtrRange objects = {(void**)nodes.beginPtr, (void**)nodes.endPtr};
    NestUtils_insertObjectsIntoPtrArray(&arr2, index, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_eraseNodeFromArray(NodeArray* arr, unsigned index) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_eraseFromPtrArray(&arr2, index);
    *arr = _fromPtrArray(arr2);
}

NodeRange Nest_getNodeRangeFromArray(NodeArray arr) {
    NodeRange res = {arr.beginPtr, arr.endPtr};
    return res;
}

unsigned Nest_nodeArraySize(NodeArray arr) { return arr.endPtr - arr.beginPtr; }
unsigned Nest_nodeArrayCapacity(NodeArray arr) { return arr.endOfStorePtr - arr.beginPtr; }

unsigned Nest_nodeRangeSize(NodeRange nodes) { return nodes.endPtr - nodes.beginPtr; }

NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned count) {
    NodeRange res = {nodes, nodes + count};
    return res;
}
