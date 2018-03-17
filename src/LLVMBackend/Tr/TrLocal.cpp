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

#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers
//

/// Translate the given node, and makes sure that the node returns a valid llvm-value
llvm::Value* translateNodeCheck(Node* node, TrContext& context) {
    llvm::Value* res = translateNode(node, context);
    if (!res)
        REP_ERROR(node->location, "Invalid LLVM value returned for node %1%") % node;
    return res;
}

/// Write the scope & temp destruct actions from the stack.
/// If no scope is given, all the destruct actions in the context are written. If a scope is given,
/// the destruct actions for everything on the stack until that scope are written; the destruct
/// actions of the scope itself are not written.
void unwind(TrContext& context, Scope* lastScope = nullptr) {
    for (Scope* s : boost::adaptors::reverse(context.scopesStack())) {
        if (s == lastScope)
            break;

        s->outputDestructActions();
    }
}

// Helper node used to implement destruct actions for conditional expression
int nkLLVMDestructActionForConditional = 0;

Node* CondDestrAct_SemanticCheck(Node* node) {
    node->type = Nest_getCheckPropertyType(node, "resType");
    return node;
}

Node* mkDestructActionForConditional(TypeRef resType, llvm::Value* cond,
        NodeRange alt1DestructActions, NodeRange alt2DestructActions) {
    // Make sure the node kind is registered
    if (nkLLVMDestructActionForConditional == 0) {
        nkLLVMDestructActionForConditional =
                Nest_registerNodeKind("LLVMBackend.destructActionForConditional",
                        &CondDestrAct_SemanticCheck, nullptr, nullptr, nullptr);
    }
    Node* res = Nest_createNode(nkLLVMDestructActionForConditional);
    res->location = NOLOC;
    Nest_nodeSetChildren(res, fromIniList({Feather_mkNodeList(NOLOC, move(alt1DestructActions)),
                                      Feather_mkNodeList(NOLOC, move(alt2DestructActions))}));
    Nest_setPropertyType(res, "resType", resType);
    Nest_setPropertyPtr(res, "cond_LLVM_value", cond);
    return res;
}
llvm::Value* CondDestrAct_condition(Node* node) {
    return static_cast<llvm::Value*>(Nest_getCheckPropertyPtr(node, "cond_LLVM_value"));
}

/// Expression class that can hold a llvm value or a non-translated node.
/// This will make sure not to translate the node twice.
class Exp {
public:
    Exp(llvm::Value* val)
        : value_(val)
        , node_(nullptr) {}
    Exp(Node* node)
        : value_(nullptr)
        , node_(node) {}

    llvm::Value* translate(TrContext& context) const {
        if (!value_)
            value_ = translateNode(node_, context);
        return value_;
    }

private:
    mutable llvm::Value* value_;
    Node* node_;
};

llvm::Value* generateConditionalCode(TypeRef destType, CompilationContext* compContext, Node* cond,
        const Exp& alt1, const Exp& alt2, TrContext& context) {
    // Create the different blocks
    llvm::BasicBlock* alt1Block =
            llvm::BasicBlock::Create(context.llvmContext(), "cond.true", context.parentFun());
    llvm::BasicBlock* alt2Block =
            llvm::BasicBlock::Create(context.llvmContext(), "cond.false", context.parentFun());
    llvm::BasicBlock* afterBlock =
            llvm::BasicBlock::Create(context.llvmContext(), "cond.end", context.parentFun());

    // Translate the condition expression
    llvm::Value* condValue = translateNode(cond, context);
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
        alt1Block = context.insertionPoint(); // Might have changed in the meantime

        destructActions1 = instrGuard.stealDestructActions();
    }
    {
        Instruction instrGuard(context);

        // Translate the second alternative
        context.setInsertionPoint(alt2Block);
        val2 = alt2.translate(context);
        context.ensureInsertionPoint();
        context.builder().CreateBr(afterBlock);
        alt2Block = context.insertionPoint(); // Might have changed in the meantime

        destructActions2 = instrGuard.stealDestructActions();
    }

    // Translate the PHI node
    context.setInsertionPoint(afterBlock);
    llvm::PHINode* phiVal = llvm::PHINode::Create(getLLVMType(destType, context.globalContext()), 2,
            "cond.res", context.insertionPoint());
    phiVal->addIncoming(val1, alt1Block);
    phiVal->addIncoming(val2, alt2Block);

    // If one of the alternative have destruct actions, create a destruct action for the Conditional
    // expression
    if (!destructActions1.empty() || !destructActions2.empty()) {
        // The destruct action is also a kind of conditional operation - reuse the condition value
        Node* destructAction = mkDestructActionForConditional(
                destType, condValue, all(destructActions1), all(destructActions2));
        Nest_setContext(destructAction, compContext);
        if (!Nest_semanticCheck(destructAction))
            return nullptr;
        context.curInstruction().addTempDestructAction(destructAction);
    }

    return phiVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intrinsic function handling
//

llvm::Value* handleLogicalOr(Node* funCall, TrContext& context) {
    if (Nest_nodeArraySize(funCall->children) != 2)
        REP_INTERNAL(funCall->location, "Logical or must have exact 2 arguments");

    Node* arg1 = at(funCall->children, 0);
    Node* arg2 = at(funCall->children, 1);
    llvm::Value* trueConst = llvm::ConstantInt::getTrue(context.llvmContext());
    llvm::Value* res = generateConditionalCode(
            arg1->type, arg1->context, arg1, Exp(trueConst), Exp(arg2), context);
    return setValue(context, *funCall, res);
}

llvm::Value* handleLogicalAnd(Node* funCall, TrContext& context) {
    if (Nest_nodeArraySize(funCall->children) != 2)
        REP_INTERNAL(funCall->location, "Logical and must have exact 2 arguments");

    Node* arg1 = at(funCall->children, 0);
    Node* arg2 = at(funCall->children, 1);
    llvm::Value* falseConst = llvm::ConstantInt::getFalse(context.llvmContext());
    llvm::Value* res = generateConditionalCode(
            arg1->type, arg1->context, arg1, Exp(arg2), Exp(falseConst), context);
    return setValue(context, *funCall, res);
}

