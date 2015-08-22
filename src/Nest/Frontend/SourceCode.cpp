#include <StdInc.h>
#include "SourceCode.h"
#include "SourceCodeKindRegistrar.h"
#include "Common/Alloc.h"

using namespace Nest;

void Nest_parseSourceCode(SourceCode* sourceCode, Nest::CompilationContext* ctx)
{
    ASSERT(sourceCode);
    FParseSourceCode f = Nest_getParseSourceCodeFun(sourceCode->kind);
    if ( f )
        f(sourceCode, ctx);
}

StringRef Nest_getSourceCodeLine(const SourceCode* sourceCode, int lineNum)
{
    ASSERT(sourceCode);
    FGetSourceCodeLine f = Nest_getGetSourceCodeLineFun(sourceCode->kind);
    if ( f )
        return f(sourceCode, lineNum);

    StringRef null = { NULL, NULL };
    return null;
}

Nest::Node* Nest_translateCtToRt(const SourceCode* sourceCode, Nest::Node* node)
{
    ASSERT(sourceCode);
    FTranslateCtToRt f = Nest_getTranslateCtToRtFun(sourceCode->kind);
    if ( f )
        return f(sourceCode, node);
    return node;
}


// DynSourceCode::DynSourceCode(const string& filename)
//     : sourceCode_{ -1, dupString(filename.c_str()), nullptr, NodeVector(), nullptr }
// {
// }

// DynSourceCode::~DynSourceCode()
// {
// }

// Node* DynSourceCode::translateCtToRt(Node* node) const
// {
//     return node;
// }
