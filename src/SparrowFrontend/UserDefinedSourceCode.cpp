#include <StdInc.h>
#include "UserDefinedSourceCode.h"
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/Ct.h>
#include <SprDebug.h>

#include <Nodes/Builder.h>
#include "Feather/Api/Feather.h"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"

#include <fstream>

using namespace SprFrontend;

StringRef inferModuleName(const char* url);

namespace {
/// Parse the function that we need to call
///
/// In the extraInfo we store two things:
///     - the filename to be imported to process the source code content
///     - the function that can parse the code (within that filename)
/// They are separated by a ':'
/// Function name can contain multiple parts, separated by '.'
///
/// We return a pair of file to import (optional) and a function name to call
pair<Node*, Node*> parseFunToCall(Location loc, const char* funInfo) {
    Node* toImport = nullptr;
    Node* toCall = nullptr;

    // Do we have a filename part?
    const char* p = strchr(funInfo, ':');
    if (p) {
        toImport = buildStringLiteral(loc, StringRef{funInfo, p});

        // The rest of the string is function name
        funInfo = p + 1;
    }

    const char* p2 = p;
    const char* end = p + strlen(p);
    while (p2 != end) {
        p2 = strchr(p, '.');
        StringRef partName;
        if (p2) {
            partName = StringRef{p, p2};
            p = p2+1;
        }
        else {
            partName = StringRef{p, end};
            p = end;
        }

        if (!partName) {
            if (!toCall)
                toCall = mkIdentifier(loc, partName);
            else
                toCall = mkCompoundExp(loc, toCall, partName);
        }
    }

    return make_pair(toImport, toCall);
}

void parseSourceCode(Nest_SourceCode* sourceCode, CompilationContext* ctx) {
    Location loc = Nest_mkLocation1(sourceCode, 1, 1);

    // Open the filename
    ifstream f(sourceCode->url);
    if (!f) {
        REP_ERROR(loc, "Cannot open source file");
        return;
    }

    // Get the content of the file
    ostringstream oss;
    string line;
    while (getline(f, line)) {
        oss << line << '\n';
    }

    // Create a package for the module
    StringRef packageName = inferModuleName(sourceCode->url);
    Node* content = Feather_mkNodeListVoid(loc, fromIniList({}));
    Node* moduleContent = mkSprPackage(loc, packageName, content);
    Nest_setContext(moduleContent, ctx);
    if (!Nest_semanticCheck(moduleContent))
        REP_INTERNAL(loc, "Cannot create main package for user defined source code (%1%)") %
                sourceCode->url;

    ctx = content->context;

    // Create a node that invokes the given function with the content of the file
    // funName(code: StringRef, location: meta.Location, context: meta.CompilationContext):
    // meta.AstNode
    Node* codeNode = buildStringLiteral(loc, StringRef(oss.str()));

    Node* scNode = buildLiteral(loc, StringRef("SourceCode"), sourceCode);
    Node* locBase = mkIdentifier(loc, StringRef("mkLocation"));
    Node* locNode = mkFunApplication(loc, locBase, fromIniList({scNode}));

    Node* ctxNode = buildLiteral(loc, StringRef("CompilationContext"), ctx);

    // Parse the extraInfo to find out the import and the function to call
    Node* toImport;
    Node* toCall;
    const char* funInfo = Nest_getSourceCodeExtraInfo(sourceCode->kind);
    tie(toImport, toCall) = parseFunToCall(loc, funInfo);

    // If provided, add an import, to make sure we can find the function to call
    if (toImport) {
        // Just add the import node to the right context, and semantically check it
        Node* importName = mkImportName(loc, toImport, nullptr);

        // Make sure to semantically check it; expanding the source code depends on it
        Nest_setContext(importName, ctx);
        Nest_semanticCheck(importName);
        Feather_addToNodeList(content, importName);
    }

    if (!toCall)
        REP_INTERNAL(loc, "Invalid parsing function '%1%', (used to parse %2%)") % funInfo %
                sourceCode->url;

    Node* funCall = mkFunApplication(loc, toCall, fromIniList({codeNode, locNode, ctxNode}));

    // Compile the function and evaluate it
    Node* implPart = mkCompoundExp(loc, funCall, StringRef("data"));
    implPart = Feather_mkMemLoad(loc, implPart); // Remove category
    Nest_setContext(implPart, ctx);
    if (!Nest_semanticCheck(implPart))
        REP_INTERNAL(loc, "Invalid parsing function '%1%', (used to parse %2%)") % funInfo %
                sourceCode->url;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    Feather_addToNodeList(content, (Node*)getByteRefCtValue(implPart));

    Nest_semanticCheck(moduleContent);
    sourceCode->mainNode = moduleContent;
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

int SprFe_registerUserDefinedSourceCode(const char* ext, const char* funName) {
    return Nest_registerSourceCodeKind(ext, "Source file registered by the user from library code",
            funName, &parseSourceCode, &getSourceCodeLine, nullptr);
}
