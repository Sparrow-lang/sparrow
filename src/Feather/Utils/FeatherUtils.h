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



StringRef Feather_BackendCode_getCode(const Node* node);
EvalMode Feather_BackendCode_getEvalMode(Node* node);

void Feather_ChangeMode_setChild(Node* node, Node* child);
EvalMode Feather_ChangeMode_getEvalMode(Node* node);

void Feather_Function_addParameter(Node* node, Node* parameter, bool first = false);
void Feather_Function_setResultType(Node* node, Node* resultType);
void Feather_Function_setBody(Node* node, Node* body);
unsigned Feather_Function_numParameters(Node* node);
Node* Feather_Function_getParameter(Node* node, unsigned idx);
TypeRef Feather_Function_resultType(Node* node);
Node* Feather_Function_body(Node* node);
CallConvention Feather_Function_callConvention(Node* node);


/// Getter for the class that introduces this data type - can be null
/// Works for only for storage types (data, l-value, array)
Node* Feather_classDecl(TypeRef type);

/// If the class associated with the given type has an associated name this will return it; otherwise it returns nullptr
/// Works for only for storage types (data, l-value, array)
const StringRef* Feather_nativeName(TypeRef type);

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
// vector<TypeRef> Feather_getFunParameters(TypeRef type);
TypeRef Feather_getFunResultType(TypeRef type);

////////////////////////////////////////////////////////////////////////////////
// Context

Node* Feather_getParentFun(CompilationContext* context);
Node* Feather_getParentClass(CompilationContext* context);
Node* Feather_getParentLoop(CompilationContext* context);


#ifdef __cplusplus
}
#endif

