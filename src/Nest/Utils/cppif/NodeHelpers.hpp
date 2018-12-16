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

//! Helper macro that implements registerNodeKind method for a NodeHandle-derived type
#define REGISTER_NODE_KIND_IMPL(T)                                                                 \
    int T::registerNodeKind() {                                                                    \
        struct RegHelper {                                                                         \
            static int registerKind(const char* name) {                                            \
                auto semCheck = reinterpret_cast<FSemanticCheck>(&semanticCheckPlain); /*NOLINT*/  \
                auto compT = reinterpret_cast<FComputeType>(&computeTypePlain);        /*NOLINT*/  \
                auto setCtx = reinterpret_cast<FSetContextForChildren>(                /*NOLINT*/  \
                        &setContextForChildrenPlain);                                              \
                auto toStr = reinterpret_cast<FToString>(&toStringPlain); /*NOLINT*/               \
                return Nest_registerNodeKind(name, semCheck, compT, setCtx, toStr);                \
            }                                                                                      \
                                                                                                   \
        private:                                                                                   \
            static NodeHandle semanticCheckPlain(T node) { return node.semanticCheckImpl(); }      \
            static Type computeTypePlain(T node) { return node.computeTypeImpl(); }                \
            static void setContextForChildrenPlain(T node) { node.setContextForChildrenImpl(); }   \
            static const char* toStringPlain(T node) { return node.toStringImpl(); }               \
        };                                                                                         \
        return RegHelper::registerKind(#T);                                                        \
    }

//! Helper macro that implements registerNodeKind method for a NodeHandle-derived type
#define REGISTER_NODE_KIND_IMPL2(T)                                                                \
    int T::registerNodeKind() {                                                                    \
        auto semCheck = reinterpret_cast<FSemanticCheck>(&T::semanticCheckImpl2); /*NOLINT*/       \
        auto compT = reinterpret_cast<FComputeType>(&T::computeTypeImpl2);        /*NOLINT*/       \
        auto setCtx = reinterpret_cast<FSetContextForChildren>(                   /*NOLINT*/       \
                &T::setContextForChildrenImpl2);                                                   \
        auto toStr = reinterpret_cast<FToString>(&T::toStringImpl2); /*NOLINT*/                    \
        staticKindValue = Nest_registerNodeKind(#T, semCheck, compT, setCtx, toStr);               \
        return staticKindValue;                                                                    \
    }

#define DECLARE_NODE_COMMON(T)                                                                     \
private:                                                                                           \
    static int staticKindValue;                                                                    \
                                                                                                   \
public:                                                                                            \
    static int registerNodeKind();                                                                 \
    static int staticKind();                                                                       \
    T() = default;                                                                                 \
    T(Node* n);

#define DEFINE_NODE_COMMON_IMPL(T, BaseType, nodeKind)                                             \
    REGISTER_NODE_KIND_IMPL2(T);                                                                   \
    int T::staticKindValue{-1};                                                                    \
    T::T(Node* n)                                                                                  \
        : BaseType(n) {                                                                            \
        REQUIRE_NODE_KIND(n, staticKindValue);                                                     \
    }                                                                                              \
    int T::staticKind() { return staticKindValue; }

namespace Nest {

//! Helper function used to create nodes
template <typename T> T createNode(int kind, const Location& loc) {
    T res;
    res.handle = NodeHandle::create(kind, loc).handle;
    return res;
}

//! Helper function used to create nodes, with children
template <typename T> T createNode(int kind, const Location& loc, NodeRange children) {
    T res;
    res.handle = NodeHandle::create(kind, loc).handle;
    res.setChildren(children);
    return res;
}

//! Helper function used to create nodes, with children and referred nodes
template <typename T>
T createNode(int kind, const Location& loc, NodeRange children, NodeRange referredNodes) {
    T res;
    res.handle = NodeHandle::create(kind, loc).handle;
    res.setChildren(children);
    res.setReferredNodes(referredNodes);
    return res;
}

} // namespace Nest
