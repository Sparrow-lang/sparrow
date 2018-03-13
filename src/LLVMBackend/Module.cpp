#include <StdInc.h>
#include "Module.h"
#include <Tr/TrTopLevel.h>

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Api/Compiler.h"

using namespace LLVMB;

Module::Module(const string& name)
    : llvmContext_(new llvm::LLVMContext()) {}

Module::~Module() = default;

bool Module::canUse(Node* decl) const {
    EvalMode mode = Feather_effectiveEvalMode(decl);
    ASSERT(mode != modeUnspecified);
    if (mode == modeRt && isCt())
        return false;
    else if (mode == modeCt && !isCt())
        return false;
    else
        return true;
}
