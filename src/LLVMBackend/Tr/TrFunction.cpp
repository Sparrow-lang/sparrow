#include <StdInc.h>
#include "TrFunction.h"
#include "TrType.h"
#include "TrContext.h"
#include "Scope.h"
#include "TrLocal.h"
#include "Module.h"
#include "Tr/DebugInfo.h"
#include "Tr/GlobalContext.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/Type.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/CompilerSettings.hpp"

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace {
//! Create the LLVM declaration of the function.
llvm::Function* createFunDecl(Node* node, GlobalContext& ctx, TranslatedFunInfo& tfi) {
    llvm::Function* res = nullptr;

    // Get the type of the function
    llvm::FunctionType* funType = static_cast<llvm::FunctionType*>(getLLVMType(node->type, ctx));

    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    Node* body = Feather_Function_body(node);

    // If we don't already have a name for this function, create one
    if (size(tfi.name_) == 0) {
        if (nativeName)
            tfi.name_ = *nativeName;
        else {
            // Try the function name, without any suffix
            StringRef origName = Feather_getName(node);
            if (ctx.targetBackend_.funNamesTaken_.find(origName) ==
                    ctx.targetBackend_.funNamesTaken_.end())
                tfi.name_ = origName;
            else {
                static int counter = 0;
                ostringstream oss;
                oss << Feather_getName(node) << "." << counter++;
                tfi.name_ = dupString(oss.str());
            }
        }
        // Remember that we've used this name (only if we have body)
        if (body)
            ctx.targetBackend_.funNamesTaken_.insert(tfi.name_);
    }

    // If we have a native external body, just create the declaration
    if (nativeName && !body) {
        res = (llvm::Function*)ctx.llvmModule_.getOrInsertFunction(nativeName->begin, funType);
    } else {
        // Make sure the function is semantically checked
        if (!Nest_semanticCheck(node))
            return nullptr;

        CompilerSettings& s = *Nest_compilerSettings();
        int lineDiff = node->location.end.line - node->location.start.line;
        bool preventInline =
                lineDiff > s.maxCountForInline_ || Nest_hasProperty(node, propNoInline);

        // Create the LLVM function object
        bool canBeNonExtern = body && !preventInline && !tfi.mainDecl_ && !ctx.separateDefDecl() &&
                              !ctx.targetBackend_.isCt();
        llvm::Function::LinkageTypes linkage =
                canBeNonExtern ? llvm::Function::InternalLinkage : llvm::Function::ExternalLinkage;
        res = llvm::Function::Create(funType, linkage, tfi.name_.begin, &ctx.llvmModule_);

        // Do we have a name conflict?
        // In case in which we make a decl first without any body
        if (res->getName() != llvm::StringRef(tfi.name_.begin)) {
            // Check the old function; is it just a decl?
            llvm::Function* oldFun = ctx.llvmModule_.getFunction(tfi.name_.begin);
            auto& definedFunctions = ctx.targetBackend_.definedFunctions_;
            if (oldFun && !(definedFunctions.find(oldFun) != definedFunctions.end())) {
                // The old function is not defined by this module, but still lives in the current
                // LLVM module; it must be an llvm backend code defined function. Delete the one we
                // just made and get the existing one
                res->eraseFromParent();
                res = oldFun;
            }
            // Otherwise, it's ok to have the llvm name changed
        }

        // Set the calling convention
        res->setCallingConv(translateCallingConv(Feather_Function_callConvention(node)));

        // If we have a result parameter, mark it as sret
        if (Nest_getPropertyNode(node, propResultParam))
            res->addAttribute(1, llvm::Attribute::StructRet);

        // Heuristic for inlining
        if (body) {
            if (preventInline)
                res->addFnAttr(llvm::Attribute::NoInline);
            else if (lineDiff == 0)
                res->addFnAttr(llvm::Attribute::AlwaysInline);
            else
                res->addFnAttr(llvm::Attribute::InlineHint);
        }

        // Currently we don't support exceptions, so add a nounwind attribute
        res->addFnAttr(llvm::Attribute::NoUnwind);
    }

    ASSERT(res);
    return res;
}

/// Called to translate the function body.
/// If 'addressToStoreResult' is given, we add a mem-store to the result of 'node' in the given
/// address
void translateFunctionBody(
        TrContext& localCtx, Node* node, llvm::Value* addressToStoreResult = nullptr) {

    // Translate the node, guarded by a scope guard
    llvm::Value* value = nullptr;
    {
        Scope scopeGuard(localCtx, node->location);
        value = translateNode(node, localCtx);
    }

    // Do we need to store the result of the node somewhere?
    if (addressToStoreResult) {
        if (!value)
            REP_INTERNAL(node->location, "Cannot CT evaluate node: %1%") % node;

        // Create a 'store' instruction
        new llvm::StoreInst(value, addressToStoreResult, localCtx.insertionPoint());
    }

    // If the body block doesn't have a termination instruction, try to create a return instruction
    if (localCtx.hasValidInsertionPoint() && !localCtx.insertionPoint()->getTerminator()) {
        if (localCtx.parentFun()->getReturnType()->isVoidTy())
            llvm::ReturnInst::Create(localCtx.llvmContext(), localCtx.insertionPoint());
        else
            new llvm::UnreachableInst(localCtx.llvmContext(), localCtx.insertionPoint());
    }
}

//! Returns true if we need to write a definition for the given node
bool needsDefinition(Node* node, TranslatedFunInfo& tfi) {
    Node* body = Feather_Function_body(node);
    return body && !tfi.mainDecl_;
}

//! Create the LLVM definition of the function.
//! Declaration of the function needs to be created previously.
void createFunDefinition(
        llvm::Function* decl, Node* node, GlobalContext& ctx, TranslatedFunInfo& tfi) {
    llvm::IRBuilder<> llvmBuilder(ctx.targetBackend_.llvmContext());

    ASSERT(decl->getParent() == &ctx.definitionsLlvmModule_);

    // If we are emitting debug information, emit function start
    auto dbgInfo = ctx.targetBackend_.debugInfo();
    if (dbgInfo)
        dbgInfo->emitFunctionStart(llvmBuilder, node, decl);

    // This is the decl that has a corresponding definition
    tfi.mainDecl_ = decl;
    ctx.targetBackend_.definedFunctions_.insert(decl);

    // Set the names for the parameters
    ASSERT(decl->arg_size() == Feather_Function_numParameters(node));
    size_t idx = 0;
    for (auto argIt = decl->arg_begin(); argIt != decl->arg_end(); ++argIt, ++idx) {
        argIt->setName(toString(Feather_getName(Feather_Function_getParameter(node, idx))));
    }

    // Create the block in which we insert the code
    llvm::BasicBlock* bodyBlock =
            llvm::BasicBlock::Create(ctx.targetBackend_.llvmContext(), "", decl, 0);

    // Create a local context for the body of the function
    TrContext localCtx(ctx, bodyBlock, llvmBuilder);

    // Similar to the web LLVM compiler, we add variables for parameters
    idx = 0;
    for (auto argIt = decl->arg_begin(); argIt != decl->arg_end(); ++argIt, ++idx) {
        Node* paramNode = Feather_Function_getParameter(node, idx);
        Node* param = Nest_ofKind(Nest_explanation(paramNode), nkFeatherDeclVar);
        if (!param)
            REP_INTERNAL(paramNode->location, "Expected Var node; found %1%") % paramNode;
        llvm::AllocaInst* newVar = new llvm::AllocaInst(
                argIt->getType(), 0, toString(Feather_getName(param)) + ".addr", bodyBlock);
        newVar->setAlignment(Nest_getCheckPropertyInt(param, "alignment"));
        new llvm::StoreInst(argIt, newVar, bodyBlock); // Copy the value of the parameter into it
        Tr::setValue(localCtx, *param, newVar);        // We point now to the new temp variable
    }

    // Translate the body
    Node* body = Feather_Function_body(node);
    translateFunctionBody(localCtx, body);

    // If we are emitting debug information, emit function end
    if (dbgInfo)
        dbgInfo->emitFunctionEnd(llvmBuilder, node->location);
}

} // namespace

