#pragma once

#include "Nest/Api/NodeArray.h"
#include "Nest/Api/StringRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_StringRef StringRef;
typedef struct Nest_CompilationContext CompilationContext;
typedef struct Nest_Node Node;

struct Nest_SourceCode {
    int kind;               ///< The kind of source code (parser)
    const char* url;        ///< The location of the source code
    Node* mainNode;         ///< The main node generated from this source code
};
typedef struct Nest_SourceCode Nest_SourceCode;
typedef struct Nest_SourceCode SourceCode;

/// Function that is capable to parse the given source code
void Nest_parseSourceCode(SourceCode* sourceCode, CompilationContext* ctx);

/// Function that gets a specific line from the source code
StringRef Nest_getSourceCodeLine(const SourceCode* sourceCode, int lineNum);

/// Function that translates a node from CT to RT for the given source code
Node* Nest_translateCtToRt(const SourceCode* sourceCode, Node* node);

#ifdef __cplusplus
}
#endif
