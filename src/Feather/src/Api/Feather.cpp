#include "Feather/src/StdInc.h"
#include "Feather/Api/Feather.h"

#include "Feather/src/Api/Feather_Types.h"
#include "Feather/src/Api/Feather_Nodes.h"
#include "Feather/src/CtApiFunctions.h"

#include "Nest/Api/CompilerModule.h"

void Feather_initModule()
{
    initFeatherTypeKinds();
    initFeatherNodeKinds();
}

void Feather_onBackendSetFun(Backend* backend)
{
    // Register the CT API functions
    Feather::registerCtApiFunctions(backend);
}

CompilerModule* Feather_getModule()
{
    CompilerModule* nestModule = new CompilerModule {
        "Feather",
        "Module that defines the basic node kinds for the compilers; it provides a minimal, C-like node set to be the base of more complex languages",
        "LucTeo",
        "www.lucteo.ro",
        1,
        0,
        &Feather_initModule,
        NULL,
        &Feather_onBackendSetFun
    };
    return nestModule;
}