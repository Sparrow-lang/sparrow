#include <StdInc.h>
#include "UserDefinedSourceCode.h"
// TODO (now)
#include "Grammar/Scanner.h"
#include <parser.yy.hpp>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/Ct.h>

#include <Nodes/SparrowNodes.h>
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
    // Open the filename
    ifstream f(filename().c_str());
    if ( !f )
        REP_ERROR(Location(*this), "Cannot open source file");

    Location loc(*this);

    // Get the content of the file
    ostringstream oss;
    string line;
    while ( getline(f, line) )
    {
        oss << line << '\n';
    }


    // Create a node that invokes the given function with the content of the file
    // funName(code: StringRef, location: Meta.Location, context: Meta.CompilationContext): Meta.AstNode
    DynNode* codeNode = mkStringLiteral(loc, oss.str());

    int* scHandle = reinterpret_cast<int*>(this);
    DynNode* scBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "SourceCode");
    DynNode* scArg = (DynNode*) Feather::mkCtValue(loc, StdDef::typeRefInt, &scHandle);
    DynNode* scNode = mkFunApplication(loc, scBase, {scArg});
    DynNode* locBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "Location");
    DynNode* locNode = mkFunApplication(loc, locBase, {scNode});

    int* ctxHandle = reinterpret_cast<int*>(context);
    DynNode* ctxBase = mkCompoundExp(loc, mkIdentifier(loc, "Meta"), "CompilationContext");
    DynNode* ctxArg = (DynNode*) Feather::mkCtValue(loc, StdDef::typeRefInt, &ctxHandle);
    DynNode* ctxNode = mkFunApplication(loc, ctxBase, {ctxArg});

    vector<string> funNameParts;
    boost::split(funNameParts, funName_, boost::is_any_of("."));
    DynNode* funBase = nullptr;
    for ( string& partName: funNameParts )
    {
        if ( !funBase )
            funBase = mkIdentifier(loc, move(partName));
        else
            funBase = mkCompoundExp(loc, funBase, move(partName));
    }
    DynNode* funCall = mkFunApplication(loc, funBase, {codeNode, locNode, ctxNode});

    // Compile the function and evaluate it
    DynNode* implPart = mkCompoundExp(loc, funCall, "impl");
    implPart = (DynNode*) Feather::mkMemLoad(loc, implPart->node());    // Remove LValue
    implPart->setContext(context);
    implPart->semanticCheck();

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
