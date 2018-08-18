#include <StdInc.h>
#include "LLVMBackendMod.h"
#include "LLVMSourceCode.h"
#include "LLVMBackend.h"

#include "Nest/Api/CompilerModule.h"

void LLVMBackend_initModule() {
    LLVMBe_registerLLVMSourceCode();
    LLVMBe_registerLLVMBackend();
}

Nest_CompilerModule* getLLVMBackendModule() {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* nestModule = new Nest_CompilerModule{"LLVMBackend", "Backend that uses LLVM to generate code",
            "LucTeo", "www.lucteo.ro", 1, 0, &LLVMBackend_initModule, nullptr, nullptr};
    return nestModule;
}