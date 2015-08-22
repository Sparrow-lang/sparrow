#pragma once

#include "DataLayoutHelper.h"
#include <Nest/Backend/Backend.h>

FWD_CLASS2(LLVMB,Tr, RtModule);
FWD_CLASS2(LLVMB,Tr, CtModule);

namespace LLVMB
{
    /// Backend class that will translate the intermediate code trough LLVM into executable
    class LLVMBackend : public Nest::Backend
    {
    public:
        LLVMBackend();
        virtual ~LLVMBackend();

        virtual void init(const string& mainFilename);
        virtual void generateMachineCode(SourceCode& code);
        virtual void link(const string& outFilename);
        virtual void ctProcess(Nest::Node* node);
        virtual Nest::Node* ctEvaluate(Nest::Node* node);
        virtual size_t sizeOf(Nest::TypeRef type);
        virtual size_t alignmentOf(Nest::TypeRef type);
        virtual void ctApiRegisterFun(const char* name, void* funPtr);

        static void registerSelf();

	private:
		Tr::CtModule* ctModule_;
		Tr::RtModule* rtModule_;
        DataLayoutHelper dataLayoutHelper_;
    };
}
