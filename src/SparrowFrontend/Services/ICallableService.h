#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Services/Callable/Callables.h"

#include "Feather/Utils/cppif/FeatherNodes.hpp"

namespace SprFrontend {

struct ICallableService {
    virtual ~ICallableService() {}

    //! Given some declarations, try to gets a list of Callable objects from it.
    //! Returns an empty list if the declarations are not callable
    //! We apply the given predicate to filter out decls we don't want.
    virtual Callables getCallables(NodeRange decls, EvalMode evalMode,
            const std::function<bool(NodeHandle)>& pred = {}, const char* ctorName = "ctor") = 0;
};

//! The callable service instance that we are using across the Sparrow compiler
extern unique_ptr<ICallableService> g_CallableService;

} // namespace SprFrontend
