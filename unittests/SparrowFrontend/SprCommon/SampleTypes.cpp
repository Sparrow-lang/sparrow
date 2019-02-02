
#include "StdInc.h"
#include "SampleTypes.hpp"
#include "ConceptsServiceMock.hpp"
#include "OverloadServiceMock.hpp"

#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"

#include "SparrowFrontend/Nodes/SprProperties.h"
#include "SparrowFrontend/Helpers/StdDef.h"

using namespace SprFrontend;

void SampleTypes::init(SparrowGeneralFixture& generalFixture, int flags) {
    TypeFactory::g_dataTypeDecls.clear();
    TypeFactory::g_conceptDecls.clear();

    using TypeFactory::g_conceptDecls;
    using TypeFactory::g_dataTypeDecls;

    auto ctx = generalFixture.globalContext_;

    // Create the declarations for the types
    NodeHandle i8TypeDecl = generalFixture.createNativeDatatypeNode("i8", ctx);
    NodeHandle i16TypeDecl = generalFixture.createNativeDatatypeNode("i16", ctx);
    NodeHandle i32TypeDecl = generalFixture.createNativeDatatypeNode("i32", ctx);
    NodeHandle fooTypeDecl = generalFixture.createDatatypeNode("FooType", ctx);
    NodeHandle barTypeDecl = generalFixture.createDatatypeNode("BarType", ctx);
    NodeHandle nullTypeDecl = generalFixture.createDatatypeNode("NullType", ctx);
    NodeHandle typeDecl = generalFixture.createDatatypeNode("Type", ctx);

    // Add the declarations to our sets of type decls
    g_dataTypeDecls.push_back(i8TypeDecl);
    g_dataTypeDecls.push_back(i16TypeDecl);
    g_dataTypeDecls.push_back(i32TypeDecl);
    if ((flags & onlyNumeric) == 0) {
        g_dataTypeDecls.push_back(fooTypeDecl);
        g_dataTypeDecls.push_back(barTypeDecl);
    }

    i8Type_ = DataType::get(i8TypeDecl, 0, modeRt);
    i16Type_ = DataType::get(i16TypeDecl, 0, modeRt);
    i32Type_ = DataType::get(i32TypeDecl, 0, modeRt);
    fooType_ = DataType::get(fooTypeDecl, 0, modeRt);
    barType_ = DataType::get(barTypeDecl, 0, modeRt);
    nullType_ = DataType::get(nullTypeDecl, 0, modeRt);

    // Ensure we set the Null type
    SprFrontend::StdDef::typeNull = nullType_;
    SprFrontend::StdDef::clsNull = nullTypeDecl;

    // Ensure we set the Type type -- but don't add it to our conversion types
    SprFrontend::StdDef::typeType = DataType::get(typeDecl, 0, modeCt);
    REQUIRE(typeDecl.computeType());

    // Create concept decls & types
    NodeHandle concept1 = generalFixture.createSimpleConcept("Concept1", ctx);
    NodeHandle concept2 = generalFixture.createSimpleConcept("Concept2", ctx);
    NodeHandle concept3 = generalFixture.createSimpleConcept("ByteType", ctx);
    REQUIRE(concept1.computeType());
    REQUIRE(concept2.computeType());
    REQUIRE(concept3.computeType());
    concept1Type_ = ConceptType::get(concept1);
    concept2Type_ = ConceptType::get(concept2);
    byteType_ = ConceptType::get(concept3);

    // Add the concepts to our list of decls
    if ((flags & onlyNumeric) == 0) {
        g_conceptDecls.push_back(concept1);
        g_conceptDecls.push_back(concept2);
    }
    if (flags & addByteType)
        g_conceptDecls.push_back(concept3);

    // Mock the overload service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockOverloadService = new OverloadServiceMock;
    g_OverloadService.reset(mockOverloadService);

    // Create an implicit conversion from FooType to BarType
    barTypeDecl.setProperty(propConvert, 1);
    mockOverloadService->implicitConversions_.emplace_back(make_pair(fooType_, barType_));

    // Mock the concepts service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockConceptsService = new ConceptsServiceMock;
    g_ConceptsService.reset(mockConceptsService);

    // Make concept -> type associations
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, barType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, barType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept3, i8Type_));

    // Concept base
    mockConceptsService->baseConcepts_.emplace_back(make_pair(concept2, concept1Type_));
}

vector<DataType> SampleTypes::typesForConcept(ConceptType t) {
    if (t.referredNode() == concept1Type_.referredNode())
        return {fooType_, barType_};
    else if (t.referredNode() == concept2Type_.referredNode())
        return {fooType_, barType_};
    else if (t.referredNode() == byteType_.referredNode())
        return {i8Type_};
    return {};
}
