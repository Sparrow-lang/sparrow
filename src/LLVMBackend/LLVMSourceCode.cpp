#include <StdInc.h>
#include "LLVMSourceCode.h"

#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Common/Alloc.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/EvalMode.h>
#include <Nest/Intermediate/Node.h>

#include <Nest/Frontend/SourceCode.h>
#include <Nest/Frontend/SourceCodeKindRegistrar.h>

#include <fstream>

namespace
{
    string readFile(const string& filename)
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

            // Check for line comments
            size_t idx = line.find("//");
            if ( idx != string::npos )
            {
                // Cut the line until the end
                line.erase(idx);
            }

            result += line;
            result += "\n";
        }
        f.close();

        // Now try to remove the multi-line comments
        size_t idx = 0;
        while ( idx != string::npos )
        {
            size_t idxStart = result.find("/*", idx);
            if ( idxStart == string::npos )
                break;
            size_t idxEnd = result.find("*/", idxStart);

            if ( idxEnd != string::npos )
                result.erase(idxStart, idxEnd-idxStart+2);
            else
                result.erase(idxStart);
            idx = idxStart;
        }
        return result;
    }

    EvalMode specifiedCtAvailability(const string& fileContent)
    {
        using namespace Nest;

        // Check for a comment on the first line
        size_t idxEnd = fileContent.find('\n');
        if ( idxEnd == string::npos )
            idxEnd = fileContent.size();
        string firstLine = fileContent.substr(0, idxEnd);
        size_t idxCommentStart = firstLine.find(';');
        if ( idxCommentStart == string::npos )
            return modeRt;

        // Search for ct modifiers in the comment
        if ( string::npos != firstLine.find("rtct", idxCommentStart) )
            return modeRtCt;
        if ( string::npos != firstLine.find("ct", idxCommentStart) )
            return modeCt;
        if ( string::npos != firstLine.find("rt", idxCommentStart) )
            return modeRt;
        return modeRt;
    }

    void parseSourceCode(SourceCode* sourceCode, CompilationContext* ctx)
    {
        // Read the LLVM content
        const string& fileContent = readFile(sourceCode->url);

        // Create a backend code with the given content
        EvalMode evalMode = specifiedCtAvailability(fileContent);
        sourceCode->mainNode = Feather::mkBackendCode(Nest_mkLocation1(sourceCode, 1, 1), fileContent, evalMode);
        Nest::setContext(sourceCode->mainNode, ctx);
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
}

int LLVMBe_kindLLVMSourceCode = -1;

void LLVMBe_registerLLVMSourceCode()
{
    LLVMBe_kindLLVMSourceCode = Nest_registerSourceCodeKind(".llvm",
        "LLVM source code", "",
        &parseSourceCode, &getSourceCodeLine, NULL);
}
