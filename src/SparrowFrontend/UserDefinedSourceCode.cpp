#include <StdInc.h>
#include "UserDefinedSourceCode.h"
#include "Grammar/Scanner.h"
#include <parser.yy.hpp>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/Ct.h>

#include <Nodes/SparrowNodes.h>
#include <Nodes/Builder.h>
#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Compiler.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Frontend/FrontendFactory.h>

#include <boost/lambda/construct.hpp>

#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace SprFrontend;

namespace
{
    static const int MAXLINE=512;

    struct Creator
    {
        Creator(string funName) : funName_(move(funName)) {}

        SourceCode* operator()(string filename)
        {
            return new UserDefinedSourceCode(funName_, move(filename));
        }

    private:
        string funName_;
    };

}

UserDefinedSourceCode::UserDefinedSourceCode(string funName, string filename)
    : SourceCode(move(filename))
    , funName_(move(funName))
{
}

UserDefinedSourceCode::~UserDefinedSourceCode()
{
}

void UserDefinedSourceCode::parse(CompilationContext* context)
{
    Location loc = Nest_mkLocation1(this, 1,1);

    // Open the filename
    ifstream f(filename().c_str());
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

    int* scHandle = reinterpret_cast<int*>(this);
    Node* scBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "SourceCode");
    Node* scArg = Feather::mkCtValue(loc, StdDef::typeRefInt, &scHandle);
    Node* scNode = mkFunApplication(loc, scBase, NodeVector(1, scArg));
    Node* locBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "Location");
    Node* locNode = mkFunApplication(loc, locBase, NodeVector(1, scNode));

    int* ctxHandle = reinterpret_cast<int*>(context);
    Node* ctxBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "CompilationContext");
    Node* ctxArg = Feather::mkCtValue(loc, StdDef::typeRefInt, &ctxHandle);
    Node* ctxNode = mkFunApplication(loc, ctxBase, NodeVector(1, ctxArg));

    vector<string> funNameParts;
    boost::split(funNameParts, funName_, boost::is_any_of("."));
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
    Nest::setContext(implPart, context);
    Nest::semanticCheck(implPart);

    iCode_ = (Node*) getIntRefCtValue(implPart);
    if ( !iCode_ )
        REP_INTERNAL(loc, "Invalid parsing function %1%, (used to parse %2%)") % funName_ % filename();
}

string UserDefinedSourceCode::getSourceCodeLine(int lineNo) const
{
    ifstream f(filename().c_str());
    if ( !f )
        return string();

    char line[MAXLINE];
    for ( int i=0; i<lineNo; ++i )
    {
        f.getline(line, MAXLINE);
        if ( !f.good() )
            return string();
    }
    return line;
}

void UserDefinedSourceCode::registerSelf(string ext, string funName)
{
    string langName = "User-define source file - " + funName;
    Nest::theCompiler().frontendFactory().registerParser(langName, move(ext), Creator(move(funName)));
}
