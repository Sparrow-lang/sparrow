#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/RcBasic.hpp"
#include "Nest/MockNode.hpp"
#include "Nest/MockType.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

using namespace Nest;

//! Fixture used in testing the Node basic functionality
struct NodesFixture : NestGeneralFixture {
};

struct Indent {
    int value;
};
ostream& operator<<(ostream& os, Indent indent) {
    for (int i = 0; i < indent.value; i++)
        os << "  ";
    return os;
}

struct NodeFmt {
    Nest::NodeHandle node;
    Indent indent;
};

ostream& operator<<(ostream& os, NodeFmt node) {
    os << node.indent << "- " << node.node.kindName() << ":\n";
    Indent indent1{node.indent.value + 1};
    Indent indent2{node.indent.value + 2};
    os << indent1 << "- type: " << node.node.type() << "\n";
    os << indent1 << "- children:\n";
    for (auto n : node.node.children())
        os << NodeFmt{n, indent2};
    os << indent1 << "- referredNodes:\n";
    for (auto n : node.node.referredNodes())
        os << NodeFmt{n, indent2};
    return os;
}

//! Checks if the type of a node is computes.
//! Can check for an expected type, can be applied recursively, and can check if the resulting type
//! is also the type of the first children (MockNode assumption).
void checkTypeComputed(NodeHandle node, Type expectedType, bool recursiveCheck = true,
        bool sameTypeAsFirst = true) {
    auto thisType = node.type();
    REQUIRE(thisType != nullptr);
    if (expectedType)
        REQUIRE(thisType == expectedType);

    REQUIRE(node.handle->nodeError == 0);
    REQUIRE(node.handle->computeTypeStarted == 1);

    if (recursiveCheck) {
        for (auto n : node.children())
            checkTypeComputed(n, nullptr, true);
    }

    if (sameTypeAsFirst && !node.children().empty()) {
        REQUIRE(thisType == node.children()[0].type());
        checkTypeComputed(node.children()[0], expectedType, recursiveCheck);
    }
}

TEST_CASE_METHOD(NodesFixture, "User can register a new node kind") {
    SECTION("first time register") {
        // Register our type
        MockNode::registerNode();
        // Create one type of this kind
        auto myNode = MockNode::create(createLocation(), NodeRange{}, NodeRange{});
        // Expect to have only one kind registered
        REQUIRE(myNode.handle->nodeKind == 0);
    }

    SECTION("second time register") {
        // Register our type
        MockNode::registerNode();
        // Create one type of this kind
        auto myNode = MockNode::create(createLocation(), NodeRange{}, NodeRange{});
        // Expect to have only one kind registered
        REQUIRE(myNode.handle->nodeKind == 0);
    }

    SECTION("register multiple node kinds") {
        // Register three types -- only keep track of the last one
        MockNode::registerNode();
        MockNode::registerNode();
        MockNode::registerNode();
        // Create one type of the last kind
        auto myNode = MockNode::create(createLocation(), NodeRange{}, NodeRange{});
        // Expect to reach the third node kind ID (starting with 0)
        REQUIRE(myNode.handle->nodeKind == 2);
    }
}

TEST_CASE_METHOD(NodesFixture, "User can compute the type of the nodes") {
    MockNode::registerNode();
    auto loc = createLocation();
    Nest_resetDiagnostic();
    Nest_enableReporting(false);

    SECTION("manual test, simple node, no children") {
        auto type = MockType::get(true, 0, modeRt);
        auto node = MockNode::create(loc, NodeRange{}, NodeRange{}, type);
        node.setContext(globalContext_);
        auto resT = node.computeType();
        REQUIRE(type == resT); // Must have the given type
        REQUIRE(node.handle->nodeError == 0);
        REQUIRE(node.handle->nodeSemanticallyChecked == 0);
        REQUIRE(node.handle->computeTypeStarted == 1);
        REQUIRE(node.handle->semanticCheckStarted == 0);
        REQUIRE(node.type() == type);
    }

    SECTION("error case, simple node, no children") {
        auto node = MockNode::create(loc, NodeRange{}, NodeRange{}, nullptr);
        node.setContext(globalContext_);
        auto resT = node.computeType();
        REQUIRE(resT == nullptr);
        REQUIRE(node.handle->nodeError == 1);
        REQUIRE(node.handle->nodeSemanticallyChecked == 0);
        REQUIRE(node.handle->computeTypeStarted == 1);
        REQUIRE(node.handle->semanticCheckStarted == 0);
        REQUIRE(Nest_getSuppressedErrorsNum() == 1);
        Nest_resetDiagnostic();
    }
    rc::prop("Computing the type works ok -- arbitrary nodes", [this](MockNode node) {
        node.setContext(globalContext_);
        auto resT = node.computeType();

        // Make sure are children have their type computed
        checkTypeComputed(node, resT);

        // Check that we don't have the type computed for all the referred nodes
        for (auto n : node.referredNodes())
            REQUIRE(n.type() == nullptr);
    });

    rc::prop("Computing the type reports mismatch -- arbitrary nodes", [this]() {
        NodeRange children = *arbMockNodeRange(5, 5);
        NodeRange referredNodes = *arbMockNodeRange(5, 5);
        MockType type = *rc::gen::arbitrary<MockType>();

        if (children.empty() || children[0].type() != type)
            return; // Nothing to test

        // Create the node to be tested
        auto node = MockNode::create(Location(), children, referredNodes, type);

        node.setContext(globalContext_);
        auto resT = node.computeType();

        // Expect error
        REQUIRE(resT == nullptr);
        REQUIRE(node.handle->nodeError == 1);
        REQUIRE(node.handle->nodeSemanticallyChecked == 0);
        REQUIRE(node.handle->computeTypeStarted == 1);
        REQUIRE(node.handle->semanticCheckStarted == 0);

        REQUIRE(Nest_getSuppressedErrorsNum() == 1);
        Nest_resetDiagnostic();
    });
}

