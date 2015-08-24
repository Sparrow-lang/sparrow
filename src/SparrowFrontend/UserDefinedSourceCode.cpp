#include <StdInc.h>
#include "UserDefinedSourceCode.h"
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/Ct.h>

#include <Nodes/Builder.h>
#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Common/Alloc.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Frontend/SourceCode.h>
#include <Nest/Frontend/SourceCodeKindRegistrar.h>

#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace SprFrontend;

namespace
{
    void parseSourceCode(SourceCode* sourceCode, CompilationContext* ctx)
    {
        Location loc = Nest_mkLocation1(sourceCode, 1,1);

        // Open the filename
        ifstream f(sourceCode->url);
        if ( !f )
            REP_ERROR(loc, "Cannot open source file");

        // Get the content of the file
        ostringstream oss;
        string line;
        while ( getline(f, line) )
        {
            oss << line << '\n';
        }

        // Create a node that invokes the given function with the content of the file
        // funName(code: StringRef, location: Meta.Location, context: Meta.CompilationContext): Meta.AstNode
        Node* codeNode = buildStringLiteral(loc, oss.str());

        int* scHandle = reinterpret_cast<int*>(sourceCode);
        Node* scBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "SourceCode");
        Node* scArg = Feather::mkCtValue(loc, StdDef::typeRefInt, &scHandle);
        Node* scNode = mkFunApplication(loc, scBase, NodeVector(1, scArg));
        Node* locBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "Location");
        Node* locNode = mkFunApplication(loc, locBase, NodeVector(1, scNode));

        int* ctxHandle = reinterpret_cast<int*>(ctx);
        Node* ctxBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "CompilationContext");
        Node* ctxArg = Feather::mkCtValue(loc, StdDef::typeRefInt, &ctxHandle);
        Node* ctxNode = mkFunApplication(loc, ctxBase, NodeVector(1, ctxArg));

        // The function name is encoded as extraInfo when registering the source code
        const char* funName = Nest_getSourceCodeExtraInfo(sourceCode->kind);
        vector<string> funNameParts;
        boost::split(funNameParts, funName, boost::is_any_of("."));
        Node* funBase = nullptr;
        for ( string& partName: funNameParts )
        {
            if ( !funBase )
                funBase = mkIdentifier(loc, move(partName));
            else
                funBase = mkCompoundExp(loc, funBase, move(partName));
        }
        Node* funCall = mkFunApplication(loc, funBase, {codeNode, locNode, ctxNode});

        // Compile the function and evaluate it
        Node* implPart = mkCompoundExp(loc, funCall, "impl");
        implPart = Feather::mkMemLoad(loc, implPart);    // Remove LValue
        Nest_setContext(implPart, ctx);
        Nest_semanticCheck(implPart);

        sourceCode->mainNode = (Node*) getIntRefCtValue(implPart);
        if ( !sourceCode->mainNode )
            REP_INTERNAL(loc, "Invalid parsing function %1%, (used to parse %2%)") % funName % sourceCode->url;
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

int SprFe_registerUserDefinedSourceCode(const char* ext, const char* funName)
{
    return Nest_registerSourceCodeKind(ext,
        "Source file registered by the user from library code", funName,
        &parseSourceCode, &getSourceCodeLine, NULL);
}
