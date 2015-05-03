#pragma once

#include <Nest/Frontend/SourceCode.h>

namespace LLVMB
{
    /// Class that can parse & interpret '.llvm' source files
    /// This will just read the file and put the content as a backend code - to be used only with LLVM backend
    class LLVMSourceCode : public Nest::SourceCode
    {
    public:
        LLVMSourceCode(const string& filename);

        virtual void parse(Nest::CompilationContext* context);
        virtual string getSourceCodeLine(int lineNo) const;

        static void registerSelf();
    };
}
