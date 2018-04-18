#include "StdInc.h"
#include "GeneralFixture.hpp"
#include "TypeFactory.hpp"

#include "SparrowFrontend/SparrowSourceCode.h"
#include "SparrowFrontend/SparrowFrontend.h"
#include "Feather/Api/Feather.h"
#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/NodeUtils.h"

GeneralFixture* GeneralFixture::lastInstance_{nullptr};

GeneralFixture::GeneralFixture() {
    // Initialize the modules
    CompilerModule* mods[] = {getNestModule(), Feather_getModule(), getSparrowFrontendModule()};
    for (int i = 0; i < 3; i++)
        mods[i]->initFun();

    globalContext_ = Nest_mkRootContext(nullptr, modeRt);
    lastInstance_ = this;
}
GeneralFixture::~GeneralFixture() {
    cleanupMemory();
    lastInstance_ = nullptr;
    TypeFactory::g_dataTypeDecls.clear();
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
