#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"

typedef struct Nest_CompilerSettings CompilerSettings;
typedef struct Nest_Node Node;
typedef struct Nest_SourceCode SourceCode;
typedef struct Nest_Backend Backend;
typedef struct Nest_CompilationContext CompilationContext;

#ifdef __cplusplus
extern "C" {
#endif

/// Called to initialize the compiler
void Nest_compilerInit();

/// Getter for the compiler settings
CompilerSettings* Nest_compilerSettings();

/// The root compilation context that contains all the code compiled
CompilationContext* Nest_getRootContext();

/// The backend object used to generate the resulting code
Backend* Nest_getCurBackend();


/// Creates the backend
void Nest_createBackend(const char* mainFilename);

/// Parse, check dependencies and the compile the given filename
SourceCode* Nest_compileFile(StringRef filename);

/// Add a source code to be compiled, given the filename
SourceCode* Nest_addSourceCodeByFilename(const SourceCode* orig, StringRef filename);

/// Get the SourceCode corresponding to the given filename
const SourceCode* Nest_getSourceCodeForFilename(StringRef filename);

/// Add the given node to be queued for semantic check
void Nest_queueSemanticCheck(Node* node);

/// Call this to CT process the given node
void Nest_ctProcess(Node* node);

/// Call this to CT evaluate the given node
Node* Nest_ctEval(Node* node);

/// Get the size of the given type
unsigned Nest_sizeOf(TypeRef type);

/// Get the alignment of the given type
unsigned Nest_alignmentOf(TypeRef type);



// Callbacks

typedef void (*FSourceCodeCallback)(SourceCode* sc);

void Nest_registerSourceCodeCreatedCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeParsedCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeCompiledCallback(FSourceCodeCallback callback);
void Nest_registerSourceCodeCodeGenCallback(FSourceCodeCallback callback);


#ifdef __cplusplus
}
#endif
