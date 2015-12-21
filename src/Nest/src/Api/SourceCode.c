#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Assert.h"

void Nest_parseSourceCode(SourceCode* sourceCode, CompilationContext* ctx)
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

    StringRef null = { 0, 0 };
    return null;
}

Node* Nest_translateCtToRt(const SourceCode* sourceCode, Node* node)
{
    ASSERT(sourceCode);
    FTranslateCtToRt f = Nest_getTranslateCtToRtFun(sourceCode->kind);
    if ( f )
        return f(sourceCode, node);
    return node;
}
