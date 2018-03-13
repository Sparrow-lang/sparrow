#include <StdInc.h>
#include "TrTopLevel.h"
#include "TrFunction.h"
#include "TrType.h"
#include "TrLocal.h"
#include "Module.h"
#include "GlobalContext.h"

#include "Nest/Api/Node.h"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include <llvm/Linker/Linker.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Support/SourceMgr.h>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace {
void translateNodeList(Node* node, GlobalContext& ctx) {
    for (Node* child : node->children) {
        if (child)
            translateTopLevelNode(child, ctx);
    }
}

void translateGlobalDestructAction(Node* node, GlobalContext& ctx) {
    Node* act = at(node->children, 0);
    llvm::Function* fun = makeFunThatCalls(act, ctx, "__global_dtor");
    if (fun)
        ctx.targetBackend_.addGlobalDtor(fun);
}

void translateGlobalConstructAction(Node* node, GlobalContext& ctx) {
    Node* act = at(node->children, 0);
    llvm::Function* fun = makeFunThatCalls(act, ctx, "__global_ctor");
    if (fun)
        ctx.targetBackend_.addGlobalCtor(fun);
}

//! Create the declaration of a global variable.
llvm::GlobalVariable* createGlobalVarDecl(
        Node* node, GlobalContext& ctx, const string& varName, llvm::Type* type, bool isNative) {

    // Check if the variable has been declared before; if not, create it
    llvm::GlobalVariable* var = nullptr;
    if (isNative)
        var = ctx.llvmModule_.getGlobalVariable(varName.c_str());
    if (!var) {
        var = new llvm::GlobalVariable(ctx.llvmModule_, type,
                false,                              // isConstant
                llvm::GlobalValue::ExternalLinkage, // linkage
                nullptr,                            // initializer - specified below
                varName.c_str());
    }
    return var;
}

//! Create the declaration of a global variable.
void createGlobalVarDefinition(
        llvm::GlobalVariable* var, Node* node, llvm::Type* type, TranslatedGVarInfo& varInfo) {
    // This is the definition of the global variable
    varInfo.definition_ = var;

    // Create a zero initializer for the variable
    if (type->isIntegerTy())
        var->setInitializer(llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(type), 0));
    else if (type->isFloatingPointTy())
        var->setInitializer(llvm::ConstantFP::get(type, 0.0));
    else if (type->isPointerTy())
        var->setInitializer(llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(type)));
    else if (type->isStructTy())
        var->setInitializer(llvm::ConstantAggregateZero::get(type));
    else
        REP_INTERNAL(node->location,
                "Don't know how to create zero initializer for the variable of type %1%") %
                node->type;
}

} // namespace

void Tr::translateTopLevelNode(Node* node, GlobalContext& ctx) {
    // Translate the additional nodes for this node
    for (Node* n : all(node->additionalNodes)) {
        translateTopLevelNode(n, ctx);
    }

    if (!node->nodeSemanticallyChecked) {
        // REP_INTERNAL(node->location,
        //         "Node not semantically checked and yet it reached backend for translation");
        // TODO (compilation): Check why we are in this situation
        if (!Nest_semanticCheck(node))
            REP_INTERNAL(node->location, "Node not semantically checked and yet it reached backend "
                                         "for translation; and we can't semantically check it");
    }

    // If this node is explained, then translate its explanation
    Node* expl = Nest_explanation(node);
    if (node != expl)
        translateTopLevelNode(expl, ctx);
    else {
        // Depending on the type of the node, do a specific translation

        switch (node->nodeKind - Feather_getFirstFeatherNodeKind()) {
        case nkRelFeatherNop:
            break;
        case nkRelFeatherNodeList:
            translateNodeList(node, ctx);
            break;
        case nkRelFeatherBackendCode:
            translateBackendCode(node, ctx);
            break;
        case nkRelFeatherGlobalDestructAction:
            translateGlobalDestructAction(node, ctx);
            break;
        case nkRelFeatherGlobalConstructAction:
            translateGlobalConstructAction(node, ctx);
            break;
        case nkRelFeatherDeclClass:
            translateClass(node, ctx);
            break;
        case nkRelFeatherDeclFunction:
            translateFunction(node, ctx);
            break;
        case nkRelFeatherDeclVar:
            translateGlobalVar(node, ctx);
            break;
        default:
            REP_INTERNAL(node->location,
                    "Don't know how to translate at top-level a node of this kind (%1%)") %
                    Nest_nodeKindName(node);
        }
    }
}

