#pragma once

#include "Module.h"

namespace LLVMB {
namespace Tr {
/// Class that represents a RT backend module.
class RtModule : public Module {
public:
    RtModule(const string& name, const string& filename);
    virtual ~RtModule();

    /// Compile the given node into the current module
    void generate(Node* rootNode);

    /// Generate code for all the added ctors and dtor
    void generateGlobalCtorDtor();

    /// Setter for the functor used to translate nodes from CT to RT
    void setCtToRtTranslator(const NodeFun& translator);

public:
    virtual bool isCt() const { return false; }
    virtual void addGlobalCtor(llvm::Function* fun);
    virtual void addGlobalDtor(llvm::Function* fun);
    virtual NodeFun ctToRtTranslator() const;
    virtual Tr::DebugInfo* debugInfo() const;

    /// Getter for the LLVM module that is used for code generation
    llvm::Module& llvmModule() const {
        ASSERT(llvmModule_);
        return *llvmModule_;
    }

private:
    typedef vector<llvm::Function*> CtorList;

    void emitCtorList(const CtorList& funs, const char* globalName, bool reverse = false);

private:
    //! The LLVM module we are adding translated code into
    unique_ptr<llvm::Module> llvmModule_;

    CtorList globalCtors_;
    CtorList globalDtors_;
    NodeFun ctToRtTranslator_;

    /// If set, it represents the object that is used to generate debug information for this module
    unique_ptr<Tr::DebugInfo> debugInfo_;
};

} // namespace Tr
} // namespace LLVMB
