#include <StdInc.h>
#include "RtModule.h"
#include <Tr/TrTopLevel.h>
#include <Tr/DebugInfo.h>

#include <Nest/Common/Diagnostic.hpp>
#include <Nest/Intermediate/Node.h>
#include <Nest/CompilerSettings.hpp>
#include <Nest/Compiler.h>
#include <Nest/Frontend/SourceCode.h>


using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

RtModule::RtModule(const string& name, const string& filename)
    : Module(name)
{
    const CompilerSettings& s = *Nest_compilerSettings();
    if ( s.generateDebugInfo_ )
        debugInfo_ = new DebugInfo(*llvmModule_, filename);
}

RtModule::~RtModule()
{
    delete debugInfo_;
}

void RtModule::generate(Node* rootNode)
{
    if ( !rootNode || !rootNode->nodeSemanticallyChecked )
        REP_INTERNAL(NOLOC, "The root node to be processed by the LLVM backend is not semantically checked");

    // Translate the root node as a top level node
    Tr::translateTopLevelNode(rootNode, *this);
}

void RtModule::generateGlobalCtorDtor()
{

    bool reverseOnCtor = false;
    bool reverseOnDtor = true;
#ifdef _WIN32
    //reverseOnCtor = true;
    //reverseOnDtor = true;
#endif

    // Emit the global ctor & dtor functions list
    emitCtorList(globalCtors_, "llvm.global_ctors", reverseOnCtor);
    emitCtorList(globalDtors_, "llvm.global_dtors", reverseOnDtor);

    globalCtors_.clear();
    globalDtors_.clear();
}


void RtModule::setCtToRtTranslator(const NodeFun& translator)
{
    ctToRtTranslator_ = translator;
}

void RtModule::addGlobalCtor(llvm::Function* fun)
{
    globalCtors_.push_back(fun);
}

void RtModule::addGlobalDtor(llvm::Function* fun)
{
    globalDtors_.push_back(fun);
}

RtModule::NodeFun RtModule::ctToRtTranslator() const
{
    return ctToRtTranslator_;
}



void RtModule::emitCtorList(const CtorList& funs, const char* globalName, bool reverse)
{
    if ( funs.empty() )
        return;

    llvm::Type* int32T = llvm::IntegerType::get(*llvmContext_, 32);

    // Ctor function type is void()*.
    llvm::FunctionType* ctorFTy = llvm::FunctionType::get(llvm::Type::getVoidTy(*llvmContext_), false);
    llvm::Type* ctorPFTy = llvm::PointerType::getUnqual(ctorFTy);

    // Get the type of a ctor entry, { i32, void ()* }.
    llvm::StructType* ctorStructTy = llvm::StructType::get(int32T, ctorPFTy, NULL);

    // Construct the constructor and destructor arrays.
    llvm::SmallVector<llvm::Constant*, 8> ctors;
    for ( size_t i=0; i<funs.size(); ++i )
    {
        llvm::Function* f = reverse ? funs[funs.size()-i-1] : funs[i];
        llvm::Constant* S[] = {
            llvm::ConstantInt::get(int32T, (int) i, false),
            llvm::ConstantExpr::getBitCast(f, ctorPFTy)
        };
        ctors.push_back(llvm::ConstantStruct::get(ctorStructTy, S));
    }

    if ( !ctors.empty() )
    {
        llvm::ArrayType *arrT = llvm::ArrayType::get(ctorStructTy, ctors.size());
        new llvm::GlobalVariable(*llvmModule_, arrT, false,
            llvm::GlobalValue::AppendingLinkage,
            llvm::ConstantArray::get(arrT, ctors),
            globalName);
    } 
}
