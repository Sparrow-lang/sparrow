#include <StdInc.h>
#include "Callable.h"
#include "Helpers/Impl/Intrinsics.h"
#include "Helpers/SprTypeTraits.h"
#include "Helpers/CommonCode.h"
#include "Helpers/Generics.h"
#include "Helpers/StdDef.h"
#include "Helpers/DeclsHelpers.h"
#include <Helpers/Ct.h>
#include "SparrowFrontendTypes.h"
#include "SprDebug.h"

using namespace SprFrontend;
using namespace Nest;

namespace {

////////////////////////////////////////////////////////////////////////////////
// Callables
//

/// Return the number of parameters that we have.
/// The last flag indicates whether we should hide or not the implicit param
/// If we hide it, we return the number of parameters the caller sees.
int getParamsCount(const CallableData& c, bool hideImplicit = false) {
    int res = size(c.params);
    // If we have an implicit arg type, hide it
    if (hideImplicit && c.implicitArgType) {
        ASSERT(res > 0);
        res--;
    }
    return res;
}

/// Get the type of parameter with the given index
/// The last flag indicates whether we should hide or not the implicit param
/// If we hide it, we return only the parameters the caller sees.
TypeRef getParamType(const CallableData& c, int idx, bool hideImplicit = false) {
    if (c.type == CallableType::concept)
        return getConceptType();
    // If we have an implicit arg type, hide it
    if (hideImplicit && c.implicitArgType)
        ++idx;
    ASSERT(idx < size(c.params));
    Node* param = at(c.params, idx);
    ASSERT(param);
    TypeRef res = param ? param->type : nullptr;
    // For generics, null params mean dependent types, so we replace it with 'any-type'
    // if (!res && (c.type==CallableType::genericFun || c.type == CallableType::genericClass))
    //     res = getConceptType();
    return res;
}

bool completeArgsWithDefaults(CallableData& c, NodeRange args) {
    // Copy the list of arguments; add default values if arguments are missing
    size_t paramsCount = getParamsCount(c);
    c.args = toVec(args);
    c.args.reserve(paramsCount);
    for (size_t i = Nest_nodeRangeSize(args); i < paramsCount; ++i) {
        Node* param = at(c.params, i);
        Node* defaultArg = param && param->nodeKind == nkSparrowDeclSprParameter
                                   ? at(param->children, 1)
                                   : nullptr;
        if (!defaultArg)
            return false; // We have a non-default parameter but we don't have an argument for that
        if (!Nest_semanticCheck(defaultArg)) // Make sure this is semantically checked
            return false;

        c.args.push_back(defaultArg);
    }
    return true;
}
// TODO (overloading): We are missing default args for the types-only checks

/**
 * Checks the eval mode for the given callable.
 *
 * Returns false if the eval mode is not ok.
 *
 * Also checks the 'autoCt' mode. If autoCt is true, and one of the arg types is not CT, this will
 * reset autoCt to false.
 * After calling this function whenever 'autoCt' is true, we should call CT.
 *
 * @param c            The callable we are operating on
 * @param argTypes     The types of the arguments passed in
 * @param evalMode     The eval mode desired for calling the callable
 * @param reportErrors True if we should report errors
 *
 * @return True if the callable can be used in the given eval mode
 */
bool checkEvalMode(
        CallableData& c, const vector<TypeRef>& argTypes, EvalMode evalMode, bool reportErrors) {

    // Nothing to check for concepts
    if (c.type == CallableType::concept)
        return true;

    // Check if eval-mode allows us to call the callable
    EvalMode declEvalMode = Feather_effectiveEvalMode(c.decl);
    if (declEvalMode == modeRt && (evalMode == modeCt || evalMode == modeRtCt)) {
        if (reportErrors)
            REP_INFO(NOLOC, "Cannot call RT only functions in CT and RTCT contexts");
        return false;
    }

    // Compute the final version of autoCt flag
    bool useCt = declEvalMode == modeCt || evalMode == modeCt;
    if (declEvalMode == modeRtCt && c.autoCt) {
        // In autoCt mode, if all the arguments are CT, make a CT call
        useCt = true;
        for (TypeRef t : argTypes) {
            if (t->mode != modeCt) {
                useCt = false;
                break;
            }
        }
    }
    c.autoCt = useCt;
    return true;
}

ConversionType canCall_common_types(CallableData& c, CompilationContext* context,
        const vector<TypeRef>& argTypes, EvalMode evalMode, CustomCvtMode customCvtMode,
        bool reportErrors) {
    size_t paramsCount = getParamsCount(c);

    c.conversions.resize(paramsCount, convNone);

    bool isGeneric = c.type == CallableType::genericFun || c.type == CallableType::genericClass;

    ConversionType res = convDirect;
    for (size_t i = 0; i < paramsCount; ++i) {
        TypeRef argType = argTypes[i];
        ASSERT(argType);
        TypeRef paramType = getParamType(c, i);
        if (!paramType) {
            if (!isGeneric) {
                // For generics, we allow null parameters for now
                if (reportErrors)
                    REP_INFO(NOLOC, "Bad parameter %1%; arg type: %2%") % i % argType;
                return convNone;
            }
            // continue;   // Will get back on this arg in a secondary pass
            paramType = getConceptType();
        }

        // If we are looking at a CT callable, make sure the parameters are in CT
        if (paramType->hasStorage && c.autoCt)
            paramType = Feather_checkChangeTypeMode(paramType, modeCt, NOLOC);

        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        c.conversions[i] = canConvertType(context, argType, paramType, flags);
        if (!c.conversions[i]) {
            if (reportErrors)
                REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % i % argType %
                        paramType;
            return convNone;
        } else if (c.conversions[i].conversionType() < res)
            res = c.conversions[i].conversionType();
    }

    return res;
}

NodeVector argsWithConversion(const CallableData& c) {
    NodeVector res(c.args.size(), nullptr);
    for (size_t i = 0; i < c.args.size(); ++i)
        res[i] = c.conversions[i].apply(c.args[i]->context, c.args[i]);
    return res;
}
TypeRef varType(Node* cls, EvalMode mode) {
    // Get the type of the temporary variable
    TypeRef t = cls->type;
    if (mode != modeRtCt)
        t = Feather_checkChangeTypeMode(t, mode, cls->location);
    return t;
}

CallableData mkFunCallable(Node* fun, TypeRef implicitArgType = nullptr) {
    NodeRange params = Feather_Function_getParameters(fun);
    if (getResultParam(fun))
        params.beginPtr++; // Always hide the result param
    ASSERT(params.beginPtr <= params.endPtr);

    CallableData res;
    res.type = CallableType::function;
    res.decl = fun;
    res.params = params;
    res.autoCt = Nest_hasProperty(fun, propAutoCt);
    res.implicitArgType = implicitArgType;
    return res;
}
CallableData mkGenericFunCallable(Node* genericFun, TypeRef implicitArgType = nullptr) {
    CallableData res;
    res.type = CallableType::genericFun;
    res.decl = genericFun;
    res.params = genericFunParams(genericFun);
    res.implicitArgType = implicitArgType;
    return res;
}
CallableData mkGenericClassCallable(Node* genericClass) {
    CallableData res;
    res.type = CallableType::genericClass;
    res.decl = genericClass;
    res.params = genericClassParams(genericClass);
    return res;
}
CallableData mkConceptCallable(Node* concept) {
    CallableData res;
    res.type = CallableType::concept;
    res.decl = concept;
    res.params = fromIniList({nullptr});
    return res;
}

//! Checks if the given decl satisfies the predicate
//! If the predicate is empty, the decl always satisfies it
bool predIsSatisfied(Node* decl, const boost::function<bool(Node*)>& pred) {
    return pred.empty() || pred(decl);
}

//! Get the class-ctor callables corresponding to the given class
void getClassCtorCallables(Node* cls, EvalMode evalMode, Callables& res,
        const boost::function<bool(Node*)>& pred, const char* ctorName) {
    // Search for the ctors associated with the class
    NodeArray decls = getClassAssociatedDecls(cls, ctorName);

    evalMode = Feather_combineMode(Feather_effectiveEvalMode(cls), evalMode, cls->location);
    TypeRef implicitArgType = varType(cls, evalMode);

    res.reserve(res.size() + Nest_nodeArraySize(decls));
    for (Node* decl : decls) {
        Node* fun = Nest_explanation(decl);
        if (fun && fun->nodeKind == nkFeatherDeclFunction && predIsSatisfied(decl, pred))
            res.push_back(mkFunCallable(fun, implicitArgType));

        Node* resDecl = resultingDecl(decl);
        if (resDecl->nodeKind == nkSparrowDeclGenericFunction && predIsSatisfied(decl, pred))
            res.push_back(mkGenericFunCallable(resDecl, implicitArgType));
        else if (resDecl->nodeKind == nkSparrowDeclGenericClass && predIsSatisfied(decl, pred))
            res.push_back(mkGenericClassCallable(resDecl));
    }
    Nest_freeNodeArray(decls);
}

////////////////////////////////////////////////////////////////////////////////
// Generic function
//

/**
 * Get the resulting eval mode for a generic class instantiation.
 *
 * This will be used for the instantiation of the generic function.
 *
 * This will look at all the 'Type' bound values, and at all the types of the concept params. For
 * all these types, we check if the types are RT only or CT only.
 *
 * If all the types checked are RT- or CT- only, we return the corresponding mode.
 * If not, return the mainEvalMode.
 *
 * @param loc           The location of the generic; used for error reporting
 * @param mainEvalMode  The effective eval mode of the generic function decl
 * @param args          The args used to perform the instantiation
 * @param genericParams The generic params of the instSet
 *
 * @return The eval mode to be used for instantiation.
 */
EvalMode getGenericFunResultingEvalMode(
        const Location& loc, EvalMode mainEvalMode, NodeRange args, NodeRange genericParams) {
    bool hasRtOnlyArgs = false;
    bool hasCtOnlyArgs = false;
    auto numGenericParams = Nest_nodeRangeSize(genericParams);
    ASSERT(size(args) == numGenericParams);
    for (size_t i = 0; i < numGenericParams; ++i) {
        Node* arg = at(args, i);
        // Test concept and non-bound arguments
        // Also test the type given to the 'Type' parameters (i.e., we need to
        // know if Vector(t) can be rtct based on the mode of t)
        Node* genParam = at(genericParams, i);
        TypeRef pType = genParam ? genParam->type : nullptr;
        TypeRef typeToCheck = nullptr;
        if (!pType || isConceptType(pType)) {
            typeToCheck = Nest_computeType(arg);
        } else {
            // Is the argument a Type?
            typeToCheck = tryGetTypeValue(arg);
        }
        if (typeToCheck) {
            if (!typeToCheck->canBeUsedAtCt)
                hasRtOnlyArgs = true;
            else if (!typeToCheck->canBeUsedAtRt)
                hasCtOnlyArgs = true;
        }
    }

    if (hasCtOnlyArgs)
        return modeCt;
    if (hasRtOnlyArgs)
        return modeRt;
    return mainEvalMode;
}

/**
 * Get the list of final params that shouls be used in an instantiated function.
 *
 * There are two types of final parameters: the RT params and the concept params.
 * The RT params are cloned directly to the list of final params.
 * For the concept params, we create new params that have the type from the bound value
 * corresponding to that param.
 *
 * This function will not return the this parameter.
 *
 * @param inst          The instantiation node
 * @param origFun       The original function node.
 * @param params        The original list of params of the function
 * @param genericParams The generic params of the function; nulls for all the non-bound params
 *
 * @return The list of nodes with the final params to be set for the instantiated function.
 */
NodeVector getGenericFunFinalParams(
        InstNode inst, Node* origFun, NodeRange params, NodeRange genericParams) {
    auto boundValues = inst.boundValues();
    ASSERT(Nest_nodeRangeSize(boundValues) != 0);
    auto numParams = Nest_nodeRangeSize(params);
    NodeVector finalParams;
    finalParams.reserve(numParams);
    for (size_t i = 0; i < numParams; ++i) {
        if (i == 0 && funHasImplicitThis(origFun))
            continue;

        Node* p = at(params, i);
        Node* boundValue = at(boundValues, i);

        if (!at(genericParams, i)) {
            // If this is not a generic parameter => final-bound parameter
            finalParams.push_back(Nest_cloneNode(p));
        } else if (!p->type || isConceptType(p->type)) {
            ASSERT(boundValue && boundValue->type);
            // For concept-type parameters, we also create a final-bound parameter
            finalParams.push_back(mkSprParameter(p->location, Feather_getName(p), boundValue));
        }
    }
    return finalParams;
}

/**
 * Get the list of final arguments for calling the instantiated function.
 *
 * We keep only the arguments corresponding toe the final params.
 * All the arguments corresponding to CT params are dropped.
 *
 * @param args          The initial list of arguments to be filtered
 * @param genericParams The generic params to know which argument to filter
 *
 * @return The list of final args that should be used to call the instantiated function
 */
NodeVector getGenericFunFinalArgs(NodeRange args, NodeRange genericParams) {
    NodeVector finalArgs;
    finalArgs.reserve(size(args));
    for (size_t i = 0; i < size(args); ++i) {
        Node* param = at(genericParams, i);
        if (!param || !param->type || isConceptType(param->type)) {
            // Get non-generic and also concept parameters
            finalArgs.push_back(at(args, i));
        }
    }
    return finalArgs;
}

/**
 * Create the instantiated function.
 *
 * This clones the original function. It will replace the list of parameters with the final params.
 *
 * @param context     The context to place the instantiated function
 * @param origFun     The original function to be copied
 * @param finalParams The list of final params to be used
 *
 * @return [description]
 */
Node* createInstFn(CompilationContext* context, Node* origFun, NodeRange finalParams) {
    const Location& loc = origFun->location;

    // REP_INFO(loc, "Instantiating %1% with %2% params") %
    // Feather_getName(origFun) % finalParams.size();

    Node* parameters = Feather_mkNodeList(loc, finalParams);
    Node* returnType = at(origFun->children, 1);
    Node* body = at(origFun->children, 2);
    returnType = returnType ? Nest_cloneNode(returnType) : nullptr;
    body = body ? Nest_cloneNode(body) : nullptr;
    Node* newFun = mkSprFunction(loc, Feather_getName(origFun), parameters, returnType, body);
    copyModifiersSetMode(origFun, newFun, context->evalMode);
    copyAccessType(newFun, origFun);
    Feather_setShouldAddToSymTab(newFun, 0);
    Nest_setContext(newFun, context);

    // REP_INFO(loc, "Instantiated %1%") % newFun->toString();
    return newFun;
}

/**
 * Creates the code that actually calls an instantiated function
 *
 * @param loc       The location of the call code
 * @param context   The context in which the call code should be placed
 * @param inst      The instantiation that we want to call
 * @param finalArgs The list of final arguments to be used for the call
 *
 * @return Node representing the calling code for the instantiated function
 */
Node* createCallFn(
        const Location& loc, CompilationContext* context, Node* inst, NodeRange finalArgs) {
    ASSERT(inst && inst->nodeKind == nkSparrowDeclSprFunction);
    if (!Nest_computeType(inst))
        return nullptr;
    Node* resultingFun = Nest_explanation(inst);
    if (!resultingFun)
        REP_ERROR_RET(nullptr, loc, "Cannot instantiate function generic %1%") %
                Feather_getName(inst);
    return createFunctionCall(loc, context, resultingFun, finalArgs);
}

/**
 * Generate the code to call a generic function.
 *
 * Given a partial instantiation that we already know we can call, this will generate the code to
 * call that function. If we haven't already fully instantiated the generic (i.e., create the
 * instantiated function), we do it now.
 *
 * @param node          The generic function node
 * @param loc           The location of the call site
 * @param context       The context of the calling code
 * @param args          The arguments we use to call the generic
 * @param instantiation The instantiation node that we want to call
 *
 * @return The call code
 */
Node* callGenericFun(GenericFunNode node, const Location& loc, CompilationContext* context,
        NodeRange args, InstNode inst) {
    // If not already created, create the actual instantiation declaration
    Node* instDecl = inst.instantiatedDecl();
    if (!instDecl) {
        Node* originalFun = node.originalFun();
        ASSERT(originalFun->nodeKind == nkSparrowDeclSprFunction);
        NodeVector finalParams = getGenericFunFinalParams(
                inst, originalFun, node.originalParams(), node.instSet().params());

        // Create the actual instantiation declaration
        CompilationContext* ctx = Nest_childrenContext(inst.boundVarsNode());
        instDecl = createInstFn(ctx, originalFun, all(finalParams));
        if (!instDecl)
            REP_INTERNAL(loc, "Cannot instantiate generic");
        if (!Nest_computeType(instDecl))
            return nullptr;
        Nest_queueSemanticCheck(instDecl);
        inst.setInstantiatedDecl(instDecl);
    }

    // Now actually create the call object
    NodeVector finalArgs = getGenericFunFinalArgs(args, node.instSet().params());
    Node* res = createCallFn(loc, context, instDecl, all(finalArgs));
    if (!res)
        REP_INTERNAL(loc, "Cannot create code that calls generic");
    return res;
}

/**
 * Apply the conversion from the given arg, to the appropriate param type.
 *
 * @param arg       The argument to be converted
 * @param paramType The type to be converted to
 * @param worstConv [in/out] Keeps track of the worst conversion seen
 * @param flags     The conversion flags to be used
 * @param autoCt    True if we need to force dest type to be CT
 *
 * @return The argument with the appropriate conversion applied
 */
Node* applyConversion(Node* arg, TypeRef paramType, ConversionType& worstConv,
        ConversionFlags flags = flagsDefault, bool autoCt = false) {
    ASSERT(arg->type);

    // If we are looking at a CT callable, make sure the parameters are in CT
    if (paramType->hasStorage && autoCt)
        paramType = Feather_checkChangeTypeMode(paramType, modeCt, NOLOC);

    ConversionResult conv = canConvertType(arg->context, arg->type, paramType, flags);
    if (!conv) {
        // if (reportErrors)
        //     REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % i % argType %
        //             paramType;
        return nullptr;
    } else if (conv.conversionType() < worstConv)
        worstConv = conv.conversionType();

    // Apply the conversion to our arg
    // We are not interested in the original arg anymore
    return conv.apply(arg->context, arg);
}

/**
 * Get the deduced type for a dependent parameter.
 *
 * @param idx               The index of the type dependent parameter
 * @param instSet           The instSet we are operating with
 * @param curInst           The current instantiation
 * @param reuseExistingInst True if we are reusing an existing instantiation
 *
 * @return The type to be applied for the parameter
 */
TypeRef getDeducedType(int idx, InstSetNode instSet, InstNode curInst, bool reuseExistingInst) {
    TypeRef paramType = nullptr;

    ASSERT(curInst);
    if (reuseExistingInst) {
        // We can get the type of this parameter from the current inst
        // The dependent type should be the same for all the instantiations that start with the same
        // bound values.
        Node* otherBoundVal = at(curInst.boundValues(), idx);
        paramType = otherBoundVal->type;
    }
    if (!paramType) {
        // Add the appropriate bound variable to the instantiation
        // We will get our deduced type by computing the type of this var
        Node* param = at(instSet.params(), idx);
        ASSERT(param);
        Node* typeNode = Nest_cloneNode(at(param->children, 0));
        Nest_setContext(typeNode, curInst.boundVarsNode()->context);
        if (!Nest_computeType(typeNode))
            return nullptr;
        paramType = getType(typeNode);
        ASSERT(paramType);
    }
    return paramType;
}

/**
 * Get the bound value for the given parameter.
 *
 * @param arg   The argument to deduce the bound value from
 * @param param The parameter to get the bound value for
 *
 * @return The bound value corresponding to the given argument/parameter
 */
Node* getBoundValue(Node* arg, Node* param) {
    bool isRefAuto = false;
    if (!param->type || isConceptType(param->type, isRefAuto)) {
        // Create a CtValue with the type of the argument corresponding to
        // the auto parameter
        TypeRef t = getAutoType(arg, isRefAuto);
        Node* typeNode = createTypeNode(arg->context, param->location, t);
        if (!Nest_computeType(typeNode))
            return nullptr;
        return typeNode;
    } else {
        // Evaluate the node and add the resulting CtValue as a bound argument
        if (!Feather_isCt(arg))
            return nullptr;
        Node* boundVal = Nest_ctEval(arg);
        if (!boundVal || boundVal->nodeKind != nkFeatherExpCtValue)
            return nullptr;
        ASSERT(boundVal->type);
        return boundVal;
    }
}

/**
 * Called to handle a generic parameter.
 *
 * This is called for each parameter/arg of a generic to handle the generic parameters. We perform
 * the following actions:
 *     - compute the bound value for generic params
 *     - check if we can reuse an existing instantiation
 *     - if we need to create a new instantiation, create it
 *     - create the bound variable (not reusing inst case)
 *     - update the vector of bound values
 *
 * @param [in] idx                   The index of the parameter/argument we are checking
 * @param [in] arg                   The argument given when calling the generic
 * @param [in/out] boundValues       The vector that contains the bound values that we have
 * @param [in] instSet               The instantiations set
 * @param [in/out] curInst           The inst we are currently using; can be reused or not
 * @param [in/out] reuseExistingInst Indicates if we are reusing an existing instantiation
 * @param [in] resultingEvalMode     The evalMode for the resulting generic
 * @param [in] insideClass           True if we are inside a class
 */
void handleGenericFunParam(int idx, Node* arg, NodeVector& boundValues, InstSetNode instSet,
        InstNode& curInst, bool& reuseExistingInst, EvalMode resultingEvalMode, bool insideClass) {
    Node* param = at(instSet.params(), idx);
    if (!param)
        return; // nothing to do

    // First compute the bound value for the current generic parameter
    Node* boundVal = getBoundValue(arg, param);
    if (!boundVal)
        REP_INTERNAL(arg->location, "Invalid argument %1% when instantiating generic") % (idx + 1);
    ASSERT(boundVal && boundVal->type);
    boundValues[idx] = boundVal;

    // Now, select the appropriate inst for the new bound value
    if (reuseExistingInst) {
        // First, check if we can continue the existing inst
        if (curInst.node && !ctValsEqual(boundVal, at(curInst.boundValues(), idx))) {
            curInst = InstNode(nullptr);
        }
        // If the current instantiation is not valid, try to search another
        if (!curInst.node) {
            curInst = searchInstantiation(instSet, subrange(boundValues, 0, idx+1));
            reuseExistingInst = curInst.node != nullptr;
        }
    }
    if (!reuseExistingInst) {
        // This is a new instantiation; check if we have to create it
        if (!curInst.node) {
            curInst = createNewInstantiation(instSet, all(boundValues), resultingEvalMode);
            ASSERT(curInst);
        } else {
            // Add the appropriate bound variable to the instantiation
            Node* boundVar = createBoundVar(param, boundVal, insideClass);
            Feather_addToNodeList(curInst.boundVarsNode(), boundVar);
            Nest_setContext(boundVar, curInst.boundVarsNode()->context);
            Nest_clearCompilationStateSimple(curInst.boundVarsNode());
        }
        at(curInst.boundValues(), idx) = boundVal;
    }
}

/**
 * Checks if we can call a generic function
 *
 * @param c             The callable data pointing to a generic function
 * @param context       The context in which we want to perform the call
 * @param loc           The location of the caller
 * @param args          The arguments used for calling the generic function
 * @param evalMode      The eval mode desired for the call
 * @param customCvtMode How to handle custom conversions
 * @param reportErrors  True if we have to report errors
 *
 * @return The worst conversion type that we find for all the arguments.
 */
ConversionType canCallGenericFun(CallableData& c, CompilationContext* context, const Location& loc,
        NodeRange args, EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    ASSERT(!c.genericInst);

    GenericFunNode genFun = c.decl;
    InstSetNode instSet = genFun.instSet();
    Node* originalFun = genFun.originalFun();

    bool insideClass = nullptr != Feather_getParentClass(c.decl->context);

    int paramsCount = getParamsCount(c);
    NodeVector boundValues;
    boundValues.resize(paramsCount, nullptr);

    // Compute the resulting eval mode
    int oldErrNum = Nest_getErrorsNum();
    EvalMode resultingEvalMode =
            Nest_hasProperty(originalFun, propCtGeneric)
                    ? modeCt // If we have a CT generic, the resulting eval mode is always CT
                    : getGenericFunResultingEvalMode(originalFun->location,
                              Feather_effectiveEvalMode(originalFun), all(c.args),
                              instSet.params());
    if ( Nest_getErrorsNum() != oldErrNum )
        REP_INFO(loc, "Resulting eval mode: %1%") % resultingEvalMode;

    // The currently working instance
    // We will use this to check/create bound vars, and to deduce the types of the dependent type
    // params. We can reuse previously created instantiations, or we can build our own inst. As much
    // as possible, reuse previously created instantiations.
    InstNode curInst(nullptr);
    bool reuseExistingInst = true;

    ConversionType worstConv = convDirect;
    for (int i = 0; i < paramsCount; ++i) {
        TypeRef argType = c.args[i]->type;
        ASSERT(argType);
        TypeRef paramType = getParamType(c, i);

        // If we don't have a param type stored in the instSet, this must be a dependent param
        // Compute its type based on the previous bound values.
        // Please note that the deduce type may or may not be another generic type
        if (!paramType) {
            paramType = getDeducedType(i, instSet, curInst, reuseExistingInst);
            if (!paramType) {
                if (reportErrors)
                    REP_INFO(NOLOC, "Cannot compute deduced type for parameter %1%") % (i + 1);
            }
        }

        Node*& arg = c.args[i];
        ASSERT(arg->type);

        // Apply the conversion
        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        arg = applyConversion(arg, paramType, worstConv, flags, c.autoCt);
        if (!arg || !Nest_computeType(arg)) {
            if (reportErrors)
                REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % (i + 1) % argType %
                        paramType;
            return convNone;
        }

        // Treat generic params
        handleGenericFunParam(i, arg, boundValues, instSet, curInst, reuseExistingInst,
                resultingEvalMode, insideClass);
    }
    // If for one arg there isn't a viable conversion, we can't call this
    if (!worstConv)
        return convNone;

    // Check if we can instantiate this
    if (!canInstantiate(curInst, instSet)) {
        if (reportErrors)
            REP_INFO(NOLOC, "Cannot instantiate generic function (if clause not satisfied)");
        return convNone;
    }
    c.genericInst = curInst.node;

    return worstConv;
}

////////////////////////////////////////////////////////////////////////////////
// Generic class
//

/**
 * Get the bound values for a generic class, from the given set of arguments.
 *
 * If any of the given arguments is not a CT value, we issue an error.
 * The assumption is that all the class parameters are CT.
 *
 * @param args The set of arguments passed when trying to instantiate the class.
 *
 * @return The vector of bound values
 */
NodeVector getGenericClassBoundValues(NodeRange args) {
    NodeVector boundValues;
    boundValues.reserve(size(args));

    for (size_t i = 0; i < size(args); ++i) {
        Node* arg = at(args, i);

        // Evaluate the node and add the resulting CtValue as a bound argument
        if (!Nest_computeType(arg))
            return {};
        if (!Feather_isCt(arg))
            REP_INTERNAL(arg->location, "Argument to a class generic must be CT (type: %1%)") %
                    arg->type;
        Node* n = Nest_ctEval(arg);
        if (!n || n->nodeKind != nkFeatherExpCtValue)
            REP_INTERNAL(arg->location, "Invalid argument %1% when instantiating generic") %
                    (i + 1);
        ASSERT(n && n->type);
        boundValues.push_back(n);
    }
    return boundValues;
}

/**
 * Get the resulting eval mode for a generic class instantiation.
 *
 * This will be used for the instantiation of the generic class.
 *
 * This checks all the bound values that are types. If all the bound types are CT types, then we
 * return a CT-only mode. If all the bound types are RT-only, then we return a RT-only mode.
 * For the rest of the cases (mixed modes, no types, etc.) We return the mainEvalMode value.
 *
 * @param loc          The location of the generic; used for error reporting
 * @param mainEvalMode The effective eval mode of the generic class decl
 * @param boundValues  The bound values to be used for instantiating the class generic
 *
 * @return The eval mode that should be used for the instantiation.
 */
EvalMode getGenericClassResultingEvalMode(
        const Location& loc, EvalMode mainEvalMode, NodeRange boundValues) {
    bool hasRtOnlyArgs = false;
    bool hasCtOnlyArgs = false;
    for (Node* boundVal : boundValues) {
        ASSERT(!boundVal || boundVal->type);
        // Test the type given to the 'Type' parameters (i.e., we need to know
        // if Vector(t) can be rtct based on the mode of t)
        TypeRef t = tryGetTypeValue(boundVal);
        if (t) {
            if (t->mode == modeRt)
                hasRtOnlyArgs = true;
            else if (t->mode == modeCt)
                hasCtOnlyArgs = true;
        } else if (!boundVal->type->canBeUsedAtRt) {
            hasCtOnlyArgs = true;
        }
    }

    if (hasCtOnlyArgs)
        return modeCt;
    if (hasRtOnlyArgs)
        return modeRt;
    return mainEvalMode;
}

/**
 * Check if we can instantiate the generic class with the given arguments.
 *
 * This will create a (partial) instantiation for the generic. If the instantiation is valid, then
 * it will return the instantiation node. If there are errors creating the partial instantiation, or
 * if evaluating the if clause yields false, this will return null.
 *
 * @param node The generic class we want to check if we can instantiate
 * @param args The list of arguments that we want to instantiate the class with.
 *
 * @return The instantiation node, or null if the instantiation is not valid
 */
Node* canInstantiateGenericClass(GenericClassNode node, NodeRange args) {
    NodeVector boundValues = getGenericClassBoundValues(args);
    Node* originalClass = node.originalClass();
    EvalMode resultingEvalMode = getGenericClassResultingEvalMode(
            originalClass->location, Feather_effectiveEvalMode(originalClass), all(boundValues));
    InstNode inst = canInstantiate(node.instSet(), all(boundValues), resultingEvalMode);
    return inst.node;
}

/**
 * Create the actual instantiated class.
 *
 * This will make a clone of the original class object. It make sure to clear any parameters and
 * returns the cloned node.s
 *
 * @param context     The context to be used for the instantiated class
 * @param orig        The original class declaration (that was a generic)
 *
 * @return The node of the instantiated class decl
 */
Node* createInstantiatedClass(CompilationContext* context, Node* orig) {
    const Location& loc = orig->location;

    Node* classChildren = at(orig->children, 1);
    classChildren = classChildren ? Nest_cloneNode(classChildren) : nullptr;
    Node* newClass =
            mkSprClass(loc, Feather_getName(orig), nullptr, nullptr, nullptr, classChildren);
    copyAccessType(newClass, orig);

    copyModifiersSetMode(orig, newClass, context->evalMode);

    // TODO (generics): Uncomment this line
    // Feather_setShouldAddToSymTab(newClass, 0);
    Nest_setContext(newClass, context);

    return newClass;
}

/**
 * Gets the description of a to-be-instantiated class.
 *
 * We use this description when generating the assembly name for the class.
 *
 * @param cls  The original class node
 * @param inst The inst node in which we are placing the instantiated node
 *
 * @return The description of the instantiated class
 */
string getGenericClassDescription(Node* originalClass, InstNode inst) {
    ostringstream oss;
    oss << toString(Feather_getName(originalClass)) << "[";
    auto boundValues = inst.boundValues();
    bool first = true;
    for (Node* bv : boundValues) {
        if (first)
            first = false;
        else
            oss << ", ";
        TypeRef t = evalTypeIfPossible(bv);
        if (t)
            oss << t;
        else
            oss << bv;
    }
    oss << "]";
    return oss.str();
}

/**
 * Generate the code to call a generic class.
 *
 * Given a partial instantiation that we already know we can call, this will generate the code to
 * call that class. If we haven't already fully instantiated the generic (i.e., create the
 * instantiated class), we do it now.
 *
 * @param node          The generic class node
 * @param loc           The location of the call site
 * @param context       The context of the calling code
 * @param args          The arguments we use to call the generic
 * @param instantiation The instantiation node that we want to call
 *
 * @return The call code
 */
Node* callGenericClass(GenericClassNode node, const Location& loc, CompilationContext* context,
        NodeRange args, InstNode inst) {

    // If not already created, create the actual instantiation declaration
    Node* instDecl = inst.instantiatedDecl();
    if (!instDecl) {
        Node* originalClass = Nest_ofKind(node.originalClass(), nkSparrowDeclSprClass);
        string description = getGenericClassDescription(originalClass, inst);

        // Create the actual instantiation declaration
        CompilationContext* ctx = Nest_childrenContext(inst.boundVarsNode());
        instDecl = createInstantiatedClass(ctx, originalClass);
        if (!instDecl)
            REP_INTERNAL(loc, "Cannot instantiate generic");
        Nest_setPropertyString(instDecl, propDescription, fromString(description));
        if (!Nest_computeType(instDecl))
            return nullptr;
        Nest_queueSemanticCheck(instDecl);
        inst.setInstantiatedDecl(instDecl);

        // Add the instantiated class as an additional node to the generic
        // node
        Nest_appendNodeToArray(&node.node->additionalNodes, inst.boundVarsNode());
        if (node.node->explanation && node.node->explanation != node)
            Nest_appendNodeToArray(&node.node->explanation->additionalNodes, inst.boundVarsNode());
    }

    // Now actually create the call object: a Type CT value
    Node* cls = Nest_ofKind(Nest_explanation(instDecl), nkFeatherDeclClass);
    ASSERT(cls);
    return createTypeNode(node.node->context, loc, Feather_getDataType(cls, 0, modeRtCt));
}
}

