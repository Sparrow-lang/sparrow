#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/EvalMode.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Api/NodeRange.h"
#include "Nest/Api/Node.h"

typedef struct Nest_Location Nest_Location;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_CompilerModule Nest_CompilerModule;

/// Getter for the Feather module
Nest_CompilerModule* Feather_getModule();

////////////////////////////////////////////////////////////////////////////////
// Feather types
//

// The type kinds for the Feather types
int Feather_getVoidTypeKind();
int Feather_getDataTypeKind();
int Feather_getConstTypeKind();
int Feather_getMutableTypeKind();
int Feather_getTempTypeKind();
int Feather_getArrayTypeKind();
int Feather_getFunctionTypeKind();

/// Returns the Void type corresponding to the given evaluation mode
Nest_TypeRef Feather_getVoidType(EvalMode mode);

/// Returns a type that represents data
/// A data type is introduced by a class definition and can have one or more references; a data type
/// must have a size
Nest_TypeRef Feather_getDataType(Nest_Node* classDecl, unsigned numReferences, EvalMode mode);

Nest_TypeRef Feather_getConstType(Nest_TypeRef base);
Nest_TypeRef Feather_getMutableType(Nest_TypeRef base);
Nest_TypeRef Feather_getTempType(Nest_TypeRef base);

/// Returns a type that holds N instances of a given storage type
Nest_TypeRef Feather_getArrayType(Nest_TypeRef unitType, unsigned count);

/// Returns a type that represents a classic function, with parameters and result type
/// This type can be constructed from a set of parameter types and a result type
/// The first parameter is a pointer to an array with the result type then the types of the
/// parameters
Nest_TypeRef Feather_getFunctionType(
        Nest_TypeRef* resultTypeAndParams, unsigned numTypes, EvalMode mode);

////////////////////////////////////////////////////////////////////////////////
// Feather nodes
//

enum Feather_FeatherNodeKinds {
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
enum Feather_CallConvention {
    ccC,    ///< The default llvm calling convention, compatible with C
    ccFast, ///< This calling convention attempts to make calls as fast as possible (e.g. by passing
            ///< things in registers)
    ccCold, ///< Makes the caller as efficient as possible, under the assumption that the call is
            ///< not commonly executed
    ccGHC,  ///< Calling convention used by the Glasgow Haskell Compiler (GHC) - only registers

