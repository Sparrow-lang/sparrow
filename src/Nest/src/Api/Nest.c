#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"

#include "Nest/Api/Backend.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.h"

void Nest_initModule() {
    Nest_compilerInit();
}

void Nest_destroyModule() {
    Nest_compilerDestroy();
    Nest_clearBackends();
    Nest_resetTypes();
    Nest_resetRegisteredTypeKinds();
    Nest_resetRegisteredNodeKinds();
    Nest_resetRegisteredSourceCodeKinds();
    Nest_resetDiagnostic();

    // Cleanup all allocated memory
    cleanupMemory();
}

static Nest_CompilerModule theNestModule = {"Nest",
        "The core of the compiler; defines the main services of the compiler, and the interactions "
        "between different components",
        "LucTeo", "www.lucteo.ro", 1, 0, &Nest_initModule, &Nest_destroyModule, 0};

Nest_CompilerModule* getNestModule() { return &theNestModule; }