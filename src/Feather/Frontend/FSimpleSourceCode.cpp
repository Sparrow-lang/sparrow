#include <StdInc.h>
#include "FSimpleSourceCode.h"
#include "Impl/SimpleLexer.h"
#include "Impl/SimpleParser.h"

#include <Nest/Common/Diagnostic.h>

#include <Nest/Common/Alloc.h>
#include <Nest/Frontend/SourceCode.h>
#include <Nest/Frontend/SourceCodeKindRegistrar.h>

#include <fstream>

using namespace Feather;
using namespace Nest;

namespace
{
    string readFileContent(const string& filename)
    {
        using namespace std;

        string result;
        string line;
        ifstream f(filename.c_str());
        if ( !f.is_open() )
            REP_ERROR(NOLOC, "Cannot open input file '%1%'") % filename;
        while ( f.good() )
        {
            getline(f, line);
            result += line;
            result += "\n";
        }
        f.close();


        //cout << result << endl;
        return result;
    }

    void parseSourceCode(SourceCode* sourceCode, Nest::CompilationContext* ctx)
    {
        // Read the file content
        const string& fileContent = readFileContent(sourceCode->url);

        // Create the lexer & parser objects
        delete (SimpleLexer*) sourceCode->additionalData;
        SimpleLexer* lexer = new SimpleLexer(*sourceCode, fileContent.c_str());
        sourceCode->additionalData = lexer;
        SimpleParser parser(*lexer);

        // Parse the file
        sourceCode->mainNode = parser.parse(ctx);
    }

    StringRef getSourceCodeLine(const SourceCode* sourceCode, int lineNo)
    {
        const SimpleLexer* lexer = (SimpleLexer*) sourceCode->additionalData;
        if ( !lexer )
            return StringRef{NULL, NULL};
        string line = lexer->getSourceCodeLine(lineNo);
        StringRef res;
        res.begin = dupString(line.c_str());
        res.end = res.begin + line.size();
        return res;
    }
}

int Feather_kindFSimpleSourceCode = -1;

void Feather_registerFSimpleSourceCode()
{
    Feather_kindFSimpleSourceCode = Nest_registerSourceCodeKind(".fsimple",
        "Simple source file with Feather nodes as function calls", "",
        &parseSourceCode, &getSourceCodeLine, NULL);
}
