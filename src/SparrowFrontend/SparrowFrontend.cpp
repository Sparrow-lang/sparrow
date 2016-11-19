#include <StdInc.h>
#include "SparrowFrontend.h"

#include "SparrowFrontendTypes.h"
#include "Nodes/SparrowNodes.h"
#include "SparrowSourceCode.h"
#include "Helpers/StdDef.h"
#include "CtApiFunctions.h"

#include "Nest/Api/CompilerModule.h"

void SparrowFrontend_initModule()
{
    SprFrontend::initSparrowFrontendTypeKinds();
    SprFrontend::initSparrowNodeKinds();
    SprFe_registerSparrowSourceCode();
}

void SparrowFrontend_onBackendSetFun(Backend* backend)
{
    // Initialize the Type type before loading anything
    SprFrontend::initTypeType(Nest_getRootContext());

    // Register the CT API functions
    SprFrontend::registerCtApiFunctions(backend);
}

CompilerModule* getSparrowFrontendModule()
{
    CompilerModule* nestModule = new CompilerModule {
        "SparrowFrontend",
        "Module that defines the frontend for the Sparrow language",
        "LucTeo",
        "www.lucteo.ro",
        1,
        0,
        &SparrowFrontend_initModule,
        NULL,
        &SparrowFrontend_onBackendSetFun
    };
    return nestModule;
}