llvm::Value* handleFunPtr(Node* funCall, TrContext& context) {
    Node* fun = at(funCall->referredNodes, 0);

    // Does the resulting function has a resulting parameter?
    // Depending on that, compute the position of the this argument
    size_t thisArgPos = Nest_getPropertyNode(fun, propResultParam) ? 1 : 0;

    ASSERT(Nest_nodeArraySize(funCall->children) > 0);

    // Get the values for all the arguments
    vector<llvm::Value*> args;
    args.reserve(Nest_nodeArraySize(funCall->children));
    for (auto arg : funCall->children) {
        llvm::Value* v = translateNode(arg, context);
        CHECK(arg->location, v);
        args.push_back(v);
    }

    context.ensureInsertionPoint();

    // Create a function type pointer based on the arguments of the function
    llvm::Type* fType = getLLVMFunctionType(fun, thisArgPos, context.globalContext());
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
    val->setCallingConv(Tr::translateCallingConv(Feather_Function_callConvention(fun)));
    return setValue(context, *funCall, val);
}

llvm::Value* handleNativeFunCall(StringRef native, Node* funCall, TrContext& context) {
#define B() context.builder()
#define ARG(argNum) translateNode(at(args, argNum), context)
#define Ti(numBits) llvm::IntegerType::get(context.llvmContext(), numBits)
#define Tf() llvm::Type::getFloatTy(context.llvmContext())
#define Td() llvm::Type::getDoubleTy(context.llvmContext())
#define CONSTi(numBits, val)                                                                       \
    llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(numBits, val, false))
