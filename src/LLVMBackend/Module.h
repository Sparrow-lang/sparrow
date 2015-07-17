#pragma once

#include <Nest/Intermediate/TypeRef.h>

#include <boost/function.hpp>
#include <boost/any.hpp>
#include <string>
#include <unordered_map>

FWD_STRUCT1(Nest, Node);
FWD_CLASS1(Feather, DynNode);
FWD_CLASS1(Feather, Decl);
FWD_CLASS2(LLVMB,Tr, DebugInfo);

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, Function);

namespace LLVMB
{
    using Feather::DynNode;
    using Nest::Node;

    /// Class that represents a backend module.
    /// It is responsible for the translation of an intermediate code into LLVM bitcode
    class Module
    {
    public:
        enum NodePropertyType
        {
            propValue = 0,
            propFunDecl,
            propTransType,
            propWhileInstr,
            propWhileStepLabel,
            propWhileEndLabel,
        };

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

    public:
        /// Getter for the LLVM context used for the code generation
        llvm::LLVMContext& llvmContext() const { return *llvmContext_; }

        /// Getter for the LLVM module that is used for code generation
        llvm::Module& llvmModule() const { return *llvmModule_; }

        /// Getter for the debug information object used for this module
        Tr::DebugInfo* debugInfo() const { return debugInfo_; }

        /// Returns true if the given declaration makes sense in the current module
        bool canUse(DynNode* decl) const;

        /// Getter/setter for the llvm functions defined by this module
        bool isFunctionDefined(llvm::Function* fun) const;
        void addDefinedFunction(llvm::Function* fun);

        /// Getters/setter for node properties
        boost::any* getNodePropertyPtr(DynNode* node, NodePropertyType type);
        void setNodeProperty(DynNode* node, NodePropertyType type, const boost::any& value);

    // Helpers
    public:
        template <typename T>
        T* getNodePropertyValue(DynNode* node, NodePropertyType type)
        {
            boost::any* anyVal = getNodePropertyPtr(node, type);
            return !anyVal || anyVal->empty() ? nullptr : boost::any_cast<T>(anyVal);
        }

    protected:
        typedef pair<DynNode*, NodePropertyType> PropKey;
        struct PropKeyHash
        {
            size_t operator()(const PropKey& k) const
            {
                return std::hash<DynNode*>()(k.first) + 393241*int(k.second);
            }
        };

        llvm::LLVMContext* llvmContext_;
        llvm::Module* llvmModule_;
        unordered_map<PropKey, boost::any, PropKeyHash> nodeProperties_;
        unordered_set<llvm::Function*> definedFunctions_;

        /// If set, it represents the object that is used to generate debug information for this module
        Tr::DebugInfo* debugInfo_;

    public:
        unordered_map<Nest::TypeRef, llvm::Type*> translatedTypes_;
    };

}
