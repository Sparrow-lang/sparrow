#pragma once

#include "Nest/Api/NodeArray.h"
#include "Nest/Api/StringRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_StringRef Nest_StringRef;
typedef struct Nest_CompilationContext Nest_CompilationContext;
typedef struct Nest_Node Nest_Node;

struct Nest_SourceCode {
    int kind;            ///< The kind of source code (parser)
    const char* url;     ///< The location of the source code
    Nest_Node* mainNode; ///< The main node generated from this source code
};
typedef struct Nest_SourceCode Nest_SourceCode;

/// Function that is capable to parse the given source code
void Nest_parseSourceCode(Nest_SourceCode* sourceCode, Nest_CompilationContext* ctx);

/// Function that gets a specific line from the source code
Nest_StringRef Nest_getSourceCodeLine(const Nest_SourceCode* sourceCode, int lineNum);

/// Function that translates a node from CT to RT for the given source code
Nest_Node* Nest_translateCtToRt(const Nest_SourceCode* sourceCode, Nest_Node* node);

#ifdef __cplusplus
}
#endif
