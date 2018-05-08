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
    g_OverloadService = new OverloadService;
    g_ConceptsService = new ConceptsService;
}

void SparrowFrontend_onBackendSetFun(Backend* backend) {
    // Initialize the Type type before loading anything
    SprFrontend::initTypeType(Nest_getRootContext());

    // Register the CT API functions
    SprFrontend::registerCtApiFunctions(backend);
}

CompilerModule* getSparrowFrontendModule() {
    auto* nestModule = new CompilerModule{"SparrowFrontend",
            "Module that defines the frontend for the Sparrow language", "LucTeo", "www.lucteo.ro",
            1, 0, &SparrowFrontend_initModule, nullptr, &SparrowFrontend_onBackendSetFun};
    return nestModule;
}