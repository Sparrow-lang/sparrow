#include <StdInc.h>
#include "ClassCtorCallable.h"
#include "FunctionCallable.h"
#include "GenericCallable.h"
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Generics.h>
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    TypeRef varType(Node* cls, EvalMode mode)
    {
        // Get the type of the temporary variable
        TypeRef t = cls->type;
        if ( mode != modeRtCt )
            t = changeTypeMode(t, mode, cls->location);
        return t;
    }
}

ClassCtorCallable::ClassCtorCallable(Node* cls, Callable* baseCallable, EvalMode evalMode)
    : cls_(cls)
    , baseCallable_(baseCallable)
    , evalMode_(evalMode)
    , tmpVar_(nullptr)
    , thisArg_(nullptr)
{
}

Callables ClassCtorCallable::getCtorCallables(Node* cls, EvalMode evalMode)
{
    NodeArray decls = Nest_symTabLookupCurrent(Nest_childrenContext(cls)->currentSymTab, "ctor");

    evalMode = combineMode(effectiveEvalMode(cls), evalMode, cls->location, false);

    Callables res;
    res.reserve(Nest_nodeArraySize(decls));
    for ( Node* decl: decls )
    {
        Node* fun = Nest_explanation(decl);
        if ( fun && fun->nodeKind == nkFeatherDeclFunction )
            res.push_back(new ClassCtorCallable(cls, new FunctionCallable(fun), evalMode));

        Node* resDecl = resultingDecl(decl);
        if ( isGeneric(resDecl) )
            res.push_back(new ClassCtorCallable(cls, new GenericCallable(resDecl), evalMode));
    }
    Nest_freeNodeArray(decls);
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

ConversionType ClassCtorCallable::canCall(CompilationContext* context, const Location& loc, const vector<TypeRef>& argTypes, EvalMode evalMode, bool noCustomCvt)
{
    TypeRef t = Feather_getLValueType(varType(cls_, evalMode_));

    vector<TypeRef> argTypes2 = argTypes;
    argTypes2.insert(argTypes2.begin(), t);
    return baseCallable_->canCall(context, loc, argTypes2, evalMode, noCustomCvt);
}

ConversionType ClassCtorCallable::canCall(CompilationContext* context, const Location& loc, NodeRange args, EvalMode evalMode, bool noCustomCvt)
{
    context_ = context;

    // Create a temporary variable - use it as a this argument
    tmpVar_ = Feather_mkVar(loc, fromCStr("tmp.v"), Feather_mkTypeNode(loc, varType(cls_, evalMode_)));
    setEvalMode(tmpVar_, evalMode_);
    Nest_setContext(tmpVar_, context);
    if ( !Nest_computeType(tmpVar_) )
        return convNone;
    thisArg_ = Feather_mkVarRef(loc, tmpVar_);
    Nest_setContext(thisArg_, context);
    if ( !Nest_computeType(thisArg_) )
        return convNone;

    NodeVector args2 = toVec(args);
    args2.insert(args2.begin(), thisArg_);
    return baseCallable_->canCall(context, loc, all(args2), evalMode, noCustomCvt);
}


Node* ClassCtorCallable::generateCall(const Location& loc)
{
    ASSERT(context_);
    ASSERT(tmpVar_);
    ASSERT(thisArg_);

    Node* ctorCall = baseCallable_->generateCall(loc);
    return createTempVarConstruct(loc, context_, ctorCall, tmpVar_);
}
