#include <StdInc.h>
#include "FSimpleSourceCode.h"
#include "Impl/SimpleLexer.h"
#include "Impl/SimpleParser.h"

#include <Nest/Common/Diagnostic.h>

#include <Nest/Compiler.h>
#include <Nest/Frontend/FrontendFactory.h>

#include <boost/lambda/construct.hpp>

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
}

FSimpleSourceCode::FSimpleSourceCode(const string& filename)
    : SourceCode(filename)
    , lexer_(nullptr)
{
}

FSimpleSourceCode::~FSimpleSourceCode()
{
    delete lexer_;
}

void FSimpleSourceCode::parse(CompilationContext* context)
{
    // Read the file content
    const string& fileContent = readFileContent(filename());

    // Create the lexer & parser objects
    delete lexer_;
    lexer_ = new SimpleLexer(*this, fileContent.c_str());
    SimpleParser parser(*lexer_);

    // Parse the file
    iCode_ = parser.parse(context)->node();
}

string FSimpleSourceCode::getSourceCodeLine(int lineNo) const
{
    return lexer_ ? lexer_->getSourceCodeLine(lineNo) : string();
}

void FSimpleSourceCode::registerSelf()
{
    using namespace boost::lambda;

    Nest::theCompiler().frontendFactory().registerParser("Sparrow Frontend simple source file", ".fsimple", new_ptr<FSimpleSourceCode>());
}
