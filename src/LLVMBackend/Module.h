#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Utils/cppif/StringRef.hpp"

#include <boost/function.hpp>
#include <string>
#include <unordered_map>

FWD_CLASS1(Feather, Decl);
FWD_CLASS2(LLVMB, Tr, DebugInfo);

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, Function);

using Nest::Node;
using Nest::StringRef;
using Nest::TypeRef;

namespace LLVMB {
// TODO (backend): Refactor these
namespace Tr {
class Scope;
}

//! Keeps track of how a particular Sparrow function was translated.
struct TranslatedFunInfo {
    //! The main declaration of the function -- where we added the body of the function
    llvm::Function* mainDecl_;
    //! The function declaration we used for our last LLVM module.
    //! Theoretically, we may have a set of decls here, one for each module. But, typically once we
    //! switch to a new LLVM module, we don't care about the decls for previous modules. So, use
    //! only the last decl, and check if we switch LLVM modules.
    llvm::Function* lastDecl_;
    //! The name of the function when first translated.
    //! We make sure that we generate unique names for functions.
    StringRef name_;

    TranslatedFunInfo()
        : mainDecl_(nullptr)
        , lastDecl_(nullptr)
        , name_{nullptr, nullptr} {}
};

//! Keeps track of how a particular Sparrow global variable was translated.
struct TranslatedGVarInfo {
    //! The definition of the global variable (if available)
    llvm::GlobalVariable* definition_;
    //! The function declaration we used for our last LLVM module.
    //! Theoretically, we may have a set of decls here, one for each module. But, typically once we
    //! switch to a new LLVM module, we don't care about the decls for previous modules. So, use
    //! only the last decl, and check if we switch LLVM modules.
    llvm::GlobalVariable* lastDecl_;

    TranslatedGVarInfo()
        : definition_(nullptr)
        , lastDecl_(nullptr) {}
};

/// Class that represents a backend module.
/// It is responsible for the translation of an intermediate code into LLVM bitcode
class Module {
public:
    typedef boost::function<Node*(Node*)> NodeFun;

public:
    explicit Module(const string& name);
    virtual ~Module();

    /// Returns true if we are generating code for CT, and false if we are generating code for RT
    virtual bool isCt() const = 0;

    /// Add a global constructor function
    virtual void addGlobalCtor(llvm::Function* fun) = 0;

    /// Add a global destructor function to this module
    virtual void addGlobalDtor(llvm::Function* fun) = 0;

    /// Getter for the functor used to translate nodes from CT to RT
    virtual NodeFun ctToRtTranslator() const = 0;

    /// Getter for the debug information object used for this module
    virtual Tr::DebugInfo* debugInfo() const { return nullptr; }

public:
    /// Getter for the LLVM context used for the code generation
    llvm::LLVMContext& llvmContext() const { return *llvmContext_; }

    /// Returns true if the given declaration makes sense in the current module
    bool canUse(Node* decl) const;

protected:
    //! The LLVM context to be used
    unique_ptr<llvm::LLVMContext> llvmContext_;

public:
    //! Map with all the translated types (per type)
    unordered_map<TypeRef, llvm::Type*> translatedTypes_;
    //! Information about the translated data structures (per node)
    unordered_map<Node*, llvm::Type*> translatedStructs_;
    //! Information about the translated functions
    unordered_map<Node*, TranslatedFunInfo> translatedFunInfos_;
    //! List of all LLVM function declarations defined in this backend target
    unordered_set<llvm::Function*> definedFunctions_;
    //! Information about the translated global variables
    unordered_map<Node*, TranslatedGVarInfo> translatedGVarInfos_;

    //! The set of all the names of functions that we've used so far
    unordered_set<StringRef> funNamesTaken_;

    //! The set of all the prepared decl nodes for translation.
    //! Any decl from this set can be translated without needing extra semantic checks.
    unordered_set<Node*> preparedDecls_;
};

} // namespace LLVMB
