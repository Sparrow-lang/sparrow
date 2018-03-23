#include <StdInc.h>
#include "RtModule.h"
#include <Tr/TrTopLevel.h>
#include <Tr/DebugInfo.h>
#include <Tr/GlobalContext.h>

#include <memory>

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Api/Node.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Api/SourceCode.h"

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

RtModule::RtModule(const string& name, const string& filename)
    : Module(name)
    , llvmModule_(new llvm::Module(name, *llvmContext_)) {
    CompilerSettings& s = *Nest_compilerSettings();

    llvmModule_->setDataLayout(s.dataLayout_);
    llvmModule_->setTargetTriple(s.targetTriple_);

    if (s.generateDebugInfo_)
        debugInfo_ = llvm::make_unique<DebugInfo>(*llvmModule_, filename);
}

RtModule::~RtModule() = default;

void RtModule::generate(Node* rootNode) {
    if (!rootNode || !rootNode->nodeSemanticallyChecked)
        REP_INTERNAL(NOLOC,
                "The root node to be processed by the LLVM backend is not semantically checked");

    // Translate the root node as a top level node
    Tr::GlobalContext ctx(*llvmModule_, *this);
    Tr::translateTopLevelNode(rootNode, ctx);
}

void RtModule::generateGlobalCtorDtor() {

    bool reverseOnCtor = false;
    bool reverseOnDtor = false;
#ifdef _WIN32
    // reverseOnCtor = true;
    // reverseOnDtor = true;
#endif

    // Emit the global ctor & dtor functions list
    emitCtorList(globalCtors_, "llvm.global_ctors", reverseOnCtor);
    emitCtorList(globalDtors_, "llvm.global_dtors", reverseOnDtor);

    globalCtors_.clear();
    globalDtors_.clear();
}

void RtModule::setCtToRtTranslator(const NodeFun& translator) { ctToRtTranslator_ = translator; }

void RtModule::addGlobalCtor(llvm::Function* fun) { globalCtors_.push_back(fun); }

void RtModule::addGlobalDtor(llvm::Function* fun) { globalDtors_.push_back(fun); }

RtModule::NodeFun RtModule::ctToRtTranslator() const { return ctToRtTranslator_; }

Tr::DebugInfo* RtModule::debugInfo() const { return debugInfo_.get(); }

void RtModule::emitCtorList(const CtorList& funs, const char* globalName, bool reverse) {
    if (funs.empty())
        return;

    llvm::Type* int32T = llvm::IntegerType::get(*llvmContext_, 32);

    // Ctor function type is void()*.
    llvm::FunctionType* ctorFTy =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*llvmContext_), false);
    llvm::Type* ctorPFTy = llvm::PointerType::getUnqual(ctorFTy);

    // Get the type of a ctor entry, { i32, void ()* }.
    llvm::StructType* ctorStructTy = llvm::StructType::get(int32T, ctorPFTy);

    // Construct the constructor and destructor arrays.
    llvm::SmallVector<llvm::Constant*, 8> ctors;
    for (size_t i = 0; i < funs.size(); ++i) {
        llvm::Function* f = reverse ? funs[funs.size() - i - 1] : funs[i];
        llvm::Constant* S[] = {llvm::ConstantInt::get(int32T, (int)i, false),
                llvm::ConstantExpr::getBitCast(f, ctorPFTy)};
        ctors.push_back(llvm::ConstantStruct::get(ctorStructTy, S));
    }

    if (!ctors.empty()) {
        ASSERT(llvmModule_);
        llvm::ArrayType* arrT = llvm::ArrayType::get(ctorStructTy, ctors.size());
        new llvm::GlobalVariable(*llvmModule_, arrT, false, llvm::GlobalValue::AppendingLinkage,
                llvm::ConstantArray::get(arrT, ctors), globalName);
    }
}
