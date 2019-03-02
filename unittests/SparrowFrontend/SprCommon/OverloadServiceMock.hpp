#pragma once

#include "SparrowFrontend/Services/IOverloadService.h"

#include "Nest/Utils/cppif/StringRef.hpp"

using Nest::Node;
using Nest::Location;
using Nest::CompilationContext;

struct OverloadServiceMock : SprFrontend::IOverloadService {
    Node* selectOverload(CompilationContext* context, const Location& loc, EvalMode evalMode,
            Nest_NodeRange decls, Nest_NodeRange args, SprFrontend::OverloadReporting errReporting,
            Nest::StringRef funName) final;

    bool selectConversionCtor(CompilationContext* context, Node* destClass, EvalMode destMode,
            Nest::Type argType) final;

    Node* selectCtToRtCtor(Node* ctArg) final;

    vector<pair<Nest::Type, Nest::Type>> implicitConversions_;
};
