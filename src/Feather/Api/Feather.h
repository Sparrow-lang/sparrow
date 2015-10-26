#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/EvalMode.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Api/NodeRange.h"

typedef struct Nest_Location Location;

#ifdef __cplusplus
extern "C" {
#endif


typedef struct Nest_CompilerModule CompilerModule;

/// Getter for the Feather module
CompilerModule* Feather_getModule();


////////////////////////////////////////////////////////////////////////////////
// Feather types
//

// The type kinds for the Feather types
int Feather_getVoidTypeKind();
int Feather_getDataTypeKind();
int Feather_getLValueTypeKind();
int Feather_getArrayTypeKind();
int Feather_getFunctionTypeKind();

/// Returns the Void type corresponding to the given evaluation mode
TypeRef Feather_getVoidType(EvalMode mode);

/// Returns a type that represents data
/// A data type is introduced by a class definition and can have one or more references; a data type must have a size
TypeRef Feather_getDataType(Node* classDecl, uint8_t numReferences, EvalMode mode);

/// Returns an L-Value type
/// This type would represents an l-value to an existing storage type
TypeRef Feather_getLValueType(TypeRef base);

/// Returns a type that holds N instances of a given storage type
TypeRef Feather_getArrayType(TypeRef unitType, uint32_t count);

/// Returns a type that represents a classic function, with parameters and result type
/// This type can be constructed from a set of parameter types and a result type
/// The first parameter is a pointer to an array with the result type then the types of the parameters
TypeRef Feather_getFunctionType(TypeRef* resultTypeAndParams, size_t numTypes, EvalMode mode);


////////////////////////////////////////////////////////////////////////////////
// Feather nodes
//

enum Feather_FeatherNodeKinds
{
    nkRelFeatherNop = 0,
    nkRelFeatherTypeNode,
    nkRelFeatherBackendCode,
    nkRelFeatherNodeList,
    nkRelFeatherLocalSpace,
    nkRelFeatherGlobalConstructAction,
    nkRelFeatherGlobalDestructAction,
    nkRelFeatherScopeDestructAction,
    nkRelFeatherTempDestructAction,
    nkRelFeatherChangeMode,
    
    nkRelFeatherDeclFunction,
    nkRelFeatherDeclClass,
    nkRelFeatherDeclVar,
    
    nkRelFeatherExpCtValue,
    nkRelFeatherExpNull,
    nkRelFeatherExpVarRef,
    nkRelFeatherExpFieldRef,
    nkRelFeatherExpFunRef,
    nkRelFeatherExpFunCall,
    nkRelFeatherExpMemLoad,
    nkRelFeatherExpMemStore,
    nkRelFeatherExpBitcast,
    nkRelFeatherExpConditional,
    
    nkRelFeatherStmtIf,
    nkRelFeatherStmtWhile,
    nkRelFeatherStmtBreak,
    nkRelFeatherStmtContinue,
    nkRelFeatherStmtReturn,
};

typedef enum Feather_FeatherNodeKinds Feather_FeatherNodeKinds;
typedef enum Feather_FeatherNodeKinds FeatherNodeKinds;

int Feather_getFirstFeatherNodeKind();

/// The possible calling conventions
enum Feather_CallConvention
{
    ccC,        ///< The default llvm calling convention, compatible with C
    ccFast,     ///< This calling convention attempts to make calls as fast as possible (e.g. by passing things in registers)
    ccCold,     ///< Makes the caller as efficient as possible, under the assumption that the call is not commonly executed
    ccGHC,      ///< Calling convention used by the Glasgow Haskell Compiler (GHC) - only registers

    ccX86Std,       ///< stdcall is the calling conventions mostly used by the Win32 API
    ccX86Fast,      ///< 'fast' analog of ccX86Std
    ccX86ThisCall,  ///< Similar to X86 StdCall. Passes first argument in ECX, others via stack
    ccARMAPCS,      ///< ARM Procedure Calling Standard calling convention
    ccARMAAPCS,     ///< ARM Architecture Procedure Calling Standard calling convention (aka EABI)
    ccARMAAPCSVFP,  ///< Same as ARM_AAPCS, but uses hard floating point ABI
    ccMSP430Intr,   ///< Calling convention used for MSP430 interrupt routines
    ccPTXKernel,    ///< Call to a PTX kernel. Passes all arguments in parameter space
    ccPTXDevice,    ///< Call to a PTX device function. Passes all arguments in register or parameter space
    ccMBLAZEIntr,   ///< Calling convention used for MBlaze interrupt routines
    ccMBLAZESVol,   ///< Calling convention used for MBlaze interrupt support routines
};

typedef enum Feather_CallConvention Feather_CallConvention;
typedef enum Feather_CallConvention CallConvention;


enum Feather_AtomicOrdering
{
    atomicNone,             ///< No atomic guarantees
    atomicUnordered,        ///< The set of values that can be read is governed by the happens-before partial order. Can model Java non-volatile shared variables
    atomicMonotonic,        ///< C++0x memory_order_relaxed
    atomicAcquire,          ///< C++0x memory_order_acquire
    atomicRelease,          ///< C++0x memory_order_release
    atomicAcquireRelease,   ///< C++0x memory_order_acq_rel
    atomicSeqConsistent,    ///< C++0x memory_order_seq_cst; Java volatile
};

typedef enum Feather_AtomicOrdering Feather_AtomicOrdering;
typedef enum Feather_AtomicOrdering AtomicOrdering;


Node* Feather_mkNodeList(const Location& loc, NodeRange children, bool voidResult = false);
Node* Feather_addToNodeList(Node* prevList, Node* element);
Node* Feather_appendNodeList(Node* list, Node* newNodes);

Node* Feather_mkNop(const Location& loc);
Node* Feather_mkTypeNode(const Location& loc, TypeRef type);
Node* Feather_mkBackendCode(const Location& loc, StringRef code, EvalMode evalMode = modeRt);
Node* Feather_mkLocalSpace(const Location& loc, NodeRange children);
Node* Feather_mkGlobalConstructAction(const Location& loc, Node* action);
Node* Feather_mkGlobalDestructAction(const Location& loc, Node* action);
Node* Feather_mkScopeDestructAction(const Location& loc, Node* action);
Node* Feather_mkTempDestructAction(const Location& loc, Node* action);
Node* Feather_mkChangeMode(const Location& loc, Node* child, EvalMode mode);

Node* Feather_mkFunction(const Location& loc, StringRef name, Node* resType, NodeRange params, Node* body, CallConvention callConv = ccC, EvalMode evalMode = modeUnspecified);
Node* Feather_mkClass(const Location& loc, StringRef name, NodeRange fields, EvalMode evalMode = modeUnspecified);
Node* Feather_mkVar(const Location& loc, StringRef name, Node* type, size_t alignment = 0, EvalMode evalMode = modeUnspecified);

Node* Feather_mkCtValue(const Location& loc, TypeRef typeNode, StringRef data);
Node* Feather_mkNull(const Location& loc, Node* typeNode);
Node* Feather_mkVarRef(const Location& loc, Node* varDecl);
Node* Feather_mkFieldRef(const Location& loc, Node* obj, Node* fieldDecl);
Node* Feather_mkFunRef(const Location& loc, Node* funDecl, Node* resType);
Node* Feather_mkFunCall(const Location& loc, Node* funDecl, NodeRange args);
Node* Feather_mkMemLoad(const Location& loc, Node* exp, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
Node* Feather_mkMemStore(const Location& loc, Node* value, Node* address, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
Node* Feather_mkBitcast(const Location& loc, Node* destType, Node* exp);
Node* Feather_mkConditional(const Location& loc, Node* condition, Node* alt1, Node* alt2);

Node* Feather_mkIf(const Location& loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt = false);
Node* Feather_mkWhile(const Location& loc, Node* condition, Node* body, Node* step, bool isCt = false);
Node* Feather_mkBreak(const Location& loc);
Node* Feather_mkContinue(const Location& loc);
Node* Feather_mkReturn(const Location& loc, Node* exp = nullptr);

#ifdef __cplusplus
}
#endif
