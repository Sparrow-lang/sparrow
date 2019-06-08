#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/GenericFunctionCallable.h"
#include "SparrowFrontend/Services/Callable/CallableHelpers.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Helpers/Ct.h"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/SprDebug.h"

#include "Nest/Utils/cppif/SmallVector.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

namespace {

//! Get the eval mode to be applied to the instantiated function
EvalMode getFinalEvalMode(NodeRangeT<ParameterDecl> genericParams, NodeRange args,
        EvalMode origEvalMode, bool isCtGeneric) {
    if (isCtGeneric)
        return modeCt; // If we have a CT generic, the resulting eval mode is always CT

    // Deduce the resulting eval mode by looking at the parameters of the
    // generic function.
    //
    // This will look at all the 'Type' bound values, and at all the types
    // of the concept params. For
    // all these types, we check if the types are RT only or CT only.
    // If all the types checked are RT- or CT- only, we return the
    // corresponding mode.
    // If not, return the original eval mode.
    bool hasCtOnlyArgs = false;
    auto numGenericParams = genericParams.size();
    ASSERT(args.size() == numGenericParams);
    for (int i = 0; i < numGenericParams; ++i) {
        auto arg = args[i];
        // Test concept and non-bound arguments
        // Also test the type given to the 'Type' parameters (i.e., we need to know if Vector(t) can
        // be rtct based on the mode of t)
        auto genParam = genericParams[i];
        Type pType = genParam ? genParam.type() : Type();
        Type typeToCheck;
        if (!pType || isConceptType(pType)) {
            typeToCheck = arg.computeType();
        } else {
            // Is the argument a Type?
            typeToCheck = tryGetTypeValue(arg);
        }
        if (typeToCheck) {
            if (!typeToCheck.canBeUsedAtRt())
                hasCtOnlyArgs = true;
        }
    }

    if (hasCtOnlyArgs)
        return modeCt;
    return origEvalMode;
}

Type getParamType(ParameterDecl param, NodeHandle prevBoundVal, CompilationContext* boundsVarCtx) {
    // Simple case: this is not a dependent param
    Type paramType = param.type();
    if (paramType)
        return paramType;

    // This is a dependent param

    // If we are reusing an existing instance, try to get its type from previous bound value
    if (prevBoundVal) {
        paramType = getType(prevBoundVal);
        if (paramType)
            return paramType;
    }

    // We have to compute the type of the dependent param
    if (boundsVarCtx) {
        auto typeNode = param.typeNode().clone();
        typeNode.setContext(boundsVarCtx);
        if (!typeNode.computeType())
            return {};
        paramType = getType(typeNode);
        ASSERT(paramType);
    }
    return paramType;
}

NodeHandle getBoundVal(NodeHandle arg, ParameterDecl param, Type paramType, EvalMode origEvalMode,
        EvalMode finalEvalMode) {
    NodeHandle boundVal;

    // Compute the type of the bound value
    Type boundValType;
    bool isCtConcept = false;
    int numRefs = 0;
    int kind = typeKindData;
    if (!param.type()) {
        // If we are here this is a dependent param
        //
        // Conditions for considering this a concept param:
        // - param is non-CT
        // - fun was originally RTCT, and now turned to CT
        //
        // A concept param will ensure the creation of a final param.
        if (paramType.mode() != modeCt || (origEvalMode == modeRt && finalEvalMode == modeCt))
            boundValType = paramType;
    } else if (isConceptType(paramType, numRefs, kind)) {
        // Deduce the type for boundVal for regular concept types
        boundValType = getAutoType(arg, numRefs, kind, paramType.mode());
        isCtConcept = paramType.mode() == modeCt;
    }

    // Now actually create the bound value, of the proper type
    if (boundValType && !isCtConcept) {
        // then the bound value will be a type node
        boundVal = createTypeNode(arg.context(), param.location(), boundValType);
        if (!boundVal.computeType())
            REP_INTERNAL(arg.location(), "Cannot compute type of generated bound value");
    } else {
        // Evaluate the node and add the resulting CtValue as a bound argument
        ASSERT(Feather_isCt(arg));
        boundVal = Nest_ctEval(arg);
        if (!boundVal /*|| boundVal.kind() != nkFeatherExpCtValue*/)
            REP_INTERNAL(arg.location(), "Generated bound value is not CT evaluable");
        ASSERT(boundVal);
        ASSERT(boundVal.type());
    }
    return boundVal;
}

} // namespace