#define CONSTf(val) llvm::ConstantFP::get(Tf(), val)
#define CONSTd(val) llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext()), val)

    NodeRange args = all(funCall->children);
    if (Nest_nodeRangeSize(args) == 1) {
        if (native == "_zero_init_1")
            return B().CreateStore(CONSTi(1, 0), ARG(0));
        else if (native == "_zero_init_8")
            return B().CreateStore(CONSTi(8, 0), ARG(0));
        else if (native == "_zero_init_16")
            return B().CreateStore(CONSTi(16, 0), ARG(0));
        else if (native == "_zero_init_32")
            return B().CreateStore(CONSTi(32, 0), ARG(0));
        else if (native == "_zero_init_64")
            return B().CreateStore(CONSTi(64, 0), ARG(0));
        else if (native == "_zero_init_f")
            return B().CreateStore(CONSTf(0.0), ARG(0));
        else if (native == "_zero_init_d")
            return B().CreateStore(CONSTd(0.0), ARG(0));

        else if (native == "_Byte_opPlus1" || native == "_Short_opPlus1" ||
                 native == "_Int_opPlus1" || native == "_Long_opPlus1" ||
                 native == "_UByte_opPlus1" || native == "_UShort_opPlus1" ||
                 native == "_UInt_opPlus1" || native == "_ULong_opPlus1")
            return ARG(0);

        else if (native == "_Byte_opMinus1")
            return B().CreateSub(CONSTi(8, 0), ARG(0));
        else if (native == "_Short_opMinus1")
            return B().CreateSub(CONSTi(16, 0), ARG(0));
        else if (native == "_Int_opMinus1")
            return B().CreateSub(CONSTi(32, 0), ARG(0));
        else if (native == "_Long_opMinus1")
            return B().CreateSub(CONSTi(64, 0), ARG(0));
        else if (native == "_DiffType_opMinus1")
            return B().CreateSub(CONSTi(64, 0), ARG(0));

        else if (native == "_Bool_opNeg")
            return B().CreateXor(CONSTi(1, 1), ARG(0));
    } else if (Nest_nodeRangeSize(args) == 2) {
        if (native == "_ass_1_1" || native == "_ass_8_8" || native == "_ass_16_16" ||
                native == "_ass_32_32" || native == "_ass_64_64" || native == "_ass_f_f" ||
                native == "_ass_d_d")
            return B().CreateStore(ARG(1), ARG(0));
        else if (native == "_ass_8_16" || native == "_ass_8_32" || native == "_ass_8_64")
            return B().CreateStore(B().CreateTrunc(ARG(1), Ti(8)), ARG(0));
        else if (native == "_ass_16_32" || native == "_ass_16_64")
            return B().CreateStore(B().CreateTrunc(ARG(1), Ti(16)), ARG(0));
        else if (native == "_ass_32_64")
            return B().CreateStore(B().CreateTrunc(ARG(1), Ti(32)), ARG(0));

        else if (native == "_ass_16_8s")
            return B().CreateStore(B().CreateSExt(ARG(1), Ti(16)), ARG(0));
        else if (native == "_ass_32_8s" || native == "_ass_32_16s")
            return B().CreateStore(B().CreateSExt(ARG(1), Ti(32)), ARG(0));
        else if (native == "_ass_64_8s" || native == "_ass_64_16s" || native == "_ass_64_32s")
            return B().CreateStore(B().CreateSExt(ARG(1), Ti(64)), ARG(0));

        else if (native == "_ass_16_8z")
            return B().CreateStore(B().CreateZExt(ARG(1), Ti(16)), ARG(0));
        else if (native == "_ass_32_8s" || native == "_ass_32_16z")
            return B().CreateStore(B().CreateZExt(ARG(1), Ti(32)), ARG(0));
        else if (native == "_ass_64_8s" || native == "_ass_64_16z" || native == "_ass_64_32z")
            return B().CreateStore(B().CreateZExt(ARG(1), Ti(64)), ARG(0));

        else if (native == "_ass_i8_f" || native == "_ass_i8_d")
            return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(8)), ARG(0));
        else if (native == "_ass_u8_f" || native == "_ass_u8_d")
            return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(8)), ARG(0));
        else if (native == "_ass_i16_f" || native == "_ass_i16_d")
            return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(16)), ARG(0));
        else if (native == "_ass_u16_f" || native == "_ass_u16_d")
            return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(16)), ARG(0));
        else if (native == "_ass_i32_f" || native == "_ass_i32_d")
            return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(32)), ARG(0));
        else if (native == "_ass_u32_f" || native == "_ass_u32_d")
            return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(32)), ARG(0));
        else if (native == "_ass_i64_f" || native == "_ass_i64_d")
            return B().CreateStore(B().CreateFPToSI(ARG(1), Ti(64)), ARG(0));
        else if (native == "_ass_u64_f" || native == "_ass_u64_d")
            return B().CreateStore(B().CreateFPToUI(ARG(1), Ti(64)), ARG(0));

        else if (native == "_ass_f_i8" || native == "_ass_f_i16" || native == "_ass_f_i32" ||
                 native == "_ass_f_i64")
            return B().CreateStore(B().CreateSIToFP(ARG(1), Tf()), ARG(0));
        else if (native == "_ass_f_u8" || native == "_ass_f_u16" || native == "_ass_f_u32" ||
                 native == "_ass_f_u64")
            return B().CreateStore(B().CreateUIToFP(ARG(1), Tf()), ARG(0));
        else if (native == "_ass_d_i8" || native == "_ass_d_i16" || native == "_ass_d_i32" ||
                 native == "_ass_d_i64")
            return B().CreateStore(B().CreateSIToFP(ARG(1), Td()), ARG(0));
        else if (native == "_ass_d_u8" || native == "_ass_d_u16" || native == "_ass_d_u32" ||
                 native == "_ass_d_u64")
            return B().CreateStore(B().CreateUIToFP(ARG(1), Td()), ARG(0));

        else if (native == "_ass_f_d")
            return B().CreateStore(B().CreateFPTrunc(ARG(1), Tf()), ARG(0));
        else if (native == "_ass_d_f")
            return B().CreateStore(B().CreateFPExt(ARG(1), Td()), ARG(0));

        else if (native == "_Byte_opEQ" || native == "_Short_opEQ" || native == "_Int_opEQ" ||
                 native == "_Long_opEQ" || native == "_UByte_opEQ" || native == "_UShort_opEQ" ||
                 native == "_UInt_opEQ" || native == "_ULong_opEQ" || native == "_SizeType_opEQ" ||
                 native == "_DiffType_opEQ" || native == "_Bool_opEQ" || native == "_Char_opEQ")
            return B().CreateICmpEQ(ARG(0), ARG(1));
        else if (native == "_Byte_opNE" || native == "_Short_opNE" || native == "_Int_opNE" ||
                 native == "_Long_opNE" || native == "_UByte_opNE" || native == "_UShort_opNE" ||
                 native == "_UInt_opNE" || native == "_ULong_opNE" || native == "_SizeType_opNE" ||
                 native == "_DiffType_opNE" || native == "_Bool_opNE" || native == "_Char_opNE")
            return B().CreateICmpNE(ARG(0), ARG(1));
        else if (native == "_Byte_opLT" || native == "_Short_opLT" || native == "_Int_opLT" ||
                 native == "_Long_opLT" || native == "_UByte_opLT" || native == "_UShort_opLT" ||
                 native == "_UInt_opLT" || native == "_ULong_opLT" || native == "_SizeType_opLT" ||
                 native == "_DiffType_opLT" || native == "_Char_opLT")
            return B().CreateICmpSLT(ARG(0), ARG(1));
        else if (native == "_Byte_opGT" || native == "_Short_opGT" || native == "_Int_opGT" ||
                 native == "_Long_opGT" || native == "_UByte_opGT" || native == "_UShort_opGT" ||
                 native == "_UInt_opGT" || native == "_ULong_opGT" || native == "_SizeType_opGT" ||
                 native == "_DiffType_opGT" || native == "_Char_opGT")
            return B().CreateICmpSGT(ARG(0), ARG(1));
        else if (native == "_Byte_opLE" || native == "_Short_opLE" || native == "_Int_opLE" ||
                 native == "_Long_opLE" || native == "_UByte_opLE" || native == "_UShort_opLE" ||
                 native == "_UInt_opLE" || native == "_ULong_opLE" || native == "_SizeType_opLE" ||
                 native == "_DiffType_opLE" || native == "_Char_opLE")
            return B().CreateICmpSLE(ARG(0), ARG(1));
        else if (native == "_Byte_opGE" || native == "_Short_opGE" || native == "_Int_opGE" ||
                 native == "_Long_opGE" || native == "_UByte_opGE" || native == "_UShort_opGE" ||
                 native == "_UInt_opGE" || native == "_ULong_opGE" || native == "_SizeType_opGE" ||
                 native == "_DiffType_opGE" || native == "_Char_opGE")
            return B().CreateICmpSGE(ARG(0), ARG(1));

        else if (native == "_Byte_opPlus" || native == "_Short_opPlus" || native == "_Int_opPlus" ||
                 native == "_Long_opPlus" || native == "_UByte_opPlus" ||
                 native == "_UShort_opPlus" || native == "_UInt_opPlus" ||
                 native == "_ULong_opPlus" || native == "_SizeType_opPlus" ||
                 native == "_DiffType_opPlus")
            return B().CreateAdd(ARG(0), ARG(1));
        else if (native == "_Byte_opMinus" || native == "_Short_opMinus" ||
                 native == "_Int_opMinus" || native == "_Long_opMinus" ||
                 native == "_UByte_opMinus" || native == "_UShort_opMinus" ||
                 native == "_UInt_opMinus" || native == "_ULong_opMinus" ||
                 native == "_SizeType_opMinu" || native == "_DiffType_opMinu")
            return B().CreateSub(ARG(0), ARG(1));
        else if (native == "_Byte_opMul" || native == "_Short_opMul" || native == "_Int_opMul" ||
                 native == "_Long_opMul" || native == "_UByte_opMul" || native == "_UShort_opMul" ||
                 native == "_UInt_opMul" || native == "_ULong_opMul" ||
                 native == "_SizeType_opMul" || native == "_DiffType_opMul")
            return B().CreateMul(ARG(0), ARG(1));
        else if (native == "_Byte_opDiv" || native == "_Short_opDiv" || native == "_Int_opDiv" ||
                 native == "_Long_opDiv" || native == "_DiffType_opDiv")
            return B().CreateSDiv(ARG(0), ARG(1));
        else if (native == "_UByte_opDiv" || native == "_UShort_opDiv" || native == "_UInt_opDiv" ||
                 native == "_ULong_opDiv" || native == "_SizeType_opPDiv")
            return B().CreateUDiv(ARG(0), ARG(1));
        else if (native == "_Byte_opMod" || native == "_Short_opMod" || native == "_Int_opMod" ||
                 native == "_Long_opMod" || native == "_DiffType_opMod")
            return B().CreateSRem(ARG(0), ARG(1));
        else if (native == "_UByte_opMod" || native == "_UShort_opMod" || native == "_UInt_opMod" ||
                 native == "_ULong_opMod" || native == "_SizeType_opMod")
            return B().CreateURem(ARG(0), ARG(1));

        // TODO: floating point operations: compares, arithmetic

        else if (native == "_Bool_opXor")
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

