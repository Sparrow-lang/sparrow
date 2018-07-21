#include <StdInc.h>
#include "LLVMSourceCode.h"

#include "Feather/Api/Feather.h"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Api/EvalMode.h"
#include "Nest/Api/Node.h"

#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"

#include <fstream>

using Nest::CompilationContext;
using Nest::StringRef;


namespace {
string readFile(const string& filename) {
    using namespace std;

    string result;
    string line;
    ifstream f(filename.c_str());
    if (!f.is_open()) {
        REP_ERROR(NOLOC, "Cannot open input file '%1%'") % filename;
        return string();
    }
    while (f.good()) {
        getline(f, line);

        // Check for line comments
        size_t idx = line.find("//");
        if (idx != string::npos) {
            // Cut the line until the end
            line.erase(idx);
        }

        result += line;
        result += "\n";
    }
    f.close();

    // Now try to remove the multi-line comments
    size_t idx = 0;
    while (idx != string::npos) {
        size_t idxStart = result.find("/*", idx);
        if (idxStart == string::npos)
            break;
        size_t idxEnd = result.find("*/", idxStart);

        if (idxEnd != string::npos)
            result.erase(idxStart, idxEnd - idxStart + 2);
        else
            result.erase(idxStart);
        idx = idxStart;
    }
    return result;
}

EvalMode specifiedCtAvailability(const string& fileContent) {
    using namespace Nest;

    // Check for a comment on the first line
    size_t idxEnd = fileContent.find('\n');
    if (idxEnd == string::npos)
        idxEnd = fileContent.size();
    string firstLine = fileContent.substr(0, idxEnd);
    size_t idxCommentStart = firstLine.find(';');
    if (idxCommentStart == string::npos)
        return modeRt;

    // Search for ct modifiers in the comment
    if (string::npos != firstLine.find("rt", idxCommentStart))
        return modeRt;
    if (string::npos != firstLine.find("ct", idxCommentStart))
        return modeCt;
    return modeRt;
}

void parseSourceCode(Nest_SourceCode* sourceCode, CompilationContext* ctx) {
    // Read the LLVM content
    const string& fileContent = readFile(sourceCode->url);

    // Create a backend code with the given content
    EvalMode evalMode = specifiedCtAvailability(fileContent);
    sourceCode->mainNode = Feather_mkBackendCode(
            Nest_mkLocation1(sourceCode, 1, 1), StringRef(fileContent), evalMode);
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
} // namespace

int LLVMBe_kindLLVMSourceCode = -1;

void LLVMBe_registerLLVMSourceCode() {
    LLVMBe_kindLLVMSourceCode = Nest_registerSourceCodeKind(
            ".llvm", "LLVM source code", "", &parseSourceCode, &getSourceCodeLine, nullptr);
}
