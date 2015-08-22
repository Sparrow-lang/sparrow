#include <StdInc.h>
#include "Feather.h"

#include "FeatherTypes.h"
#include "Nodes/FeatherNodes.h"
#include "Frontend/FSimpleSourceCode.h"
#include "CtApiFunctions.h"

#include <Nest/CompilerModule.h>

void Feather_initModule()
{
    Feather::initFeatherTypeKinds();
    Feather::initFeatherNodeKinds();
    Feather_registerFSimpleSourceCode();
}

void Feather_onBackendSetFun(Nest::Backend* backend)
{
    // Register the CT API functions
    Feather::registerCtApiFunctions(*backend);
}

CompilerModule* getFeatherModule()
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