void SprFrontend::getCallables(NodeRange decls, EvalMode evalMode, Callables& res) {
    getCallables(decls, evalMode, res, boost::function<bool(Node*)>());
}

void SprFrontend::getCallables(NodeRange decls, EvalMode evalMode, Callables& res,
        const boost::function<bool(Node*)>& pred, const char* ctorName) {
    NodeArray declsEx = expandDecls(decls, nullptr);

    for (Node* decl : declsEx) {
        Node* node = decl;

        // If we have a resolved decl, get the callable for it
        if (node) {
            if (!Nest_computeType(node))
                continue;

            Node* decl = resultingDecl(node);
            if (!decl)
                continue;

            // Is this a normal function call?
            if (decl && decl->nodeKind == nkFeatherDeclFunction && predIsSatisfied(decl, pred))
                res.emplace_back(mkFunCallable(decl));

            // Is this a generic?
            else if (decl->nodeKind == nkSparrowDeclGenericFunction && predIsSatisfied(decl, pred))
                res.push_back(mkGenericFunCallable(decl));
            else if (decl->nodeKind == nkSparrowDeclGenericClass && predIsSatisfied(decl, pred))
                res.push_back(mkGenericClassCallable(decl));

            // Is this a concept?
            else if (decl->nodeKind == nkSparrowDeclSprConcept && predIsSatisfied(decl, pred))
                res.emplace_back(mkConceptCallable(decl));

            // Is this a temporary object creation?
            else if (decl->nodeKind == nkFeatherDeclClass) {
                getClassCtorCallables(decl, evalMode, res, pred, ctorName);
            }
        }
    }
}