#define emitLocationInfo(name, loc, context) /*nothing*/
//    void emitLocationInfo(const char* name, Location loc, TrContext& context)
//    {
//        context.ensureInsertionPoint();
//
//        StringRef sourceLine = Nest_getSourceCodeLine(loc.sourceCode, loc.start.line);
//        ostringstream ss;
//        ss << name << ": " << sourceLine;
//
//        // Create a the data array constant
//        llvm::Value* bytesVal = llvm::ConstantDataArray::getString(context.llvmContext(),
//        llvm::StringRef(ss.str()));
//
//        // Allocate a temporary variable to hold it
//        llvm::Value* constTmpVar = context.addVariable(bytesVal->getType(), "location-info");
//        context.builder().CreateStore(bytesVal, constTmpVar);
//    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common nodes
//

llvm::Value* translateNodeList(Node* node, TrContext& context) {
    llvm::Value* res = nullptr;
    for (Node* child : node->children) {
        if (child)
            res = translateNode(child, context);
    }
    return res;
}

llvm::Value* translateLocalSpace(Node* node, TrContext& context) {
    emitLocationInfo("space ", node->location, context);

    Scope scopeGuard(context, node->location);

    // Translate all the instructions in the local space
    for (Node* child : node->children) {
        emitLocationInfo("instr", child->location, context);

        Instruction instrGuard(context);
        translateNode(child, context);
    }

    return nullptr;
}

llvm::Value* translateNop(Node* /*node*/, TrContext& /*context*/) {
    // Do nothing
    return nullptr;
}

llvm::Value* translateTempDestructAction(Node* node, TrContext& context) {
    // Add the action to the temp destruct actions in the context
    Node* act = at(node->children, 0);
    context.curInstruction().addTempDestructAction(act);
    return nullptr;
}

