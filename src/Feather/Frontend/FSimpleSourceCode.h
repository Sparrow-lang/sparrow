#pragma once

#include <Nest/Frontend/SourceCode.h>

FWD_CLASS1(Feather, SimpleLexer)

namespace Feather
{
    /// Class that can parse & interpret '.test' source files
    class FSimpleSourceCode : public Nest::SourceCode
    {
    public:
        FSimpleSourceCode(const string& filename);
        ~FSimpleSourceCode();

        virtual void parse(Nest::CompilationContext* context);
        virtual string getSourceCodeLine(int lineNo) const;

        static void registerSelf();

    private:
        /// The lexer used to parse the source
        SimpleLexer* lexer_;
    };
}