    ccX86Std,      ///< stdcall is the calling conventions mostly used by the Win32 API
    ccX86Fast,     ///< 'fast' analog of ccX86Std
    ccX86ThisCall, ///< Similar to X86 StdCall. Passes first argument in ECX, others via stack
    ccARMAPCS,     ///< ARM Procedure Calling Standard calling convention
    ccARMAAPCS,    ///< ARM Architecture Procedure Calling Standard calling convention (aka EABI)
    ccARMAAPCSVFP, ///< Same as ARM_AAPCS, but uses hard floating point ABI
    ccMSP430Intr,  ///< Calling convention used for MSP430 interrupt routines
    ccPTXKernel,   ///< Call to a PTX kernel. Passes all arguments in parameter space
    ccPTXDevice,   ///< Call to a PTX device function. Passes all arguments in register or parameter
                   ///< space
    ccMBLAZEIntr,  ///< Calling convention used for MBlaze interrupt routines
    ccMBLAZESVol,  ///< Calling convention used for MBlaze interrupt support routines
};

typedef enum Feather_CallConvention Feather_CallConvention;
typedef enum Feather_CallConvention CallConvention;

enum Feather_AtomicOrdering {
    atomicNone,           ///< No atomic guarantees
    atomicUnordered,      ///< The set of values that can be read is governed by the happens-before
                          ///< partial order. Can model Java non-volatile shared variables
    atomicMonotonic,      ///< C++0x memory_order_relaxed
    atomicAcquire,        ///< C++0x memory_order_acquire
    atomicRelease,        ///< C++0x memory_order_release
    atomicAcquireRelease, ///< C++0x memory_order_acq_rel
    atomicSeqConsistent,  ///< C++0x memory_order_seq_cst; Java volatile
};

typedef enum Feather_AtomicOrdering Feather_AtomicOrdering;
typedef enum Feather_AtomicOrdering AtomicOrdering;

Nest_Node* Feather_mkNodeList(Nest_Location loc, Nest_NodeRange children);
Nest_Node* Feather_mkNodeListVoid(Nest_Location loc, Nest_NodeRange children);
Nest_Node* Feather_addToNodeList(Nest_Node* prevList, Nest_Node* element);
Nest_Node* Feather_appendNodeList(Nest_Node* list, Nest_Node* newNodes);

Nest_Node* Feather_mkNop(Nest_Location loc);
Nest_Node* Feather_mkTypeNode(Nest_Location loc, Nest_TypeRef type);
Nest_Node* Feather_mkBackendCode(Nest_Location loc, Nest_StringRef code, EvalMode evalMode);
Nest_Node* Feather_mkLocalSpace(Nest_Location loc, Nest_NodeRange children);
Nest_Node* Feather_mkGlobalConstructAction(Nest_Location loc, Nest_Node* action);
Nest_Node* Feather_mkGlobalDestructAction(Nest_Location loc, Nest_Node* action);
Nest_Node* Feather_mkScopeDestructAction(Nest_Location loc, Nest_Node* action);
Nest_Node* Feather_mkTempDestructAction(Nest_Location loc, Nest_Node* action);
Nest_Node* Feather_mkChangeMode(Nest_Location loc, Nest_Node* child, EvalMode mode);

Nest_Node* Feather_mkFunction(Nest_Location loc, Nest_StringRef name, Nest_Node* resType,
        Nest_NodeRange params, Nest_Node* body);
Nest_Node* Feather_mkClass(Nest_Location loc, Nest_StringRef name, Nest_NodeRange fields);
Nest_Node* Feather_mkVar(Nest_Location loc, Nest_StringRef name, Nest_Node* type);

Nest_Node* Feather_mkCtValue(Nest_Location loc, Nest_TypeRef typeNode, Nest_StringRef data);
Nest_Node* Feather_mkNull(Nest_Location loc, Nest_Node* typeNode);
Nest_Node* Feather_mkVarRef(Nest_Location loc, Nest_Node* varDecl);
Nest_Node* Feather_mkFieldRef(Nest_Location loc, Nest_Node* obj, Nest_Node* fieldDecl);
Nest_Node* Feather_mkFunRef(Nest_Location loc, Nest_Node* funDecl, Nest_Node* resType);
Nest_Node* Feather_mkFunCall(Nest_Location loc, Nest_Node* funDecl, Nest_NodeRange args);
Nest_Node* Feather_mkMemLoad(Nest_Location loc, Nest_Node* exp);
Nest_Node* Feather_mkMemStore(Nest_Location loc, Nest_Node* value, Nest_Node* address);
Nest_Node* Feather_mkBitcast(Nest_Location loc, Nest_Node* destType, Nest_Node* exp);
Nest_Node* Feather_mkConditional(
        Nest_Location loc, Nest_Node* condition, Nest_Node* alt1, Nest_Node* alt2);

Nest_Node* Feather_mkIf(
        Nest_Location loc, Nest_Node* condition, Nest_Node* thenClause, Nest_Node* elseClause);
Nest_Node* Feather_mkWhile(
        Nest_Location loc, Nest_Node* condition, Nest_Node* body, Nest_Node* step);
Nest_Node* Feather_mkBreak(Nest_Location loc);
Nest_Node* Feather_mkContinue(Nest_Location loc);
Nest_Node* Feather_mkReturn(Nest_Location loc, Nest_Node* exp);

#ifdef __cplusplus
}
#endif
