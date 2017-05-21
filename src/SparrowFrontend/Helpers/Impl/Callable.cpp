#include <StdInc.h>
#include "Callable.h"
#include "Helpers/Impl/Intrinsics.h"
#include "Helpers/CommonCode.h"
#include "Helpers/Generics.h"
#include "Helpers/StdDef.h"
#include "Helpers/DeclsHelpers.h"
#include "SparrowFrontendTypes.h"

using namespace SprFrontend;
using namespace Nest;

namespace {

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
TypeRef getParamType(
        const CallableData& c, int idx, bool hideImplicit = false) {
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

ConversionType canCall_common_types(CallableData& c,
        CompilationContext* context,
        const vector<TypeRef>& argTypes, EvalMode evalMode,
        CustomCvtMode customCvtMode, bool reportErrors) {
    size_t paramsCount = getParamsCount(c);

    c.conversions.resize(paramsCount, convNone);

    bool isGeneric = c.type==CallableType::genericFun || c.type == CallableType::genericClass;

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
CallableData mkGenericFunCallable(
        Node* genericFun, TypeRef implicitArgType = nullptr) {
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

    evalMode = Feather_combineMode(
            Feather_effectiveEvalMode(cls), evalMode, cls->location);
    TypeRef implicitArgType = varType(cls, evalMode);

    res.reserve(res.size() + Nest_nodeArraySize(decls));
    for (Node* decl : decls) {
        Node* fun = Nest_explanation(decl);
        if (fun && fun->nodeKind == nkFeatherDeclFunction &&
                predIsSatisfied(decl, pred))
            res.push_back(mkFunCallable(fun, implicitArgType));

        Node* resDecl = resultingDecl(decl);
        if (resDecl->nodeKind == nkSparrowDeclGenericFunction &&
                predIsSatisfied(decl, pred))
            res.push_back(mkGenericFunCallable(resDecl, implicitArgType));
        else if (resDecl->nodeKind == nkSparrowDeclGenericClass &&
                 predIsSatisfied(decl, pred))
            res.push_back(mkGenericClassCallable(resDecl));
    }
    Nest_freeNodeArray(decls);
}
}

void SprFrontend::getCallables(
        NodeRange decls, EvalMode evalMode, Callables& res) {
    getCallables(decls, evalMode, res, boost::function<bool(Node*)>());
}

void SprFrontend::getCallables(NodeRange decls, EvalMode evalMode,
        Callables& res, const boost::function<bool(Node*)>& pred,
        const char* ctorName) {
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
            if (decl && decl->nodeKind == nkFeatherDeclFunction &&
                    predIsSatisfied(decl, pred))
                res.emplace_back(mkFunCallable(decl));

            // Is this a generic?
            else if (decl->nodeKind == nkSparrowDeclGenericFunction &&
                     predIsSatisfied(decl, pred))
                res.push_back(mkGenericFunCallable(decl));
            else if (decl->nodeKind == nkSparrowDeclGenericClass &&
                     predIsSatisfied(decl, pred))
                res.push_back(mkGenericClassCallable(decl));

            // Is this a concept?
            else if (decl->nodeKind == nkSparrowDeclSprConcept &&
                     predIsSatisfied(decl, pred))
                res.emplace_back(mkConceptCallable(decl));

            // Is this a temporary object creation?
            else if (decl->nodeKind == nkFeatherDeclClass) {
                getClassCtorCallables(decl, evalMode, res, pred, ctorName);
            }
        }
    }
}

ConversionType SprFrontend::canCall(CallableData& c,
        CompilationContext* context, const Location& loc, NodeRange args,
        EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    NodeVector args2;

    // If this callable requires an added this argument, add it
    if (c.implicitArgType) {
        Node* thisTempVar = Feather_mkVar(loc, fromCStr("tmp.v"),
                Feather_mkTypeNode(loc, c.implicitArgType));
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
    if ( !checkEvalMode(c, argTypes, evalMode, reportErrors) )
        return convNone;

    // Do the checks on types
    ConversionType res =
            canCall_common_types(c, context, argTypes, evalMode, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    if (c.type == CallableType::genericFun) {
        // Check if we can instantiate the generic with the given arguments
        // (with conversions applied)
        // Note: we overwrite the args with their conversions;
        // We don't use the old arguments anymore
        c.args = argsWithConversion(c);
        ASSERT(!c.genericInst);
        c.genericInst = canInstantiateGenericFun(c.decl, all(c.args));
        if (!c.genericInst && reportErrors) {
            REP_INFO(NOLOC, "Cannot instantiate generic function");
        }
        if (!c.genericInst)
            return convNone;
    } else if (c.type == CallableType::genericClass) {
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
ConversionType SprFrontend::canCall(CallableData& c,
        CompilationContext* context, const Location& loc,
        const vector<TypeRef>& argTypes, EvalMode evalMode,
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
    if ( !checkEvalMode(c, *argTypesToUse, evalMode, reportErrors) )
        return convNone;

    return canCall_common_types(c, context, *argTypesToUse, evalMode,
            customCvtMode, reportErrors);
}

int SprFrontend::moreSpecialized(CompilationContext* context,
        const CallableData& f1, const CallableData& f2, bool noCustomCvt) {
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

        ConversionFlags flags =
                noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
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

Node* SprFrontend::generateCall(
        CallableData& c, CompilationContext* context, const Location& loc) {
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

const Location& SprFrontend::location(const CallableData& c) {
    return c.decl->location;
}

string SprFrontend::toString(const CallableData& c) {
    return Nest_toString(c.decl);
}
