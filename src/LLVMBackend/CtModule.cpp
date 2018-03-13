#include <StdInc.h>
#include "CtModule.h"
#include "Generator.h"
#include <Tr/TrTopLevel.h>
#include <Tr/TrFunction.h>
#include <Tr/TrType.h>
#include <Tr/GlobalContext.h>
#include <Tr/PrepareTranslate.h>

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/Node.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/CompilerStats.hpp"

#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h> // We need this include to instantiate the JIT engine

#include <memory>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Feather;

CtModule::CtModule(const string& name)
    : Module(name)
    , llvmExecutionEngine_(nullptr)
    , llvmModule_(new llvm::Module(name, *llvmContext_))
    , dataLayout_("") {
    CompilerSettings& s = *Nest_compilerSettings();

    llvmModule_->setDataLayout(s.dataLayout_);
    llvmModule_->setTargetTriple(s.targetTriple_);

    // Create the execution engine object
    string errStr;
    llvmExecutionEngine_ = llvm::EngineBuilder(move(llvmModule_))
                                   .setErrorStr(&errStr)
                                   .setEngineKind(llvm::EngineKind::JIT)
                                   .setOptLevel(llvm::CodeGenOpt::None)
                                   .create();

    if (!llvmExecutionEngine_)
        REP_INTERNAL(NOLOC, "Failed to construct LLVM ExecutionEngine: %1%") % errStr;

    // Get the actual data layout to be used
    dataLayout_ = llvmExecutionEngine_->getDataLayout();

    // The execution engine consumed the current LLVM module
    // Make sure to create another one.
    recreateModule();
}

CtModule::~CtModule() { delete llvmExecutionEngine_; }

void CtModule::ctProcess(Node* node) {
    // This should be called now only for BackendCode; the rest of CT declarations are processed
    // when referenced

    // Make sure the node is semantically checked
    if (!node->nodeSemanticallyChecked)
        REP_INTERNAL(
                node->location, "Node should be semantically checked when passed to the backend");

    // Make sure the type of the node can be used at compile time
    if (node->type->mode == modeRt)
        REP_INTERNAL(
                node->location, "Cannot CT process this node: it has no meaning at compile-time");

    switch (node->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherExpCtValue:
        return;
    case nkRelFeatherDeclVar:
        ctProcessVariable(node);
        break;
    case nkRelFeatherDeclFunction:
        ctProcessFunction(node);
        break;
    case nkRelFeatherDeclClass:
        ctProcessClass(node);
        break;
    case nkRelFeatherBackendCode:
        ctProcessBackendCode(node);
        break;
    default:
        REP_INTERNAL(node->location, "Don't know how to CT process node (%1%)") % node;
    }
}

Node* CtModule::ctEvaluate(Node* node) {
    // Make sure the node is semantically checked
    if (!node->nodeSemanticallyChecked) {
        REP_INTERNAL(
                node->location, "Node should be semantically checked when passed to the backend");
        return nullptr;
    }

    // Make sure the type of the node can be used at compile time
    if (!Feather_isCt(node))
        REP_INTERNAL(
                node->location, "Cannot CT evaluate this node: it has no meaning at compile-time");

    if (!node->type->hasStorage && node->type->typeKind != typeKindVoid)
        REP_INTERNAL(node->location, "Cannot CT evaluate node with non-storage type (type: %1%)") %
                node->type;

    node = Nest_explanation(node);
    if (node->nodeKind == nkFeatherExpCtValue) {
        return node; // Nothing to do
    } else {
        return ctEvaluateExpression(node);
    }
}

void CtModule::ctApiRegisterFun(const char* name, void* funPtr) {
    ASSERT(llvmModule_);
    llvm::FunctionType* llvmFunType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*llvmContext_), {}, false);
    llvm::Function* fun = llvm::Function::Create(
            llvmFunType, llvm::Function::ExternalLinkage, name, llvmModule_.get());
    llvmExecutionEngine_->addGlobalMapping(fun, funPtr);
}

void CtModule::addGlobalCtor(llvm::Function* /*fun*/) {}

void CtModule::addGlobalDtor(llvm::Function* /*fun*/) {}

CtModule::NodeFun CtModule::ctToRtTranslator() const { return NodeFun(); }

void CtModule::recreateModule() {
    ASSERT(!llvmModule_);
    llvmModule_ = llvm::make_unique<llvm::Module>("CT module", *llvmContext_);
    ASSERT(llvmModule_);

    // Ensure we have exactly the same data layout as our execution engine
    llvmModule_->setDataLayout(dataLayout_);
}

void CtModule::syncModule() {
    if (!llvmModule_->empty() || !llvmModule_->global_empty()) {
        // Uncomment this for debugging
        // llvmModule_->dump();

        // Generate a dump for the CT module - just for debugging
        const auto& s = *Nest_compilerSettings();
        if (s.dumpCtAssembly_)
            generateCtAssembly(*llvmModule_);

        ASSERT(llvmModule_);
        llvmExecutionEngine_->addModule(move(llvmModule_));

        // Recreate the module, to use it for the anonymous expression evaluation
        recreateModule();
    }
}

void CtModule::ctProcessVariable(Node* node) {
    Tr::GlobalContext ctx(*llvmModule_, *this);
    Tr::translateGlobalVar(node, ctx);
}

void CtModule::ctProcessFunction(Node* node) {
    Tr::GlobalContext ctx(*llvmModule_, *this);
    llvm::Function* f = Tr::translateFunction(node, ctx);
    ((void)f);
}

