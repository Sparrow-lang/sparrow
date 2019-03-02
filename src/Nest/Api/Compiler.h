#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"

typedef struct Nest_CompilerSettings Nest_CompilerSettings;
typedef struct Nest_Node Nest_Node;
typedef struct Nest_SourceCode Nest_SourceCode;
typedef struct Nest_Backend Nest_Backend;
typedef struct Nest_CompilationContext Nest_CompilationContext;

#ifdef __cplusplus
extern "C" {
#endif

/// Called to initialize the compiler
void Nest_compilerInit();

//! Called to de-initialize the compiler
void Nest_compilerDestroy();

/// Getter for the compiler settings
Nest_CompilerSettings* Nest_compilerSettings();

/// The root compilation context that contains all the code compiled
Nest_CompilationContext* Nest_getRootContext();

/// The backend object used to generate the resulting code
Nest_Backend* Nest_getCurBackend();

/// Creates the backend
void Nest_createBackend(const char* mainFilename);

/// Parse, check dependencies and the compile the given filename
Nest_SourceCode* Nest_compileFile(Nest_StringRef filename);

/// Add a source code to be compiled, given the filename
Nest_SourceCode* Nest_addSourceCodeByFilename(const Nest_SourceCode* orig, Nest_StringRef filename);

/// Get the SourceCode corresponding to the given filename
const Nest_SourceCode* Nest_getSourceCodeForFilename(Nest_StringRef filename);

/// Add the given node to be queued for semantic check
void Nest_queueSemanticCheck(Nest_Node* node);

/// Make sure that all the queued nodes are semantically checked
void Nest_semanticCheckQueuedNodes();

/// Call this to CT process the given node
void Nest_ctProcess(Nest_Node* node);

/// Call this to CT evaluate the given node
Nest_Node* Nest_ctEval(Nest_Node* node);

/// Get the size of the given type
unsigned Nest_sizeOf(Nest_TypeRef type);

/// Get the alignment of the given type
unsigned Nest_alignmentOf(Nest_TypeRef type);

// Callbacks

typedef void (*FSourceCodeCallback)(Nest_SourceCode* sc);

void Nest_registerSourceCodeCreatedCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeParsedCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeCompiledCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeCodeGenCallback(FSourceCodeCallback callback);

#ifdef __cplusplus
}
#endif
