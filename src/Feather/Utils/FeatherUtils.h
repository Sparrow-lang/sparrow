#pragma once

#include "Feather/Api/Feather.h"

#include "Nest/Api/EvalMode.h"

typedef struct Nest_CompilationContext CompilationContext;

#ifdef __cplusplus
extern "C" {
#endif

// The IDs for all the feather node kinds
extern int nkFeatherNop;
extern int nkFeatherTypeNode;
extern int nkFeatherBackendCode;
extern int nkFeatherNodeList;
extern int nkFeatherLocalSpace;
extern int nkFeatherGlobalConstructAction;
extern int nkFeatherGlobalDestructAction;
extern int nkFeatherScopeDestructAction;
extern int nkFeatherTempDestructAction;
extern int nkFeatherChangeMode;

extern int nkFeatherDeclFunction;
extern int nkFeatherDeclClass;
extern int nkFeatherDeclVar;

extern int nkFeatherExpCtValue;
extern int nkFeatherExpNull;
extern int nkFeatherExpVarRef;
extern int nkFeatherExpFieldRef;
extern int nkFeatherExpFunRef;
extern int nkFeatherExpFunCall;
extern int nkFeatherExpMemLoad;
extern int nkFeatherExpMemStore;
extern int nkFeatherExpBitcast;
extern int nkFeatherExpConditional;

extern int nkFeatherStmtIf;
extern int nkFeatherStmtWhile;
extern int nkFeatherStmtBreak;
extern int nkFeatherStmtContinue;
extern int nkFeatherStmtReturn;

// The IDs for all the feather type kinds
extern int typeKindVoid;
extern int typeKindData;
extern int typeKindLValue;
extern int typeKindArray;
extern int typeKindFunction;

// Common feather property names
extern const char* propNativeName;
extern const char* propDescription;
extern const char* propResultParam;
extern const char* propAutoCt;
extern const char* propCode;
extern const char* propEvalMode;
extern const char* propResultingDecl;
extern const char* propNoInline;
extern const char* propEmptyBody; ///< Node will not generate meaningful instructions

StringRef Feather_BackendCode_getCode(const Node* node);
EvalMode Feather_BackendCode_getEvalMode(Node* node);

void Feather_ChangeMode_setChild(Node* node, Node* child);
EvalMode Feather_ChangeMode_getEvalMode(Node* node);

void Feather_Function_addParameter(Node* node, Node* parameter);
void Feather_Function_addParameterFirst(Node* node, Node* parameter);
void Feather_Function_setResultType(Node* node, Node* resultType);
void Feather_Function_setBody(Node* node, Node* body);
unsigned Feather_Function_numParameters(Node* node);
Node* Feather_Function_getParameter(Node* node, unsigned idx);
NodeRange Feather_Function_getParameters(Node* node);
TypeRef Feather_Function_resultType(Node* node);
Node* Feather_Function_body(Node* node);
CallConvention Feather_Function_callConvention(Node* node);

/// Getter for the class that introduces this data type - can be null
/// Works for only for storage types (data, l-value, array)
Node* Feather_classDecl(TypeRef type);

/// If the class associated with the given type has an associated name this will return it;
/// otherwise it returns nullptr Works for only for storage types (data, l-value, array)
StringRef Feather_nativeName(TypeRef type);

/// The number of references applied
/// Works for only for storage types (data, l-value, array)
int Feather_numReferences(TypeRef type);

/// Returns the base type of this type
/// Works for l-value and array types
TypeRef Feather_baseType(TypeRef type);

/// Getter for the number of units in the buffer type
/// Works only for array types
int Feather_getArraySize(TypeRef type);

unsigned Feather_numFunParameters(TypeRef type);
TypeRef Feather_getFunParameter(TypeRef type, unsigned idx);
TypeRef Feather_getFunResultType(TypeRef type);

////////////////////////////////////////////////////////////////////////////////
// Context

Node* Feather_getParentFun(CompilationContext* context);
Node* Feather_getParentClass(CompilationContext* context);
Node* Feather_getParentLoop(CompilationContext* context);

////////////////////////////////////////////////////////////////////////////////
// Decl

/// Get the name of the given declaration
/// Throws if the name was not set for the node
StringRef Feather_getName(const Node* decl);

/// Tests if the given node has a node set
int Feather_hasName(const Node* decl);

/// Setter for the name of a declaration
void Feather_setName(Node* decl, StringRef name);

/// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
EvalMode Feather_nodeEvalMode(const Node* decl);

/// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
/// If the given declaration doesn't have an evaluation mode, the evaluation mode of the context is
/// used
EvalMode Feather_effectiveEvalMode(const Node* decl);

/// Setter for the evaluation mode of the given declaration
void Feather_setEvalMode(Node* decl, EvalMode val);

/// Add the given declaration to the sym tab
void Feather_addToSymTab(Node* decl);

/// Should we add the given declaration to the symbols table?
void Feather_setShouldAddToSymTab(Node* decl, int shouldAdd);

////////////////////////////////////////////////////////////////////////////////
// TypeTraits

/// Tests if the given node is available at CT
int Feather_isCt(Node* node);

/// Tests if the given node has a "Testable" type
int Feather_isTestable(Node* node);

/// Checks if the given type is a basic numeric type (bool, integral, floating point, char)
int Feather_isBasicNumericType(TypeRef type);

/// Creates a new type from the original type, and change its mode
TypeRef Feather_checkChangeTypeMode(TypeRef type, EvalMode mode, Location loc);

/// Add a reference to the given type
TypeRef Feather_addRef(TypeRef type);

/// Remove a reference to the given type
TypeRef Feather_removeRef(TypeRef type);

/// Remove all the references from the given type
TypeRef Feather_removeAllRef(TypeRef type);

/// Remove the lvalue from a type - the number of references reported will also decrease
TypeRef Feather_removeLValue(TypeRef type);
TypeRef Feather_removeLValueIfPresent(TypeRef type);

/// Transforms a lvalue into a reference - the reported number of references will remain the same
TypeRef Feather_lvalueToRef(TypeRef type);
TypeRef Feather_lvalueToRefIfPresent(TypeRef type);

/// Gets the class declaration for the given type; if it doesn't have one, return null
Node* Feather_classForType(TypeRef t);

/// Check the types are the same, but also consider the operation mode
int Feather_isSameTypeIgnoreMode(TypeRef t1, TypeRef t2);

// Combines two modes together; raises error if the modes cannot be combined
EvalMode Feather_combineMode(EvalMode mode, EvalMode baseMode, Location loc);
EvalMode Feather_combineModeForceBase(EvalMode mode, EvalMode baseMode, Location loc);

/// Adjust the mode of the type, to match the evaluation mode of the compilation context
TypeRef Feather_adjustMode(TypeRef srcType, CompilationContext* context, Location loc);
/// Adjust the mode of the type, to match the evaluation mode of the compilation context; takes in
/// account a 'baseMode' that is the means trough which we can actually access the given type
TypeRef Feather_adjustModeBase(
        TypeRef srcType, EvalMode baseMode, CompilationContext* context, Location loc);

/// Check if the given node has the eval-mode correctly set
void Feather_checkEvalMode(Node* src, EvalMode referencedEvalMode);

#ifdef __cplusplus
}
#endif
