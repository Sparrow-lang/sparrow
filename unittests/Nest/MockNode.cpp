#include "StdInc.h"
#include "MockNode.hpp"
#include "MockType.hpp"
#include "Common/RcBasic.hpp"
#include "Nest/Api/Node.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"

using namespace Nest;

int MockNode::nodeKind = -1;

void MockNode::registerNode() {
    nodeKind = Nest_registerNodeKind("MockNode", &MockNode::semanticCheckCb,
            &MockNode::computeTypeCb, &MockNode::setContextForChildrenCb, &MockNode::toStringCb);
    ASSERT(nodeKind >= 0);
}

MockNode MockNode::create(Location loc, NodeRange children, NodeRange referredNodes, TypeRef t) {
    NodeHandle base = NodeHandle::create(nodeKind);
    base.handle->location = loc;
    base.setChildren(children);
    base.setReferredNodes(referredNodes);
    if (t)
        base.setProperty("givenType", t);
    return {base.handle};
}

MockNode::MockNode(Node* n)
    : NodeHandle(n) {
    ASSERT(n);
    ASSERT(n->nodeKind == MockNode::nodeKind);
}

TypeRef MockNode::givenType() const {
    return getCheckPropertyType("givenType");
}

const char* MockNode::toStringCb(Node* node) {
    NodeHandle n(node);
    ostringstream oss;
    oss << "MockNode(" << n.children() << ", "
        << "refs: " << n.referredNodes();
    const TypeRef* givenTypePtr = n.getPropertyType("givenType");
    if (givenTypePtr)
        oss << ", type: " << *givenTypePtr;
    oss << ")";
    return StringRef(oss.str()).dup().begin;
}
void MockNode::setContextForChildrenCb(Node* node) {
    MockNode n(node);
    for (auto child : n.children()) {
        child.setContext(n.context());
    }
}
TypeRef MockNode::computeTypeCb(Node* node) {
    MockNode n(node);
    for (auto child : n.children()) {
        child.computeType();
    }
    const TypeRef* givenTypePtr = n.getPropertyType("givenType");
    if (givenTypePtr)
        return *givenTypePtr;
    if (!n.children().empty())
        return n.children()[0].type();

    REP_ERROR(n.location(), "No type for MockNode");
    return nullptr;
}
Node* MockNode::semanticCheckCb(Node* node) {
    MockNode n(node);
    // Compute the type of this node
    TypeRef type = Nest_computeType(node);
    // Semantically check all children
    for (auto child : n.children()) {
        if (!child.semanticCheck())
            return nullptr;
    }
    // Compute the type of all the referred nodes
    for (auto child : n.children()) {
        if (!child.computeType())
            return nullptr;
    }
    if (!n.children().empty()) {
        // Type check
        if (!n.children().empty() && n.children()[0].type() != type) {
            REP_ERROR(n.location(), "Type doesn't match");
            return nullptr;
        }
        return n.children()[0];
    } else
        return node;
}

rc::Gen<MockNode> arbMockNode(int maxDepth, Location loc) {
    using namespace rc;

    auto rangeGen = arbMockNodeRange(maxDepth - 1, maxDepth, loc);
    auto res1 = gen::apply(
            [=](NodeRange children, NodeRange referredNodes, MockType type) -> MockNode {
                return MockNode::create(loc, children, referredNodes, type);

            },
            rangeGen, rangeGen, gen::arbitrary<MockType>());
    auto res = gen::exec([=]() -> MockNode {
        auto rangeGen = arbMockNodeRange(maxDepth - 1, maxDepth, loc);
        NodeRange children = *rangeGen;
        NodeRange referredNodes = *rangeGen;
        MockType type = children.empty() ? *gen::arbitrary<MockType>() : MockType{};
        return MockNode::create(loc, children, referredNodes, type);

    });

    return res;
}

rc::Gen<NodeRange> arbMockNodeRange(int maxDepth, int maxNodes, Location loc) {
    using namespace rc;

    auto res = gen::exec([=]() -> NodeRange {
        int numNodes = *gen::inRange(0, maxNodes);

        auto nodes = Nest_allocNodeArray(numNodes);
        for (int i = 0; i < numNodes; i++) {
            auto n = *arbMockNode(maxDepth - 1, loc);
            Nest_appendNodeToArray(&nodes, n.handle);
        }
        return all(nodes);
    });
    return res;
}
