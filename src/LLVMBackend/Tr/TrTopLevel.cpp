#include <StdInc.h>
#include "TrTopLevel.h"
#include "TrFunction.h"
#include "TrType.h"
#include "TrLocal.h"
#include "Module.h"

#include "Nest/Api/Node.h"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <llvm/Linker/Linker.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Support/SourceMgr.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace
{
    void translateNodeList(Node* node, Module& module)
    {
        for ( Node* child: node->children )
        {
            if ( child )
                translateTopLevelNode(child, module);
        }
    }

    void translateGlobalDestructAction(Node* node, Module& module)
    {
        Node* act = at(node->children, 0);
        llvm::Function* fun = makeFunThatCalls(act, module, "__global_dtor");
        if ( fun )
            module.addGlobalDtor(fun);
    }

    void translateGlobalConstructAction(Node* node, Module& module)
    {
        Node* act = at(node->children, 0);
        llvm::Function* fun = makeFunThatCalls(act, module, "__global_ctor");
        if ( fun )
            module.addGlobalCtor(fun);
    }
}



void Tr::translateTopLevelNode(Node* node, Module& module)
{
    // Translate the additional nodes for this node
    for ( Node* n: all(node->additionalNodes) )
    {
        translateTopLevelNode(n, module);
    }

    if ( !node->nodeSemanticallyChecked ) {
        // REP_INTERNAL(node->location, "Node not semantically checked and yet it reached backend for translation");
        // TODO (compilation): Check why we are in this situation
        if ( !Nest_semanticCheck(node) )
            REP_INTERNAL(node->location, "Node not semantically checked and yet it reached backend for translation; and we can't semantically check it");
    }

    // If this node is explained, then translate its explanation
    Node* expl = Nest_explanation(node);
    if ( node != expl )
    {
        translateTopLevelNode(expl, module);
    }
    else
    {
        // Depending on the type of the node, do a specific translation

        switch ( node->nodeKind - Feather_getFirstFeatherNodeKind() )
        {
        case nkRelFeatherNop:                      break;
        case nkRelFeatherNodeList:                 translateNodeList(node, module); break;
        case nkRelFeatherBackendCode:              translateBackendCode(node, module); break;
        case nkRelFeatherGlobalDestructAction:     translateGlobalDestructAction(node, module); break;
        case nkRelFeatherGlobalConstructAction:    translateGlobalConstructAction(node, module); break;
        case nkRelFeatherDeclClass:                translateClass(node, module); break;
        case nkRelFeatherDeclFunction:             translateFunction(node, module); break;
        case nkRelFeatherDeclVar:                  translateGlobalVar(node, module); break;
        default:
            REP_INTERNAL(node->location, "Don't know how to translate at top-level a node of this kind (%1%)") % Nest_nodeKindName(node);
        }
    }
}


void Tr::translateBackendCode(Node* node, Module& module)
{
    // Generate a new module from the given backend code
    llvm::SMDiagnostic error;
    StringRef code = Nest_getCheckPropertyString(node, propCode);
    llvm::Module* resModule = llvm::ParseAssemblyString(code.begin, nullptr, error, module.llvmContext());
    if ( !resModule )
    {
        Location loc = node->location;
        if ( loc.start.line == 1 && loc.start.col == 1 )
            loc = Nest_mkLocation1(node->location.sourceCode, error.getLineNo(), error.getColumnNo());
        REP_ERROR(loc, "Cannot parse backend code node: %1% (line: %2%)") % string(error.getMessage()) % error.getLineNo();
    }

    // Make sure it has the same data layout and target triple
    resModule->setDataLayout(module.llvmModule().getDataLayout());
    resModule->setTargetTriple(module.llvmModule().getTargetTriple());

    // Merge the new module into the current module
    string errMsg;
    llvm::Linker::LinkModules(&module.llvmModule(), resModule, llvm::Linker::DestroySource, &errMsg);
    if ( !errMsg.empty() )
        REP_ERROR(node->location, "Cannot merge backend code node into main LLVM module: %1%") % errMsg;
}

llvm::Type* Tr::translateClass(Node* node, Module& module)
{
    // Check for ct/non-ct compatibility
    if ( !module.canUse(node) )
        return nullptr;

    if ( !node->type || 0 == strcmp(node->type->description, "BasicBlock") )
    {
        ASSERT(true);
    }

    // If this class was already translated, don't do it again
    llvm::Type** transType = module.getNodePropertyValue<llvm::Type*>(node, Module::propTransType);
    if ( transType )
        return *transType;

    // Check if this is a standard/native type
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    if ( nativeName )
    {
        llvm::Type* t = getNativeLLVMType(node->location, *nativeName, module.llvmContext());
        if ( t )
        {
            // Set the translation type for this item
            module.setNodeProperty(node, Module::propTransType, t);
            return t;
        }
    }

    // Get or create the type, and set it as a property (don't add any subtypes yet to avoid endless loops)
    llvm::StructType* t = nullptr;
    if ( nativeName )
        t = module.llvmModule().getTypeByName(nativeName->begin);    // Make sure we reuse the name
    if ( !t )
    {
        const StringRef* description = Nest_getPropertyString(node, propDescription);
        StringRef desc = description ? *description : Feather_getName(node);
        // Create a new struct type, possible with another name
        t = llvm::StructType::create(module.llvmContext(), desc.begin);
    }
    module.setNodeProperty(node, Module::propTransType, t);

    // Now add the subtypes
    vector<llvm::Type*> fieldTypes;
    fieldTypes.reserve(Nest_nodeArraySize(node->children));
    for ( auto field: node->children )
    {
        fieldTypes.push_back(getLLVMType(field->type, module));
    }
    if ( t->isOpaque() )
    {
        t->setBody(fieldTypes);
    }
    return t;
}

llvm::Value* Tr::translateGlobalVar(Node* node, Module& module)
{
    // Check for ct/non-ct compatibility
    if ( !module.canUse(node) )
        return nullptr;

    if ( node->nodeKind != nkFeatherDeclVar )
        REP_ERROR_RET(nullptr, node->location, "Invalid global variable %1%") % Feather_getName(node);

    // If we already translated this variable, make sure not to translate it again
    llvm::Value* val = getValue(module, *node, false);
    if ( val )
        return val;

    llvm::Type* t = getLLVMType(node->type, module);

    // Check if the variable has been declared before; if not, create it
    llvm::GlobalVariable* var = nullptr;
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    if ( nativeName )
        var = module.llvmModule().getGlobalVariable(nativeName->begin);
    if ( !var )
    {
        var = new llvm::GlobalVariable(
            module.llvmModule(),
            t,
            false, // isConstant
            llvm::GlobalValue::ExternalLinkage, // linkage
            0, // initializer - specified below
            Feather_getName(node).begin
            );
    }

    // Create a zero initializer for the variable
    if ( t->isIntegerTy() )
    {
        var->setInitializer(llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(t), 0));
    }
    else if ( t->isFloatingPointTy() )
    {
        var->setInitializer(llvm::ConstantFP::get(t, 0.0));
    }
    else if ( t->isPointerTy() )
    {
        var->setInitializer(llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(t)));
    }
    else if ( t->isStructTy() )
    {
        var->setInitializer(llvm::ConstantAggregateZero::get(t));
    }
    else
    {
        REP_INTERNAL(node->location, "Don't know how to create zero initializer for the variable of type %1%")
            % node->type;
    }

    // Set the value for the variable
    setValue(module, *node, var);
    return var;
}