void CtModule::ctProcessClass(Node* node) {
    Tr::GlobalContext ctx(*llvmModule_, *this);
    Tr::translateClass(node, ctx);
}

void CtModule::ctProcessBackendCode(Node* node) {
    Tr::GlobalContext ctx(*llvmModule_, *this);
    Tr::translateBackendCode(node, ctx);
}

Node* CtModule::ctEvaluateExpression(Node* node) {
    ASSERT(llvmModule_);

    // Gather statistics if requested
    CompilerStats& stats = CompilerStats::instance();
    if (stats.enabled)
        ++stats.numCtEvals;
    ScopedTimeCapture timeCapture(stats.enabled, stats.timeCtEvals);

    static int counter = 0;
    ostringstream oss;
    oss << "__anonExprEval." << counter++;
    const string& funName = oss.str();

    // Ensure the node is prepared for translation.
    // This means we won't be doing any more semantic checks while translating, avoiding reentrant
    // calls.
    Tr::GlobalContext ctxMain(*llvmModule_, *this);
    Tr::prepareTranslate(node, ctxMain);

    // Check for re-entrant calls
    static volatile int numActiveCalls = 0;
    struct IncDec {
        volatile int& val_;
        IncDec(volatile int& val)
            : val_(++val) {}
        ~IncDec() { --val_; }
    };
    IncDec scopeIncDec(numActiveCalls);
    if (numActiveCalls > 1) {
        static int numReentrantCalls = 0;
        ++numReentrantCalls;
        REP_INTERNAL(node->location, "Reentrant ctEval detected: %1%") % Nest_toStringEx(node);
    }

    // If we've added some definitions so far, add them to the execution engine
    syncModule();

    // We will add our anonymous expression into this module
    ASSERT(llvmModule_);

    // Create a new LLVM module for this function, an a corresponding global context
    unique_ptr<llvm::Module> anonExprEvalModule(
            new llvm::Module("CT anonymous expression eval module", *llvmContext_));
    llvm::Module* tmpMod = anonExprEvalModule.get();
    anonExprEvalModule->setDataLayout(dataLayout_);
    Tr::GlobalContext ctx(*anonExprEvalModule, *llvmModule_, *this);

    // Create a function of type 'void f(void*)', which will execute our expression node and put the
    // result at the address
    llvm::Function* f = Tr::makeFunThatCalls(node, ctx, funName.c_str(), node->type->hasStorage);
    (void)f;

    // Translate the type here, before adding the module to the execution engine
    llvm::Type* resLlvmType = nullptr;
    if (node->type->hasStorage)
        resLlvmType = Tr::getLLVMType(node->type, ctx);

    // Sync again, just for safety
    syncModule();

    // Uncomment this for CT debugging
    // int old = Nest_isReportingEnabled();
    // Nest_enableReporting(1);
    // REP_INFO(node->location, "CT eval %1%: %2%") % counter % Nest_toStringEx(node);
    // Nest_enableReporting(old);
    // cerr << "----------------------------" << endl;
    // f->print(llvm::errs());
    // tmpMod->print(llvm::errs(), nullptr);
    // cerr << "----------------------------" << endl;

    // Generate a dump for the CT module - just for debugging
    const auto& s = *Nest_compilerSettings();
    if (s.dumpCtAssembly_)
        generateCtAssembly(*anonExprEvalModule);

    // Add the module containing this into the execution engine
    llvmExecutionEngine_->addModule(move(anonExprEvalModule));

    // Validate the generated code, checking for consistency.
    // string errMsg;
    // llvm::raw_string_ostream errStream(errMsg);
    // if ( llvm::verifyFunction(*f, &errStream) )
    // {
    //     REP_ERROR(node->location, "Error constructing CT evaluation function: %1%") % errMsg;
    //     if ( Nest_isReportingEnabled() )
    //         f->print(llvm::errs());
    //     return nullptr;
    // }

    Node* res = nullptr;
    if (node->type->hasStorage) {
        // Create a memory space where to put the result
        size_t size = dataLayout_.getTypeAllocSize(resLlvmType);
        StringRef dataBuffer = allocStringRef(size);

        // The magic is here:
        //  - finalize everything in the engine and get the function address
        //  - transform this into a function pointer that receives the output as a parameter
        //  - call the function, to fill up the data buffer
        using FunType = void (*)(const char*);
        auto fptr = (FunType)llvmExecutionEngine_->getFunctionAddress(funName);
        fptr(dataBuffer.begin);

        // Create a CtValue containing the data resulted from expression evaluation
        TypeRef t = node->type;
        if (t->mode != modeCt)
            t = Feather_checkChangeTypeMode(t, modeCt, node->location);
        res = Feather_mkCtValue(node->location, t, dataBuffer);
    } else {
        // The magic is here:
        //  - finalize everything in the engine and get the function address
        //  - transform this into a function pointer with no parameters
        //  - call the function
        using FunType = void (*)();
        auto fptr = (FunType)llvmExecutionEngine_->getFunctionAddress(funName);
        fptr();

        // Create a Nop operation for return
        res = Feather_mkNop(node->location);
    }

    // Remove our anonymous function from the execution engine
    llvmExecutionEngine_->updateGlobalMapping(funName, 0);

    // Try to remove the module that we just added
    llvmExecutionEngine_->removeModule(tmpMod);

    return res;
}
