#include <StdInc.h>
#include "BackendCode.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

namespace
{
    class CtProcessMod : public Nest::Modifier
    {
    public:
        virtual void afterSemanticCheck(Node* node)
        {
            theCompiler().ctProcess(node);
        };
    };
}

BackendCode::BackendCode(const Location& location, string code, EvalMode evalMode)
    : Node(location)
{
    setProperty(propCode, move(code));
    setProperty(propEvalMode, (int) evalMode);
}

string BackendCode::code() const
{
    return move(getCheckPropertyString(propCode));
}

EvalMode BackendCode::evalMode() const
{
    EvalMode curMode = (EvalMode) getCheckPropertyInt(propEvalMode);
    return curMode != modeUnspecified ? curMode : context_->evalMode();
}

void BackendCode::dump(ostream& os) const
{
    os << "backendCode(\"" << code() << "\")";
}

void BackendCode::doSemanticCheck()
{
    EvalMode mode = evalMode();
    if ( !type_ )
        type_ = getVoidType(mode);

    if ( mode != modeRt )
    {
        // CT process this node right after semantic check
        modifiers_.push_back(new CtProcessMod);
    }
}