llvm::Function* Tr::translateFunction(Node* node, GlobalContext& ctx) {
    // If this function was already translated (in the current LLVM module), don't do it again
    TranslatedFunInfo& funInfo = ctx.targetBackend_.translatedFunInfos_[node];
    if (funInfo.lastDecl_ && funInfo.lastDecl_->getParent() == &ctx.llvmModule_)
        return funInfo.lastDecl_;

    // Ensure type is correctly computed
    if (!Nest_computeType(node))
        return nullptr;

    // Check for ct/non-ct compatibility
    if (!ctx.targetBackend_.canUse(node))
        return nullptr;

    // Create a declaration for this function
    llvm::Function* decl = createFunDecl(node, ctx, funInfo);
    funInfo.lastDecl_ = decl;

    if (needsDefinition(node, funInfo)) {
        if (ctx.separateDefDecl()) {
            GlobalContext ctxDefs = ctx.defsContext();

            // Translate again the decl in the definition's llvm::Module
            llvm::Function* declDef = createFunDecl(node, ctxDefs, funInfo);
            funInfo.lastDecl_ = declDef;

            // Create a definition for the function
            createFunDefinition(declDef, node, ctxDefs, funInfo);
        } else {
            // Create a definition for the function
            createFunDefinition(decl, node, ctx, funInfo);
        }
    }

    // We should always return the decl corresponding to 'llvmModule_'
    return decl;
}

