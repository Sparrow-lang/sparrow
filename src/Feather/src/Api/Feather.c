#include "Feather/Api/Feather.h"

#include "Feather/src/Api/Feather_Types.h"
#include "Feather/src/Api/Feather_Nodes.h"
#include "Feather/src/CtApiFunctions.h"

#include "Nest/Api/CompilerModule.h"

void _Feather_initModule() {
    initFeatherTypeKinds();
    initFeatherNodeKinds();
}

void _Feather_onBackendSetFun(Backend* backend) {
    // Register the CT API functions
    Feather_registerCtApiFunctions(backend);
}

static CompilerModule theFeatherModule = {"Feather",
        "Module that defines the basic node kinds for the compilers; it provides a minimal, C-like "
        "node set to be the base of more complex languages",
        "LucTeo", "www.lucteo.ro", 1, 0, &_Feather_initModule, 0, &_Feather_onBackendSetFun};

CompilerModule* Feather_getModule() { return &theFeatherModule; }