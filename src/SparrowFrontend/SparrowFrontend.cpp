#include <StdInc.h>
#include "SparrowFrontend.h"

#include "SparrowFrontendTypes.h"
#include "Nodes/SparrowNodes.h"
#include "SparrowSourceCode.h"
#include "Helpers/StdDef.h"
#include "CtApiFunctions.h"

#include "Helpers/Convert.h"
#include "Helpers/Overload.h"
#include "Helpers/Generics.h"

#include "Nest/Api/CompilerModule.h"

void SparrowFrontend_initModule() {
    SprFrontend::initSparrowFrontendTypeKinds();
    SprFrontend::initSparrowNodeKinds();
    SprFe_registerSparrowSourceCode();

    // Create the service objects
    setDefaultConvertService();
    setDefaultOverloadService();
    setDefaultConceptsService();
}

void SparrowFrontend_destroyModule() {
    g_ConvertService.reset();
    g_OverloadService.reset();
    g_ConceptsService.reset();
}

void SparrowFrontend_onBackendSetFun(Nest_Backend* backend) {
    // Initialize the Type type before loading anything
    SprFrontend::initTypeType(Nest_getRootContext());

    // Register the CT API functions
    SprFrontend::registerCtApiFunctions(backend);
}

Nest_CompilerModule* getSparrowFrontendModule() {
    auto* nestModule = new Nest_CompilerModule{"SparrowFrontend",
            "Module that defines the frontend for the Sparrow language", "LucTeo", "www.lucteo.ro",
            1, 0, &SparrowFrontend_initModule, &SparrowFrontend_destroyModule,
            &SparrowFrontend_onBackendSetFun};
    return nestModule;
}