llvm::Value* translateScopeDestructAction(Node* node, TrContext& context) {
    // Add the action to the scope destruct actions in the context
    Node* act = at(node->children, 0);
    context.curScope().addScopeDestructAction(act);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Expressions
//

llvm::AtomicOrdering llvmOrdering(AtomicOrdering ord) {
    switch (ord) {
    case atomicNone:
        return llvm::AtomicOrdering::NotAtomic;
    case atomicUnordered:
        return llvm::AtomicOrdering::Unordered;
    case atomicMonotonic:
        return llvm::AtomicOrdering::Monotonic;
    case atomicAcquire:
        return llvm::AtomicOrdering::Acquire;
    case atomicRelease:
        return llvm::AtomicOrdering::Release;
    case atomicAcquireRelease:
        return llvm::AtomicOrdering::AcquireRelease;
    case atomicSeqConsistent:
        return llvm::AtomicOrdering::SequentiallyConsistent;
    default:
        return llvm::AtomicOrdering::NotAtomic;
    }
}

llvm::Value* translateStringRefConstant(
        llvm::Type* t, const char* begin, const char* end, TrContext& context) {
    uint64_t size = end - begin;

    context.ensureInsertionPoint();

    llvm::Value* constInt0 = context.builder().getInt32(0);
    llvm::Value* constInt1 = context.builder().getInt32(1);
    llvm::Value* constIntSize = context.builder().getInt32(size);

    // Create a global string constant
    llvm::Value* globalStr =
            context.builder().CreateGlobalStringPtr(llvm::StringRef(begin, (size_t)size), "str");

    // Obtain pointers to the end of the global string
    llvm::Value* globalStrEnd = context.builder().CreateInBoundsGEP(globalStr, constIntSize, "");

    // Create a temporary variable for the actual structure object
    llvm::Value* tmpVar = context.addVariable(t, "tmp.StringRef");

    // Copy pointers of the string constant into the StringRef structure
    llvm::Value* indices[2] = {constInt0, constInt0};
    llvm::Value* beginAddr = context.builder().CreateInBoundsGEP(tmpVar, indices, "");
    indices[1] = constInt1;
    llvm::Value* endAddr = context.builder().CreateInBoundsGEP(tmpVar, indices, "");
    context.builder().CreateStore(globalStr, beginAddr);
    context.builder().CreateStore(globalStrEnd, endAddr);

    return context.builder().CreateLoad(tmpVar);
}

llvm::Value* translateCtValue(Node* node, TrContext& context) {
    // Get the type of the ct value
    llvm::Type* t = Tr::getLLVMType(node->type, context.globalContext());

    // Check for String CtValues
    if (!context.module().isCt()) {
        TypeRef tt = node->type;
        if (tt->typeKind == typeKindData) {
            StringRef nativeName = Feather_nativeName(tt);
            if (nativeName == "StringRef") {
                StringRef data = *Feather_getCtValueData<StringRef>(node);
                return translateStringRefConstant(t, data.begin, data.end, context);
            }
        }
    }

    // Get the resulting value
    if (t->isIntegerTy()) {
        int width = static_cast<llvm::IntegerType*>(t)->getBitWidth();
        uint64_t val;

        switch (width) {
        case 1:
            val = 0 != (*Feather_getCtValueData<unsigned char>(node));
            break;
        case 8:
            val = *Feather_getCtValueData<unsigned char>(node);
            break;
        case 16:
            val = *Feather_getCtValueData<unsigned short>(node);
            break;
        case 32:
            val = *Feather_getCtValueData<unsigned int>(node);
            break;
        case 64:
            val = *Feather_getCtValueData<uint64_t>(node);
            break;
        default: {
            val = 0;
            REP_INTERNAL(node->location, "Invalid bit width (%1%) for numeric literal (%2%)") %
                    width % node;
        }
        }

        return setValue(
                context, *node, llvm::ConstantInt::get(static_cast<llvm::IntegerType*>(t), val));
    } else if (t->isFloatTy()) {
        return setValue(
                context, *node, llvm::ConstantFP::get(t, *Feather_getCtValueData<float>(node)));
    } else if (t->isDoubleTy()) {
        return setValue(
                context, *node, llvm::ConstantFP::get(t, *Feather_getCtValueData<double>(node)));
    } else if (t->isPointerTy()) {
        uint64_t val = *Feather_getCtValueData<uint64_t>(node);
        llvm::Value* intVal =
                llvm::ConstantInt::get(llvm::IntegerType::get(context.llvmContext(), 64), val);
        return new llvm::IntToPtrInst(intVal, t, "", context.insertionPoint());
    } else if (t->isStructTy()) {
        // If our size is zero, return a zero constant aggregate
        size_t size = context.llvmModule().getDataLayout().getTypeAllocSize(t);
        if (size == 0)
            return llvm::ConstantAggregateZero::get(t);

        context.ensureInsertionPoint();

        // Type of pointer to our structure
        llvm::Type* ptr = llvm::PointerType::getUnqual(t);

        // Create an array constant
        const uint8_t* valBegin = Feather_getCtValueData<uint8_t>(node);
        llvm::Value* constArrVal = llvm::ConstantDataArray::get(
                context.llvmContext(), llvm::ArrayRef<uint8_t>(valBegin, size));

        // Create an array value and initialize it with our array constant
        llvm::Type* arrType =
                llvm::ArrayType::get(llvm::IntegerType::get(context.llvmContext(), 8), size);
        llvm::AllocaInst* tmpVar = context.addVariable(arrType, "const.arr");
        tmpVar->setAlignment(1);
        context.builder().CreateStore(constArrVal, tmpVar);

        llvm::Value* ptrToVal = context.builder().CreateBitCast(tmpVar, ptr);
        return context.builder().CreateLoad(ptrToVal);
    } else {
        REP_INTERNAL(node->location, "Don't know how to translate ct value of type %1%") %
                node->type;
        return nullptr;
    }
}

llvm::Value* translateMemLoad(Node* node, TrContext& context) {
    // Create a 'load' instruction
    Node* exp = at(node->children, 0);
    llvm::Value* argVal = translateNodeCheck(exp, context);
    CHECK(node->location, argVal);
    context.ensureInsertionPoint();
    llvm::LoadInst* val = context.builder().CreateLoad(argVal);
    int alignment = Nest_getCheckPropertyInt(node, "alignment");
    if (alignment > 0)
        val->setAlignment(alignment);
    val->setVolatile(0 != Nest_getCheckPropertyInt(node, "volatile"));
    val->setOrdering(
            llvmOrdering((AtomicOrdering)Nest_getCheckPropertyInt(node, "atomicOrdering")));
    val->setSyncScopeID(Nest_getCheckPropertyInt(node, "singleThreaded")
                                ? llvm::SyncScope::SingleThread
                                : llvm::SyncScope::System);
    return setValue(context, *node, val);
}

llvm::Value* translateMemStore(Node* node, TrContext& context) {
    context.ensureInsertionPoint();

    // Create a 'store' instruction
    Node* valueNode = at(node->children, 0);
    Node* addressNode = at(node->children, 1);
    llvm::Value* value = translateNode(valueNode, context);
    CHECK(node->location, value);
    llvm::Value* address = translateNode(addressNode, context);
    CHECK(node->location, address);
    context.ensureInsertionPoint();
    llvm::StoreInst* val = context.builder().CreateStore(value, address);
    int alignment = Nest_getCheckPropertyInt(node, "alignment");
    if (alignment > 0)
        val->setAlignment(alignment);
    val->setVolatile(0 != Nest_getCheckPropertyInt(node, "volatile"));
    val->setOrdering(
            llvmOrdering((AtomicOrdering)Nest_getCheckPropertyInt(node, "atomicOrdering")));
    val->setSyncScopeID(Nest_getCheckPropertyInt(node, "singleThreaded")
                                ? llvm::SyncScope::SingleThread
                                : llvm::SyncScope::System);
    return setValue(context, *node, val);
}

llvm::Value* translateVarRef(Node* node, TrContext& context) {
    // Simply take the value from the referenced variable
    Node* variable = at(node->referredNodes, 0);
    CHECK(node->location, variable);
    llvm::Value* varVal = getValue(context, *variable, false);
    if (!varVal) {
        // If we are here, we are trying to reference a variable that wasn't declared yet.
        // This can only happen for global variables that were not yet translated
        if (variable->nodeKind != nkFeatherDeclVar)
            REP_INTERNAL(variable->location, "Cannot find variable %1%") %
                    Feather_getName(variable);

        varVal = Tr::translateGlobalVar(variable, context.globalContext());
        if (!varVal) {
            if (Feather_effectiveEvalMode(variable) == modeCt && !context.module().isCt())
                REP_INTERNAL(node->location,
                        "Trying to reference a compile-time variable %1% from run-time") %
                        Feather_getName(variable);
            else
                REP_INTERNAL(node->location, "Cannot find variable %1% in the current module") %
                        Feather_getName(variable);
        }
    }
    return setValue(context, *node, varVal);
}

llvm::Value* translateFieldRef(Node* node, TrContext& context) {
    // Make sure the object is translated
    Node* object = at(node->children, 0);
    Node* field = at(node->referredNodes, 0);
    CHECK(node->location, object);
    llvm::Value* objVal = translateNode(object, context);
    CHECK(node->location, objVal);

    // Compute the index of the field
    uint64_t idx = 0;
    ASSERT(object->type);
    Node* clsDecl = Feather_classForType(object->type);
    CHECK(node->location, clsDecl);
    for (auto f : clsDecl->children) {
        if (field == f)
            break;
        ++idx;
    }
    if (idx == Nest_nodeArraySize(clsDecl->children))
        REP_INTERNAL(node->location, "Cannot find field '%1%' in class '%2%'") %
                Feather_getName(field) % Feather_getName(clsDecl);

    // Create a 'getelementptr' instruction
    vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, 0, false)));
    indices.push_back(llvm::ConstantInt::get(context.llvmContext(), llvm::APInt(32, idx, false)));
    context.ensureInsertionPoint();
    llvm::Value* val = context.builder().CreateInBoundsGEP(objVal, indices, "");
    return setValue(context, *node, val);
}

