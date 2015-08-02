#include <StdInc.h>
#include "SparrowSourceCode.h"
#include "Grammar/Scanner.h"
#include <parser.yy.hpp>
#include <Helpers/SprTypeTraits.h>

#include <Nest/Compiler.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Frontend/FrontendFactory.h>

#include <boost/lambda/construct.hpp>

#include <fstream>

using namespace SprFrontend;

SparrowSourceCode::SparrowSourceCode(const string& filename)
    : SourceCode(filename)
{
}

SparrowSourceCode::~SparrowSourceCode()
{
}

void SparrowSourceCode::parse(CompilationContext* context)
{
    Location loc = Location(*this);

    // Open the filename
    ifstream f(filename().c_str());
    if ( !f )
        REP_ERROR(loc, "Cannot open source file");

    Scanner scanner(f, Parser::token::START_PROGRAM);
    Parser parser(scanner, loc, &iCode_);
    int rc = parser.parse();
    if ( rc != 0 )
        REP_ERROR(loc, "Cannot parse the source file");

    Nest::setContext(iCode_, context);
}

string SparrowSourceCode::getSourceCodeLine(int lineNo) const
{
    ifstream f(filename().c_str());
    if ( !f )
        return string();

    static const int MAXLINE=512;
    char line[MAXLINE];
    for ( int i=0; i<lineNo; ++i )
    {
        f.getline(line, MAXLINE);
        if ( !f.good() )
            return string();
    }
    return line;
}

boost::function<Nest::Node*(Nest::Node*)> SparrowSourceCode::ctToRtTranslator() const
{
    return &convertCtToRt;
}

DynNode* SparrowSourceCode::parseExpression(Location loc, const string& code) const
{
    // Only use the start part of the location
    loc.setAsStartOf(loc);

    istringstream f(code);
    Scanner scanner(f, Parser::token::START_EXPRESSION);
    Node* res;
    Parser parser(scanner, loc, &res);
    int rc = parser.parse();
    if ( rc != 0 )
        REP_ERROR(loc, "Cannot parse the expression code");
    return (DynNode*) res;
}

void SparrowSourceCode::registerSelf()
{
    using namespace boost::lambda;

    Nest::theCompiler().frontendFactory().registerParser("Sparrow source file", ".spr", new_ptr<SparrowSourceCode>());
}

