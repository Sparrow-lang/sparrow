#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Utils/StringRef.h"

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
void Nest_compileFile(StringRef filename);

/// Add a source code to be compiled, given the filename
void Nest_addSourceCodeByFilename(const SourceCode* orig, StringRef filename);

/// Add all source code found in the given directory
void Nest_addSourceCodeFromDir(const SourceCode* orig, StringRef dirName);

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


#ifdef __cplusplus
}
#endif
