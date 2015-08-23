#pragma once

#include <Nest/Intermediate/NodeVector.h>

FWD_CLASS1(Nest, CompilationContext);

typedef struct Nest_StringRef StringRef;

struct Nest_SourceCode {
    int kind;               ///< The kind of source code (parser)
    const char* url;        ///< The location of the source code
    Nest::Node* mainNode;   ///< The main node generated from this source code
    Nest::NodeVector additionalNodes; ///< Additional nodes generated for this source code
    void* additionalData;   ///< Additional data used bythe parser of the source code
};
typedef struct Nest_SourceCode Nest_SourceCode;
typedef struct Nest_SourceCode SourceCode;

/// Function that is capable to parse the given source code
void Nest_parseSourceCode(SourceCode* sourceCode, Nest::CompilationContext* ctx);

/// Function that gets a specific line from the source code
StringRef Nest_getSourceCodeLine(const SourceCode* sourceCode, int lineNum);

/// Function that translates a node from CT to RT for the given source code
Nest::Node* Nest_translateCtToRt(const SourceCode* sourceCode, Nest::Node* node);
