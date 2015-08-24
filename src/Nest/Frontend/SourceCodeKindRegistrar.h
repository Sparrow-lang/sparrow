#pragma once

#include <Nest/Common/StringRef.h>

#include <functional>

typedef struct Nest_Node Node;
typedef struct Nest_SourceCode SourceCode;
typedef struct Nest_CompilationContext CompilationContext;


/// Function that is capable to parse the given source code
typedef void (*FParseSourceCode)(SourceCode*, CompilationContext*);
/// Function that gets a specific line from the source code
typedef StringRef (*FGetSourceCodeLine)(const SourceCode*, int);
/// Function that translates a node from CT to RT for the given source code
typedef Node* (*FTranslateCtToRt)(const SourceCode*, Node*);

/// Registers a new sourceCode kind
///
/// @return the ID of the new sourceCode kind
int Nest_registerSourceCodeKind(
        const char* extension,
        const char* description,
        const char* extraInfo,
        FParseSourceCode funParseSourceCode,
        FGetSourceCodeLine funGetSourceCodeLine = NULL,
        FTranslateCtToRt funTranslateCtToRt = NULL);

const char* Nest_getSourceCodeExtension(int sourceCodeKind);
const char* Nest_getSourceCodeDescription(int sourceCodeKind);
const char* Nest_getSourceCodeExtraInfo(int sourceCodeKind);
FParseSourceCode Nest_getParseSourceCodeFun(int sourceCodeKind);
FGetSourceCodeLine Nest_getGetSourceCodeLineFun(int sourceCodeKind);
FTranslateCtToRt Nest_getTranslateCtToRtFun(int sourceCodeKind);

/// Find the source code kind for the given extension
/// Returns a negative value if no source code kind can be found
int Nest_getSourceCodeKindForExtension(const char* extension);

/// Find the source code kind for the given filename
/// Returns a negative value if no source code kind can be found
int Nest_getSourceCodeKindForFilename(const char* filename);
