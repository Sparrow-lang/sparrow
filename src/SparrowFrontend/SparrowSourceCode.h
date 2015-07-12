#pragma once

#include <Nest/Frontend/SourceCode.h>

namespace SprFrontend
{
    /// A Sparrow source code; this class knows how to parse a Sparrow source code
    class SparrowSourceCode : public Nest::SourceCode
    {
    public:
        SparrowSourceCode(const string& filename);
        virtual ~SparrowSourceCode();

        virtual void parse(Nest::CompilationContext* context);
        virtual string getSourceCodeLine(int lineNo) const;
        virtual boost::function<Nest::Node*(Nest::Node*)> ctToRtTranslator() const;

        Nest::DynNode* parseExpression(Nest::Location loc, const string& code) const;

        static void registerSelf();
    };
}
