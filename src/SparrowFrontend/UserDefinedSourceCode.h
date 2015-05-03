#pragma once

#include <Nest/Frontend/SourceCode.h>

namespace SprFrontend
{
    /// An user-defined source code
    /// This class invokes an user-defined function to parse the source code
    /// The function must be of the form: (code: StringRef, location: Meta.Location, context: Meta.CompilationContext): Meta.AstNode
    class UserDefinedSourceCode : public Nest::SourceCode
    {
    public:
        UserDefinedSourceCode(string funName, string filename);
        virtual ~UserDefinedSourceCode();

        virtual void parse(Nest::CompilationContext* context);
        virtual string getSourceCodeLine(int lineNo) const;

        static void registerSelf(string ext, string funName);

    private:
        /// The name of the function that is used to do the parsing
        string funName_;
    };
}
