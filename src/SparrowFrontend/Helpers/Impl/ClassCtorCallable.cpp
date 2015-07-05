#include <StdInc.h>
#include "ClassCtorCallable.h"
#include "FunctionCallable.h"
#include "GenericCallable.h"
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    Type* varType(Class* cls, EvalMode mode)
    {
        // Get the type of the temporary variable
        Type* t = cls->type();
        if ( mode != modeRtCt )
            t = changeTypeMode(t, mode, cls->location());
        return t;
    }
}

ClassCtorCallable::ClassCtorCallable(Class* cls, Callable* baseCallable, EvalMode evalMode)
    : cls_(cls)
    , baseCallable_(baseCallable)
    , evalMode_(evalMode)
    , tmpVar_(nullptr)
    , thisArg_(nullptr)
{
}

Callables ClassCtorCallable::getCtorCallables(Class* cls, EvalMode evalMode)
{
    NodeVector decls = cls->childrenContext()->currentSymTab()->lookupCurrent("ctor");

    evalMode = combineMode(effectiveEvalMode(cls), evalMode, cls->location(), false);

    Callables res;
    res.reserve(decls.size());
    for ( Node* decl: decls )
    {
        Function* fun = decl->explanation()->as<Function>();
        if ( fun )
            res.push_back(new ClassCtorCallable(cls, new FunctionCallable(fun), evalMode));

        Node* resDecl = resultingDecl(decl);
        Generic* generic = dynamic_cast<Generic*>(resDecl);
        if ( generic)
            res.push_back(new ClassCtorCallable(cls, new GenericCallable(generic), evalMode));
    }
    return res;
}

const Location& ClassCtorCallable::location() const
{
    return baseCallable_->location();
}

string ClassCtorCallable::toString() const
{
    return baseCallable_->toString();
}

size_t ClassCtorCallable::paramsCount() const
{
    return baseCallable_->paramsCount()-1;      // The 'this' param is hidden
}

Node* ClassCtorCallable::param(size_t idx) const
{
    return baseCallable_->param(idx+1);        // The 'this' param is hidden
}
EvalMode ClassCtorCallable::evalMode() const
{
    return evalMode_;
}
bool ClassCtorCallable::isAutoCt() const
{
    return baseCallable_->isAutoCt();
}

ConversionType ClassCtorCallable::canCall(CompilationContext* context, const Location& loc, const vector<Type*>& argTypes, EvalMode evalMode, bool noCustomCvt)
{
    Type* t = Type::fromBasicType(getLValueType(varType(cls_, evalMode_)->data_));

    vector<Type*> argTypes2 = argTypes;
    argTypes2.insert(argTypes2.begin(), t);
    return baseCallable_->canCall(context, loc, argTypes2, evalMode, noCustomCvt);
}

ConversionType ClassCtorCallable::canCall(CompilationContext* context, const Location& loc, const NodeVector& args, EvalMode evalMode, bool noCustomCvt)
{
    context_ = context;

    // Create a temporary variable - use it as a this argument
    tmpVar_ = Feather::mkVar(loc, "tmp.v", mkTypeNode(loc, varType(cls_, evalMode_)), 0, evalMode_);
    tmpVar_->setContext(context);
    tmpVar_->computeType();
    thisArg_ = mkVarRef(loc, tmpVar_);
    thisArg_->setContext(context);
    thisArg_->computeType();

    NodeVector args2 = args;
    args2.insert(args2.begin(), thisArg_);
    return baseCallable_->canCall(context, loc, args2, evalMode, noCustomCvt);
}


Node* ClassCtorCallable::generateCall(const Location& loc)
{
    ASSERT(context_);
    ASSERT(tmpVar_);
    ASSERT(thisArg_);

    Node* ctorCall = baseCallable_->generateCall(loc);
    return createTempVarConstruct(loc, context_, ctorCall, tmpVar_);
}
