#pragma once

#include "Module.h"

namespace LLVMB { namespace Tr
{
    /// Class that represents a RT backend module.
    class RtModule : public Module
    {
    public:
        RtModule(const string& name, const string& filename);
        virtual ~RtModule();

        /// Compile the given node into the current module
        void generate(DynNode* rootNode);

        /// Generate code for all the added ctors and dtor
        void generateGlobalCtorDtor();

        /// Setter for the functor used to translate nodes from CT to RT
        void setCtToRtTranslator(const NodeFun& translator);

    public:
        virtual bool isCt() const { return false; }
        virtual void addGlobalCtor(llvm::Function* fun);
        virtual void addGlobalDtor(llvm::Function* fun);
        virtual NodeFun ctToRtTranslator() const;

    private:
        typedef vector<llvm::Function*> CtorList;

        void emitCtorList(const CtorList& funs, const char* globalName, bool reverse = false);

    protected:
        CtorList globalCtors_;
        CtorList globalDtors_;
        NodeFun ctToRtTranslator_;
    };

}}
