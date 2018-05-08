#include "StdInc.h"
#include "GeneralFixture.hpp"
#include "TypeFactory.hpp"
#include "BackendMock.hpp"

#include "SparrowFrontend/SparrowSourceCode.h"
#include "SparrowFrontend/SparrowFrontend.h"
#include "SparrowFrontend/Nodes/SparrowNodes.h"
#include "Feather/Api/Feather.h"
#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/CompilerSettings.hpp"

GeneralFixture* GeneralFixture::lastInstance_{nullptr};

GeneralFixture::GeneralFixture() {
    // Initialize the modules
    CompilerModule* mods[] = {getNestModule(), Feather_getModule(), getSparrowFrontendModule()};
    for (int i = 0; i < 3; i++)
        mods[i]->initFun();

    // Create the backend object and register it
    backend_.reset(new BackendMock);
    int backendIdx = Nest_registerBackend(backend_.get());
    REQUIRE(backendIdx == 0);

    // Initialize the compiler
    Nest_createBackend("unittest_app");
    globalContext_ = Nest_getRootContext();

    // Set up some compiler settings
    Nest_compilerSettings()->noColors_ = true;

    lastInstance_ = this;
}
GeneralFixture::~GeneralFixture() {
    // Cleanup any test statics
    TypeFactory::g_dataTypeDecls.clear();
    TypeFactory::g_conceptDecls.clear();

    // Cleanup Nest
    Nest_clearBackends();
    // Nest_resetRegisteredTypeKinds();
    Nest_resetRegisteredNodeKinds();

    // Cleanup all allocated memory
    cleanupMemory();

    // Reset the last instance of this class
    lastInstance_ = nullptr;
}

GeneralFixture& GeneralFixture::instance() { return *lastInstance_; }

Location GeneralFixture::createLocation() {
    // If we don't have a sourceCode yet, create one
    if (!sourceCode_) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        sourceCode_ = (SourceCode*)alloc(sizeof(SourceCode), allocGeneral);
        sourceCode_->kind = SprFe_kindSparrowSourceCode;
        sourceCode_->url = "$testFile$.spr";
        sourceCode_->mainNode = nullptr;
    }
    const unsigned int line = lastLineNum_++;
    return {sourceCode_, {line, 1}, {line, 2}};
}

Node* GeneralFixture::createDatatypeNode(StringRef name, CompilationContext* ctx) {
    NodeRange fields{nullptr, nullptr};
    auto res = Feather_mkClass(createLocation(), name, fields);
    if (ctx)
        Nest_setContext(res, ctx);
    return res;
}

Node* GeneralFixture::createNativeDatatypeNode(StringRef name, CompilationContext* ctx) {
    auto res = createDatatypeNode(name, ctx);
    Nest_setPropertyString(res, propNativeName, name);
    return res;
}

Node* GeneralFixture::createSimpleConcept(StringRef name, CompilationContext* ctx) {
    auto res = SprFrontend::mkSprConcept(createLocation(), name, fromCStr("x"), nullptr, nullptr);
    if (ctx)
        Nest_setContext(res, ctx);
    return res;
}
