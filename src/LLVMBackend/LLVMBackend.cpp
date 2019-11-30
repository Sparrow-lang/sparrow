#include <StdInc.h>
#include "LLVMBackend.h"
#include "Generator.h"
#include "RtModule.h"
#include "CtModule.h"
#include "DataLayoutHelper.h"
#include "Tr/DebugInfo.h"

#include "Nest/Api/Node.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/Backend.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Debug.h>

#include <functional>

using namespace LLVMB;
using Nest::TypeRef;

/// The structure representing a LLVM backend
struct LLVMBackend {
    Nest_Backend baseData;
    Tr::RtModule* rtModule;
    CtModule* ctModule;
    DataLayoutHelper* dataLayoutHelper;
};

void _llvmBeInit(Nest_Backend* backend, const char* mainFilename);
void _llvmBeGenerateMachineCode(Nest_Backend* backend, const Nest_SourceCode* code);
void _llvmBeLink(Nest_Backend* backend, const char* outFilename);
void _llvmBeCtProcess(Nest_Backend* backend, Node* node);
Node* _llvmBeCtEvaluate(Nest_Backend* backend, Node* node);
unsigned int _llvmBeSizeOf(Nest_Backend* backend, TypeRef type);
unsigned int _llvmBeAlignmentOf(Nest_Backend* backend, TypeRef type);
void _llvmBeCtApiRegisterFun(Nest_Backend* backend, const char* name, void* funPtr);

struct LLVMBackend _llvmBackend = {
        {"llvm", "backend that uses LLVM to generate code", &_llvmBeInit,
                &_llvmBeGenerateMachineCode, &_llvmBeLink, &_llvmBeCtProcess, &_llvmBeCtEvaluate,
                &_llvmBeSizeOf, &_llvmBeAlignmentOf, &_llvmBeCtApiRegisterFun},
        nullptr, nullptr, nullptr};

void _llvmBeInit(Nest_Backend* backend, const char* mainFilename) {
    // Init LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // llvm::DebugFlag = true;
    // llvm::setCurrentDebugType("dyld");

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    _llvmBackend.rtModule = new Tr::RtModule("LLVM backend module Runtime", mainFilename);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    _llvmBackend.ctModule = new CtModule("LLVM backend module CT");
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    _llvmBackend.dataLayoutHelper = new DataLayoutHelper();
}

void _llvmBeGenerateMachineCode(Nest_Backend* backend, const Nest_SourceCode* code) {
    ASSERT(_llvmBackend.rtModule);

    _llvmBackend.rtModule->setCtToRtTranslator(std::bind(&Nest_translateCtToRt, code, placeholders::_1));

    // Translate the root node
    Node* rootNode = code->mainNode;
    ASSERT(rootNode);
    ASSERT(rootNode->type);
    ASSERT(rootNode->nodeSemanticallyChecked);
    _llvmBackend.rtModule->generate(rootNode);

    _llvmBackend.rtModule->setCtToRtTranslator(LLVMB::Module::NodeFun());

    // TODO (modules): Add missing parts here
    //
    // This is called for every source-code that we compile.
    // However, as we don't have proper module linkage, we cannot generate code
    // here for every module. Therefore, we move the activities that we need to
    // do here in the linker part.
    //
    // Activities to be performed here:
    //      - add global ctors/dtors
    //      - verify LLVM module
    //      - assemble (generate .o file)
}

void _llvmBeLink(Nest_Backend* backend, const char* outFilename) {
    ASSERT(_llvmBackend.rtModule);

    // Generate code for the the global ctors and dtors
    _llvmBackend.rtModule->generateGlobalCtorDtor();

    // If we are emitting debug information, finalize it
    if (_llvmBackend.rtModule->debugInfo())
        _llvmBackend.rtModule->debugInfo()->finalize();

    const auto& s = *Nest_compilerSettings();

    // Generate a dump for the RT module - just for debugging
    if (s.dumpAssembly_)
        generateRtAssembly(_llvmBackend.rtModule->llvmModule());

    // Do the linking for the RT module
    vector<llvm::Module*> modules;
    modules.push_back(&_llvmBackend.rtModule->llvmModule());
    LLVMB::link(modules, outFilename);
}

void _llvmBeCtProcess(Nest_Backend* backend, Node* node) { _llvmBackend.ctModule->ctProcess(node); }

Node* _llvmBeCtEvaluate(Nest_Backend* backend, Node* node) {
    return _llvmBackend.ctModule->ctEvaluate(node);
}

unsigned int _llvmBeSizeOf(Nest_Backend* backend, TypeRef type) {
    return _llvmBackend.dataLayoutHelper->getSizeOf(type);
}
unsigned int _llvmBeAlignmentOf(Nest_Backend* backend, TypeRef type) {
    return _llvmBackend.dataLayoutHelper->getAlignOf(type);
}

void _llvmBeCtApiRegisterFun(Nest_Backend* backend, const char* name, void* funPtr) {
    return _llvmBackend.ctModule->ctApiRegisterFun(name, funPtr);
}

int LLVMBe_registerLLVMBackend() {
    // NOLINTNEXTLINE
    return Nest_registerBackend(reinterpret_cast<Nest_Backend*>(&_llvmBackend));
}