llvm::Function* Tr::makeFunThatCalls(
        Node* node, GlobalContext& ctx, const char* funName, bool expectsResult) {

    llvm::LLVMContext& llvmContext = ctx.targetBackend_.llvmContext();

    // Create the function type
    vector<llvm::Type*> llvmParamTypes;
    if (expectsResult) {
        ASSERT(node->type->hasStorage);

        // Get the type of the expression, to be used with llvm code
        llvm::Type* t = getLLVMType(node->type, ctx);
        llvm::Type* pt = llvm::PointerType::get(t, 0);

        llvmParamTypes.reserve(1);
        llvmParamTypes.push_back(pt);
    }
    llvm::FunctionType* funType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext), llvmParamTypes, false);

    // Actually create the function
    // WARNING: we place the function in 'llvmModule_', not in 'definitionsLlvmModule_'
    llvm::Function* f = llvm::Function::Create(
            funType, llvm::Function::InternalLinkage, funName, &ctx.llvmModule_);
    if (expectsResult)
        f->addAttribute(1, llvm::Attribute::StructRet);

    // Create the block in which we insert the code
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(llvmContext, "", f, 0);

    // Create a local context for the body of the function
    llvm::IRBuilder<> llvmBuilder(llvmContext);
    TrContext localCtx(ctx, bodyBlock, llvmBuilder);

    // Add the action as the body of the function
    llvm::Value* addressToStoreResult = expectsResult ? f->arg_begin() : nullptr;
    translateFunctionBody(localCtx, node, addressToStoreResult);

    return f;
}

llvm::CallingConv::ID Tr::translateCallingConv(CallConvention conv) {
    switch (conv) {
    case ccC:
        return llvm::CallingConv::C;
    case ccFast:
        return llvm::CallingConv::Fast;
    case ccCold:
        return llvm::CallingConv::Cold;
    case ccGHC:
        return llvm::CallingConv::GHC;

    case ccX86Std:
        return llvm::CallingConv::X86_StdCall;
    case ccX86Fast:
        return llvm::CallingConv::X86_FastCall;
    case ccX86ThisCall:
        return llvm::CallingConv::X86_ThisCall;
    case ccARMAPCS:
        return llvm::CallingConv::ARM_APCS;
    case ccARMAAPCS:
        return llvm::CallingConv::ARM_AAPCS;
    case ccARMAAPCSVFP:
        return llvm::CallingConv::ARM_AAPCS_VFP;
    case ccMSP430Intr:
        return llvm::CallingConv::MSP430_INTR;
    case ccPTXKernel:
        return llvm::CallingConv::PTX_Kernel;
    case ccPTXDevice:
        return llvm::CallingConv::PTX_Device;
    default:
        return llvm::CallingConv::C;
    }
}
