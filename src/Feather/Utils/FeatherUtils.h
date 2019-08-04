#pragma once

#include "Feather/Api/Feather.h"

#include "Nest/Api/EvalMode.h"

typedef struct Nest_CompilationContext Nest_CompilationContext;

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
extern int typeKindConst;
extern int typeKindMutable;
extern int typeKindTemp;
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
extern const char* propTypeCat;

/// Getter for the class that introduces this data type - can be null
/// Works for only for storage types (data-like and array)
Nest_Node* Feather_classDecl(Nest_TypeRef type);

/// If the class associated with the given type has an associated name this will return it;
/// otherwise it returns nullptr Works for only for storage types (data-like, array)
Nest_StringRef Feather_nativeName(Nest_TypeRef type);

/// Returns the base type of this type
/// Works for category and array types
Nest_TypeRef Feather_baseType(Nest_TypeRef type);

/// Getter for the number of units in the buffer type
/// Works only for array types
int Feather_getArraySize(Nest_TypeRef type);

unsigned Feather_numFunParameters(Nest_TypeRef type);
Nest_TypeRef Feather_getFunParameter(Nest_TypeRef type, unsigned idx);
Nest_TypeRef Feather_getFunResultType(Nest_TypeRef type);

////////////////////////////////////////////////////////////////////////////////
// Context

Nest_Node* Feather_getParentFun(Nest_CompilationContext* context);
Nest_Node* Feather_getParentClass(Nest_CompilationContext* context);
Nest_Node* Feather_getParentLoop(Nest_CompilationContext* context);

////////////////////////////////////////////////////////////////////////////////
// Decl

/// Get the name of the given declaration
/// Throws if the name was not set for the node
Nest_StringRef Feather_getName(Nest_Node* decl);

/// Tests if the given node has a node set
int Feather_hasName(Nest_Node* decl);

/// Setter for the name of a declaration
void Feather_setName(Nest_Node* decl, Nest_StringRef name);

/// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
EvalMode Feather_nodeEvalMode(Nest_Node* decl);

/// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
/// If the given declaration doesn't have an evaluation mode, the evaluation mode of the context is
/// used
EvalMode Feather_effectiveEvalMode(Nest_Node* decl);

/// Setter for the evaluation mode of the given declaration
void Feather_setEvalMode(Nest_Node* decl, EvalMode val);

/// Add the given declaration to the sym tab
void Feather_addToSymTab(Nest_Node* decl);

/// Should we add the given declaration to the symbols table?
void Feather_setShouldAddToSymTab(Nest_Node* decl, int shouldAdd);

////////////////////////////////////////////////////////////////////////////////
// TypeTraits

/// Tests if the given node is available at CT
int Feather_isCt(Nest_Node* node);

/// Tests if the given node has a "Testable" type
int Feather_isTestable(Nest_Node* node);

//! Combines two modes together.
//! If one of the modes is modeCt, the result will be modeCt
//! If one of the given modes is unspecified, we return the other one
//! I.e., use this when placing one node into a given context
EvalMode Feather_combineMode(EvalMode mode1, EvalMode mode2);

//! Combine modes of two children together.
//! If one of the modes is modeRt, it will return modeRt.
//! If one of the given modes is unspecified, we return the other one.
//! I.e., use this when deducing the modes based on the modes of the children
EvalMode Feather_combineModeBottom(EvalMode mode1, EvalMode mode2);

//! Adjust the mode of the type, to be compatible with the one in the context.
//! I.e., if the context is CT, the type will be made CT.
Nest_TypeRef Feather_adjustMode(
        Nest_TypeRef srcType, Nest_CompilationContext* context, Nest_Location loc);

//! Check if the given node has the eval-mode correctly set.
//! That is, we don't have a RT node in a CT context, and we don't have RT children to a CT node.
void Feather_checkEvalMode(Nest_Node* src);
//! Same as Feather_checkEvalMode, but also checks if node's mode is compatible the given mode.
//! If expectedMode is CT, then the node mode needs to also be CT
void Feather_checkEvalModeWithExpected(Nest_Node* src, EvalMode expectedMode);

#ifdef __cplusplus
}
#endif
