#include <StdInc.h>
#include "BackendCode.h"
#include "FeatherNodeCommonsCpp.h"

using namespace Feather;

namespace
{
    class CtProcessMod : public Nest::Modifier
    {
    public:
        virtual void afterSemanticCheck(DynNode* node)
        {
            theCompiler().ctProcess(node);
        };
    };
}

BackendCode::BackendCode(const Location& location, string code, EvalMode evalMode)
    : DynNode(classNodeKind(), location)
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
    return curMode != modeUnspecified ? curMode : data_->context->evalMode();
}

void BackendCode::dump(ostream& os) const
{
    os << "backendCode(\"" << code() << "\")";
}

void BackendCode::doSemanticCheck()
{
    EvalMode mode = evalMode();
    if ( !data_->type )
        data_->type = getVoidType(mode);

    if ( mode != modeRt )
    {
        // CT process this node right after semantic check
        addModifier(new CtProcessMod);
    }
}