void Tr::translateBackendCode(Node* node, GlobalContext& ctx) {
    // Generate a new module from the given backend code
    llvm::SMDiagnostic error;
    StringRef code = Nest_getCheckPropertyString(node, propCode);
    unique_ptr<llvm::Module> resModule =
            llvm::parseAssemblyString(code.begin, error, ctx.targetBackend_.llvmContext());
    if (!resModule) {
        Location loc = node->location;
        if (loc.start.line == 1 && loc.start.col == 1)
            loc = Nest_mkLocation1(
                    node->location.sourceCode, error.getLineNo(), error.getColumnNo());
        REP_ERROR(loc, "Cannot parse backend code node: %1% (line: %2%)") %
                string(error.getMessage()) % error.getLineNo();
        return;
    }

    // Make sure it has the same data layout and target triple
    resModule->setDataLayout(ctx.llvmModule_.getDataLayout());
    resModule->setTargetTriple(ctx.llvmModule_.getTargetTriple());

    // Merge the new module into the current module
    if (llvm::Linker::linkModules(ctx.llvmModule_, move(resModule), llvm::Linker::OverrideFromSrc))
        REP_ERROR(node->location, "Cannot merge backend code node into main LLVM module");
}

llvm::Type* Tr::translateClass(Node* node, GlobalContext& ctx) {
    // Check for ct/non-ct compatibility
    if (!ctx.targetBackend_.canUse(node))
        return nullptr;

    // If this class was already translated, don't do it again
    auto it = ctx.targetBackend_.translatedStructs_.find(node);
    if (it != ctx.targetBackend_.translatedStructs_.end())
        return it->second;

    // Check if this is a standard/native type
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    if (nativeName) {
        llvm::Type* t =
                getNativeLLVMType(node->location, *nativeName, ctx.targetBackend_.llvmContext());
        if (t) {
            // Set the translation type for this item
            ctx.targetBackend_.translatedStructs_[node] = t;
            return t;
        }
    }

    // Get or create the type, and set it as a property (don't add any subtypes yet to avoid endless
    // loops)
    llvm::StructType* t = nullptr;
    if (nativeName)
        t = ctx.llvmModule_.getTypeByName(nativeName->begin); // Make sure we reuse the name
    if (!t) {
        const StringRef* description = Nest_getPropertyString(node, propDescription);
        StringRef desc = description ? *description : Feather_getName(node);
        // Create a new struct type, possible with another name
        t = llvm::StructType::create(ctx.targetBackend_.llvmContext(), desc.begin);
    }
    // Mark it as translated
    ctx.targetBackend_.translatedStructs_[node] = t;

    // Now add the subtypes
    vector<llvm::Type*> fieldTypes;
    fieldTypes.reserve(Nest_nodeArraySize(node->children));
    for (auto field : node->children)
        fieldTypes.push_back(getLLVMType(field->type, ctx));
    if (t->isOpaque())
        t->setBody(fieldTypes);
    return t;
}

llvm::Value* Tr::translateGlobalVar(Node* node, GlobalContext& ctx) {
    // If this global var was already translated (in the current LLVM module), don't do it again
    TranslatedGVarInfo& varInfo = ctx.targetBackend_.translatedGVarInfos_[node];
    if (varInfo.lastDecl_ && varInfo.lastDecl_->getParent() == &ctx.llvmModule_)
        return varInfo.lastDecl_;

    // Check for ct/non-ct compatibility
    if (!ctx.targetBackend_.canUse(node))
        return nullptr;

    if (node->nodeKind != nkFeatherDeclVar)
        REP_ERROR_RET(nullptr, node->location, "Invalid global variable %1%") %
                Feather_getName(node);

    llvm::Type* t = getLLVMType(node->type, ctx);

    // Get the name of the variable
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    static int counter = 0;
    string varName;
    if (nativeName)
        // Use the native name
        varName = toString(*nativeName);
    else if (varInfo.definition_)
        // Reuse the same name as of the definition
        varName = varInfo.definition_->getName();
    else {
        // Build a new name
        StringRef origName = Feather_getName(node);
        varName.reserve(size(origName) + 1 + 7);
        varName = toString(origName) + "." + to_string(counter++);
    }

    // Check if the variable has been declared before; if not, create it
    llvm::GlobalVariable* var = createGlobalVarDecl(node, ctx, varName, t, nativeName != nullptr);
    // Make sure we are not creating more global variables for this module
    varInfo.lastDecl_ = var;

    if (!varInfo.definition_) {
        llvm::GlobalVariable* varDef = var;

        if (ctx.separateDefDecl()) {
            // Translate again the decl in the definition's llvm::Module
            GlobalContext ctxDefs = ctx.defsContext();
            varDef = createGlobalVarDecl(node, ctxDefs, varName, t, nativeName != nullptr);
            varInfo.lastDecl_ = varDef;
        }

        // Create a definition for the function
        createGlobalVarDefinition(varDef, node, t, varInfo);
    }

    return var;
}
