#include <StdInc.h>
#include "Module.h"
#include <Tr/TrTopLevel.h>

#include <Feather/Util/Decl.h>
#include <Nest/CompilerSettings.h>
#include <Nest/Compiler.h>

using namespace LLVMB;


Module::Module(const string& name)
    : llvmContext_(new llvm::LLVMContext())
	, llvmModule_(new llvm::Module(name, *llvmContext_))
    , debugInfo_(nullptr)
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    llvmModule_->setDataLayout(s.dataLayout_);
    llvmModule_->setTargetTriple(s.targetTriple_);
}

Module::~Module()
{
    delete llvmModule_;
    delete llvmContext_;
}

bool Module::canUse(Node* decl) const
{
    Nest::EvalMode mode = Feather::effectiveEvalMode((Node*) decl);
    ASSERT(mode != Nest::modeUnspecified);
    if ( mode == Nest::modeRt && isCt() )
        return false;
    else if ( mode == Nest::modeCt && !isCt() )
        return false;
    else
        return true;
}


bool Module::isFunctionDefined(llvm::Function* fun) const
{
    return definedFunctions_.find(fun) != definedFunctions_.end();
}

void Module::addDefinedFunction(llvm::Function* fun)
{
    definedFunctions_.insert(fun);
}

boost::any* Module::getNodePropertyPtr(Node* node, NodePropertyType type)
{
    auto it = nodeProperties_.find(PropKey(node, type));
    return it == nodeProperties_.end() ? nullptr : &it->second;
}

void Module::setNodeProperty(Node* node, NodePropertyType type, const boost::any& value)
{
    nodeProperties_[PropKey(node, type)] = value;
}
