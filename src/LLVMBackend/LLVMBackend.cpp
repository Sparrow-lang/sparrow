#include <StdInc.h>
#include "LLVMBackend.h"
#include "Generator.h"
#include "RtModule.h"
#include "CtModule.h"
#include "DataLayoutHelper.h"
#include "Tr/DebugInfo.h"

#include <Nest/Intermediate/Node.h>
#include <Nest/Frontend/SourceCode.h>
#include <Nest/Backend/Backend.h>
#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>

#include <boost/bind.hpp>

using namespace LLVMB;

/// The structure representing a LLVM backend
struct LLVMBackend {
    Backend baseData;
    Tr::RtModule* rtModule;
    Tr::CtModule* ctModule;
    DataLayoutHelper* dataLayoutHelper;
};

void _llvmBeInit(Backend* backend, const char* mainFilename);
void _llvmBeGenerateMachineCode(Backend* backend, const SourceCode* code);
void _llvmBeLink(Backend* backend, const char* outFilename);
void _llvmBeCtProcess(Backend* backend, Node* node);
Node* _llvmBeCtEvaluate(Backend* backend, Node* node);
unsigned int _llvmBeSizeOf(Backend* backend, TypeRef type);
unsigned int _llvmBeAlignmentOf(Backend* backend, TypeRef type);
void _llvmBeCtApiRegisterFun(Backend* backend, const char* name, void* funPtr);

struct LLVMBackend _llvmBackend = {
    { "llvm", "backend that uses LLVM to generate code",
        &_llvmBeInit,
        &_llvmBeGenerateMachineCode,
        &_llvmBeLink,
        &_llvmBeCtProcess,
        &_llvmBeCtEvaluate,
        &_llvmBeSizeOf,
        &_llvmBeAlignmentOf,
        &_llvmBeCtApiRegisterFun
    },
    NULL,
    NULL,
    NULL
};

void _llvmBeInit(Backend* backend, const char* mainFilename)
{
    _llvmBackend.rtModule = new Tr::RtModule("LLVM backend module Runtime", mainFilename);
    _llvmBackend.ctModule = new Tr::CtModule("LLVM backend module CT");
    _llvmBackend.dataLayoutHelper = new DataLayoutHelper();
}

void _llvmBeGenerateMachineCode(Backend* backend, const SourceCode* code)
{
    ASSERT(_llvmBackend.rtModule);

    _llvmBackend.rtModule->setCtToRtTranslator(boost::bind(&Nest_translateCtToRt, code, _1));

    // Translate the root node
    Node* rootNode = code->mainNode;
    ASSERT(rootNode);
    ASSERT(rootNode->type);
    ASSERT(rootNode->nodeSemanticallyChecked);
    _llvmBackend.rtModule->generate(rootNode);

    // Translate the additional nodes
    for ( Node* n: code->additionalNodes )
    {
        _llvmBackend.rtModule->generate(n);
    }

    _llvmBackend.rtModule->setCtToRtTranslator(LLVMB::Module::NodeFun());
}

void _llvmBeLink(Backend* backend, const char* outFilename)
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    ASSERT(_llvmBackend.rtModule);

    // Generate code for the the global ctors and dtors
    _llvmBackend.rtModule->generateGlobalCtorDtor();

    // If we are emitting debug information, finalize it
    if ( _llvmBackend.rtModule->debugInfo() )
        _llvmBackend.rtModule->debugInfo()->finalize();


    // Generate a dump for the RT module - just for debugging
    if ( s.dumpAssembly_ )
        generateAssembly(_llvmBackend.rtModule->llvmModule(), string(outFilename) + ".one.llvm");

    // Generate a dump for the CT module - just for debugging
    if ( s.dumpCtAssembly_ )
        generateAssembly(_llvmBackend.ctModule->llvmModule(), string(outFilename) + ".ct.llvm");

    // Do the linking for the RT module
    vector<llvm::Module*> modules;
    modules.push_back(&_llvmBackend.rtModule->llvmModule());
    LLVMB::link(modules, outFilename);
}

void _llvmBeCtProcess(Backend* backend, Node* node)
{
    _llvmBackend.ctModule->ctProcess(node);
}

Node* _llvmBeCtEvaluate(Backend* backend, Node* node)
{
    return _llvmBackend.ctModule->ctEvaluate(node);
}

unsigned int _llvmBeSizeOf(Backend* backend, TypeRef type)
{
    return _llvmBackend.dataLayoutHelper->getSizeOf(type);
}
unsigned int _llvmBeAlignmentOf(Backend* backend, TypeRef type)
{
    return _llvmBackend.dataLayoutHelper->getAlignOf(type);
}

void _llvmBeCtApiRegisterFun(Backend* backend, const char* name, void* funPtr)
{
    return _llvmBackend.ctModule->ctApiRegisterFun(name, funPtr);
}

int LLVMBe_registerLLVMBackend() {
    return Nest_registerBackend((Backend*) &_llvmBackend);
}

