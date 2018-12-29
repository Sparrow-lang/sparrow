
#include "StdInc.h"
#include "SampleTypes.hpp"
#include "ConceptsServiceMock.hpp"
#include "OverloadServiceMock.hpp"

#include "Common/TypeFactory.hpp"
#include "Common/GeneralFixture.hpp"

#include "SparrowFrontend/Nodes/SprProperties.h"
#include "SparrowFrontend/Helpers/StdDef.h"

using namespace SprFrontend;


void SampleTypes::init(SparrowGeneralFixture& generalFixture) {
    // Mock the overload service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockOverloadService = new OverloadServiceMock;
    g_OverloadService.reset(mockOverloadService);

    // Mock the concepts service
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto mockConceptsService = new ConceptsServiceMock;
    g_ConceptsService.reset(mockConceptsService);

    using TypeFactory::g_conceptDecls;
    using TypeFactory::g_dataTypeDecls;

    // Create the basic types
    g_dataTypeDecls.push_back(generalFixture.createNativeDatatypeNode(StringRef("i8"), generalFixture.globalContext_));
    g_dataTypeDecls.push_back(generalFixture.createNativeDatatypeNode(StringRef("i16"), generalFixture.globalContext_));
    g_dataTypeDecls.push_back(generalFixture.createNativeDatatypeNode(StringRef("i32"), generalFixture.globalContext_));
    Node* fooTypeDecl = generalFixture.createDatatypeNode(StringRef("FooType"), generalFixture.globalContext_);
    Node* barTypeDecl = generalFixture.createDatatypeNode(StringRef("BarType"), generalFixture.globalContext_);
    Node* nullTypeDecl = generalFixture.createDatatypeNode(StringRef("NullType"), generalFixture.globalContext_);
    g_dataTypeDecls.push_back(fooTypeDecl);
    g_dataTypeDecls.push_back(barTypeDecl);
    g_dataTypeDecls.push_back(nullTypeDecl);


    // Create an implicit conversion from FooType to BarType
    Nest::NodeHandle(barTypeDecl).setProperty(propConvert, 1);
    fooType_ = DataType::get(fooTypeDecl, 0, modeRt);
    barType_ = DataType::get(barTypeDecl, 0, modeRt);
    mockOverloadService->implicitConversions_.emplace_back(make_pair(fooType_, barType_));

    // Ensure we set the Null type
    nullType_ = DataType::get(nullTypeDecl, 0, modeRt);
    SprFrontend::StdDef::typeNull = nullType_;
    SprFrontend::StdDef::clsNull = nullTypeDecl;

    // Ensure we set the Type type -- but don't add it to our conversion types
    Node* typeDecl = generalFixture.createDatatypeNode(StringRef("Type"), generalFixture.globalContext_);
    SprFrontend::StdDef::typeType = DataType::get(typeDecl, 0, modeCt);
    REQUIRE(Nest_computeType(typeDecl) != nullptr);

    // Create concept types
    Node* concept1 = generalFixture.createSimpleConcept(StringRef("Concept1"), generalFixture.globalContext_);
    Node* concept2 = generalFixture.createSimpleConcept(StringRef("Concept2"), generalFixture.globalContext_);
    REQUIRE(Nest_computeType(concept1) != nullptr);
    REQUIRE(Nest_computeType(concept2) != nullptr);
    g_conceptDecls.push_back(concept1);
    g_conceptDecls.push_back(concept2);
    concept1Type_ = ConceptType::get(concept1);
    concept2Type_ = ConceptType::get(concept2);

    // Make concept -> type associations
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept1, barType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, fooType_));
    mockConceptsService->conceptFulfillments_.emplace_back(make_pair(concept2, barType_));

    // Concept base
    mockConceptsService->baseConcepts_.emplace_back(make_pair(concept2, concept1Type_));
}