llvm::Value* translateFunRef(Node* node, TrContext& context) {
    // Make sure the function is translated; get a function pointer value
    Node* funDecl = at(node->referredNodes, 0);
    llvm::Function* func = translateFunction(funDecl, context.globalContext());
    CHECK(node->location, func);

    // Get the functor data type, plain and with 1 or 2 pointers
    llvm::Type* t = getLLVMType(node->type, context.globalContext());
    llvm::Type* pt = llvm::PointerType::get(t, 0);
    CHECK(node->location, t);

    // Use bitcast of pointers to convert from function pointer to data type
    llvm::Value* tmpVal = context.addVariable(func->getType(), "funptr");
    context.ensureInsertionPoint();
    context.builder().CreateStore(func, tmpVal);
    llvm::Value* bc = context.builder().CreateBitCast(tmpVal, pt);
    llvm::Value* res = context.builder().CreateLoad(bc);
    return setValue(context, *node, res);
}

llvm::Value* translateFunCall(Node* node, TrContext& context) {
    Node* funDecl = at(node->referredNodes, 0);
    CHECK(node->location, funDecl);

    // Do we have any valuable instructions generated by this function call?
    // We may have them because of the arguments, and because of the function
    if (Nest_hasProperty(node, propEmptyBody)) {
        return nullptr;
    }

    // Check for intrinsic native functions
    const StringRef* nativeName = Nest_getPropertyString(funDecl, propNativeName);
    if (nativeName && size(*nativeName) > 0 && nativeName->begin[0] == '$') {
        if (*nativeName == "$logicalOr")
            return handleLogicalOr(node, context);
        if (*nativeName == "$logicalAnd")
            return handleLogicalAnd(node, context);
        if (*nativeName == "$funptr")
            return handleFunPtr(node, context);
    }
    if (nativeName && size(*nativeName) > 0 && nativeName->begin[0] == '_') {
        context.ensureInsertionPoint();
        auto res = handleNativeFunCall(*nativeName, node, context);
        if (res)
            return setValue(context, *node, res);
    }

    llvm::Function* func = translateFunction(funDecl, context.globalContext());
    CHECK(node->location, func);

    llvm::Value* res = nullptr;

    // Get the values for all the arguments
    vector<llvm::Value*> args;
    args.reserve(Nest_nodeArraySize(node->children));
    for (auto arg : node->children) {
        llvm::Value* v = translateNode(arg, context);
        CHECK(arg->location, v);
        args.push_back(v);
    }

    // Arguments may have some instructions, but the function may not; check this
    if (Nest_hasProperty(funDecl, propEmptyBody)) {
        return nullptr;
    }

    context.ensureInsertionPoint();

    // Apply a bitcast if the types don't match
    llvm::Constant* toCall = func;
    auto* funType =
            static_cast<llvm::FunctionType*>(getLLVMType(funDecl->type, context.globalContext()));
    llvm::PointerType* funTypePtr = llvm::PointerType::getUnqual(funType);
    if (func->getType() != funTypePtr)
        toCall = llvm::ConstantExpr::getBitCast(func, funTypePtr);

    // Create a 'call' instruction
    llvm::CallInst* val = context.builder().CreateCall(toCall, args, "");
    ;
    val->setCallingConv(Tr::translateCallingConv(Feather_Function_callConvention(funDecl)));
    if (res)
        return context.builder().CreateLoad(res);
    else
        return val;
}

llvm::Value* translateBitcast(Node* node, TrContext& context) {
    Node* expNode = at(node->children, 0);
    Node* destTypeNode = at(node->children, 1);
    llvm::Value* exp = translateNode(expNode, context);
    CHECK(node->location, exp);
    llvm::Type* destType = Tr::getLLVMType(destTypeNode->type, context.globalContext());
    if (exp->getType() == destType)
        return setValue(context, *node, exp);

    // Create a 'bitcast' instruction
    llvm::Value* val;
    ASSERT(destTypeNode->type->numReferences > 0);
    ASSERT(expNode->type->numReferences > 0);
    context.ensureInsertionPoint();
    val = context.builder().CreateBitCast(exp, destType);
    return setValue(context, *node, val);
}

llvm::Value* translateConditional(Node* node, TrContext& context) {
    Node* cond = at(node->children, 0);
    Node* alt1 = at(node->children, 1);
    Node* alt2 = at(node->children, 2);

    // Create the different blocks
    llvm::BasicBlock* alt1Block =
            llvm::BasicBlock::Create(context.llvmContext(), "cond_alt1", context.parentFun());
    llvm::BasicBlock* alt2Block =
            llvm::BasicBlock::Create(context.llvmContext(), "cond_alt2", context.parentFun());
    llvm::BasicBlock* afterBlock =
            llvm::BasicBlock::Create(context.llvmContext(), "cond_end", context.parentFun());

    // Translate the condition expression
    llvm::Value* condValue = translateNode(cond, context);
    CHECK(cond->location, condValue);
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
        val1 = translateNode(alt1, context);
        context.ensureInsertionPoint();
        context.builder().CreateBr(afterBlock);

        destructActions1 = instrGuard.stealDestructActions();
    }
    {
        Instruction instrGuard(context);

        // Translate the second alternative
        context.setInsertionPoint(alt2Block);
        val2 = translateNode(alt2, context);
        context.ensureInsertionPoint();
        context.builder().CreateBr(afterBlock);

        destructActions2 = instrGuard.stealDestructActions();
    }

    // Translate the PHI node
    context.setInsertionPoint(afterBlock);
    llvm::Type* destType = Tr::getLLVMType(node->type, context.globalContext());
    llvm::PHINode* phiVal = llvm::PHINode::Create(destType, 2, "cond", context.insertionPoint());
    phiVal->addIncoming(val1, alt1Block);
    phiVal->addIncoming(val2, alt2Block);

    // If one of the alternative have destruct actions, create a destruct action for the Conditional
    // expression
    if (!destructActions1.empty() || !destructActions2.empty()) {
        // The destruct action is also a kind of conditional operation - reuse the condition value
        Node* destructAction = mkDestructActionForConditional(
                node->type, condValue, all(destructActions1), all(destructActions2));
        Nest_setContext(destructAction, node->context);
        if (!Nest_semanticCheck(destructAction))
            return nullptr;
        ASSERT(!context.scopesStack().empty());
        ASSERT(!context.scopesStack().back()->instructionsStack().empty());
        context.scopesStack().back()->instructionsStack().back()->addTempDestructAction(
                destructAction);
    }

    return phiVal;
}

