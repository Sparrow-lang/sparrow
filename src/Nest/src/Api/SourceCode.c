#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Assert.h"

void Nest_parseSourceCode(Nest_SourceCode* sourceCode, Nest_CompilationContext* ctx) {
    ASSERT(sourceCode);
    FParseSourceCode f = Nest_getParseSourceCodeFun(sourceCode->kind);
    if (f)
        f(sourceCode, ctx);
}

Nest_StringRef Nest_getSourceCodeLine(const Nest_SourceCode* sourceCode, int lineNum) {
    ASSERT(sourceCode);
    FGetSourceCodeLine f = Nest_getGetSourceCodeLineFun(sourceCode->kind);
    if (f)
        return f(sourceCode, lineNum);

    Nest_StringRef null = {0, 0};
    return null;
}

Nest_Node* Nest_translateCtToRt(const Nest_SourceCode* sourceCode, Nest_Node* node) {
    ASSERT(sourceCode);
    FTranslateCtToRt f = Nest_getTranslateCtToRtFun(sourceCode->kind);
    if (f)
        return f(sourceCode, node);
    return node;
}
