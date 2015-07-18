#include <StdInc.h>
#include "TrFunction.h"
#include "TrType.h"
#include "TrContext.h"
#include "Scope.h"
#include "TrLocal.h"
#include "Module.h"
#include "Tr/DebugInfo.h"

#include <Feather/Nodes/Properties.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Var.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/Context.h>

#include <Nest/Intermediate/Type.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace
{
    /// Called to translate the function body
    /// If 'addressToStoreResult' is given, we add a mem-store to the result of 'node' in the given address
    void translateFunctionBody(Module& module, Node* node, llvm::BasicBlock& insertionPoint, llvm::IRBuilder<>& llvmBuilder, llvm::Value* addressToStoreResult = nullptr)
    {
        // Create a context for the body of the function
        TrContext trContext(module, &insertionPoint, llvmBuilder);

        // Translate the node, guarded by a scope guard
        {
            llvm::Value* value = nullptr;
            try
            {
                Scope scopeGuard(trContext, node->location);
	            value = translateNode(node, trContext);
            }
            catch (...)
            {
            }

            // Do we need to store the result of the node somewhere?
            if ( addressToStoreResult )
            {
                if ( !value )
                    REP_INTERNAL(node->location, "Cannot CT evaluate node: %1%") % node;

                // Create a 'store' instruction
                new llvm::StoreInst(value, addressToStoreResult, trContext.insertionPoint());
            }
        }

        // If the body block doesn't have a termination instruction, try to create a return instruction
        if ( trContext.hasValidInsertionPoint() && !trContext.insertionPoint()->getTerminator() )
        {
            if ( trContext.parentFun()->getReturnType()->isVoidTy() )
                llvm::ReturnInst::Create(module.llvmContext(), trContext.insertionPoint());
            else
                new llvm::UnreachableInst(module.llvmContext(), trContext.insertionPoint());
        }
    }
}


llvm::Function* Tr::translateFunction(Feather::Function* node, Module& module)
{
    node->computeType();

    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    // Check for ct/non-ct compatibility
    if ( !module.canUse(node->node()) )
        return nullptr;

    // If this function was already translated, don't do it again
    llvm::Function** funDecl = module.getNodePropertyValue<llvm::Function*>(node->node(), Module::propFunDecl);
    if ( funDecl )
        return *funDecl;

    // First, translate the prototype
    llvm::FunctionType* funType = static_cast<llvm::FunctionType*>(getLLVMType(node->type(), module));

    llvm::Function* f = nullptr;

    // Check if this is a standard/native type
    const string* nativeName = node->getPropertyString(propNativeName);
    string name = getName(node->node());
//    Feather::Class* cls = getParentClass(node->context());
//    if ( cls )
//    {
//        const string* clsDescription = cls->getPropertyString(propDescription);
//        name = (clsDescription ? *clsDescription : getName(cls)) + "." + name;
//    }
    const string& funName = nativeName ? *nativeName : name;

    // Debugging
//     if ( module.isCt() )
//         REP_INFO(node->location, "Preparing to translate function %1%") % node;

    // If we have a native external body, just create the declaration
    if ( nativeName && !node->body() )
    {
        f =  (llvm::Function*) module.llvmModule().getOrInsertFunction(*nativeName, funType);
        //f->dump();
    }
    else
    {
        // Make sure the function is semantically checked
        node->semanticCheck();

        size_t lineDiff = node->location().endLineNo() - node->location().startLineNo();
        bool preventInline = lineDiff > s.maxCountForInline_ || node->hasProperty(propNoInline);

        // Create the LLVM function object
        llvm::Function::LinkageTypes linkage = node->body() && !preventInline
            ? llvm::Function::PrivateLinkage
            : llvm::Function::ExternalLinkage;
        f = llvm::Function::Create(funType, linkage, funName, &module.llvmModule());

        // Do we have a name conflict?
        if ( f->getName() != funName )
        {
            // Check the old function
            llvm::Function* oldFun = module.llvmModule().getFunction(funName);
            if ( oldFun && !module.isFunctionDefined(oldFun) )
            {
                // The old function is not defined by this module, but still lives in the current LLVM module; it must be
                // an llvm backend code defined function.
                // Delete the one we just made and get the existing one
                f->eraseFromParent();
                f = oldFun;
            }
            // Otherwise, it's ok to have the llvm name changed
        }

        // Set the calling convention
        f->setCallingConv(translateCallingConv(node->callConvention()));

        // If we have a result parameter, mark it as sret
        if ( node->getPropertyNode(propResultParam) )
            f->addAttribute(1, llvm::Attribute::StructRet);

        // Heuristic for inlining
        if ( node->body() )
        {
            if ( preventInline )
                f->addFnAttr(llvm::Attribute::NoInline);
            else if ( lineDiff == 0 )
                f->addFnAttr(llvm::Attribute::AlwaysInline);
            else
                f->addFnAttr(llvm::Attribute::InlineHint);
        }

        // Currently we don't support exceptions, so add a nounwind attribute
        f->addFnAttr(llvm::Attribute::NoUnwind);

        // Set the function value as a property of the node
        module.setNodeProperty(node->node(), Module::propFunDecl, boost::any(f));
    }
    ASSERT(f);

    // If we are emitting debug information, emit function start
    llvm::IRBuilder<> llvmBuilder(module.llvmContext());
    if ( module.debugInfo() )
        module.debugInfo()->emitFunctionStart(llvmBuilder, node, f);

    if ( node->body() )
    {
        /// Mark this llvm function as added by this module
        module.addDefinedFunction(f);

        // Set the names for the parameters
        ASSERT(f->arg_size() == node->numParameters());
        size_t idx = 0;
        for ( auto argIt=f->arg_begin(); argIt!=f->arg_end(); ++argIt, ++idx )
        {
            argIt->setName(getName(node->getParameter(idx)->node()));
        }

        // Create the block in which we insert the code
        llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(module.llvmContext(), "", f, 0);

        // Similar to the web LLVM compiler, we add variables for parameters
        idx = 0;
        for ( auto argIt=f->arg_begin(); argIt!=f->arg_end(); ++argIt, ++idx )
        {
            DynNode* paramNode = node->getParameter(idx);
            Var* param = paramNode->explanation()->as<Var>();
            if ( !param )
                REP_INTERNAL(paramNode->location(), "Expected Var node; found %1%") % paramNode;
            llvm::AllocaInst* newVar = new llvm::AllocaInst(argIt->getType(), getName(param->node())+".addr", bodyBlock);
            newVar->setAlignment(param->alignment());
            new llvm::StoreInst(argIt, newVar, bodyBlock); // Copy the value of the parameter into it
            module.setNodeProperty(param->node(), Module::propValue, boost::any(newVar));
            Tr::setValue(module, *param->node(), newVar); // We point now to the new temp variable
        }

        // Translate the body
        translateFunctionBody(module, node->body()->node(), *bodyBlock, llvmBuilder);
    }

    // If we are emitting debug information, emit function end
    if ( module.debugInfo() )
        module.debugInfo()->emitFunctionEnd(llvmBuilder, node->location());

    // Debugging
//     if ( module.isCt() )
//     if ( getName(node) == "test5" )
//         f->dump();
        //REP_INFO(node->location, "CT process fun: %1%") % node;

    return f;
}

