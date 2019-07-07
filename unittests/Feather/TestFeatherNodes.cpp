#include "StdInc.h"
#include "Common/GeneralFixture.hpp"
#include "Common/FeatherNodeFactory.hpp"
#include "Common/RcBasic.hpp"
#include "Common/TypeFactory.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using namespace Nest;
using namespace Feather;

//! Fixture used in testing the Feather types
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct FeatherNodesFixture : SparrowGeneralFixture {
    FeatherNodesFixture();
    ~FeatherNodesFixture();

    void clearAuxNodes();
    void setContextForAuxNodes();
};

FeatherNodesFixture::FeatherNodesFixture() {
    using TypeFactory::g_dataTypeDecls;
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i1"), globalContext_));
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i8"), globalContext_));
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i16"), globalContext_));
    g_dataTypeDecls.emplace_back(createNativeDatatypeNode(StringRef("i32"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("FooType"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("BarType"), globalContext_));
    g_dataTypeDecls.emplace_back(createDatatypeNode(StringRef("NullType"), globalContext_));
}
FeatherNodesFixture::~FeatherNodesFixture() {
    FeatherNodeFactory::instance().reset();
}

void FeatherNodesFixture::clearAuxNodes() {
    FeatherNodeFactory::instance().clearAuxNodes();
}

void FeatherNodesFixture::setContextForAuxNodes() {
    FeatherNodeFactory::instance().setContextForAuxNodes(globalContext_);
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather expressions generation") {
    rc::prop("Test expected type", [=](Nest::TypeWithStorage expectedType) {
        clearAuxNodes();
        NodeHandle node = *FeatherNodeFactory::instance().arbExp(expectedType);
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        REQUIRE(t);
        if (expectedType.kind() != Feather_getFunctionTypeKind())
            REQUIRE(sameTypeIgnoreMode(t, expectedType));
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::Nop node") {
    SECTION("Has type Void") {
        clearAuxNodes();
        auto nop = Nop::create(createLocation());
        setContextForAuxNodes();
        nop.setContext(globalContext_);
        auto t = nop.computeType();
        REQUIRE(t);
        REQUIRE(t.kind() == Feather_getVoidTypeKind());
        REQUIRE(t == nop.type());
    }
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::TypeNode node") {
    rc::prop("TypeNode has proper type", [=](Feather::TypeNode node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto expectedType = node.givenType();
        auto t = node.computeType();
        REQUIRE(t == expectedType);
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::CtValueExp node") {
    rc::prop("CtValueExp has proper type - CT", [=](Feather::CtValueExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto expectedType = node.valueType();
        auto t = node.computeType();
        REQUIRE(sameTypeIgnoreMode(t, expectedType));
        REQUIRE(t.mode() == modeCt);
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::NullExp node") {
    rc::prop("NullExp has proper type - as given", [=](Feather::NullExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        auto expectedType = node.typeNode();
        REQUIRE(t);
        REQUIRE(expectedType.type());
        REQUIRE(sameTypeIgnoreMode(t, expectedType.type()));
        REQUIRE(t.hasStorage());
        REQUIRE(TypeWithStorage(t).numReferences() > 0);
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::VarRefExp node") {
    rc::prop("VarRefExp has proper type - MutableType(var)", [=](Feather::VarRefExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        auto varDecl = node.varDecl();
        REQUIRE(t);
        REQUIRE(t.kind() == typeKindMutable);
        REQUIRE(varDecl.type());
        REQUIRE(varDecl.type().hasStorage());
        REQUIRE(sameTypeIgnoreMode(t, MutableType::get(varDecl.type())));
    });
    rc::prop("VarRefExp pointing to params have proper type",
            [=](DataType t1, DataType t2, DataType t3) {
                // Create a function decl with the given param types
                auto loc = createLocation();
                auto p1 = VarDecl::create(loc, "p1", TypeNode::create(loc, t1));
                auto p2 = VarDecl::create(loc, "p2", TypeNode::create(loc, t2));
                auto p3 = VarDecl::create(loc, "p3", TypeNode::create(loc, t3));
                auto retType = TypeNode::create(loc, VoidType::get(modeRt));
                auto funDecl = FunctionDecl::create(loc, "f", retType, NodeRange{p1, p2, p3}, {});

                // Populate our body with some var-ref expressions
                auto vr1 = VarRefExp::create(loc, p1);
                auto vr2 = VarRefExp::create(loc, p2);
                auto vr3 = VarRefExp::create(loc, p3);
                auto body = LocalSpace::create(loc, NodeRange{vr1, vr2, vr3});
                funDecl.setBody(body);

                // Compile the whole function
                funDecl.setContext(globalContext_);
                funDecl.semanticCheck();

                // Now check the types of our var-ref expressions
                auto vrt1 = vr1.type();
                auto vrt2 = vr2.type();
                auto vrt3 = vr3.type();
                REQUIRE(vrt1);
                REQUIRE(vrt2);
                REQUIRE(vrt3);
                REQUIRE(vrt1.kind() == typeKindMutable);
                REQUIRE(vrt2.kind() == typeKindMutable);
                REQUIRE(vrt3.kind() == typeKindMutable);
                REQUIRE(sameTypeIgnoreMode(vrt1, MutableType::get(t1)));
                REQUIRE(sameTypeIgnoreMode(vrt2, MutableType::get(t2)));
                REQUIRE(sameTypeIgnoreMode(vrt3, MutableType::get(t3)));
            });
}

// TODO: FieldRefExp
// TODO: FunRefExp

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::FunCallExp node") {
    rc::prop("FunCallExp has proper type - the type of the result", [=](Feather::FunCallExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        REQUIRE(t);
        REQUIRE(node.funDecl().type().kind() == Feather_getFunctionTypeKind());

        // If the result type is CT, then all the args are CT
        if ( t.mode() == modeCt ) {
            for( auto arg: node.arguments()) {
                REQUIRE(arg.type());
                REQUIRE(arg.type().mode() == modeCt);
            }
        }
        // If autoCt and all all args are CT, then result is CT
        if (node.hasProperty(propAutoCt)) {
            bool allParamsCt = false;
            for( auto arg: node.arguments()) {
                REQUIRE(arg.type());
                allParamsCt = allParamsCt && arg.type().mode() == modeCt;
            }
            if (allParamsCt)
                REQUIRE(t.mode() == modeCt);
        }
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::MemLoadExp node") {
    rc::prop("MemLoadExp has proper type - one ref less than given", [=](Feather::MemLoadExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        auto tAddr = node.address().type();
        REQUIRE(t);
        REQUIRE(tAddr);
        REQUIRE(tAddr.hasStorage());
        REQUIRE(sameTypeIgnoreMode(t, removeCatOrRef(TypeWithStorage(tAddr))));
        REQUIRE(t.kind() != typeKindMutable);
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::MemStoreExp node") {
    rc::prop("MemStoreExp has proper type - Void", [=](Feather::MemStoreExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        REQUIRE(t);
        REQUIRE(t.kind() == Feather_getVoidTypeKind());
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::BitcastExp node") {
    rc::prop("BitcastExp has proper type - the given type", [=](Feather::BitcastExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        auto tDest = node.destTypeNode().type();
        REQUIRE(t);
        REQUIRE(tDest);
        REQUIRE(sameTypeIgnoreMode(t, tDest));
    });
}

TEST_CASE_METHOD(FeatherNodesFixture, "Testing Feather::ConditionalExp node") {
    rc::prop("ConditionalExp has proper type - the alt type", [=](Feather::ConditionalExp node) {
        setContextForAuxNodes();
        node.setContext(globalContext_);
        auto t = node.computeType();
        auto tAlt1 = node.alt1().type();
        auto tAlt2 = node.alt2().type();
        REQUIRE(t);
        REQUIRE(tAlt1);
        REQUIRE(tAlt2);
        REQUIRE(sameTypeIgnoreMode(t, tAlt1));
        REQUIRE(sameTypeIgnoreMode(t, tAlt2));
    });
}
