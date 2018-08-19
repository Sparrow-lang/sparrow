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
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/CompilerSettings.hpp"

namespace {

void destroyModule(Nest_CompilerModule* mod) {
    if (mod->destroyFun)
        mod->destroyFun();
}

} // namespace

NestGeneralFixture::NestGeneralFixture() {
    // Initialize the Nest module
    getNestModule()->initFun();

    // Create the backend object and register it
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    backend_.reset(new BackendMock);
    int backendIdx = Nest_registerBackend(backend_.get());
    REQUIRE(backendIdx == 0);

    // Initialize the compiler
    Nest_createBackend("unittest_app");
    globalContext_ = Nest_getRootContext();

    // Set up some compiler settings
    Nest_compilerSettings()->noColors_ = true;
}
NestGeneralFixture::~NestGeneralFixture() {
    // Cleanup the Nest module
    destroyModule(getNestModule());
}

Location NestGeneralFixture::createLocation() {
    // If we don't have a sourceCode yet, create one
    if (!sourceCode_) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        sourceCode_ = (Nest_SourceCode*)alloc(sizeof(Nest_SourceCode), allocGeneral);
        sourceCode_->kind = SprFe_kindSparrowSourceCode;
        sourceCode_->url = "$testFile$.spr";
        sourceCode_->mainNode = nullptr;
    }
    const unsigned int line = lastLineNum_++;
    return {sourceCode_, {line, 1}, {line, 2}};
}


SparrowGeneralFixture::SparrowGeneralFixture() {
    // Initialize the Feather & Sparrowmodules
    Feather_getModule()->initFun();
    getSparrowFrontendModule()->initFun();
}
SparrowGeneralFixture::~SparrowGeneralFixture() {
    // Cleanup any test statics
    TypeFactory::g_dataTypeDecls.clear();
    TypeFactory::g_conceptDecls.clear();

    // Cleanup our modules
    destroyModule(getSparrowFrontendModule());
    destroyModule(Feather_getModule());
}

Node* SparrowGeneralFixture::createDatatypeNode(StringRef name, CompilationContext* ctx) {
    Nest_NodeRange fields{nullptr, nullptr};
    auto res = Feather_mkClass(createLocation(), name, fields);
    if (ctx)
        Nest_setContext(res, ctx);
    return res;
}

Node* SparrowGeneralFixture::createNativeDatatypeNode(StringRef name, CompilationContext* ctx) {
    auto res = createDatatypeNode(name, ctx);
    Nest_setPropertyString(res, propNativeName, name);
    return res;
}

Node* SparrowGeneralFixture::createSimpleConcept(StringRef name, CompilationContext* ctx) {
    auto res = SprFrontend::mkSprConcept(createLocation(), name, StringRef("x"), nullptr, nullptr);
    if (ctx)
        Nest_setContext(res, ctx);
    return res;
}