llvm::Value* translateDestructActionForConditional(Node* node, TrContext& context) {
    Node* alt1Actions = at(node->children, 0);
    Node* alt2Actions = at(node->children, 1);

    // Create the different blocks
    llvm::BasicBlock* alt1Block = llvm::BasicBlock::Create(
            context.llvmContext(), "cond_destruct_alt1", context.parentFun());
    llvm::BasicBlock* alt2Block = llvm::BasicBlock::Create(
            context.llvmContext(), "cond_destruct_alt2", context.parentFun());
    llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(
            context.llvmContext(), "cond_destruct_end", context.parentFun());

    // Translate the condition expression
    context.ensureInsertionPoint();
    context.builder().CreateCondBr(CondDestrAct_condition(node), alt1Block, alt2Block);

    // Alternative 1 destruct actions
    context.setInsertionPoint(alt1Block);
    unsigned size1 = Nest_nodeArraySize(alt1Actions->children);
    for (int i = size1 - 1; i >= 0; --i) {
        Node* n = at(alt1Actions->children, i);
        translateNode(n, context);
    }
    context.ensureInsertionPoint();
    context.builder().CreateBr(afterBlock);

    // Alternative 2 destruct actions
    context.setInsertionPoint(alt2Block);
    unsigned size2 = Nest_nodeArraySize(alt2Actions->children);
    for (int i = size2 - 1; i >= 0; --i) {
        Node* n = at(alt2Actions->children, i);
        translateNode(n, context);
    }
    context.ensureInsertionPoint();
    context.builder().CreateBr(afterBlock);

    context.setInsertionPoint(afterBlock);
    return nullptr;
}

llvm::Value* translateNull(Node* node, TrContext& context) {
    llvm::Type* resType = Tr::getLLVMType(node->type, context.globalContext());
    if (!resType->isPointerTy())
        REP_INTERNAL(node->location, "Null type should be a pointer (we have: %1%)") % node->type;

    return llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(resType));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Statements
//

llvm::Value* translateReturn(Node* node, TrContext& context) {
    emitLocationInfo("return", node->location, context);

    llvm::Value* retVal = nullptr;
    Node* expression = at(node->children, 0);
    if (expression) {
        retVal = translateNode(expression, context);
        CHECK(expression->location, retVal);
        llvm::Function* f = context.parentFun();
        if (f->hasStructRetAttr()) {
            context.ensureInsertionPoint();
            context.builder().CreateStore(retVal, f->arg_begin());
            retVal = nullptr;
        }
    }

    unwind(context);
    context.ensureInsertionPoint();
    if (retVal)
        context.builder().CreateRet(retVal);
    else
        context.builder().CreateRetVoid();
    context.setInsertionPoint(nullptr); // No other instruction will be placed in this block
    return nullptr;
}

