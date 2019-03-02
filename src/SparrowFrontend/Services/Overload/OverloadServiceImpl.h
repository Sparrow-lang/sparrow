#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Services/IOverloadService.h"

namespace SprFrontend {

//! The Sparrow implementation of the overload service
struct OverloadServiceImpl : IOverloadService {
    Node* selectOverload(CompilationContext* context, const Location& loc, EvalMode evalMode,
            Nest_NodeRange decls, Nest_NodeRange args, OverloadReporting errReporting,
            StringRef funName) final;

    bool selectConversionCtor(
            CompilationContext* context, Node* destClass, EvalMode destMode, Type argType) final;

    Node* selectCtToRtCtor(Node* ctArg) final;
};

} // namespace SprFrontend