ConversionType SprFrontend::canCall(CallableData& c, CompilationContext* context,
        const Location& loc, NodeRange args, EvalMode evalMode, CustomCvtMode customCvtMode,
        bool reportErrors) {
    NodeVector args2;

    // If this callable requires an added this argument, add it
    if (c.implicitArgType) {
        Node* thisTempVar =
                Feather_mkVar(loc, fromCStr("tmp.v"), Feather_mkTypeNode(loc, c.implicitArgType));
        Nest_setContext(thisTempVar, context);
        if (!Nest_computeType(thisTempVar)) {
            if (reportErrors)
                REP_INFO(loc, "Cannot create temporary variable");
            return convNone;
        }
        Node* thisArg = Feather_mkVarRef(loc, thisTempVar);
        Nest_setContext(thisArg, context);
        if (!Nest_computeType(thisArg)) {
            if (reportErrors)
                REP_INFO(loc, "Cannot compute the type of this argument");
            return convNone;
        }

        c.tmpVar = thisTempVar;

        args2 = toVec(args);
        args2.insert(args2.begin(), thisArg);
        args = all(args2);
    }

    // Complete the missing args with defaults
    if (!completeArgsWithDefaults(c, args))
        return convNone;

    // Check argument count (including hidden params)
    size_t paramsCount = getParamsCount(c);
    if (paramsCount != c.args.size()) {
        if (reportErrors)
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") %
                    c.args.size() % paramsCount;
        return convNone;
    }

    // Get the arg types to perform the check on types
    vector<TypeRef> argTypes(c.args.size(), nullptr);
    for (size_t i = 0; i < c.args.size(); ++i)
        argTypes[i] = c.args[i]->type;

    // Check evaluation mode
    if (!checkEvalMode(c, argTypes, evalMode, reportErrors))
        return convNone;

    if (c.type == CallableType::genericFun) {
        return canCallGenericFun(c, context, loc, args, evalMode, customCvtMode, reportErrors);
    }

    // Do the checks on types
    ConversionType res =
            canCall_common_types(c, context, argTypes, evalMode, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    if (c.type == CallableType::genericClass) {
        // Check if we can instantiate the generic with the given arguments
        // (with conversions applied)
        // Note: we overwrite the args with their conversions;
        // We don't use the old arguments anymore
        c.args = argsWithConversion(c);
        ASSERT(!c.genericInst);
        c.genericInst = canInstantiateGenericClass(c.decl, all(c.args));
        if (!c.genericInst && reportErrors) {
            REP_INFO(NOLOC, "Cannot instantiate generic class");
        }
        if (!c.genericInst)
            return convNone;
    }

    return res;
}
ConversionType SprFrontend::canCall(CallableData& c, CompilationContext* context,
        const Location& loc, const vector<TypeRef>& argTypes, EvalMode evalMode,
        CustomCvtMode customCvtMode, bool reportErrors) {
    vector<TypeRef> argTypes2;
    const vector<TypeRef>* argTypesToUse = &argTypes;

    // If this callable requires an added this argument, add it
    if (c.implicitArgType) {
        TypeRef t = Feather_getLValueType(c.implicitArgType);

        argTypes2 = argTypes;
        argTypes2.insert(argTypes2.begin(), t);
        argTypesToUse = &argTypes2;
    }

    // Check argument count (including hidden params)
    size_t paramsCount = getParamsCount(c);
    if (paramsCount != argTypesToUse->size()) {
        if (reportErrors)
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") %
                    argTypesToUse->size() % paramsCount;
        return convNone;
    }

    // Check evaluation mode
    if (!checkEvalMode(c, *argTypesToUse, evalMode, reportErrors))
        return convNone;

    return canCall_common_types(c, context, *argTypesToUse, evalMode, customCvtMode, reportErrors);
}

int SprFrontend::moreSpecialized(CompilationContext* context, const CallableData& f1,
        const CallableData& f2, bool noCustomCvt) {
    // Check parameter count
    size_t paramsCount = getParamsCount(f1, true);
    if (paramsCount != getParamsCount(f2, true))
        return 0;

    bool firstIsMoreSpecialized = false;
    bool secondIsMoreSpecialized = false;
    for (size_t i = 0; i < paramsCount; ++i) {
        TypeRef t1 = getParamType(f1, i, true);
        TypeRef t2 = getParamType(f2, i, true);
        // At this point, all param types must be valid
        ASSERT(t1 && t2);

        // Ignore parameters of same type
        if (t1 == t2)
            continue;

        ConversionFlags flags = noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
        ConversionResult c1 = canConvertType(context, t1, t2, flags);
        if (c1) {
            firstIsMoreSpecialized = true;
            if (secondIsMoreSpecialized)
                return 0;
        }
        ConversionResult c2 = canConvertType(context, t2, t1, flags);
        if (c2) {
            secondIsMoreSpecialized = true;
            if (firstIsMoreSpecialized)
                return 0;
        }
    }
    if (firstIsMoreSpecialized && !secondIsMoreSpecialized)
        return -1;
    else if (!firstIsMoreSpecialized && secondIsMoreSpecialized)
        return 1;
    else
        return 0;
}

Node* SprFrontend::generateCall(CallableData& c, CompilationContext* context, const Location& loc) {
    Node* res = nullptr;

    // Regular function call case
    if (c.type == CallableType::function) {
        if (!Nest_computeType(c.decl))
            return nullptr;

        // Get the arguments with conversions
        auto argsCvt = argsWithConversion(c);

        // Check if the call is an intrinsic
        res = handleIntrinsic(c.decl, context, loc, argsCvt);
        if (res) {
            Nest_setContext(res, context);
            return res;
        } else {
            // Otherwise, generate a function call
            res = createFunctionCall(loc, context, c.decl, all(argsCvt));
        }
    }

    // Generic call case
    if (c.type == CallableType::genericFun) {
        ASSERT(c.genericInst);
        res = callGenericFun(c.decl, loc, context, all(c.args), c.genericInst);
        Nest_setContext(res, context);
    } else if (c.type == CallableType::genericClass) {
        ASSERT(c.genericInst);
        res = callGenericClass(c.decl, loc, context, all(c.args), c.genericInst);
        Nest_setContext(res, context);
    }

    // Concept check case
    if (c.type == CallableType::concept) {
        ASSERT(c.decl);

        // Get the argument, and compile it
        auto argsCvt = argsWithConversion(c);
        ASSERT(argsCvt.size() == 1);
        Node* arg = argsCvt.front();
        ASSERT(arg);
        if (!Nest_semanticCheck(arg))
            return nullptr;

        // Check if the type of the argument fulfills the concept
        bool conceptFulfilled = conceptIsFulfilled(c.decl, arg->type);
        res = Feather_mkCtValueT(loc, StdDef::typeBool, &conceptFulfilled);
        Nest_setContext(res, context);
        res = Nest_semanticCheck(res);
    }

    // If this callable is a class-ctor, wrap the exiting result in a temp-var
    // construct
    if (c.implicitArgType) {
        ASSERT(c.tmpVar);

        res = createTempVarConstruct(loc, context, res, c.tmpVar);
    }

    return res;
}

const Location& SprFrontend::location(const CallableData& c) { return c.decl->location; }

string SprFrontend::toString(const CallableData& c) { return Nest_toString(c.decl); }