llvm::Value* translateIf(Node* node, TrContext& context) {
    emitLocationInfo("if", node->location, context);

    Node* condition = at(node->children, 0);
    Node* thenClause = at(node->children, 1);
    Node* elseClause = at(node->children, 2);

    // Create the different blocks
    llvm::BasicBlock* ifBlock =
            llvm::BasicBlock::Create(context.llvmContext(), "if_block", context.parentFun());
    llvm::BasicBlock* thenBlock = thenClause ? llvm::BasicBlock::Create(context.llvmContext(),
                                                       "if_then", context.parentFun())
                                             : nullptr;
    llvm::BasicBlock* elseBlock = elseClause ? llvm::BasicBlock::Create(context.llvmContext(),
                                                       "if_else", context.parentFun())
                                             : nullptr;
    llvm::BasicBlock* afterBlock =
            llvm::BasicBlock::Create(context.llvmContext(), "if_end", context.parentFun());
    if (!thenBlock)
        thenBlock = afterBlock;
    if (!elseBlock)
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
        if (thenClause) {
            ASSERT(thenBlock);
            context.setInsertionPoint(thenBlock);
            {
                emitLocationInfo("if-then", thenClause->location, context);

                Instruction instrGuard(context);
                translateNode(thenClause, context);
            }
            context.ensureInsertionPoint();
            context.builder().CreateBr(afterBlock);
        }

        // Translate the else branch
        if (elseClause) {
            ASSERT(elseBlock);
            context.setInsertionPoint(elseBlock);
            {
                emitLocationInfo("if-else", elseClause->location, context);

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

llvm::Value* translateWhile(Node* node, TrContext& context) {
    emitLocationInfo("while", node->location, context);

    Node* condition = at(node->children, 0);
    Node* step = at(node->children, 1);
    Node* body = at(node->children, 2);

    // Create a new scope for the while instruction
    Scope scopeGuard(context, node->location);
    Instruction instructionGuard(context);

    // Create the different blocks
    llvm::BasicBlock* whileBlock =
            llvm::BasicBlock::Create(context.llvmContext(), "while_block", context.parentFun());
    llvm::BasicBlock* whileBody =
            llvm::BasicBlock::Create(context.llvmContext(), "while_body", context.parentFun());
    llvm::BasicBlock* whileStep =
            llvm::BasicBlock::Create(context.llvmContext(), "while_step", context.parentFun());
    llvm::BasicBlock* whileEnd =
            llvm::BasicBlock::Create(context.llvmContext(), "while_end", context.parentFun());

    // Store the step & the end labels (blocks) in the object
    ASSERT(!context.scopesStack().empty());
    context.whileInstrGuards_[node] = &scopeGuard;
    context.whileStepLabels_[node] = whileStep;
    context.whileEndLabels_[node] = whileEnd;

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
        if (body) {
            emitLocationInfo("while-body", body->location, context);

            Instruction instrGuard(context);
            translateNode(body, context);
        }
        context.ensureInsertionPoint();
        context.builder().CreateBr(whileStep);
        context.setInsertionPoint(whileStep);
        if (step) {
            emitLocationInfo("while-step", step->location, context);

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

llvm::Value* translateBreak(Node* node, TrContext& context) {
    emitLocationInfo("break", node->location, context);

    Node* whileNode = Nest_getCheckPropertyNode(node, "loop");
    CHECK(node->location, whileNode);

    // Get the instruction guard for the while instruction
    Scope* whileInstr = context.whileInstrGuards_[whileNode];
    CHECK(node->location, whileInstr);

    // Translate the destruct actions until the while instruction
    unwind(context, whileInstr);

    // Get the while's end label
    llvm::BasicBlock* endLabel = context.whileEndLabels_[whileNode];
    CHECK(node->location, endLabel);

    // Create a jump to the end-of-while label
    context.ensureInsertionPoint();
    context.builder().CreateBr(endLabel);
    context.setInsertionPoint(nullptr); // No other instruction will be placed in this block

    return nullptr;
}

llvm::Value* translateContinue(Node* node, TrContext& context) {
    emitLocationInfo("continue", node->location, context);

    Node* whileNode = Nest_getCheckPropertyNode(node, "loop");
    CHECK(node->location, whileNode);

    // Get the instruction guard for the while instruction
    Scope* whileInstr = context.whileInstrGuards_[whileNode];
    CHECK(node->location, whileInstr);

    // Translate the destruct actions until the while instruction
    unwind(context, whileInstr);

    // Get the while's step label
    llvm::BasicBlock* stepLabel = context.whileStepLabels_[whileNode];
    CHECK(node->location, stepLabel);

    // Create a jump to the while-step label
    context.ensureInsertionPoint();
    context.builder().CreateBr(stepLabel);
    context.setInsertionPoint(nullptr); // No other instruction will be placed in this block

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Local declarations
//

llvm::Value* translateVar(Node* node, TrContext& context) {
    ASSERT(context.parentFun());

    // Create an 'alloca' instruction for the local variable
    llvm::Type* t = Tr::getLLVMType(node->type, context.globalContext());
    llvm::AllocaInst* val = context.addVariable(t, Feather_getName(node).begin);
    int alignment = Nest_getCheckPropertyInt(node, "alignment");
    if (alignment > 0)
        val->setAlignment(alignment);
    return setValue(context, *node, val);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General
//

Node* convertCtToRt(Node* node, TrContext& context) {
    Node* res = node;
    if (context.module().ctToRtTranslator())
        res = context.module().ctToRtTranslator()(node);
    return res;
}
} // namespace

llvm::Value* Tr::translateNode(Node* node, TrContext& context) {
    // Make sure the node is compiled
    if (!Nest_semanticCheck(node))
        return nullptr;
    if (!node->type)
        REP_INTERNAL(node->location, "No type found for node (%1%)") % node;

    // Translate the additional nodes for this node
    for (Node* n : all(node->additionalNodes)) {
        translateTopLevelNode(n, context.globalContext());
    }

    // If this node is explained, then translate its explanation
    Node* expl = Nest_explanation(node);
    if (node != expl) {
        return translateNode(expl, context);
    }

    // Check if the node is CT available and we are in RT mode. If so, translate the node into RT
    if (!context.module().isCt() && Feather_isCt(node)) {
        node = convertCtToRt(node, context);
    }

    // Now, depending on the type of the node, do a specific translation

    // Set the correct location when translating a node
    if (context.module().debugInfo())
        context.module().debugInfo()->emitLocation(context.builder(), node->location);

    switch (node->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherNodeList:
        return translateNodeList(node, context);
    case nkRelFeatherLocalSpace:
        return translateLocalSpace(node, context);
    case nkRelFeatherNop:
        return translateNop(node, context);
    case nkRelFeatherTempDestructAction:
        return translateTempDestructAction(node, context);
    case nkRelFeatherScopeDestructAction:
        return translateScopeDestructAction(node, context);
    case nkRelFeatherExpCtValue:
        return translateCtValue(node, context);
    case nkRelFeatherExpMemLoad:
        return translateMemLoad(node, context);
    case nkRelFeatherExpMemStore:
        return translateMemStore(node, context);
    case nkRelFeatherExpVarRef:
        return translateVarRef(node, context);
    case nkRelFeatherExpFieldRef:
        return translateFieldRef(node, context);
    case nkRelFeatherExpFunRef:
        return translateFunRef(node, context);
    case nkRelFeatherExpFunCall:
        return translateFunCall(node, context);
    case nkRelFeatherExpBitcast:
        return translateBitcast(node, context);
    case nkRelFeatherExpConditional:
        return translateConditional(node, context);
    case nkRelFeatherExpNull:
        return translateNull(node, context);
    case nkRelFeatherStmtReturn:
        return translateReturn(node, context);
    case nkRelFeatherStmtIf:
        return translateIf(node, context);
    case nkRelFeatherStmtWhile:
        return translateWhile(node, context);
    case nkRelFeatherStmtBreak:
        return translateBreak(node, context);
    case nkRelFeatherStmtContinue:
        return translateContinue(node, context);
    case nkRelFeatherDeclVar:
        return translateVar(node, context);
    default:
        if (node->nodeKind == nkLLVMDestructActionForConditional)
            return translateDestructActionForConditional(node, context);
        else {
            REP_INTERNAL(node->location,
                    "Don't know how to translate (locally) a node of this kind (%1%)") %
                    Nest_nodeKindName(node);
            return nullptr;
        }
    }
}

llvm::Value* Tr::setValue(TrContext& context, Node& node, llvm::Value* val) {
    context.resultingValues_[&node] = val;
    return val;
}

llvm::Value* Tr::getValue(TrContext& context, Node& node, bool doCheck) {
    auto it = context.resultingValues_.find(&node);
    bool isValid = it != context.resultingValues_.end();
    if (!isValid && doCheck)
        REP_INTERNAL(node.location, "Expected LLVM value for node %1%") % Nest_toStringEx(&node);
    return isValid ? it->second : nullptr;
}
