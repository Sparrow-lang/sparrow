#include <StdInc.h>
#include "SparrowFrontend.h"

#include "SparrowFrontendTypes.hpp"
#include "Nodes/SparrowNodes.h"
#include "SparrowSourceCode.h"
#include "Helpers/StdDef.h"
#include "CtApiFunctions.h"

#include "SparrowFrontend/Services/Convert/ConvertServiceImpl.h"
#include "SparrowFrontend/Services/Overload/OverloadServiceImpl.h"
#include "SparrowFrontend/Services/Callable/CallableServiceImpl.h"
#include "SparrowFrontend/Services/Concepts/ConceptsServiceImpl.h"
#include "Helpers/Generics.h"

#include "Nest/Api/CompilerModule.h"

namespace SprFrontend {
unique_ptr<IConvertService> g_ConvertService{};
unique_ptr<IOverloadService> g_OverloadService;
unique_ptr<ICallableService> g_CallableService;
unique_ptr<IConceptsService> g_ConceptsService;
} // namespace SprFrontend

void SparrowFrontend_initModule() {
    SprFrontend::initSparrowFrontendTypeKinds();
    SprFrontend::initSparrowNodeKinds();
    SprFe_registerSparrowSourceCode();

    // Create the service objects

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_ConvertService.reset(new ConvertServiceImpl);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_OverloadService.reset(new OverloadServiceImpl);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_CallableService.reset(new CallableServiceImpl);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_ConceptsService.reset(new ConceptsServiceImpl);
}

void SparrowFrontend_destroyModule() {
    g_ConvertService.reset();
    g_OverloadService.reset();
    g_CallableService.reset();
    g_ConceptsService.reset();
}

void SparrowFrontend_onBackendSetFun(Nest_Backend* backend) {
    // Initialize the Type type before loading anything
    SprFrontend::initTypeType(Nest_getRootContext());

    // Register the CT API functions
    SprFrontend::registerCtApiFunctions(backend);
}

Nest_CompilerModule* getSparrowFrontendModule() {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* nestModule = new Nest_CompilerModule{"SparrowFrontend",
            "Module that defines the frontend for the Sparrow language", "LucTeo", "www.lucteo.ro",
            1, 0, &SparrowFrontend_initModule, &SparrowFrontend_destroyModule,
            &SparrowFrontend_onBackendSetFun};
    return nestModule;
}