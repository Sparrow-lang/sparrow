#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

#define REQUIRE_NODE(loc, node)                                                                    \
    if (node)                                                                                      \
        ;                                                                                          \
    else                                                                                           \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % (#node)

#define REQUIRE_NODE_KIND(node, kind)                                                              \
    if (node && node->nodeKind != kind)                                                            \
        REP_INTERNAL(NOLOC, "Expected AST node of kind %1%, found %2% (inside %3%)") % (#kind) %   \
                NodeHandle(node).kindName() % __FUNCTION__;                                        \
    else                                                                                           \
        ;

//! To be used inside node classes to declare node boilerplate code
#define DECLARE_NODE_COMMON(T)                                                                     \
private:                                                                                           \
    static int staticKindValue;                                                                    \
                                                                                                   \
public:                                                                                            \
    using ThisNodeType = T;                                                                        \
    static int registerNodeKind();                                                                 \
    static int staticKind();                                                                       \
    T() = default;                                                                                 \
    T(Node* n);                                                                                    \
    T clone();

#define DEFINE_NODE_COMMON_IMPL(T, BaseType)                                                       \
    int T::registerNodeKind() {                                                                    \
        auto semCheck = reinterpret_cast<FSemanticCheck>(&T::semanticCheckImpl); /*NOLINT*/        \
        auto compT = reinterpret_cast<FComputeType>(&T::computeTypeImpl);        /*NOLINT*/        \
        auto setCtx = reinterpret_cast<FSetContextForChildren>(                  /*NOLINT*/        \
                &T::setContextForChildrenImpl);                                                    \
        auto toStr = reinterpret_cast<FToString>(&T::toStringImpl); /*NOLINT*/                     \
        staticKindValue = Nest_registerNodeKind(#T, semCheck, compT, setCtx, toStr);               \
        return staticKindValue;                                                                    \
    }                                                                                              \
    int T::staticKindValue{-1};                                                                    \
    T::T(Node* n)                                                                                  \
        : BaseType(n) {                                                                            \
        REQUIRE_NODE_KIND(n, staticKindValue);                                                     \
    }                                                                                              \
    int T::staticKind() { return staticKindValue; }                                                \
    T T::clone() { return T(NodeHandle::clone()); }

namespace Nest {

//! Helper function used to create nodes
template <typename T> T createNode(const Location& loc) {
    T res;
    res.handle = NodeHandle::create(T::staticKind(), loc).handle;
    return res;
}

//! Helper function used to create nodes, with children
template <typename T> T createNode(const Location& loc, NodeRange children) {
    T res;
    res.handle = NodeHandle::create(T::staticKind(), loc).handle;
    res.setChildren(children);
    return res;
}

//! Helper function used to create nodes, with children and referred nodes
template <typename T>
T createNode(const Location& loc, NodeRange children, NodeRange referredNodes) {
    T res;
    res.handle = NodeHandle::create(T::staticKind(), loc).handle;
    res.setChildren(children);
    res.setReferredNodes(referredNodes);
    return res;
}

} // namespace Nest
