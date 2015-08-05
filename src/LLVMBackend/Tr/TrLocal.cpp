#include <StdInc.h>
#include "TrLocal.h"
#include "TrContext.h"
#include "Scope.h"
#include "Instruction.h"
#include "TrType.h"
#include "TrFunction.h"
#include "TrTopLevel.h"
#include "DebugInfo.h"
#include "Module.h"

#include <Nest/Intermediate/Type.h>
#include <Nest/Common/Diagnostic.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Exp/CtValue.h>
#include <Feather/Nodes/Exp/StackAlloc.h>
#include <Feather/Nodes/Exp/MemLoad.h>
#include <Feather/Nodes/Exp/MemStore.h>
#include <Feather/Nodes/Exp/VarRef.h>
#include <Feather/Nodes/Exp/FieldRef.h>
#include <Feather/Nodes/Exp/FunRef.h>
#include <Feather/Nodes/Exp/FunCall.h>
#include <Feather/Nodes/Exp/Bitcast.h>
#include <Feather/Nodes/Exp/Conditional.h>
#include <Feather/Nodes/Exp/Null.h>
#include <Feather/Nodes/Properties.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/StringData.h>
#include <Feather/FeatherTypes.h>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    //

    /// Translate the given node, and makes sure that the node returns a valid llvm-value
    llvm::Value* translateNodeCheck(Node* node, TrContext& context)
    {
        llvm::Value* res = translateNode(node, context);
        if ( !res )
            REP_ERROR(node->location, "Invalid LLVM value returned for node %1%") % node;
        return res;
    }


    /// Write the scope & temp destruct actions from the stack.
    /// If no scope is given, all the destruct actions in the context are written. If a scope is given, the destruct
    /// actions for everything on the stack until that scope are written; the destruct actions of the scope itself are
    /// not written.
    void unwind(TrContext& context, Scope* lastScope = nullptr)
    {
        for ( Scope* s: boost::adaptors::reverse(context.scopesStack()) )
        {
            if ( s == lastScope )
                break;

            s->outputDestructActions();
        }
    }

    // Helper node used to implement destruct actions for conditional expression
    class DestructActionForConditional : public DynNode
    {
        DEFINE_NODE(DestructActionForConditional, getNodeKind(), "LLVMBackend.DestructActionForConditional");

        static int getNodeKind()
        {
            static bool initialized = false;
            if ( !initialized )
            {
                registerSelf();
                initialized = true;
            }
            return classNodeKind();
        }
    public:
        DestructActionForConditional(TypeRef resType, llvm::Value* cond, NodeVector alt1DestructActions, NodeVector alt2DestructActions)
            : DynNode(getNodeKind(), NOLOC, {(DynNode*) mkNodeList(NOLOC, move(alt1DestructActions)), (DynNode*) mkNodeList(NOLOC, move(alt2DestructActions)) })
        {
            setProperty("resType", resType);
            setProperty("cond_LLVM_value", reinterpret_cast<Node*>(cond));
            // store the condition llvm value as a pointer to a node
        }

        void doSemanticCheck()
        {
            data_.type = resType();
        }

        TypeRef resType() const
        {
            return getCheckPropertyType("resType");
        }

        llvm::Value* condition() const
        {
            return reinterpret_cast<llvm::Value*>(getCheckPropertyNode("cond_LLVM_value"));
        }

        const NodeVector& alt1DestructActions() const
        {
            return data_.children[0]->children;
        }

        const NodeVector& alt2DestructActions() const
        {
            return data_.children[1]->children;
        }
    };

    /// Expression class that can hold a llvm value or a non-translated node.
    /// This will make sure not to translate the node twice.
    class Exp
    {
    public:
        Exp(llvm::Value* val) : value_(val), node_(nullptr) {}
        Exp(DynNode* node) : value_(nullptr), node_(node) {}

        llvm::Value* translate(TrContext& context) const
        {
            if ( !value_ )
                value_ = translateNode(node_->node(), context);
            return value_;
        }

    private:
        mutable llvm::Value* value_;
        DynNode* node_;
    };


    llvm::Value* generateConditionalCode(TypeRef destType, CompilationContext* compContext,
        DynNode* cond, const Exp& alt1, const Exp& alt2, TrContext& context)
    {
        // Create the different blocks
        llvm::BasicBlock* alt1Block = llvm::BasicBlock::Create(context.llvmContext(), "cond.true", context.parentFun());
        llvm::BasicBlock* alt2Block = llvm::BasicBlock::Create(context.llvmContext(), "cond.false", context.parentFun());
        llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(context.llvmContext(), "cond.end", context.parentFun());

        // Translate the condition expression
        llvm::Value* condValue = translateNode(cond->node(), context);
        context.ensureInsertionPoint();
        context.builder().CreateCondBr(condValue, alt1Block, alt2Block);

        NodeVector destructActions1;
        NodeVector destructActions2;
        llvm::Value* val1;
        llvm::Value* val2;

        {
            Instruction instrGuard(context);

            // Translate the first alternative
            context.setInsertionPoint(alt1Block);
            val1 = alt1.translate(context);
            context.ensureInsertionPoint();
            context.builder().CreateBr(afterBlock);
            alt1Block = context.insertionPoint();   // Might have changed in the meantime

            destructActions1 = instrGuard.stealDestructActions();
        }
        {
            Instruction instrGuard(context);

            // Translate the second alternative
            context.setInsertionPoint(alt2Block);
            val2 = alt2.translate(context);
            context.ensureInsertionPoint();
            context.builder().CreateBr(afterBlock);
            alt2Block = context.insertionPoint();   // Might have changed in the meantime

            destructActions2 = instrGuard.stealDestructActions();
        }

        // Translate the PHI node
        context.setInsertionPoint(afterBlock);
        llvm::PHINode* phiVal = llvm::PHINode::Create(getLLVMType(destType, context.module()), 2, "cond.res", context.insertionPoint());
        phiVal->addIncoming(val1, alt1Block);
        phiVal->addIncoming(val2, alt2Block);

        // If one of the alternative have destruct actions, create a destruct action for the Conditional expression
        if ( !destructActions1.empty() || !destructActions2.empty() )
        {
            // The destruct action is also a kind of conditional operation - reuse the condition value
            DynNode* destructAction = new DestructActionForConditional(destType, condValue, move(destructActions1), move(destructActions2));
            destructAction->setContext(compContext);
            destructAction->semanticCheck();
            context.curInstruction().addTempDestructAction(destructAction->node());
        }

        return phiVal;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Intrinsic function handling
    //

    llvm::Value* handleLogicalOr(FunCall& funCall, TrContext& context)
    {
        if ( funCall.arguments().size() != 2 )
            REP_INTERNAL(funCall.location(), "Logical or must have exact 2 arguments");

        DynNode* arg1 = funCall.arguments()[0];
        DynNode* arg2 = funCall.arguments()[1];
        llvm::Value* trueConst = llvm::ConstantInt::getTrue(context.llvmContext());
        llvm::Value* res = generateConditionalCode(arg1->type(), arg1->context(), arg1, Exp(trueConst), Exp(arg2), context);
        return setValue(context.module(), *funCall.node(), res);
    }

    llvm::Value* handleLogicalAnd(FunCall& funCall, TrContext& context)
    {
        if ( funCall.arguments().size() != 2 )
            REP_INTERNAL(funCall.location(), "Logical and must have exact 2 arguments");

        DynNode* arg1 = funCall.arguments()[0];
        DynNode* arg2 = funCall.arguments()[1];
        llvm::Value* falseConst = llvm::ConstantInt::getFalse(context.llvmContext());
        llvm::Value* res = generateConditionalCode(arg1->type(), arg1->context(), arg1, Exp(arg2), Exp(falseConst), context);
        return setValue(context.module(), funCall.data_, res);
    }

    llvm::Value* handleFunPtr(FunCall& funCall, TrContext& context)
    {
        Node* fun = funCall.funDecl();

        // Does the resulting function has a resulting parameter?
        // Depending on that, compute the position of the this argument
        size_t thisArgPos = getPropertyNode(fun, propResultParam) ? 1 : 0;

        ASSERT(!funCall.arguments().empty());

        // Get the values for all the arguments
        vector<llvm::Value*> args;
        args.reserve(funCall.arguments().size());
        for ( auto arg: funCall.arguments() )
        {
            llvm::Value* v = translateNode(arg->node(), context);
            CHECK(arg->location(), v);
            args.push_back(v);
        }

        context.ensureInsertionPoint();

        // Create a function type pointer based on the arguments of the function
        llvm::Type* fType = getLLVMFunctionType(fun, thisArgPos, context.module());
        llvm::Type* pfType = llvm::PointerType::get(fType, 0);
        llvm::Type* ppfType = llvm::PointerType::get(pfType, 0);

        // Reinterpret cast from 'this' argument to the actual function type
        ASSERT(!args.empty());
        llvm::Value* pptrToFun = context.builder().CreateBitCast(args[thisArgPos], ppfType);
        llvm::Value* ptrToFun = context.builder().CreateLoad(pptrToFun);

        // Remove the this argument, as this is the actual function
        args.erase(args.begin() + thisArgPos);

        // Create a call instruction to the pointer to function
        llvm::CallInst* val = context.builder().CreateCall(ptrToFun, args, "");
        val->setCallingConv(Tr::translateCallingConv(Function_callConvention(fun)));
        return setValue(context.module(), *funCall.node(), val);
    }

    llvm::Value* handleNativeFunCall(const string& native, FunCall& funCall, TrContext& context)
    {
#define B()                     context.builder()
#define ARG(argNum)             translateNode(args[argNum]->node(), context)
#define Ti(numBits)             llvm::IntegerType::get(context.llvmContext(), numBits)
#define Tf()                    llvm::Type::getFloatTy(context.llvmContext())
#define Td()                    llvm::Type::getDoubleTy(context.llvmContext())
#define CONSTi(numBits, val)    llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(numBits, val, false))
#define CONSTf(val)             llvm::ConstantFP::get(Tf(), val)
#define CONSTd(val)             llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext()), val)


        const DynNodeVector& args = funCall.arguments();
        if ( args.size() == 1 )
        {
            if ( native == "_zero_init_1" )
                return B().CreateStore(CONSTi(1, 0), ARG(0));
            else if ( native == "_zero_init_8" )
                return B().CreateStore(CONSTi(8, 0), ARG(0));
            else if ( native == "_zero_init_16" )
                return B().CreateStore(CONSTi(16, 0), ARG(0));
            else if ( native == "_zero_init_32" )
                return B().CreateStore(CONSTi(32, 0), ARG(0));
            else if ( native == "_zero_init_64" )
                return B().CreateStore(CONSTi(64, 0), ARG(0));
            else if ( native == "_zero_init_f" )
                return B().CreateStore(CONSTf(0.0), ARG(0));
            else if ( native == "_zero_init_d" )
                return B().CreateStore(CONSTd(0.0), ARG(0));

            else if ( native == "_Byte_opPlus1" || native == "_Short_opPlus1" || native == "_Int_opPlus1" || native == "_Long_opPlus1"
                   || native == "_UByte_opPlus1" || native == "_UShort_opPlus1" || native == "_UInt_opPlus1" || native == "_ULong_opPlus1" )
                return ARG(0);

            else if ( native == "_Byte_opMinus1" )
                return B().CreateSub(CONSTi(8, 0), ARG(0));
            else if ( native == "_Short_opMinus1" )
                return B().CreateSub(CONSTi(16, 0), ARG(0));
            else if ( native == "_Int_opMinus1" )
                return B().CreateSub(CONSTi(32, 0), ARG(0));
            else if ( native == "_Long_opMinus1" )
                return B().CreateSub(CONSTi(64, 0), ARG(0));
            else if ( native == "_DiffType_opMinus1" )
                return B().CreateSub(CONSTi(64, 0), ARG(0));

            else if ( native == "_Bool_opNeg" )
                return B().CreateXor(CONSTi(1, 1), ARG(0));
        }
        else if ( args.size() == 2 )
        {
            if ( native == "_ass_1_1" || native == "_ass_8_8" || native == "_ass_16_16"
                || native == "_ass_32_32" || native == "_ass_64_64"
                || native == "_ass_f_f" || native == "_ass_d_d" )
                return B().CreateStore(ARG(1), ARG(0));
            else if ( native == "_ass_8_16" || native == "_ass_8_32" || native == "_ass_8_64" )
                return B().CreateStore(B().CreateTrunc(ARG(1), Ti(8)), ARG(0));
            else if ( native == "_ass_16_32" || native == "_ass_16_64" )
                return B().CreateStore(B().CreateTrunc(ARG(1), Ti(16)), ARG(0));
            else if ( native == "_ass_32_64" )
                return B().CreateStore(B().CreateTrunc(ARG(1), Ti(32)), ARG(0));

            else if ( native == "_ass_16_8s" )
                return B().CreateStore(B().CreateSExt(ARG(1), Ti(16)), ARG(0));
            else if ( native == "_ass_32_8s" || native == "_ass_32_16s" )
                return B().CreateStore(B().CreateSExt(ARG(1), Ti(32)), ARG(0));
            else if ( native == "_ass_64_8s" || native == "_ass_64_16s" || native == "_ass_64_32s" )
                return B().CreateStore(B().CreateSExt(ARG(1), Ti(64)), ARG(0));

            else if ( native == "_ass_16_8z" )
                return B().CreateStore(B().CreateZExt(ARG(1), Ti(16)), ARG(0));
            else if ( native == "_ass_32_8s" || native == "_ass_32_16z" )
                return B().CreateStore(B().CreateZExt(ARG(1), Ti(32)), ARG(0));
            else if ( native == "_ass_64_8s" || native == "_ass_64_16z" || native == "_ass_64_32z" )
                return B().CreateStore(B().CreateZExt(ARG(1), Ti(64)), ARG(0));

            else if ( native == "_ass_i8_f" || native == "_ass_i8_d" )
                return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(8)), ARG(0));
            else if ( native == "_ass_u8_f" || native == "_ass_u8_d" )
                return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(8)), ARG(0));
            else if ( native == "_ass_i16_f" || native == "_ass_i16_d" )
                return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(16)), ARG(0));
            else if ( native == "_ass_u16_f" || native == "_ass_u16_d" )
                return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(16)), ARG(0));
            else if ( native == "_ass_i32_f" || native == "_ass_i32_d" )
                return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(32)), ARG(0));
            else if ( native == "_ass_u32_f" || native == "_ass_u32_d" )
                return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(32)), ARG(0));
            else if ( native == "_ass_i64_f" || native == "_ass_i64_d" )
                return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(64)), ARG(0));
            else if ( native == "_ass_u64_f" || native == "_ass_u64_d" )
                return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(64)), ARG(0));

            else if ( native == "_ass_f_i8" || native == "_ass_f_i16" || native == "_ass_f_i32" || native == "_ass_f_i64" )
                return B().CreateStore(B().CreateSIToFP(ARG(1), Tf()), ARG(0));
            else if ( native == "_ass_f_u8" || native == "_ass_f_u16" || native == "_ass_f_u32" || native == "_ass_f_u64" )
                return B().CreateStore(B().CreateUIToFP(ARG(1), Tf()), ARG(0));
            else if ( native == "_ass_d_i8" || native == "_ass_d_i16" || native == "_ass_d_i32" || native == "_ass_d_i64" )
                return B().CreateStore(B().CreateSIToFP(ARG(1), Td()), ARG(0));
            else if ( native == "_ass_d_u8" || native == "_ass_d_u16" || native == "_ass_d_u32" || native == "_ass_d_u64" )
                return B().CreateStore(B().CreateUIToFP(ARG(1), Td()), ARG(0));

            else if ( native == "_ass_f_d" )
                return B().CreateStore(B().CreateFPTrunc(ARG(1), Tf()), ARG(0));
            else if ( native == "_ass_d_f" )
                return B().CreateStore(B().CreateFPExt(ARG(1), Td()), ARG(0));

            else if ( native == "_Byte_opEQ" || native == "_Short_opEQ" || native == "_Int_opEQ" || native == "_Long_opEQ"
                   || native == "_UByte_opEQ" || native == "_UShort_opEQ" || native == "_UInt_opEQ" || native == "_ULong_opEQ"
                   || native == "_SizeType_opEQ" || native == "_DiffType_opEQ"
                   || native == "_Bool_opEQ" || native == "_Char_opEQ" )
                return B().CreateICmpEQ(ARG(0), ARG(1));
            else if ( native == "_Byte_opNE" || native == "_Short_opNE" || native == "_Int_opNE" || native == "_Long_opNE"
                   || native == "_UByte_opNE" || native == "_UShort_opNE" || native == "_UInt_opNE" || native == "_ULong_opNE"
                   || native == "_SizeType_opNE" || native == "_DiffType_opNE"
                   || native == "_Bool_opNE" || native == "_Char_opNE" )
                return B().CreateICmpNE(ARG(0), ARG(1));
            else if ( native == "_Byte_opLT" || native == "_Short_opLT" || native == "_Int_opLT" || native == "_Long_opLT"
                   || native == "_UByte_opLT" || native == "_UShort_opLT" || native == "_UInt_opLT" || native == "_ULong_opLT"
                   || native == "_SizeType_opLT" || native == "_DiffType_opLT"
                   || native == "_Char_opLT" )
                return B().CreateICmpSLT(ARG(0), ARG(1));
            else if ( native == "_Byte_opGT" || native == "_Short_opGT" || native == "_Int_opGT" || native == "_Long_opGT"
                   || native == "_UByte_opGT" || native == "_UShort_opGT" || native == "_UInt_opGT" || native == "_ULong_opGT"
                   || native == "_SizeType_opGT" || native == "_DiffType_opGT"
                   || native == "_Char_opGT" )
                return B().CreateICmpSGT(ARG(0), ARG(1));
            else if ( native == "_Byte_opLE" || native == "_Short_opLE" || native == "_Int_opLE" || native == "_Long_opLE"
                   || native == "_UByte_opLE" || native == "_UShort_opLE" || native == "_UInt_opLE" || native == "_ULong_opLE"
                   || native == "_SizeType_opLE" || native == "_DiffType_opLE"
                   || native == "_Char_opLE" )
                return B().CreateICmpSLE(ARG(0), ARG(1));
            else if ( native == "_Byte_opGE" || native == "_Short_opGE" || native == "_Int_opGE" || native == "_Long_opGE"
                   || native == "_UByte_opGE" || native == "_UShort_opGE" || native == "_UInt_opGE" || native == "_ULong_opGE"
                   || native == "_SizeType_opGE" || native == "_DiffType_opGE"
                   || native == "_Char_opGE" )
                return B().CreateICmpSGE(ARG(0), ARG(1));

            else if ( native == "_Byte_opPlus" || native == "_Short_opPlus" || native == "_Int_opPlus" || native == "_Long_opPlus"
                   || native == "_UByte_opPlus" || native == "_UShort_opPlus" || native == "_UInt_opPlus" || native == "_ULong_opPlus"
                   || native == "_SizeType_opPlus" || native == "_DiffType_opPlus" )
                return B().CreateAdd(ARG(0), ARG(1));
            else if ( native == "_Byte_opMinus" || native == "_Short_opMinus" || native == "_Int_opMinus" || native == "_Long_opMinus"
                   || native == "_UByte_opMinus" || native == "_UShort_opMinus" || native == "_UInt_opMinus" || native == "_ULong_opMinus"
                   || native == "_SizeType_opMinu" || native == "_DiffType_opMinu" )
                return B().CreateSub(ARG(0), ARG(1));
            else if ( native == "_Byte_opMul" || native == "_Short_opMul" || native == "_Int_opMul" || native == "_Long_opMul"
                   || native == "_UByte_opMul" || native == "_UShort_opMul" || native == "_UInt_opMul" || native == "_ULong_opMul"
                   || native == "_SizeType_opMul" || native == "_DiffType_opMul" )
                return B().CreateMul(ARG(0), ARG(1));
            else if ( native == "_Byte_opDiv" || native == "_Short_opDiv" || native == "_Int_opDiv" || native == "_Long_opDiv"
                   || native == "_DiffType_opDiv" )
                return B().CreateSDiv(ARG(0), ARG(1));
            else if ( native == "_UByte_opDiv" || native == "_UShort_opDiv" || native == "_UInt_opDiv" || native == "_ULong_opDiv"
                   || native == "_SizeType_opPDiv" )
                return B().CreateUDiv(ARG(0), ARG(1));
            else if ( native == "_Byte_opMod" || native == "_Short_opMod" || native == "_Int_opMod" || native == "_Long_opMod"
                   || native == "_DiffType_opMod" )
                return B().CreateSRem(ARG(0), ARG(1));
            else if ( native == "_UByte_opMod" || native == "_UShort_opMod" || native == "_UInt_opMod" || native == "_ULong_opMod"
                   || native == "_SizeType_opMod" )
                return B().CreateURem(ARG(0), ARG(1));

            // TODO: floating point operations: compares, arithmetic

            else if ( native == "_Bool_opXor" )
                return B().CreateXor(ARG(0), ARG(1));
        }

        return nullptr;
