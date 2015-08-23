#include <StdInc.h>
#include "LLVMBackendMod.h"
#include "LLVMSourceCode.h"
#include "LLVMBackend.h"

#include <Nest/CompilerModule.h>

void LLVMBackend_initModule()
{
    LLVMBe_registerLLVMSourceCode();
    LLVMBe_registerLLVMBackend();
}

CompilerModule* getLLVMBackendModule()
{
    CompilerModule* nestModule = new CompilerModule {
        "LLVMBackend",
        "Backend that uses LLVM to generate code",
        "LucTeo",
        "www.lucteo.ro",
        1,
        0,
        &LLVMBackend_initModule,
        NULL,
        NULL
    };
    return nestModule;
}