TEST_CASE_METHOD(NodesFixture, "User can semantically check nodes") {
    MockNode::registerNode();
    auto loc = createLocation();
    Nest_resetDiagnostic();
    Nest_enableReporting(false);

    SECTION("manual test, simple node, no children") {
        auto type = MockType::get(true, 0, modeRt);
        auto node = MockNode::create(loc, NodeRange{}, NodeRange{}, type);
        node.setContext(globalContext_);
        auto resNode = node.semanticCheck();
        REQUIRE(node.handle->nodeError == 0);
        REQUIRE(node.handle->nodeSemanticallyChecked == 1);
        REQUIRE(node.handle->computeTypeStarted == 1);
        REQUIRE(node.handle->semanticCheckStarted == 1);
        REQUIRE(node.type() == type);
        REQUIRE(node.explanation() == node);
        REQUIRE(resNode == node);
    }

    rc::prop("Semantic check ok -- arbitrary nodes", [this](MockNode node) {
        node.setContext(globalContext_);
        auto resNode = node.semanticCheck();

        REQUIRE(node.curExplanation() == resNode);
        if (!node.children().empty())
            REQUIRE(resNode != node);

        // Make sure are children have their type computed
        checkTypeComputed(node, node.type());

        // Check that we don't have the type computed for all the referred nodes
        for (auto n : node.referredNodes())
            checkTypeComputed(node, nullptr);
    });
}

TEST_CASE_METHOD(NodesFixture, "User can set and retrieve properties for nodes") {
    MockNode::registerNode();
    auto loc = createLocation();
    Nest_resetDiagnostic();
    Nest_enableReporting(false);

    rc::prop("Int properties", [=](MockNode node, string name, int val) {
        const char* propName = name.c_str();
        REQUIRE(!node.hasProperty(propName));
        node.setProperty(propName, val);
        REQUIRE(node.getCheckPropertyInt(propName) == val);
    });
    rc::prop("String properties", [=](MockNode node, string name, string val) {
        const char* propName = name.c_str();
        REQUIRE(!node.hasProperty(propName));
        node.setProperty(propName, val);
        REQUIRE(node.getCheckPropertyString(propName) == val);
    });
    rc::prop("Node properties", [=](MockNode node, string name, MockNode val) {
        const char* propName = name.c_str();
        REQUIRE(!node.hasProperty(propName));
        node.setProperty(propName, val);
        REQUIRE(node.getCheckPropertyNode(propName) == val);
    });
    rc::prop("Type properties", [=](MockNode node, string name, MockType val) {
        const char* propName = name.c_str();
        REQUIRE(!node.hasProperty(propName));
        node.setProperty(propName, val);
        REQUIRE(node.getCheckPropertyType(propName) == val);
    });
    rc::prop("Multiple times", [=](MockNode node, string name, int val) {
        const char* propName = name.c_str();
        REQUIRE(!node.hasProperty(propName));
        for (int i = 0; i < 10; i++) {
            node.setProperty(propName, val + i);
            REQUIRE(node.getCheckPropertyInt(propName) == val + i);
        }
    });
    rc::prop("Multiple props", [=](MockNode node, vector<string> names) {
        // Ensure property names are unique
        sort(begin(names), end(names));
        auto last = unique(begin(names), end(names));
        names.erase(last, names.end());
        // Set all the properties
        for (int i = 0; i < int(names.size()); i++) {
            const char* propName = names[i].c_str();
            REQUIRE(!node.hasProperty(propName));
            node.setProperty(propName, i);
        }
        // Not check them
        for (int i = 0; i < int(names.size()); i++) {
            const char* propName = names[i].c_str();
            REQUIRE(node.getCheckPropertyInt(propName) == i);
        }
    });
}