#undef B
#undef ARG
#undef Ti
#undef Tf
#undef Td
#undef CONSTi
#undef CONSTf
#undef CONSTd
    }
    

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Common nodes
    //

    llvm::Value* translateNodeList(Node* node, TrContext& context)
    {
        llvm::Value* res = nullptr;
        for ( Node* child: node->children )
        {
            if ( child )
                res = translateNode(child, context);
        }
        return res;
    }

    llvm::Value* translateLocalSpace(Node* node, TrContext& context)
    {
        Scope scopeGuard(context, node->location);

        // Translate all the instructions in the local space
        for ( Node* child: node->children )
        {
            Instruction instrGuard(context);
            translateNode(child, context);
        }

        return nullptr;
    }

    llvm::Value* translateNop(Node* /*node*/, TrContext& /*context*/)
    {
        // Do nothing
        return nullptr;
    }

    llvm::Value* translateTempDestructAction(Node* node, TrContext& context)
    {
        // Add the action to the temp destruct actions in the context
        context.curInstruction().addTempDestructAction(node->children[0]);
        return nullptr;
    }

    llvm::Value* translateScopeDestructAction(Node* node, TrContext& context)
    {
        // Add the action to the scope destruct actions in the context
        context.curScope().addScopeDestructAction(node->children[0]);
        return nullptr;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Expressions
    //

    llvm::AtomicOrdering llvmOrdering(AtomicOrdering ord)
    {
        switch ( ord )
        {
        case atomicNone:            return llvm::NotAtomic;
        case atomicUnordered:       return llvm::Unordered;
        case atomicMonotonic:       return llvm::Monotonic;
        case atomicAcquire:         return llvm::Acquire;
        case atomicRelease:         return llvm::Release;
        case atomicAcquireRelease:  return llvm::AcquireRelease;
        case atomicSeqConsistent:   return llvm::SequentiallyConsistent;
        default:                    return llvm::NotAtomic;
        }
    }

    llvm::Value* translateDataTypeConstant(llvm::Type* t, const char* begin, const char* end, TrContext& context)
    {
        uint64_t size = end - begin;

        context.ensureInsertionPoint();
                    
        llvm::Value* constInt0 = llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, 0, false));
        llvm::Value* constInt1 = llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, 1, false));
        llvm::Value* constIntSize = llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, size, false));
                    
        // Create a the data array constant
        llvm::Value* bytesVal = llvm::ConstantDataArray::getString(context.llvmContext(), llvm::StringRef(begin, (size_t) size));
                    
        // Allocate a temporary variable to hold it
        llvm::Value* constTmpVar = context.addVariable(bytesVal->getType(), "const.bytes");
        context.builder().CreateStore(bytesVal, constTmpVar);
        //bytesVal->getType()->dump();
                    
        // Obtain pointers to begin and end in the allocated temporary variable
        vector<llvm::Value*> indices(2);
        indices[0] = constInt0;
        indices[1] = constInt0;
        llvm::Value* constPtrBegin = context.builder().CreateInBoundsGEP(constTmpVar, indices, "");
        indices[1] = constIntSize;
        llvm::Value* constPtrEnd = context.builder().CreateInBoundsGEP(constTmpVar, indices, "");

        // Create a temporary variable for the actual structure object
        llvm::Value* tmpVar = context.addVariable(t, "const.struct");

        // Index for the the begin and end fields in our structure
        indices[1] = constInt0;
        llvm::Value* beginAddr = context.builder().CreateInBoundsGEP(tmpVar, indices, "");
        indices[1] = constInt1;
        llvm::Value* endAddr = context.builder().CreateInBoundsGEP(tmpVar, indices, "");

        // Copy pointers
        context.builder().CreateStore(constPtrBegin, beginAddr);
        context.builder().CreateStore(constPtrEnd, endAddr);
                    
        //context.parentFun()->dump();
                    
        return context.builder().CreateLoad(tmpVar);
    }

    llvm::Value* translate(CtValue& node, TrContext& context)
    {
        // Get the type of the ct value
        llvm::Type* t = Tr::getLLVMType(node.type(), context.module());
        
        // Check for String CtValues
        if ( !context.module().isCt() )
        {
            TypeRef tt = node.type();
            if ( tt->typeKind == typeKindData )
            {
                const string* nativeName = Feather::nativeName(tt);
                if ( nativeName && *nativeName == "StringRef" )
                {
                    StringData data = *node.value<StringData>();
                    return translateDataTypeConstant(t, data.begin, data.end, context);
                }
            }
        }
        
        // Get the resulting value
        if ( t->isIntegerTy() )
        {
            int width = static_cast<llvm::IntegerType*>(t)->getBitWidth();
            uint64_t val;

            switch ( width )
            {
            case 1:     val = 0 != (*node.value<unsigned char>()); break;
            case 8:     val = *node.value<unsigned char>(); break;
            case 16:    val = *node.value<unsigned short>(); break;
            case 32:    val = *node.value<unsigned int>(); break;
            case 64:    val = *node.value<uint64_t>(); break;
            default:
                {
                    val = 0;
                    REP_ERROR(node.location(), "Invalid bit width (%1%) for numeric literal (%2%)")
                        % width % node.toString();
                }
            }

            return setValue(context.module(), *node.node(), llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(t), val));
        }
        else if ( t->isFloatTy() )
        {
            return setValue(context.module(), *node.node(), llvm::ConstantFP::get(t, *node.value<float>()));
        }
        else if ( t->isDoubleTy() )
        {
            return setValue(context.module(), *node.node(), llvm::ConstantFP::get(t, *node.value<double>()));
        }
        else if ( t->isPointerTy() )
        {
            uint64_t val = *node.value<uint64_t>();
            llvm::Value* intVal = llvm::ConstantInt::get(llvm::IntegerType::get(context.llvmContext(), 64), val);
            return new llvm::IntToPtrInst(intVal, t, "", context.insertionPoint());
        }
        else if ( t->isStructTy() )
        {
            // If our size is zero, return a zero constant aggregate
            size_t size = context.llvmModule().getDataLayout()->getTypeAllocSize(t);
            if ( size == 0 )
                return llvm::ConstantAggregateZero::get(t);

            context.ensureInsertionPoint();

            // Type of pointer to our structure
            llvm::Type* ptr = llvm::PointerType::getUnqual(t);

            // Create an array constant
            const uint8_t* valBegin = node.value<uint8_t>();
            llvm::Value* constArrVal = llvm::ConstantDataArray::get(context.llvmContext(), llvm::ArrayRef<uint8_t>(valBegin, size));

            // Create an array value and initialize it with our array constant
            llvm::Type* arrType = llvm::ArrayType::get(llvm::IntegerType::get(context.llvmContext(), 8), size);
            llvm::AllocaInst* tmpVar = context.addVariable(arrType, "const.arr");
            tmpVar->setAlignment(1);
            context.builder().CreateStore(constArrVal, tmpVar);

            llvm::Value* ptrToVal = context.builder().CreateBitCast(tmpVar, ptr);
            return context.builder().CreateLoad(ptrToVal);
        }
        else
        {
            REP_ERROR(node.location(), "Don't know how to translate ct value of type %1%")
                % node.type();
            return nullptr;
        }
    }

    llvm::Value* translate(StackAlloc& node, TrContext& context)
    {
        // Get the type for the stack alloc
        llvm::Type* t = Tr::getLLVMType(node.elemType(), context.module());
        if ( node.numElements() > 1 )
            t = llvm::ArrayType::get(t, node.numElements());

        // Create an 'alloca' instruction
        llvm::AllocaInst* val = context.addVariable(t, "stackAlloc");
        if ( node.alignment() > 0 )
            val->setAlignment(node.alignment());
        return setValue(context.module(), *node.node(), val);
    }

    llvm::Value* translate(MemLoad& node, TrContext& context)
    {
        // Create a 'load' instruction
        llvm::Value* argVal = translateNodeCheck(node.argument()->node(), context);
        CHECK(node.location(), argVal);
        context.ensureInsertionPoint();
        llvm::LoadInst* val = context.builder().CreateLoad(argVal);
        if ( node.alignment() > 0 )
            val->setAlignment(node.alignment());
        val->setVolatile(node.isVolatile());
        val->setOrdering(llvmOrdering(node.atomicOrdering()));
        val->setSynchScope(node.isSingleThread() ? llvm::SingleThread : llvm::CrossThread);
        return setValue(context.module(), *node.node(), val);
    }

    llvm::Value* translate(MemStore& node, TrContext& context)
    {
        context.ensureInsertionPoint();

        // Create a 'store' instruction
        llvm::Value* value = translateNode(node.value()->node(), context);
        CHECK(node.location(), value);
        llvm::Value* address = translateNode(node.address()->node(), context);
        CHECK(node.location(), address);
        context.ensureInsertionPoint();
        llvm::StoreInst* val = context.builder().CreateStore(value, address);
        if ( node.alignment() > 0 )
            val->setAlignment(node.alignment());
        val->setVolatile(node.isVolatile());
        val->setOrdering(llvmOrdering(node.atomicOrdering()));
        val->setSynchScope(node.isSingleThread() ? llvm::SingleThread : llvm::CrossThread);
        return setValue(context.module(), node.data_, val);
    }

    llvm::Value* translate(VarRef& node, TrContext& context)
    {
        // Simply take the value from the referenced variable
        CHECK(node.location(), node.variable());
        llvm::Value* varVal = nullptr;
        llvm::Value** val = context.module().getNodePropertyValue<llvm::Value*>(node.variable()->node(), Module::propValue);
        if ( val )
        {
            varVal = *val;
        }
        else
        {
            // If we are here, we are trying to reference a variable that wasn't declared yet.
            // This can only happen for global variables that were not yet translated
            if ( node.variable()->nodeKind() != nkFeatherDeclVar )
                REP_INTERNAL(node.variable()->location(), "Cannot find variable %1%") % getName(node.variable()->node());

            varVal = Tr::translateGlobalVar(node.variable()->node(), context.module());
            if ( !varVal )
            {
                if ( effectiveEvalMode(node.variable()->node()) == modeCt && !context.module().isCt() )
                    REP_INTERNAL(node.location(), "Trying to reference a compile-time variable %1% from run-time") % getName(node.variable()->node());
                else
                    REP_INTERNAL(node.location(), "Cannot find variable %1% in the current module") % getName(node.variable()->node());
            }
        }
        return setValue(context.module(), *node.node(), varVal);
    }

    llvm::Value* translate(FieldRef& node, TrContext& context)
    {
        // Make sure the object is translated
        CHECK(node.location(), node.object());
        llvm::Value* objVal = translateNode(node.object()->node(), context);
        CHECK(node.location(), objVal);

        // Compute the index of the field
        uint64_t idx = 0;
        ASSERT(node.object()->type());
        Node* clsDecl = classForType(node.object()->type());
        CHECK(node.location(), clsDecl);
        for ( auto f: clsDecl->children )
        {
            if ( node.field()->node() == f )
                break;
            ++idx;
        }
        if ( idx == clsDecl->children.size() )
            REP_INTERNAL(node.location(), "Cannot find field '%1%' in class '%2%'")
                % getName(node.field()->node()) % getName(clsDecl);

        // Create a 'getelementptr' instruction
        vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, 0, false)));
        indices.push_back(llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, idx, false)));
        context.ensureInsertionPoint();
        llvm::Value* val = context.builder().CreateInBoundsGEP(objVal, indices, "");
        return setValue(context.module(), node.data_, val);
    }

    llvm::Value* translate(FunRef& node, TrContext& context)
    {
        // Make sure the function is translated; get a function pointer value
        llvm::Function* func = translateFunction(node.funDecl(), context.module());
        CHECK(node.location(), func);

        // Get the functor data type, plain and with 1 or 2 pointers
        llvm::Type* t = getLLVMType(node.type(), context.module());
        llvm::Type* pt = llvm::PointerType::get(t, 0);
        CHECK(node.location(), t);

        // Use bitcast of pointers to convert from function pointer to data type
        llvm::Value* tmpVal = context.addVariable(func->getType(), "funptr");
        context.ensureInsertionPoint();
        context.builder().CreateStore(func, tmpVal);
        llvm::Value* bc = context.builder().CreateBitCast(tmpVal, pt);
        llvm::Value* res = context.builder().CreateLoad(bc);
        return setValue(context.module(), node.data_, res);
    }

    llvm::Value* translate(FunCall& node, TrContext& context)
    {
        Node* funDecl = node.funDecl();
        CHECK(node.location(), funDecl);

        // Check for intrinsic native functions
        const string* nativeName = getPropertyString(funDecl, propNativeName);
        if ( nativeName && !nativeName->empty() && (*nativeName)[0] == '$' )
        {
            if ( *nativeName == "$logicalOr" )
                return handleLogicalOr(node, context);
            if ( *nativeName == "$logicalAnd" )
                return handleLogicalAnd(node, context);
            if ( *nativeName == "$funptr" )
                return handleFunPtr(node, context);
        }
        if ( nativeName && !nativeName->empty() && (*nativeName)[0] == '_' )
        {
            context.ensureInsertionPoint();
            auto res = handleNativeFunCall(*nativeName, node, context);
            if ( res )
                return setValue(context.module(), node.data_, res);
        }

        llvm::Function* func = translateFunction(funDecl, context.module());
        CHECK(node.location(), func);

        llvm::Value* res = nullptr;

        // Get the values for all the arguments
        vector<llvm::Value*> args;
        args.reserve(node.arguments().size());
        for ( auto arg: node.arguments() )
        {
            llvm::Value* v = translateNode(arg->node(), context);
            CHECK(arg->location(), v);
            args.push_back(v);
        }

        // Create a 'call' instruction
        context.ensureInsertionPoint();
        llvm::CallInst* val = context.builder().CreateCall(func, args, "");;
        val->setCallingConv(Tr::translateCallingConv(Function_callConvention(funDecl)));
        if ( res )
            return context.builder().CreateLoad(res);
        else
            return val;
    }

    llvm::Value* translate(Bitcast& node, TrContext& context)
    {
        llvm::Value* exp = translateNode(node.exp()->node(), context);
        CHECK(node.location(), exp);
        llvm::Type* destType = Tr::getLLVMType(node.destType(), context.module());
        if ( exp->getType() == destType )
            return setValue(context.module(), node.data_, exp);

        // Create a 'bitcast' instruction
        llvm::Value* val;
        ASSERT(node.destType()->numReferences > 0 );
        ASSERT(node.exp()->type()->numReferences > 0 );
        context.ensureInsertionPoint();
        val = context.builder().CreateBitCast(exp, destType);
        return setValue(context.module(), node.data_, val);
    }

    llvm::Value* translate(Conditional& node, TrContext& context)
    {
        // Create the different blocks
        llvm::BasicBlock* alt1Block = llvm::BasicBlock::Create(context.llvmContext(), "cond_alt1", context.parentFun());
        llvm::BasicBlock* alt2Block = llvm::BasicBlock::Create(context.llvmContext(), "cond_alt2", context.parentFun());
        llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(context.llvmContext(), "cond_end", context.parentFun());

        // Translate the condition expression
        llvm::Value* condValue = translateNode(node.condition()->node(), context);
        CHECK(node.condition()->location(), condValue);
        context.ensureInsertionPoint();
        context.builder().CreateCondBr(condValue, alt1Block, alt2Block);

        NodeVector destructActions1;
        NodeVector destructActions2;
        llvm::Value* val1;
        llvm::Value* val2;

        {
            Instruction instrGuard(context);

            // Translate the first alternative
            context.setInsertionPoint(alt1Block);
            val1 = translateNode(node.alternative1()->node(), context);
            context.ensureInsertionPoint();
            context.builder().CreateBr(afterBlock);

            destructActions1 = instrGuard.stealDestructActions();
        }
        {
            Instruction instrGuard(context);

            // Translate the second alternative
            context.setInsertionPoint(alt2Block);
            val2 = translateNode(node.alternative2()->node(), context);
            context.ensureInsertionPoint();
            context.builder().CreateBr(afterBlock);

            destructActions2 = instrGuard.stealDestructActions();
        }

        // Translate the PHI node
        context.setInsertionPoint(afterBlock);
        llvm::Type* destType = Tr::getLLVMType(node.type(), context.module());
        llvm::PHINode* phiVal = llvm::PHINode::Create(destType, 2, "cond", context.insertionPoint());
        phiVal->addIncoming(val1, alt1Block);
        phiVal->addIncoming(val2, alt2Block);

        // If one of the alternative have destruct actions, create a destruct action for the Conditional expression
        if ( !destructActions1.empty() || !destructActions2.empty() )
        {
            // The destruct action is also a kind of conditional operation - reuse the condition value
            DynNode* destructAction = new DestructActionForConditional(node.type(), condValue, move(destructActions1), move(destructActions2));
            destructAction->setContext(node.context());
            destructAction->semanticCheck();
            ASSERT(!context.scopesStack().empty());
            ASSERT(!context.scopesStack().back()->instructionsStack().empty());
            context.scopesStack().back()->instructionsStack().back()->addTempDestructAction(destructAction->node());
        }

        return phiVal;
    }

    llvm::Value* translate(DestructActionForConditional& node, TrContext& context)
    {
        // Create the different blocks
        llvm::BasicBlock* alt1Block = llvm::BasicBlock::Create(context.llvmContext(), "cond_destruct_alt1", context.parentFun());
        llvm::BasicBlock* alt2Block = llvm::BasicBlock::Create(context.llvmContext(), "cond_destruct_alt2", context.parentFun());
        llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(context.llvmContext(), "cond_destruct_end", context.parentFun());

        // Translate the condition expression
        context.ensureInsertionPoint();
        context.builder().CreateCondBr(node.condition(), alt1Block, alt2Block);

        // Alternative 1 destruct actions
        context.setInsertionPoint(alt1Block);
        for ( Node* n: boost::adaptors::reverse(node.alt1DestructActions()) )
        {
            translateNode(n, context);
        }
        context.ensureInsertionPoint();
        context.builder().CreateBr(afterBlock);

        // Alternative 2 destruct actions
        context.setInsertionPoint(alt2Block);
        for ( Node* n: boost::adaptors::reverse(node.alt2DestructActions()) )
        {
            translateNode(n, context);
        }
        context.ensureInsertionPoint();
        context.builder().CreateBr(afterBlock);


        context.setInsertionPoint(afterBlock);
        return nullptr;
    }

    llvm::Value* translate(Null& node, TrContext& context)
    {
        llvm::Type* resType = Tr::getLLVMType(node.type(), context.module());
        if ( !resType->isPointerTy() )
            REP_INTERNAL(node.location(), "Null type should be a pointer (we have: %1%)") % node.type();
        
        return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(resType));
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Statements
    //

    llvm::Value* translateReturn(Node* node, TrContext& context)
    {
        llvm::Value* retVal = nullptr;
        Node* expression = node->children[0];
        if ( expression )
        {
            retVal = translateNode(expression, context);
            CHECK(expression->location, retVal);
            llvm::Function* f = context.parentFun();
            if ( f->hasStructRetAttr() )
            {
                context.ensureInsertionPoint();
                context.builder().CreateStore(retVal, f->arg_begin());
                retVal = nullptr;
            }
        }

        unwind(context);
        context.ensureInsertionPoint();
        if ( retVal )
            context.builder().CreateRet(retVal);
        else
            context.builder().CreateRetVoid();
        context.setInsertionPoint(nullptr); // No other instruction will be placed in this block
        return nullptr;
    }

    llvm::Value* translateIf(Node* node, TrContext& context)
    {
        Node* condition = node->children[0];
        Node* thenClause = node->children[1];
        Node* elseClause = node->children[2];

        // Create the different blocks
        llvm::BasicBlock* ifBlock = llvm::BasicBlock::Create(context.llvmContext(), "if_block", context.parentFun());
        llvm::BasicBlock* thenBlock = thenClause ? llvm::BasicBlock::Create(context.llvmContext(), "if_then", context.parentFun()) : nullptr;
        llvm::BasicBlock* elseBlock = elseClause ? llvm::BasicBlock::Create(context.llvmContext(), "if_else", context.parentFun()) : nullptr;
        llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(context.llvmContext(), "if_end", context.parentFun());
        if ( !thenBlock )
            thenBlock = afterBlock;
        if ( !elseBlock )
            elseBlock = afterBlock;

        // Jump in the if block
        {
            context.ensureInsertionPoint();
            context.builder().CreateBr(ifBlock);
            context.setInsertionPoint(ifBlock);

            // Translate the condition expression
            llvm::Value* condValue = translateNode(condition, context);
            CHECK(condition->location, condValue);
            context.ensureInsertionPoint();
            context.builder().CreateCondBr(condValue, thenBlock, elseBlock);

            // Translate the then branch
            if ( thenClause )
            {
                ASSERT(thenBlock);
                context.setInsertionPoint(thenBlock);
                {
                    Instruction instrGuard(context);
                    translateNode(thenClause, context);
                }
                context.ensureInsertionPoint();
                context.builder().CreateBr(afterBlock);
            }

            // Translate the else branch
            if ( elseClause )
            {
                ASSERT(elseBlock);
                context.setInsertionPoint(elseBlock);
                {
                    Instruction instrGuard(context);
                    translateNode(elseClause, context);
                }
                context.ensureInsertionPoint();
                context.builder().CreateBr(afterBlock);
            }
        }

        context.setInsertionPoint(afterBlock);
        return nullptr;
    }

    llvm::Value* translateWhile(Node* node, TrContext& context)
    {
        Node* condition = node->children[0];
        Node* step = node->children[1];
        Node* body = node->children[2];

        // Create a new scope for the while instruction
        Scope scopeGuard(context, node->location);
        Instruction instructionGuard(context);

        // Create the different blocks
        llvm::BasicBlock* whileBlock = llvm::BasicBlock::Create(context.llvmContext(), "while_block", context.parentFun());
        llvm::BasicBlock* whileBody = llvm::BasicBlock::Create(context.llvmContext(), "while_body", context.parentFun());
        llvm::BasicBlock* whileStep = llvm::BasicBlock::Create(context.llvmContext(), "while_step", context.parentFun());
        llvm::BasicBlock* whileEnd = llvm::BasicBlock::Create(context.llvmContext(), "while_end", context.parentFun());

        // Store the step & the end labels (blocks) in the object
        ASSERT(!context.scopesStack().empty());
        context.module().setNodeProperty(node, Module::propWhileInstr, boost::any(&scopeGuard));
        context.module().setNodeProperty(node, Module::propWhileStepLabel, boost::any(whileStep));
        context.module().setNodeProperty(node, Module::propWhileEndLabel, boost::any(whileEnd));

        // Jump in the while block
        {
            context.ensureInsertionPoint();
            context.builder().CreateBr(whileBlock);

            // Translate the condition
            context.setInsertionPoint(whileBlock);
            llvm::Value* condValue = translateNode(condition, context);
            CHECK(condition->location, condValue);
            context.ensureInsertionPoint();
            context.builder().CreateCondBr(condValue, whileBody, whileEnd);

            // Translate the body and the step instruction
            context.setInsertionPoint(whileBody);
            if ( body )
            {
                Instruction instrGuard(context);
                translateNode(body, context);
            }
            context.ensureInsertionPoint();
            context.builder().CreateBr(whileStep);
            context.setInsertionPoint(whileStep);
            if ( step )
            {
                Instruction instrGuard(context);
                translateNode(step, context);
            }

            // Translate the end - jump to the beginning of the while
            context.ensureInsertionPoint();
            context.builder().CreateBr(whileBlock);
        }
        context.setInsertionPoint(whileEnd);

        return nullptr;
    }

    llvm::Value* translateBreak(Node* node, TrContext& context)
    {
        Node* whileNode = getCheckPropertyNode(node, "loop");
        CHECK(node->location, whileNode);

        // Get the instruction guard for the while instruction
        Scope** whileInstr = context.module().getNodePropertyValue<Scope*>(whileNode, Module::propWhileInstr);
        CHECK(node->location, whileInstr);

        // Translate the destruct actions until the while instruction
        unwind(context, *whileInstr);

        // Get the while's end label
        llvm::BasicBlock** endLabel = context.module().getNodePropertyValue<llvm::BasicBlock*>(whileNode, Module::propWhileEndLabel);
        CHECK(node->location, endLabel);

        // Create a jump to the end-of-while label
        context.ensureInsertionPoint();
        context.builder().CreateBr(*endLabel);
        context.setInsertionPoint(nullptr); // No other instruction will be placed in this block

        return nullptr;
    }

    llvm::Value* translateContinue(Node* node, TrContext& context)
    {
        Node* whileNode = getCheckPropertyNode(node, "loop");
        CHECK(node->location, whileNode);

        // Get the instruction guard for the while instruction
        Scope** whileInstr = context.module().getNodePropertyValue<Scope*>(whileNode, Module::propWhileInstr);
        CHECK(node->location, whileInstr);

        // Translate the destruct actions until the while instruction
        unwind(context, *whileInstr);

        // Get the while's step label
        llvm::BasicBlock** stepLabel = context.module().getNodePropertyValue<llvm::BasicBlock*>(whileNode, Module::propWhileStepLabel);
        CHECK(node->location, stepLabel);

        // Create a jump to the while-step label
        context.ensureInsertionPoint();
        context.builder().CreateBr(*stepLabel);
        context.setInsertionPoint(nullptr); // No other instruction will be placed in this block

        return nullptr;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Local declarations
    //

    llvm::Value* translateVar(Node* node, TrContext& context)
    {
		ASSERT(context.parentFun());

        // Create an 'alloca' instruction for the local variable
        llvm::Type* t = Tr::getLLVMType(node->type, context.module());
		llvm::AllocaInst* val = context.addVariable(t, getName(node).c_str());
        int alignment = getCheckPropertyInt(node, "alignment");
		if ( alignment > 0 )
			val->setAlignment(alignment);
		return setValue(context.module(), *node, val);
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // General
    //
    
    Node* convertCtToRt(Node* node, TrContext& context)
    {
        Node* res = node;
        if ( context.module().ctToRtTranslator() )
            res = context.module().ctToRtTranslator()(node);
        return res;
    }
}


llvm::Value* Tr::translateNode(Node* node, TrContext& context)
{
    // Make sure the node is compiled
    Nest::semanticCheck(node);
    if ( !node->type )
        REP_INTERNAL(node->location, "No type found for node (%1%)") % node;

    // If this node is explained, then translate its explanation
    Node* expl = Nest::explanation(node);
    if ( node != expl )
    {
        return translateNode(expl, context);
    }

    // Check if the node is CT available and we are in RT mode. If so, translate the node into RT
    if ( !context.module().isCt() && Feather::isCt(node) )
    {
        node = convertCtToRt(node, context);
    }
        
    // Now, depending on the type of the node, do a specific translation

    // Set the correct location when translating a node
    if ( context.module().debugInfo() )
        context.module().debugInfo()->emitLocation(context.builder(), node->location);

    switch ( node->nodeKind - firstFeatherNodeKind )
    {
    case nkRelFeatherNodeList:                         return translateNodeList(node, context);
    case nkRelFeatherLocalSpace:                       return translateLocalSpace(node, context);
    case nkRelFeatherNop:                              return translateNop(node, context);
    case nkRelFeatherTempDestructAction:               return translateTempDestructAction(node, context);
    case nkRelFeatherScopeDestructAction:              return translateScopeDestructAction(node, context);
    case nkRelFeatherExpCtValue:                       return translate((CtValue&) *node, context);
    case nkRelFeatherExpStackAlloc:                    return translate((StackAlloc&) *node, context);
    case nkRelFeatherExpMemLoad:                       return translate((MemLoad&) *node, context);
    case nkRelFeatherExpMemStore:                      return translate((MemStore&) *node, context);
    case nkRelFeatherExpVarRef:                        return translate((VarRef&) *node, context);
    case nkRelFeatherExpFieldRef:                      return translate((FieldRef&) *node, context);
    case nkRelFeatherExpFunRef:                        return translate((FunRef&) *node, context);
    case nkRelFeatherExpFunCall:                       return translate((FunCall&) *node, context);
    case nkRelFeatherExpBitcast:                       return translate((Bitcast&) *node, context);
    case nkRelFeatherExpConditional:                   return translate((Conditional&) *node, context);
    case nkRelFeatherExpNull:                          return translate((Null&) *node, context);
    case nkRelFeatherStmtReturn:                       return translateReturn(node, context);
    case nkRelFeatherStmtIf:                           return translateIf(node, context);
    case nkRelFeatherStmtWhile:                        return translateWhile(node, context);
    case nkRelFeatherStmtBreak:                        return translateBreak(node, context);
    case nkRelFeatherStmtContinue:                     return translateContinue(node, context);
    case nkRelFeatherDeclVar:                          return translateVar(node, context);
    default:
        if ( node->nodeKind == DestructActionForConditional::classNodeKind() )
            return translate((DestructActionForConditional&) *node, context);
        else
        {
            REP_ERROR(node->location, "Don't know how to interpret a node of this kind (%1%)") % Nest::nodeKindName(node);
            return nullptr;
        }
    }
}

llvm::Value* Tr::setValue(Module& module, Node& node, llvm::Value* val)
{
    module.setNodeProperty(&node, Module::propValue, boost::any(val));
    return val;
}

llvm::Value* Tr::getValue(Module& module, Node& node, bool doCheck)
{
    llvm::Value** val = module.getNodePropertyValue<llvm::Value*>(&node, Module::propValue);
    if ( !val && doCheck )
        REP_INTERNAL(node.location, "Expected LLVM value for node %1%") % &node;
    return val ? *val : nullptr;
}
