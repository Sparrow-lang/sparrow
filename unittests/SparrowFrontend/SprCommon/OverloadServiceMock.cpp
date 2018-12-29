#include "StdInc.h"
#include "OverloadServiceMock.hpp"

#include "Feather/Utils/cppif/FeatherNodes.hpp"

Node* OverloadServiceMock::selectOverload(CompilationContext* context, const Location& loc,
        EvalMode evalMode, Nest_NodeRange decls, Nest_NodeRange args,
        SprFrontend::OverloadReporting errReporting, Nest::StringRef funName) {
    return nullptr;
}

bool OverloadServiceMock::selectConversionCtor(
        CompilationContext* context, Node* destClass, EvalMode destMode, Nest::Type argType) {
    if (argType.mode() != destMode)
        return false;
    // If we know about the conversion, we return true
    Nest::Type destType = Feather::DataType::get(destClass, 0, destMode);
    for (const auto& p : implicitConversions_) {
        if (p.first == argType && p.second == destType)
            return true;
    }
    return false;
}

Node* OverloadServiceMock::selectCtToRtCtor(Node* ctArg) { return nullptr; }
