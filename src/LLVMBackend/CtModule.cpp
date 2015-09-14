#include <StdInc.h>
#include "CtModule.h"
#include <Tr/TrTopLevel.h>
#include <Tr/TrFunction.h>
#include <Tr/TrType.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/FeatherTypes.h>

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/Type.h>
#include <Nest/Common/Diagnostic.hpp>

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>               // We need this include to instantiate the JIT engine
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Feather;

CtModule::CtModule(const string& name)
    : Module(name)
	, llvmExecutionEngine_(nullptr)
{
	llvm::InitializeNativeTarget();

	// Now we going to create JIT
	string errStr;
	llvmExecutionEngine_ =
		llvm::EngineBuilder(llvmModule_)
		.setErrorStr(&errStr)
		.setEngineKind(llvm::EngineKind::JIT)
        .setOptLevel(llvm::CodeGenOpt::None)
		.create();

	if ( !llvmExecutionEngine_ )
        REP_INTERNAL(NOLOC, "Failed to construct LLVM ExecutionEngine: %1%") % errStr;
}

CtModule::~CtModule()
{
    if ( llvmExecutionEngine_ )
    {
        delete llvmExecutionEngine_;    // Also deletes llvmCtModule_
        llvmModule_ = nullptr;
    }
}

void CtModule::ctProcess(Node* node)
{
    // This should be called now only for BackendCode; the rest of CT declarations are processed when referenced

	// Uncomment this for CT debugging
//     cerr << "----------------------------" << endl;
//     cerr << "CT process: " << node << endl;
    //REP_INFO(node->location, "CT process: %1%") % node;

    // Make sure the node is semantically checked
	if ( !node->nodeSemanticallyChecked )
		REP_INTERNAL(node->location, "Node should be semantically checked when passed to the backend");

	// Make sure the type of the node can be used at compile time
	if ( node->type->mode == modeRt )
		REP_INTERNAL(node->location, "Cannot CT process this node: it has no meaning at compile-time");

    switch ( node->nodeKind - firstFeatherNodeKind )
    {
    case nkRelFeatherExpCtValue:   return;
    case nkRelFeatherDeclVar:      ctProcessVariable(node); break;
    case nkRelFeatherDeclFunction: ctProcessFunction(node); break;
    case nkRelFeatherDeclClass:    ctProcessClass(node); break;
    case nkRelFeatherBackendCode:  ctProcessBackendCode(node); break;
    default:
        REP_INTERNAL(node->location, "Don't know how to CT process node (%1%)") % node;
    }
}

Node* CtModule::ctEvaluate(Node* node)
{
    // Make sure the node is semantically checked
	if ( !node->nodeSemanticallyChecked )
	{
		REP_INTERNAL(node->location, "Node should be semantically checked when passed to the backend");
		return nullptr;
	}

	// Make sure the type of the node can be used at compile time
	if ( !Feather::isCt(node) )
		REP_INTERNAL(node->location, "Cannot CT evaluate this node: it has no meaning at compile-time");

    if ( !node->type->hasStorage && node->type->typeKind != typeKindVoid )
		REP_INTERNAL(node->location, "Cannot CT evaluate node with non-storage type (type: %1%)") % node->type;


    node = Nest_explanation(node);
	if ( node->nodeKind == nkFeatherExpCtValue )
	{
		return node;    // Nothing to do
	}
	else
	{
		return ctEvaluateExpression(node);
	}
}

void CtModule::ctApiRegisterFun(const char* name, void* funPtr)
{
    llvm::FunctionType* llvmFunType = llvm::FunctionType::get(llvm::Type::getVoidTy(*llvmContext_), {}, false);
    llvm::Function* fun = llvm::Function::Create(llvmFunType, llvm::Function::ExternalLinkage, name, llvmModule_);
    llvmExecutionEngine_->addGlobalMapping(fun, funPtr);
}

void CtModule::addGlobalCtor(llvm::Function* /*fun*/)
{
}

void CtModule::addGlobalDtor(llvm::Function* /*fun*/)
{
}

CtModule::NodeFun CtModule::ctToRtTranslator() const
{
    return NodeFun();
}


void CtModule::ctProcessVariable(Node* node)
{
    Tr::translateGlobalVar(node, *this);
	//llvmModule().dump();
}

void CtModule::ctProcessFunction(Node* node)
{
    llvm::Function* f = Tr::translateFunction(node, *this);
    ((void) f);

	// Uncomment this for CT debugging
    //cerr << "----------------------------" << endl;
    //cerr << "CT process fun: " << node << endl;
	//f->dump();
	//llvmCtModule_->dump();
    //REP_INFO(node->location, "CT process fun: %1%") % node;
}

void CtModule::ctProcessClass(Node* node)
{
    Tr::translateClass(node, *this);
}

void CtModule::ctProcessBackendCode(Node* node)
{
    Tr::translateBackendCode(node, *this);
}

Node* CtModule::ctEvaluateExpression(Node* node)
{
	// Create a function of type 'void f(void*)', which will execute our expression node and put the result at the address 
	llvm::Function* f = Tr::makeFunThatCalls(node, *this, "ctEval", node->type->hasStorage);


	// Uncomment this for CT debugging
//    REP_INFO(node->location, "CT eval: %1%") % node;
//    cerr << "----------------------------" << endl;
//    cerr << node->location.toString() << " - eval: " << node << endl;
//    f->dump();
//    llvmModule_->dump();

	// Validate the generated code, checking for consistency.
    if ( llvm::verifyFunction(*f) )
    {
        REP_ERROR(node->location, "Error constructing CT evaluation function");
        f->dump();
        return nullptr;
    }

    if ( node->type->hasStorage )
    {
	    // Create a memory space where to put the result
        llvm::Type* llvmType = Tr::getLLVMType(node->type, *this);
        size_t size = llvmModule_->getDataLayout()->getTypeAllocSize(llvmType);
	    string dataBuffer(size, (char) 0);

	    vector<llvm::GenericValue> args(1);
	    args[0] = llvm::GenericValue(&dataBuffer[0]);
	    llvmExecutionEngine_->runFunction(f, args);
        llvmExecutionEngine_->freeMachineCodeForFunction(f);

        // Create a CtValue containing the data resulted from expression evaluation
        TypeRef t = node->type;
        if ( !Feather::isCt(t) )
	        t = Feather::changeTypeMode(t, modeCt, node->location);
	    return mkCtValue(node->location, t, dataBuffer);
    }
    else
    {
	    vector<llvm::GenericValue> args(0);
	    llvmExecutionEngine_->runFunction(f, args);
        llvmExecutionEngine_->freeMachineCodeForFunction(f);

	    // Create a Nop operation for return
        return mkNop(node->location);
    }
}
