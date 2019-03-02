#pragma once

#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

/**
 * @brief   A mock for a compiler Node.
 *
 * It can be constructed with a range of children and referred nodes, and an optional type.
 *
 * The purpose of this is to be able to create hierarchies of nodes, and compile them.
 *
 * Behavior of compute type:
 * - compute the type for all its children
 * - if a type is given, return that type
 * - if no type is given, and has children, return the type of the first children
 * - if no type and no children are given, yield an error
 *
 * Behavior of semantic check:
 * - compute the type of this node
 * - semantically check all the children
 * - compute the types of all the referred nodes
 * - if the type of this node is different than the type of first children, raise an error
 * - sets the first children as expansion (if we have children)
 *
 * Behavior of setting the context:
 * - just pass the context to the children
 */
struct MockNode : Nest::NodeHandle {
    //! Called to register this node
    static void registerNode();

    static MockNode create(Nest::Location loc, Nest::NodeRange children,
            Nest::NodeRange referredNodes, Nest::Type t = {});

    MockNode() = default;
    MockNode(Nest::Node* n);

    //! Returns the given type set to this node
    Nest::Type givenType() const;

private:
    static const char* toStringCb(Nest::Node* node);
    static void setContextForChildrenCb(Nest::Node* node);
    static Nest::TypeRef computeTypeCb(Nest::Node* node);
    static Nest::Node* semanticCheckCb(Nest::Node* node);

    //! The kind of this node; set after registering
    static int nodeKind;
};

//! Returns a generator for our mock node
rc::Gen<MockNode> arbMockNode(int maxDepth = 5, Nest::Location loc = Nest::Location{});

//! Returns a generator for a range of mock nodes
rc::Gen<Nest::NodeRange> arbMockNodeRange(
        int maxDepth, int maxNodes, Nest::Location loc = Nest::Location{});

namespace rc {

//! Helper to make RapidCheck generate arbitrary types
template <> struct Arbitrary<MockNode> {
    static Gen<MockNode> arbitrary() { return arbMockNode(); }
};

} // namespace rc