llvm::Function* Tr::makeFunThatCalls(Node* node, Module& module, const char* funName, bool expectsResult)
{
	// Create the function type
	vector<llvm::Type*> llvmParamTypes;
    if ( expectsResult )
    {
        ASSERT(node->type->hasStorage);

	    // Get the type of the expression, to be used with llvm code
	    llvm::Type* t = getLLVMType(node->type, module);
	    llvm::Type* pt = llvm::PointerType::get(t, 0);

        llvmParamTypes.reserve(1);
	    llvmParamTypes.push_back(pt);
    }
	llvm::FunctionType* funType = llvm::FunctionType::get(llvm::Type::getVoidTy(module.llvmContext()), llvmParamTypes, false);

	// Actually create the function
	llvm::Function* f = llvm::Function::Create(funType, llvm::Function::PrivateLinkage, funName, &module.llvmModule());
    if ( expectsResult )
    {
        f->addAttribute(1, llvm::Attribute::StructRet);
    }

    // Create the block in which we insert the code
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(module.llvmContext(), "", f, 0);

    // Debugging
//     if ( module.isCt() )
//         REP_INFO(node->location, "Trying to generate: %1%") % (string) f->getName();

    // Add the action as the body of the function
    llvm::IRBuilder<> llvmBuilder(module.llvmContext());
    llvm::Value* addressToStoreResult = expectsResult ? f->arg_begin() : nullptr;
    translateFunctionBody(module, node, *bodyBlock, llvmBuilder, addressToStoreResult);

    // Debugging
//     if ( module.isCt() )
//         f->dump();
        //REP_INFO(node->location, "CT process fun: %1%") % node;

    return f;
}

llvm::CallingConv::ID Tr::translateCallingConv(CallConvention conv)
{
    switch ( conv )
    {
    case ccC:           return llvm::CallingConv::C;
    case ccFast:        return llvm::CallingConv::Fast;
    case ccCold:        return llvm::CallingConv::Cold;
    case ccGHC:         return llvm::CallingConv::GHC;

    case ccX86Std:      return llvm::CallingConv::X86_StdCall;
    case ccX86Fast:     return llvm::CallingConv::X86_FastCall;
    case ccX86ThisCall: return llvm::CallingConv::X86_ThisCall;
    case ccARMAPCS:     return llvm::CallingConv::ARM_APCS;
    case ccARMAAPCS:    return llvm::CallingConv::ARM_AAPCS;
    case ccARMAAPCSVFP: return llvm::CallingConv::ARM_AAPCS_VFP;
    case ccMSP430Intr:  return llvm::CallingConv::MSP430_INTR;
    case ccPTXKernel:   return llvm::CallingConv::PTX_Kernel;
    case ccPTXDevice:   return llvm::CallingConv::PTX_Device;
//    case ccMBLAZEIntr:  return llvm::CallingConv::MBLAZE_INTR;
//    case ccMBLAZESVol:  return llvm::CallingConv::MBLAZE_SVOL;
    default:            return llvm::CallingConv::C;
    }
}
