#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/FunctionCallable.h"
#include "SparrowFrontend/Services/Callable/CallableHelpers.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Helpers/Impl/Intrinsics.h"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/SprDebug.h"

#include "Nest/Utils/cppif/SmallVector.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

FunctionCallable::FunctionCallable(Feather::FunctionDecl fun, TypeWithStorage implicitArgType)
    : Callable(fun)
    , implicitArgType_(implicitArgType)
    , params_(fun.parameters())
    , autoCt_(fun.hasProperty(propAutoCt)) {

    if (getResultParam(fun))
        params_ = params_.skip(1); // Always hide the result param
    ASSERT(params_.size() >= 0);
}

ConversionType FunctionCallable::canCall(const CCLoc& ccloc, NodeRange args, EvalMode evalMode,
        CustomCvtMode customCvtMode, bool reportErrors) {

    // This can be called a second time to report the errors
    args_.clear();

    SmallVector<NodeHandle> args2;

    // If this callable requires an added this argument, add it
    if (implicitArgType_) {
        auto thisTempVar = Feather::VarDecl::create(
                ccloc.loc_, "tmp.this", Feather::TypeNode::create(ccloc.loc_, implicitArgType_));
        thisTempVar.setContext(ccloc.context_);
        if (!thisTempVar.computeType()) {
            if (reportErrors)
                REP_INFO(ccloc.loc_, "Cannot create temporary variable");
            return convNone;
        }
        auto thisArg = VarRefExp::create(ccloc.loc_, thisTempVar);
        thisArg.setContext(ccloc.context_);
        if (!thisArg.computeType()) {
            if (reportErrors)
                REP_INFO(ccloc.loc_, "Cannot compute the type of this argument");
            return convNone;
        }

        tmpVar_ = thisTempVar;

        args2.reserve(args.size() + 1);
        args2.push_back(thisArg);
        args2.insert(args);
        args = NodeRangeT<NodeHandle>(args2);
    }

    // Complete the missing args with defaults
    if (!completeArgsWithDefaults(args_, args, params_))
        return convNone;

    // Check argument count (including hidden param)
    int paramsCount = params_.size();
    if (paramsCount != args_.size()) {
        if (reportErrors) {
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") % args_.size() %
                    paramsCount;
        }
        return convNone;
    }

    // Get the arg types to perform the check on types
    SmallVector<Type> argTypes;
    argTypes.reserve(args_.size());
    for (auto arg : args_)
        argTypes.push_back(arg.type());

    // Check if we need to force CT call
    bool useCt = shouldUseCt(decl_, autoCt_, argTypes, evalMode);

    // Get the param types
    SmallVector<Type> paramTypes;
    paramTypes.reserve(params_.size());
    for (auto p : params_) {
        if (!p || !p.type())
            return convNone;
        auto paramType = p.type();
        // If we are looking at a CT callable, make sure the parameters are in CT
        if (useCt)
            paramType = paramType.changeMode(modeCt, NOLOC);
        paramTypes.push_back(paramType);
    }

    // Do the checks on types
    // Make sure that all argument types are convertible to param types
    // Capture the conversions required.
    ConversionType res = checkTypeConversions(
            conversions_, ccloc, argTypes, paramTypes, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    return res;
}
ConversionType FunctionCallable::canCall(const CCLoc& ccloc, Range<Type> argTypes,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {

    Range<Type> argTypesToUse = argTypes;
    SmallVector<Type> argTypes2;

    // If this callable requires an added this argument, add it
    if (implicitArgType_) {
        Type t = implicitArgType_;
        if (!Feather::isCategoryType(t))
            t = MutableType::get(implicitArgType_);

        argTypes2.reserve(argTypes.size() + 1);
        argTypes2.push_back(t);
        argTypes2.insert(argTypes);
        argTypesToUse = Range<Type>(argTypes2);
    }

    // Check argument count (including hidden param)
    int paramsCount = params_.size();
    if (paramsCount != argTypesToUse.size()) {
        if (reportErrors) {
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") %
                    argTypesToUse.size() % paramsCount;
        }
        return convNone;
    }

    // Check if we need to force CT call
    bool useCt = shouldUseCt(decl_, autoCt_, argTypes, evalMode);

    // Get the param types
    SmallVector<Type> paramTypes;
    paramTypes.reserve(params_.size());
    for (auto p : params_) {
        if (!p || !p.type())
            return convNone;
        auto paramType = p.type();
        // If we are looking at a CT callable, make sure the parameters are in CT
        if (useCt)
            paramType = paramType.changeMode(modeCt, NOLOC);
        paramTypes.push_back(paramType);
    }

    ConversionType res = checkTypeConversions(
            conversions_, ccloc, argTypesToUse, paramTypes, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    return res;
}

NodeHandle FunctionCallable::generateCall(const CCLoc& ccloc) {

    if (!decl_.computeType())
        return {};

    // Apply the conversions to the arguments, and ensure they are semantically checked
    applyConversions(args_, conversions_);
    for (auto arg : args_)
        arg.semanticCheck();

    auto argsRange = NodeRangeT<NodeHandle>{args_};

    // Check if the call is an intrinsic
    NodeHandle res = handleIntrinsic(FunctionDecl(decl_), ccloc.context_, ccloc.loc_, argsRange);
    if (res) {
        res.setContext(ccloc.context_);
        return res;
    }

    // Generate a function call
    // Use this wrapper to obey the return-value conventions of the function
    res = createFunctionCall(ccloc.loc_, ccloc.context_, decl_, argsRange);

    // If this callable is a class-ctor, wrap the exiting result in a temp-var construct
    if (res && tmpVar_)
        res = createTempVarConstruct(ccloc.loc_, ccloc.context_, res, tmpVar_);

    return res;
}

int FunctionCallable::numParams() const {
    int res = params_.size();
    return implicitArgType_ ? res - 1 : res;
}

Type FunctionCallable::paramType(int idx) const {
    // If we have an implicit arg type, hide it
    if (implicitArgType_)
        ++idx;

    ASSERT(idx < params_.size());
    auto param = params_[idx];
    ASSERT(param);
    return param.type();
}

string FunctionCallable::toString() const { return declToStringClassic(decl_, params_); }
} // namespace SprFrontend