GenericFunctionCallable::GenericFunctionCallable(
        GenericFunction decl, TypeWithStorage implicitArgType)
    : Callable(decl)
    , implicitArgType_(implicitArgType)
    , params_(decl.originalParams())
    , autoCt_(decl.hasProperty(propAutoCt)) {}

ConversionType GenericFunctionCallable::canCall(const CCLoc& ccloc, NodeRange args,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {

    // This can be called a second time to report the errors
    genericInst_ = Instantiation();
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

    // Check argument count
    int paramsCount = params_.size();
    if (paramsCount != args_.size()) {
        if (reportErrors) {
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") % args_.size() %
                    paramsCount;
        }
        return convNone;
    }

    // Check if we need to force CT call
    bool forceCt = shouldUseCt(decl_, autoCt_, args_, evalMode);
    auto genFun = GenericFunction(decl_);
    bool isCtGeneric = genFun.original().hasProperty(propCtGeneric);
    InstantiationsSet instSet = genFun.instSet();
    EvalMode origEvalMode = genFun.original().effectiveMode();
    EvalMode finalEvalMode = getFinalEvalMode(
            instSet.params(), NodeRangeT<NodeHandle>(args_), origEvalMode, isCtGeneric);

    IterativeInstantiationBuilder instBuilder(instSet, params_.size(), finalEvalMode, isCtGeneric);

    ConversionType worstConv = convDirect;
    for (int i = 0; i < paramsCount; ++i) {
        // Get the current parameter type
        // Deduce parameter type if needed; reuse the param type if we are reusing the inst
        Type paramType = getParamType(
                params_[i], instBuilder.existingBoundVal(i), instBuilder.boundVarContext());
        if (!paramType) {
            if (reportErrors)
                REP_INFO(params_[i].location(), "Cannot compute deduced type for parameter %1%") %
                        (i + 1);
            return convNone;
        }

        Type argType = args_[i].type();
        ASSERT(argType);
        NodeHandle& arg = args_[i];
        ASSERT(arg.type());

        // Apply the conversion
        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        arg = applyConversion(arg, paramType, worstConv, flags, forceCt);
        if (!worstConv || !arg || !arg.computeType()) {
            if (reportErrors)
                REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % (i + 1) % argType %
                        paramType;
            return convNone;
        }
        arg.semanticCheck();

        // Handle generic params
        auto param = instSet.params()[i];
        if (param) {
            // Get the bound value that we need for this generic param
            NodeHandle boundVal = getBoundVal(arg, param, paramType, origEvalMode, finalEvalMode);
            ASSERT(boundVal && boundVal.type());

            // Iteratively build our instantiation
            instBuilder.addBoundVal(i, boundVal, param, paramType);
        }
    }

    Instantiation curInst = instBuilder.inst();
    if (!curInst) {
        REP_INTERNAL(decl_.location(), "Failed to generate an instantiation for %1%") %
                Nest_toStringEx(decl_);
    }
    // Check if we can instantiate this
    if (!canInstantiate(curInst, instSet)) {
        if (reportErrors)
            REP_INFO(NOLOC, "Cannot instantiate generic function (if clause not satisfied)");
        return convNone;
    }
    genericInst_ = curInst;

    return worstConv;
}
ConversionType GenericFunctionCallable::canCall(const CCLoc& ccloc, Range<Type> argTypes,
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
    bool useCt = shouldUseCt(decl_, autoCt_, argTypesToUse, evalMode);

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

    Nest::SmallVector<ConversionResult> conversions;
    ConversionType res = checkTypeConversions(
            conversions, ccloc, argTypesToUse, paramTypes, customCvtMode, reportErrors);

    return res;
}

NodeHandle GenericFunctionCallable::generateCall(const CCLoc& ccloc) {
    ASSERT(genericInst_);

    // If not already created, create the actual instantiation declaration
    auto instDecl = genericInst_.instantiatedDecl().kindCast<SprFunctionDecl>();
    if (!instDecl) {
        SmallVector<ParameterDecl> finalParams;
        createFinalParams(finalParams);
        instDecl = createInstantiatedFunction(NodeRangeT<ParameterDecl>(finalParams));
        if (!instDecl)
            REP_INTERNAL(ccloc.loc_, "Cannot instantiate function generic");
        if (!instDecl.computeType())
            return {};
        Nest_queueSemanticCheck(instDecl);
        genericInst_.setInstantiatedDecl(instDecl);
    }

    // Now actually create the call object
    SmallVector<NodeHandle> finalArgs;
    getFinalArgs(finalArgs);
    NodeHandle resultingFun = instDecl.explanation();
    if (!resultingFun)
        REP_ERROR_RET(nullptr, ccloc.loc_, "Cannot instantiate function generic %1%") %
                instDecl.name();
    NodeHandle res = createFunctionCall(
            ccloc.loc_, ccloc.context_, resultingFun, NodeRangeT<NodeHandle>(finalArgs));

    if (!res)
        REP_INTERNAL(ccloc.loc_, "Cannot create code that calls generic function");
    res.setContext(ccloc.context_);

    // If this callable is a class-ctor, wrap the exiting result in a temp-var construct
    if (tmpVar_)
        res = createTempVarConstruct(ccloc.loc_, ccloc.context_, res, tmpVar_);

    return res;
}

int GenericFunctionCallable::numParams() const {
    int res = params_.size();
    return implicitArgType_ ? res - 1 : res;
}

Type GenericFunctionCallable::paramType(int idx) const {
    // If we have an implicit arg type, hide it
    if (implicitArgType_)
        ++idx;

    ASSERT(idx < params_.size());
    auto param = params_[idx];
    ASSERT(param);
    return param.type();
}

string GenericFunctionCallable::toString() const { return genericToStringClassic(decl_, params_); }

void GenericFunctionCallable::createFinalParams(SmallVector<ParameterDecl>& finalParams) {

    auto genericParams = GenericFunction(decl_).instSet().params();

    auto boundValues = genericInst_.boundValues();
    ASSERT(boundValues.size() != 0);
    auto numParams = genericParams.size();
    finalParams.clear();
    for (int i = 0; i < numParams; ++i) {
        ParameterDecl param = genericParams[i];
        // If a parameter is not generic, it must be final => add final param
        if (!param)
            finalParams.push_back(params_[i].clone());
        else {
            // For concept-type parameters, we also create a final parameter
            auto boundValue = boundValues[i];
            bool isConcept = isConceptParam(param.type(), boundValue);
            if (isConcept)
                finalParams.push_back(
                        ParameterDecl::create(param.location(), param.name(), boundValue));
        }
    }
}

SprFunctionDecl GenericFunctionCallable::createInstantiatedFunction(
        NodeRangeT<ParameterDecl> finalParams) {
    SprFunctionDecl orig = GenericFunction(decl_).original();

    // Place the instantiated function in the context where we can access the bound variables
    CompilationContext* ctx = genericInst_.boundVarsNode().childrenContext();
    const Location& loc = orig.location();

    auto parameters = Feather::NodeList::create(loc, finalParams);
    auto returnType = orig.returnType();
    auto body = orig.body();
    returnType = returnType ? returnType.clone() : NodeHandle();
    body = body ? body.clone() : NodeHandle{};
    auto newFun = SprFunctionDecl::create(loc, orig.name(), parameters, returnType, body);

    copyModifiersSetMode(orig, newFun, ctx->evalMode);
    copyAccessType(newFun, orig);
    copyOverloadPrio(orig, newFun);
    Feather_setShouldAddToSymTab(newFun, 0);
    newFun.setContext(ctx);
    // printNode(newFun);

    return newFun;
}

void GenericFunctionCallable::getFinalArgs(SmallVector<NodeHandle>& finalArgs) {

    auto genericParams = GenericFunction(decl_).instSet().params();
    auto boundValues = genericInst_.boundValues();

    for (int i = 0; i < args_.size(); ++i) {
        ParameterDecl param = genericParams[i];
        // If a parameter is not generic, it must be final => add final arg
        if (!param)
            finalArgs.push_back(args_[i]);
        else {
            // Also add final arg for concept params
            auto boundValue = boundValues[i];
            if (isConceptParam(param.type(), boundValue))
                finalArgs.push_back(args_[i]);
        }
    }
}

} // namespace SprFrontend
