#include <StdInc.h>
#include "SparrowSourceCode.h"
#include "Grammar/Parser.h"
#include <Helpers/SprTypeTraits.h>

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/Profiling.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"

#include <fstream>

using namespace SprFrontend;

namespace {
void parseSourceCode(Nest_SourceCode* sourceCode, CompilationContext* ctx) {
    Location loc = Nest_mkLocation1(sourceCode, 1, 1);

    {
        PROFILING_ZONE_NAMED("parseModule");
        Parser parser(loc);
        sourceCode->mainNode = parser.parseModule();
    }

    if (sourceCode->mainNode)
        Nest_setContext(sourceCode->mainNode, ctx);
}

Nest_StringRef getSourceCodeLine(const Nest_SourceCode* sourceCode, int lineNo) {
    StringRef res{nullptr, nullptr};
    ifstream f(sourceCode->url);
    if (!f)
        return res;

    static const int MAXLINE = 512;
    char line[MAXLINE];
    for (int i = 0; i < lineNo; ++i) {
        f.getline(line, MAXLINE);
        if (!f.good())
            return res;
    }
    res.begin = dupString(line);
    res.end = res.begin + strlen(line);
    return res;
}

Node* doConvertCtToRt(const Nest_SourceCode* /*sourceCode*/, Node* node) {
    return convertCtToRt(node);
}
} // namespace

int SprFe_kindSparrowSourceCode = -1;

void SprFe_registerSparrowSourceCode() {
    SprFe_kindSparrowSourceCode = Nest_registerSourceCodeKind(".spr", "Sparrow source file", "",
            &parseSourceCode, &getSourceCodeLine, &doConvertCtToRt);
}

Node* SprFe_parseSparrowExpression(Location loc, const char* code) {
    PROFILING_ZONE();

    // Only use the start part of the location
    loc.end = loc.start;

    StringRef toParse = {code, code + strlen(code)};

    Parser parser(loc, toParse);
    Node* res = parser.parseExpression();
    if (!res)
        REP_ERROR(loc, "Cannot parse the expression code");
    return res;
}
