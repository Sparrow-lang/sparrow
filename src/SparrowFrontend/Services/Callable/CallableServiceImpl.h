#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/ICallableService.h"

namespace SprFrontend {

struct CallableServiceImpl : ICallableService {
    //! @copydoc ICallableService::getCallables()
    Callables getCallables(NodeRange decls, EvalMode evalMode,
            const std::function<bool(NodeHandle)>& pred = {},
            const char* ctorName = "ctor") override;
};

//! The callable service instance that we are using across the Sparrow compiler
extern unique_ptr<ICallableService> g_CallableService;

} // namespace SprFrontend
