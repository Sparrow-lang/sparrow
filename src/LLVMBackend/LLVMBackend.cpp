#include <StdInc.h>
#include "LLVMBackend.h"
#include "Generator.h"
#include "RtModule.h"
#include "CtModule.h"
#include "Tr/DebugInfo.h"

#include <Nest/Intermediate/Node.h>
#include <Nest/Frontend/SourceCode.h>
#include <Nest/Backend/BackendFactory.h>
#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>

#include <boost/lambda/construct.hpp>
#include <boost/bind.hpp>

using namespace LLVMB;
using namespace Nest;

LLVMBackend::LLVMBackend()
	: ctModule_(nullptr)
    , rtModule_(nullptr)
{
}

LLVMBackend::~LLVMBackend()
{
	delete rtModule_;
	delete ctModule_;
}

void LLVMBackend::init(const string& mainFilename)
{
    rtModule_ = new Tr::RtModule("LLVM backend module Runtime", mainFilename);
    ctModule_ = new Tr::CtModule("LLVM backend module CT");
}

void LLVMBackend::generateMachineCode(SourceCode& code)
{
    ASSERT(rtModule_);

    rtModule_->setCtToRtTranslator(boost::bind(&Nest_translateCtToRt, &code, _1));

    // Translate the root node
    Node* rootNode = code.mainNode;
    ASSERT(rootNode);
    ASSERT(rootNode->type);
    ASSERT(rootNode->nodeSemanticallyChecked);
    rtModule_->generate(rootNode);

    // Translate the additional nodes
    for ( Node* n: code.additionalNodes )
    {
        rtModule_->generate(n);
    }


    rtModule_->setCtToRtTranslator(boost::function<Node*(Node*)>());
}

void LLVMBackend::link(const string& outFilename)
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    ASSERT(rtModule_);

    // Generate code for the the global ctors and dtors
    rtModule_->generateGlobalCtorDtor();

    // If we are emitting debug information, finalize it
    if ( rtModule_->debugInfo() )
        rtModule_->debugInfo()->finalize();


    // Generate a dump for the RT module - just for debugging
    if ( s.dumpAssembly_ )
        generateAssembly(rtModule_->llvmModule(), outFilename + ".one.llvm");

    // Generate a dump for the CT module - just for debugging
    if ( s.dumpCtAssembly_ )
        generateAssembly(ctModule_->llvmModule(), outFilename + ".ct.llvm");

    // Do the linking for the RT module
    vector<llvm::Module*> modules;
    modules.push_back(&rtModule_->llvmModule());
    LLVMB::link(modules, outFilename);
}

void LLVMBackend::ctProcess(Node* node)
{
    ctModule_->ctProcess(node);
}

Node* LLVMBackend::ctEvaluate(Node* node)
{
    return ctModule_->ctEvaluate(node);
}

size_t LLVMBackend::sizeOf(TypeRef type)
{
    return dataLayoutHelper_.getSizeOf(type);
}
size_t LLVMBackend::alignmentOf(TypeRef type)
{
    return dataLayoutHelper_.getAlignOf(type);
}

void LLVMBackend::ctApiRegisterFun(const char* name, void* funPtr)
{
    return ctModule_->ctApiRegisterFun(name, funPtr);
}

void LLVMBackend::registerSelf()
{
    Nest::theCompiler().backendFactory().registerBackend("llvm",
        boost::lambda::new_ptr<LLVMBackend>());
}