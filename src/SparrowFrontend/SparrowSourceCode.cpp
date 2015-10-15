#include <StdInc.h>
#include "SparrowSourceCode.h"
#include "Grammar/Scanner.h"
#include <parser.yy.hpp>
#include <Helpers/SprTypeTraits.h>

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"

#include <fstream>

using namespace SprFrontend;

namespace
{
    void parseSourceCode(SourceCode* sourceCode, CompilationContext* ctx)
    {
        Location loc = Nest_mkLocation1(sourceCode, 1, 1);

        // Open the filename
        ifstream f(sourceCode->url);
        if ( !f )
        {
            REP_ERROR(loc, "Cannot open source file: %1%") % sourceCode->url;
            sourceCode->mainNode = nullptr;
            return;
        }

        Scanner scanner(f, Parser::token::START_PROGRAM);
        Parser parser(scanner, loc, &sourceCode->mainNode);
        int rc = parser.parse();
        if ( rc != 0 )
        {
            return;
        }

        Nest_setContext(sourceCode->mainNode, ctx);
    }

    StringRef getSourceCodeLine(const SourceCode* sourceCode, int lineNo)
    {
        StringRef res {NULL, NULL};
        ifstream f(sourceCode->url);
        if ( !f )
            return res;

        static const int MAXLINE=512;
        char line[MAXLINE];
        for ( int i=0; i<lineNo; ++i )
        {
            f.getline(line, MAXLINE);
            if ( !f.good() )
                return res;
        }
        res.begin = dupString(line);
        res.end = res.begin + strlen(line);
        return res;
    }

    Node* doConvertCtToRt(const SourceCode* /*sourceCode*/, Node* node)
    {
        return convertCtToRt(node);
    }
}

int SprFe_kindSparrowSourceCode = -1;

void SprFe_registerSparrowSourceCode()
{
    SprFe_kindSparrowSourceCode = Nest_registerSourceCodeKind(".spr",
        "Sparrow source file", "",
        &parseSourceCode, &getSourceCodeLine, &doConvertCtToRt);
}

Node* SprFe_parseSparrowExpression(Location loc, const char* code)
{
    // Only use the start part of the location
    loc.end = loc.start;

    istringstream f(code);
    Scanner scanner(f, Parser::token::START_EXPRESSION);
    Node* res;
    Parser parser(scanner, loc, &res);
    int rc = parser.parse();
    if ( rc != 0 )
        REP_ERROR_RET(nullptr, loc, "Cannot parse the expression code");
    return res;
}
