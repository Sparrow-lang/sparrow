#pragma once

namespace LLVMB {

class Module;

namespace Tr {

//! The global context in which we are generating code.
//! This represents the LLVM module and the target data for which we are generating code.
struct GlobalContext {
    //! The LLVM module we are generating code into.
    //! We will typically add here only one definition, and the rest are just declarations.
    llvm::Module& llvmModule_;

    //! LLVM module used to translate definitions of referred top-level nodes.
    //! For each definition we add here, we need a decl in 'llvmModule_'
    llvm::Module& definitionsLlvmModule_;

    //! The target backend we are generating code for
    Module& targetBackend_;

    GlobalContext(llvm::Module& mod, Module& targetBackend)
        : llvmModule_(mod)
        , definitionsLlvmModule_(mod)
        , targetBackend_(targetBackend) {}

    GlobalContext(llvm::Module& mod, llvm::Module& definitionsMod, Module& targetBackend)
        : llvmModule_(mod)
        , definitionsLlvmModule_(definitionsMod)
        , targetBackend_(targetBackend) {}

    //! Returns true if we need to separate definitions from declarations.
    bool separateDefDecl() const { return &llvmModule_ != &definitionsLlvmModule_; }

    //! Return a global context in which we store all the definitions
    //! Both decls and definitions will be placed in the same 'definitionsLlvmModule_' LLVM module.
    GlobalContext defsContext() {
        return GlobalContext(definitionsLlvmModule_, definitionsLlvmModule_, targetBackend_);
    }
};

} // namespace Tr
} // namespace LLVMB