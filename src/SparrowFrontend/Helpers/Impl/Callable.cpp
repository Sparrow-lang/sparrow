#include <StdInc.h>
#include "Callable.h"
#include "Nodes/Exp.hpp"
#include "Helpers/Impl/Intrinsics.h"
#include "Helpers/SprTypeTraits.h"
#include "Helpers/CommonCode.h"
#include "Helpers/Generics.h"
#include "Helpers/StdDef.h"
#include "Helpers/DeclsHelpers.h"
#include <Helpers/Ct.h>
#include "Utils/cppif/SparrowFrontendTypes.hpp"
#include "SprDebug.h"

#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Utils/cppif/NodeRange.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {
unique_ptr<ICallableService> g_CallableService;

namespace {

/// The type of a callable entity
enum class CallableType {
    function,
    genericFun,
    genericClass,
    genericPackage,
    concept,
};

/// Data build to evaluate a callable.
/// Holds additional data about the callable, and data used in the process
/// of determining whether the decl is callable, and generating the actual call
struct CallableData {
    /// The type of callable that we have
    CallableType type;

    /// Indicates if the callable is valid (it hasn't been excluded)
    bool valid;

    /// The decls we want to call
    Feather::DeclNode decl;
    /// The parameters of the decl to call
    /// For Feather functions these are variables, for others, these are ParameterDecl
    NodeRange params;
    /// True if we need to call the function in autoCt mode
    bool autoCt;

    /// The arguments used to call the callable
    /// If the callable has default parameters, this will be extended
    /// Computed by 'canCall'
    vector<NodeHandle> args;

    /// The conversions needed for each argument
    /// Computed by 'canCall'
    vector<ConversionResult> conversions;

    /// This is set for class-ctor callables to add an implicit this argument
    /// when calling the underlying callable. This is the type of the argument
    /// to be added.
    Nest::TypeWithStorage implicitArgType;

    /// Temporary data: the generic instantiation (generic case)
    Node* genericInst;
    /// Temporary data: the variable created for implicit this (class-ctor case)
    Node* tmpVar;

    CallableData()
        : type(CallableType::function)
        , valid(true)
        , decl{nullptr}
        , params{nullptr, nullptr}
        , autoCt{false}
        , implicitArgType(nullptr)
        , genericInst(nullptr)
        , tmpVar(nullptr) {}
};

////////////////////////////////////////////////////////////////////////////////
// Callables
//

/// Get the type of parameter with the given index
/// The last flag indicates whether we should hide or not the implicit param
/// If we hide it, we return only the parameters the caller sees.
Type getParamType(const CallableData& c, int idx, bool hideImplicit = false) {
    if (c.type == CallableType::concept)
        return ConceptType::get();
    // If we have an implicit arg type, hide it
    if (hideImplicit && c.implicitArgType)
        ++idx;
    ASSERT(idx < c.params.size());
    auto param = c.params[idx];
    ASSERT(param);
    Type res = param ? param.type() : Type();
    // Parameters of generic classes or packages are always CT
    if (c.type == CallableType::genericClass || c.type == CallableType::genericPackage)
        res = res.changeMode(modeCt, param.location());
    return res;
}

bool completeArgsWithDefaults(CallableData& c, NodeRange args) {
    // Copy the list of arguments; add default values if arguments are missing
    int paramsCount = c.params.size();
    c.args = args.toVec();
    c.args.reserve(paramsCount);
    for (int i = args.size(); i < paramsCount; ++i) {
        auto param = c.params[i].kindCast<ParameterDecl>();
        if (!param)
            return false;
        auto defaultArg = param.init();
        if (!defaultArg)
            return false; // We have a non-default parameter but we don't have an argument for that
        if (!defaultArg.semanticCheck()) // Make sure this is semantically checked
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
        CallableData& c, const vector<Type>& argTypes, EvalMode evalMode, bool reportErrors) {

    // Nothing to check for concepts
    if (c.type == CallableType::concept)
        return true;

    EvalMode declEvalMode = c.decl.effectiveMode();

    // Compute the final version of autoCt flag. We force a CT call in the following cases:
    //  - the target callable is CT
    //  - the calling mode is CT
    //  - if we have a true 'autoCt' function, and all params are CT, make a CT call
    bool useCt = declEvalMode == modeCt || evalMode == modeCt;
    if (!useCt && declEvalMode == modeRt && c.autoCt) {
        // In autoCt mode, if all the arguments are CT, make a CT call
        useCt = true;
        for (Type t : argTypes) {
            if (t.mode() != modeCt) {
                useCt = false;
                break;
            }
        }
    }
    c.autoCt = useCt;
    return true;
}

ConversionType canCall_common_types(CallableData& c, CompilationContext* context,
        const vector<Type>& argTypes, EvalMode evalMode, CustomCvtMode customCvtMode,
        bool reportErrors) {
    int paramsCount = c.params.size();

    c.conversions.resize(paramsCount, convNone);

    bool isGeneric = c.type == CallableType::genericFun || c.type == CallableType::genericClass ||
                     c.type == CallableType::genericPackage;

    ConversionType res = convDirect;
    for (int i = 0; i < paramsCount; ++i) {
        Type argType = argTypes[i];
        ASSERT(argType);
        Type paramType = getParamType(c, i);
        if (!paramType) {
            if (!isGeneric) {
                // For generics, we allow null parameters for now
                if (reportErrors)
                    REP_INFO(NOLOC, "Bad parameter %1%; arg type: %2%") % i % argType;
                return convNone;
            }
            // continue;   // Will get back on this arg in a secondary pass
            paramType = ConceptType::get();
        }

        // If we are looking at a CT callable, make sure the parameters are in CT
        if (c.autoCt)
            paramType = paramType.changeMode(modeCt, NOLOC);

        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        c.conversions[i] = g_ConvertService->checkConversion(context, argType, paramType, flags);
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

vector<NodeHandle> argsWithConversion(const CallableData& c) {
    vector<NodeHandle> res(c.args.size(), NodeHandle());
    for (int i = 0; i < c.args.size(); ++i)
        res[i] = c.conversions[i].apply(c.args[i].context(), c.args[i]);
    return res;
}

struct CallableImpl : Callable {
    CallableData data_;

    CallableImpl(CallableData data)
        : Callable(data.decl)
        , data_(std::move(data)) {}

    /// Checks if we can call this with the given arguments
    /// This method can cache some information needed by the 'generateCall'
    ConversionType canCall(CompilationContext* context, const Location& loc, NodeRange args,
            EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors = false) override;
    /// Same as above, but makes the check only on type, and not on the actual
    /// argument; doesn't cache any args
    ConversionType canCall(CompilationContext* context, const Location& loc,
            const vector<Type>& argTypes, EvalMode evalMode, CustomCvtMode customCvtMode,
            bool reportErrors = false) override;

    /// Generates the node that actually calls this callable
    /// This must be called only if 'canCall' method returned a success conversion
    /// type
    NodeHandle generateCall(CompilationContext* context, const Location& loc) override;

    /// Gets a string representation of the callable (i.e., function name)
    string toString() const override;

    //! Returns the number of parameters the callable has
    int numParams() const override;

    //! Get the type of the parameter with the given index.
    Type paramType(int idx) const override;
};

CallableImpl* mkFunCallable(FunctionDecl fun, TypeWithStorage implicitArgType = {}) {
    auto params = fun.parameters();
    if (getResultParam(fun))
        params = params.skip(1); // Always hide the result param
    ASSERT(params.size() >= 0);

    CallableData res;
    res.type = CallableType::function;
    res.decl = fun;
    res.params = params;
    res.autoCt = fun.hasProperty(propAutoCt);
    res.implicitArgType = implicitArgType;
    return new CallableImpl{res};
}
CallableImpl* mkGenericFunCallable(
        GenericFunction genericFun, TypeWithStorage implicitArgType = {}) {
    CallableData res;
    res.type = CallableType::genericFun;
    res.decl = genericFun;
    res.params = NodeRange(genericFun.originalParams());
    res.implicitArgType = implicitArgType;
    return new CallableImpl{res};
}
CallableImpl* mkGenericClassCallable(GenericDatatype genericDatatype) {
    CallableData res;
    res.type = CallableType::genericClass;
    res.decl = genericDatatype;
    res.params = NodeRange(genericDatatype.instSet().params());
    return new CallableImpl{res};
}
CallableImpl* mkGenericPackageCallable(GenericPackage genericPackage) {
    CallableData res;
    res.type = CallableType::genericPackage;
    res.decl = genericPackage;
    res.params = NodeRange(genericPackage.instSet().params());
    return new CallableImpl{res};
}
CallableImpl* mkConceptCallable(ConceptDecl concept) {
    CallableData res;
    res.type = CallableType::concept;
    res.decl = concept;
    res.params = NodeRange{ParameterDecl()};
    return new CallableImpl{res};
}

//! Checks if the given decl satisfies the predicate
//! If the predicate is empty, the decl always satisfies it
bool predIsSatisfied(NodeHandle decl, const std::function<bool(NodeHandle)>& pred) {
    return !pred || pred(decl);
}

//! Get the class-ctor callables corresponding to the given class
void getClassCtorCallables(Feather::StructDecl structDecl, EvalMode evalMode, Callables& res,
        const std::function<bool(NodeHandle)>& pred, const char* ctorName) {
    // Search for the ctors associated with the class
    auto decls = getClassAssociatedDecls(structDecl, ctorName);

    evalMode = Feather_combineMode(structDecl.effectiveMode(), evalMode);
    if (!structDecl.computeType())
        return;

    // Get the type of the temporary variable created when constructing the datatype
    TypeWithStorage implicitArgType = structDecl.type();
    if (evalMode != modeRt)
        implicitArgType = implicitArgType.changeMode(evalMode, structDecl.location());

    res.callables_.reserve(res.size() + Nest_nodeArraySize(decls));
    for (NodeHandle decl : decls) {
        if (!decl.computeType())
            continue;
        auto fun = decl.explanation().kindCast<Feather::FunctionDecl>();
        if (fun && predIsSatisfied(decl, pred))
            res.callables_.push_back(mkFunCallable(fun, implicitArgType));

        NodeHandle resDecl = resultingDecl(decl);
        auto genFun = resDecl.kindCast<GenericFunction>();
        if (genFun && predIsSatisfied(decl, pred))
            res.callables_.push_back(mkGenericFunCallable(genFun, implicitArgType));
        auto genDatatype = resDecl.kindCast<GenericDatatype>();
        if (genDatatype && predIsSatisfied(decl, pred))
            res.callables_.push_back(mkGenericClassCallable(genDatatype));
    }
    Nest_freeNodeArray(decls);
}

////////////////////////////////////////////////////////////////////////////////
// Generic function
//
// Call tree:
//
// - SprFrontend::canCall
//      - canCallGenericFun
//          - getDeducedType
//          - applyConversion
//          - handleGenericFunParam
// - SprFrontend::generateCall
//      - callGenericFun
//          - getGenericFunFinalParams
//          - createInstFn
//          - getGenericFunFinalArgs
//          - createCallFn

//! Class to be used by the 'canCall' functionality to store the relevant parameters of the generic
//! function. It describes how the generic function should be instantiated
//!
//! We use this as a blackboard for the canCallGenericFun. We store all the info needed here
class GenericFunCallParams {
public:
    const GenericFunction genFun_;   //!< The generic function to be called
    const bool isCtGeneric_;         //!< True if we are calling a CT-generic
    const EvalMode callEvalMode_;    //!< The eval mode in which this generic should be called
    const EvalMode origEvalMode_;    //!< The original eval mode for the generic
    const EvalMode finalEvalMode_;   //!< The final eval mode to be used for the generic function
    vector<NodeHandle> boundValues_; //!< The bound values that we have; constructed iteratively

    //! Constructor. Initializes most of the parameters here
    GenericFunCallParams(CallableData& c, EvalMode callEvalMode);

private:
    //! Compute the final eval mode, based on the params, args and the original eval mode
    static EvalMode getFinalEvalMode(NodeRangeT<ParameterDecl> genericParams, NodeRange args,
            EvalMode origEvalMode, bool isCtGeneric);
};

GenericFunCallParams::GenericFunCallParams(CallableData& c, EvalMode callEvalMode)
    : genFun_(c.decl)
    , isCtGeneric_(genFun_.original().hasProperty(propCtGeneric))
    , callEvalMode_(callEvalMode)
    , origEvalMode_(genFun_.original().effectiveMode())
    , finalEvalMode_(
              getFinalEvalMode(genFun_.instSet().params(), c.args, origEvalMode_, isCtGeneric_))
    , boundValues_(c.params.size(), nullptr) {}

EvalMode GenericFunCallParams::getFinalEvalMode(NodeRangeT<ParameterDecl> genericParams,
        NodeRange args, EvalMode origEvalMode, bool isCtGeneric) {
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

/**
 * Get the list of final params that should be used in an instantiated function.
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
vector<ParameterDecl> getGenericFunFinalParams(Instantiation inst, NodeHandle origFun,
        NodeRangeT<ParameterDecl> params, NodeRangeT<ParameterDecl> genericParams) {
    auto boundValues = inst.boundValues();
    ASSERT(boundValues.size() != 0);
    auto numParams = genericParams.size();
    vector<ParameterDecl> finalParams;
    finalParams.reserve(numParams);
    for (int i = 0; i < numParams; ++i) {
        ParameterDecl param = genericParams[i];
        // If a parameter is not generic, it must be final => add final param
        if (!param)
            finalParams.push_back(params[i].clone());
        else {
            // For concept-type parameters, we also create a final parameter
            auto boundValue = boundValues[i];
            bool isConcept = isConceptParam(param.type(), boundValue);
            if (isConcept)
                finalParams.push_back(
                        ParameterDecl::create(param.location(), param.name(), boundValue));
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
 * @param inst          The instantiation node
 * @param args          The initial list of arguments to be filtered
 * @param genericParams The generic params to know which argument to filter
 *
 * @return The list of final args that should be used to call the instantiated function
 */
vector<NodeHandle> getGenericFunFinalArgs(
        Instantiation inst, NodeRange args, NodeRangeT<ParameterDecl> genericParams) {
    auto boundValues = inst.boundValues();
    vector<NodeHandle> finalArgs;
    finalArgs.reserve(args.size());
    for (int i = 0; i < args.size(); ++i) {
        ParameterDecl param = genericParams[i];
        // If a parameter is not generic, it must be final => add final arg
        if (!param)
            finalArgs.push_back(args[i]);
        else {
            // Also add final arg for concept params
            auto boundValue = boundValues[i];
            if (isConceptParam(param.type(), boundValue))
                finalArgs.push_back(args[i]);
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
SprFunctionDecl createInstFn(CompilationContext* context, SprFunctionDecl origFun,
        NodeRangeT<ParameterDecl> finalParams) {
    const Location& loc = origFun.location();

    auto parameters = Feather::NodeList::create(loc, NodeRange(finalParams));
    auto returnType = origFun.returnType();
    auto body = origFun.body();
    returnType = returnType ? returnType.clone() : NodeHandle();
    body = body ? Nest_cloneNode(body) : nullptr;
    auto newFun = SprFunctionDecl::create(loc, origFun.name(), parameters, returnType, body);
    copyModifiersSetMode(origFun, newFun, context->evalMode);
    copyAccessType(newFun, origFun);
    copyOverloadPrio(origFun, newFun);
    Feather_setShouldAddToSymTab(newFun, 0);
    newFun.setContext(context);

    return newFun;
}

/**
 * Creates the code that actually calls an instantiated function
 *
 * @param loc       The location of the call code
 * @param context   The context in which the call code should be placed
 * @param instDecl  The instantiation that we want to call
 * @param finalArgs The list of final arguments to be used for the call
 *
 * @return Node representing the calling code for the instantiated function
 */
NodeHandle createCallFn(const Location& loc, CompilationContext* context, SprFunctionDecl instDecl,
        NodeRange finalArgs) {
    ASSERT(instDecl);
    if (!instDecl.computeType())
        return {};
    NodeHandle resultingFun = instDecl.explanation();
    if (!resultingFun)
        REP_ERROR_RET(nullptr, loc, "Cannot instantiate function generic %1%") % instDecl.name();
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
NodeHandle callGenericFun(GenericFunction node, const Location& loc, CompilationContext* context,
        NodeRange args, Instantiation inst) {
    // If not already created, create the actual instantiation declaration
    auto instDecl = inst.instantiatedDecl().kindCast<SprFunctionDecl>();
    if (!instDecl) {
        auto originalFun = node.original();
        auto finalParams = getGenericFunFinalParams(
                inst, originalFun, node.originalParams(), node.instSet().params());

        // Create the actual instantiation declaration
        CompilationContext* ctx = inst.boundVarsNode().childrenContext();
        instDecl = createInstFn(ctx, originalFun, finalParams);
        if (!instDecl)
            REP_INTERNAL(loc, "Cannot instantiate generic");
        if (!instDecl.computeType())
            return {};
        // Debugging
        // cout << "Bound values: " << NNodeRange(inst.boundValues()).toString() << endl;
        // printStart();
        // printNode(instDecl);
        // printEnd();
        Nest_queueSemanticCheck(instDecl);
        inst.setInstantiatedDecl(instDecl);
    }

    // Now actually create the call object
    auto finalArgs = getGenericFunFinalArgs(inst, args, node.instSet().params());
    auto res = createCallFn(loc, context, instDecl, NodeRange(finalArgs));
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
 * @param forceCt   True if we need to force dest type to be CT
 *
 * @return The argument with the appropriate conversion applied
 */
NodeHandle applyConversion(NodeHandle arg, Type paramType, ConversionType& worstConv,
        ConversionFlags flags = flagsDefault, bool forceCt = false) {
    ASSERT(arg.type());

    // If we are looking at a CT callable, make sure the parameters are in CT
    if (forceCt)
        paramType = paramType.changeMode(modeCt, NOLOC);

    ConversionResult conv =
            g_ConvertService->checkConversion(arg.context(), arg.type(), paramType, flags);
    if (!conv) {
        // if (reportErrors)
        //     REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % i % argType %
        //             paramType;
        return {};
    } else if (conv.conversionType() < worstConv)
        worstConv = conv.conversionType();

    // Apply the conversion to our arg
    // We are not interested in the original arg anymore
    return conv.apply(arg.context(), arg);
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
Type getDeducedType(
        int idx, InstantiationsSet instSet, Instantiation curInst, bool reuseExistingInst) {
    Type paramType = nullptr;

    ASSERT(curInst);
    if (reuseExistingInst) {
        // We can get the type of this parameter from the current inst
        // The dependent type should be the same for all the instantiations that start with the same
        // bound values.
        auto otherBoundVal = curInst.boundValues()[idx];
        if (otherBoundVal) {
            paramType = getType(otherBoundVal);
            // REP_INFO(NOLOC, "Deduced type (reused): %1%") % paramType;
        }
    }
    if (!paramType) {
        // Add the appropriate bound variable to the instantiation
        // We will get our deduced type by computing the type of this var
        auto param = instSet.params()[idx];
        ASSERT(param);
        auto typeNode = param.typeNode().clone();
        typeNode.setContext(curInst.boundVarsNode().context());
        if (!typeNode.computeType())
            return {};
        paramType = getType(typeNode);
        ASSERT(paramType);
        // REP_INFO(NOLOC, "Deduced type (computed): %1%") % paramType;
    }
    return paramType;
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
 * @param [in/out] callParams        Blackboard object with all params for checking the call
 * @param [in] idx                   The index of the parameter/argument we are checking
 * @param [in] arg                   The argument given when calling the generic
 * @param [in] paramType             The type to use for the current parameter
 * @param [in/out] curInst           The inst we are currently using; can be reused or not
 * @param [in/out] reuseExistingInst Indicates if we are reusing an existing instantiation
 */
void handleGenericFunParam(GenericFunCallParams& callParams, int idx, NodeHandle arg,
        Type paramType, Instantiation& curInst, bool& reuseExistingInst) {
    InstantiationsSet instSet = callParams.genFun_.instSet();
    auto param = instSet.params()[idx];
    if (!param)
        return; // nothing to do
    ASSERT(paramType);
    ASSERT(!param.type() || param.type() == paramType);

    // First compute the bound value for the current generic parameter
    NodeHandle boundVal;
    bool isRefAuto = false;
    Type boundValType; // used for concept types
    bool isCtConcept = false;
    if (!param.type()) {
        // If we are here this is a dependent param
        //
        // Conditions for considering this a concept param:
        // - param is non-CT
        // - fun was originally RTCT, and now turned to CT
        //
        // A concept param will ensure the creation of a final param.
        if (paramType.mode() != modeCt ||
                (callParams.origEvalMode_ == modeRt && callParams.finalEvalMode_ == modeCt))
            boundValType = paramType;
    } else if (isConceptType(paramType, isRefAuto)) {
        // Deduce the type for boundVal for regular concept types
        boundValType = getAutoType(arg, isRefAuto, paramType.mode());
        isCtConcept = paramType.mode() == modeCt;
    }
    if (boundValType && !isCtConcept) {
        // then the bound value will be a type node
        boundVal = createTypeNode(arg.context(), param.location(), boundValType);
        if (!boundVal.computeType())
            REP_INTERNAL(arg.location(), "Invalid argument %1% when instantiating generic (cannot "
                                         "compute type of typenode)") %
                    (idx + 1);
    } else {
        // Evaluate the node and add the resulting CtValue as a bound argument
        ASSERT(Feather_isCt(arg));
        boundVal = Nest_ctEval(arg);
        if (!boundVal /*|| boundVal.kind() != nkFeatherExpCtValue*/)
            REP_INTERNAL(arg.location(),
                    "Invalid argument %1% when instantiating generic (arg is not CT evaluable)") %
                    (idx + 1);
        ASSERT(boundVal);
        ASSERT(boundVal.type());
    }

    // If we don't have a bound value, this is not a proper generic param/arg
    if (!boundVal)
        return;
    ASSERT(boundVal && boundVal.type());
    callParams.boundValues_[idx] = boundVal;

    // Now, select the appropriate inst for the new bound value
    if (reuseExistingInst) {
        // First, check if we can continue the existing inst
        if (curInst) {
            auto existingBoundVal = curInst.boundValues()[idx];
            if (!existingBoundVal || !ctValsEqual(boundVal, existingBoundVal)) {
                curInst = Instantiation();
            }
        }
        // If the current instantiation is not valid, try to search another
        if (!curInst) {
            NodeRange boundValues = NodeRange(callParams.boundValues_).shrinkTo(idx + 1);
            curInst = searchInstantiation(instSet, boundValues);
            reuseExistingInst = !!(curInst);
        }
    }
    if (!reuseExistingInst) {
        // This is a new instantiation; check if we have to create it
        if (!curInst) {
            curInst = createNewInstantiation(
                    instSet, NodeRange(callParams.boundValues_), callParams.finalEvalMode_);
            ASSERT(curInst);
        } else {
            // Add the appropriate bound variable to the instantiation
            auto boundVar = createBoundVar(curInst.boundVarsNode().context(), param, paramType,
                    boundVal, callParams.isCtGeneric_);
            curInst.boundVarsNode().addChild(boundVar);
            curInst.boundVarsNode().clearCompilationStateSimple();
        }
        curInst.boundValuesM()[idx] = boundVal;
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

    GenericFunCallParams callParams{c, evalMode};

    InstantiationsSet instSet = callParams.genFun_.instSet();

    // The currently working instance
    // We will use this to check/create bound vars, and to deduce the types of the dependent type
    // params. We can reuse previously created instantiations, or we can build our own inst. As much
    // as possible, reuse previously created instantiations.
    Instantiation curInst;
    bool reuseExistingInst = true;

    ConversionType worstConv = convDirect;
    int paramsCount = c.params.size();
    for (int i = 0; i < paramsCount; ++i) {
        Type argType = c.args[i].type();
        ASSERT(argType);
        Type paramType = getParamType(c, i);

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

        NodeHandle& arg = c.args[i];
        ASSERT(arg.type());

        // Apply the conversion
        ConversionFlags flags = flagsDefault;
        if (customCvtMode == noCustomCvt || (customCvtMode == noCustomCvtForFirst && i == 0))
            flags = flagDontCallConversionCtor;
        arg = applyConversion(arg, paramType, worstConv, flags, c.autoCt);
        if (!arg || !arg.computeType()) {
            if (reportErrors)
                REP_INFO(NOLOC, "Cannot convert argument %1% from %2% to %3%") % (i + 1) % argType %
                        paramType;
            return convNone;
        }

        // Treat generic params
        handleGenericFunParam(callParams, i, arg, paramType, curInst, reuseExistingInst);
    }
    // If for one arg there isn't a viable conversion, we can't call this
    if (!worstConv)
        return convNone;

    if (!curInst) {
        REP_INTERNAL(c.decl.location(), "Failed to generate an instantiation for %1%") %
                Nest_toStringEx(c.decl);
    }
    // Check if we can instantiate this
    if (!canInstantiate(curInst, instSet)) {
        if (reportErrors)
            REP_INFO(NOLOC, "Cannot instantiate generic function (if clause not satisfied)");
        return convNone;
    }
    c.genericInst = curInst;

    return worstConv;
}

////////////////////////////////////////////////////////////////////////////////
// Generic class
//

/**
 * Get the bound values for a generic class/package, from the given set of arguments.
 *
 * If any of the given arguments is not a CT value, we issue an error.
 * The assumption is that all the parameters are CT.
 *
 * @param args The set of arguments passed when trying to instantiate the class/pacakge.
 *
 * @return The vector of bound values
 */
NodeVector getGenericClassOrPackageBoundValues(NodeRange args) {
    NodeVector boundValues;
    boundValues.reserve(args.size());

    for (int i = 0; i < args.size(); ++i) {
        auto arg = args[i];

        // Evaluate the node and add the resulting CtValue as a bound argument
        if (!arg.computeType())
            return {};
        if (!Feather_isCt(arg))
            REP_INTERNAL(arg.location(), "Argument to a class generic must be CT (type: %1%)") %
                    arg.type();
        NodeHandle n = Nest_ctEval(arg);
        if (!n || n.kind() != nkFeatherExpCtValue)
            REP_INTERNAL(arg.location(), "Invalid argument %1% when instantiating generic") %
                    (i + 1);
        ASSERT(n && n.type());
        boundValues.push_back(n);
    }
    return boundValues;
}

/**
 * Get the resulting eval mode for a generic class/package instantiation.
 *
 * This will be used for the instantiation of the generic class/package.
 *
 * This checks all the bound values that are types. If all the bound types are CT types, then we
 * return a CT-only mode. If all the bound types are RT-only, then we return a RT-only mode.
 * For the rest of the cases (mixed modes, no types, etc.) We return the mainEvalMode value.
 *
 * @param loc          The location of the generic; used for error reporting
 * @param mainEvalMode The effective eval mode of the generic decl
 * @param boundValues  The bound values to be used for instantiating the generic
 *
 * @return The eval mode that should be used for the instantiation.
 */
EvalMode getGenericClassOrPackageResultingEvalMode(
        const Location& loc, EvalMode mainEvalMode, NodeRange boundValues) {
    bool hasCtOnlyArgs = false;
    for (NodeHandle boundVal : boundValues) {
        if (!boundVal)
            continue;
        ASSERT(!boundVal || boundVal.type());
        // Test the type given to the 'Type' parameters (i.e., we need to know
        // if Vector(t) can be rtct based on the mode of t)
        Type t = tryGetTypeValue(boundVal);
        if (t) {
            if (t.mode() == modeCt)
                hasCtOnlyArgs = true;
        } else if (!boundVal.type().canBeUsedAtRt()) {
            hasCtOnlyArgs = true;
        }
    }

    if (hasCtOnlyArgs)
        return modeCt;
    return mainEvalMode;
}

/**
 * Check if we can instantiate the generic class with the given arguments.
 *
 * This will create a (partial) instantiation for the generic. If the instantiation is valid, then
 * it will return the instantiation node. If there are errors creating the partial instantiation, or
 * if evaluating the if clause yields false, this will return null.
 *
 * @param originalDecl The original decl node (the one that it's a generic)
 * @param instSet The instSet associated with the generic
 * @param args The list of arguments that we want to instantiate the class with.
 *
 * @return The instantiation node, or null if the instantiation is not valid
 */
NodeHandle canInstantiateGenericClassOrPackage(
        Feather::DeclNode originalDecl, InstantiationsSet instSet, NodeRange args) {
    NodeVector boundValues = getGenericClassOrPackageBoundValues(args);
    EvalMode resultingEvalMode = getGenericClassOrPackageResultingEvalMode(
            originalDecl.location(), originalDecl.effectiveMode(), NodeRange(boundValues));
    Instantiation inst = canInstantiate(instSet, NodeRange(boundValues), resultingEvalMode);
    return inst;
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
DataTypeDecl createInstantiatedClass(CompilationContext* context, DataTypeDecl orig) {
    const Location& loc = orig.location();

    auto body = orig.body();
    body = body ? body.clone() : NodeList();
    auto res = DataTypeDecl::create(loc, orig.name(), nullptr, nullptr, nullptr, body);
    copyAccessType(res, orig);

    copyModifiersSetMode(orig, res, context->evalMode);

    // TODO (generics): Uncomment this line
    // Feather_setShouldAddToSymTab(res, 0);
    res.setContext(context);

    return res;
}

/**
 * Gets the description of a to-be-instantiated class/package.
 *
 * We use this description when generating the assembly name for the class/package.
 *
 * @param originalDecl  The original decl node
 * @param inst The inst node in which we are placing the instantiated node
 *
 * @return The description of the instantiated class/package
 */
string getGenericClassOrPackageDescription(Feather::DeclNode originalDecl, Instantiation inst) {
    ostringstream oss;
    oss << originalDecl.name() << "[";
    auto boundValues = inst.boundValues();
    bool first = true;
    for (auto val : boundValues) {
        if (first)
            first = false;
        else
            oss << ", ";
        Type t = evalTypeIfPossible(val);
        if (t)
            oss << t;
        else
            oss << val;
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
NodeHandle callGenericClass(GenericDatatype node, const Location& loc, CompilationContext* context,
        NodeRange args, Instantiation inst) {

    // If not already created, create the actual instantiation declaration
    NodeHandle instDecl = inst.instantiatedDecl();
    if (!instDecl) {
        auto origDatatype = node.original().kindCast<DataTypeDecl>();
        string description = getGenericClassOrPackageDescription(origDatatype, inst);

        // Create the actual instantiation declaration
        CompilationContext* ctx = inst.boundVarsNode().childrenContext();
        instDecl = createInstantiatedClass(ctx, origDatatype);
        if (!instDecl)
            REP_INTERNAL(loc, "Cannot instantiate generic");
        instDecl.setProperty(propDescription, StringRef(description));
        if (!instDecl.computeType())
            return {};
        Nest_queueSemanticCheck(instDecl);
        inst.setInstantiatedDecl(instDecl);

        // Add the instantiated class as an additional node to the generic node
        node.addAdditionalNode(inst.boundVarsNode());
        if (node.curExplanation())
            node.curExplanation().addAdditionalNode(inst.boundVarsNode());
    }

    // Now actually create the call object: a Type CT value
    auto structDecl = instDecl.explanation().kindCast<Feather::StructDecl>();
    ASSERT(structDecl);
    return createTypeNode(node.context(), loc, Feather::DataType::get(structDecl, 0, modeRt));
}

////////////////////////////////////////////////////////////////////////////////
// Generic package
//

/**
 * Create the actual instantiated package.
 *
 * This will make a clone of the original package object. It make sure to clear any parameters and
 * returns the cloned node.
 *
 * @param context     The context to be used for the instantiated package
 * @param orig        The original package declaration (that was a generic)
 *
 * @return The node of the instantiated package decl
 */
PackageDecl createInstantiatedPackage(CompilationContext* context, PackageDecl orig) {
    const Location& loc = orig.location();

    auto body = orig.body();
    body = body ? body.clone() : NodeList();
    auto res = PackageDecl::create(loc, orig.name(), body);
    copyAccessType(res, orig);

    copyModifiersSetMode(orig, res, context->evalMode);

    // TODO (generics): Uncomment this line
    // Feather_setShouldAddToSymTab(res, 0);
    res.setContext(context);

    return res;
}

/**
 * Generate the code to call a generic package.
 *
 * Given a partial instantiation that we already know we can call, this will generate the code to
 * call that package. If we haven't already fully instantiated the generic (i.e., create the
 * instantiated package), we do it now.
 *
 * @param node          The generic package node
 * @param loc           The location of the call site
 * @param context       The context of the calling code
 * @param args          The arguments we use to call the generic
 * @param instantiation The instantiation node that we want to call
 *
 * @return The call code
 */
NodeHandle callGenericPackage(GenericPackage node, const Location& loc, CompilationContext* context,
        NodeRange args, Instantiation inst) {

    // If not already created, create the actual instantiation declaration
    NodeHandle instDecl = inst.instantiatedDecl();
    if (!instDecl) {
        auto origPackage = node.original().kindCast<PackageDecl>();
        string description = getGenericClassOrPackageDescription(origPackage, inst);

        // Create the actual instantiation declaration
        CompilationContext* ctx = inst.boundVarsNode().childrenContext();
        instDecl = createInstantiatedPackage(ctx, origPackage);
        if (!instDecl)
            REP_INTERNAL(loc, "Cannot instantiate generic");
        instDecl.setProperty(propDescription, StringRef(description));
        if (!instDecl.computeType())
            return {};
        Nest_queueSemanticCheck(instDecl);
        inst.setInstantiatedDecl(instDecl);

        // Add the instantiated package as an additional node to the generic node
        node.addAdditionalNode(inst.boundVarsNode());
        if (node.curExplanation())
            node.curExplanation().addAdditionalNode(inst.boundVarsNode());
    }

    return DeclExp::create(loc, NodeRange({instDecl}), nullptr);
}

ConversionType CallableImpl::canCall(CompilationContext* context, const Location& loc,
        NodeRange args, EvalMode evalMode, CustomCvtMode customCvtMode, bool reportErrors) {
    NodeVector args2;

    // If this callable requires an added this argument, add it
    if (data_.implicitArgType) {
        auto thisTempVar = Feather::VarDecl::create(
                loc, "tmp.v", Feather::TypeNode::create(loc, data_.implicitArgType));
        thisTempVar.setContext(context);
        if (!thisTempVar.computeType()) {
            if (reportErrors)
                REP_INFO(loc, "Cannot create temporary variable");
            return convNone;
        }
        auto thisArg = VarRefExp::create(loc, thisTempVar);
        thisArg.setContext(context);
        if (!thisArg.computeType()) {
            if (reportErrors)
                REP_INFO(loc, "Cannot compute the type of this argument");
            return convNone;
        }

        data_.tmpVar = thisTempVar;

        args2 = toVec(args);
        args2.insert(args2.begin(), thisArg);
        args = all(args2);
    }

    // Complete the missing args with defaults
    if (!completeArgsWithDefaults(data_, args))
        return convNone;

    // Check argument count (including hidden params)
    size_t paramsCount = data_.params.size();
    if (paramsCount != data_.args.size()) {
        if (reportErrors)
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") %
                    data_.args.size() % paramsCount;
        return convNone;
    }

    // Get the arg types to perform the check on types
    vector<Type> argTypes(data_.args.size(), nullptr);
    for (size_t i = 0; i < data_.args.size(); ++i)
        argTypes[i] = data_.args[i].type();

    // Check evaluation mode
    if (!checkEvalMode(data_, argTypes, evalMode, reportErrors))
        return convNone;

    if (data_.type == CallableType::genericFun) {
        return canCallGenericFun(data_, context, loc, args, evalMode, customCvtMode, reportErrors);
    }

    // Do the checks on types
    ConversionType res =
            canCall_common_types(data_, context, argTypes, evalMode, customCvtMode, reportErrors);
    if (!res)
        return convNone;

    if (data_.type == CallableType::genericClass) {
        // Check if we can instantiate the generic with the given arguments
        // (with conversions applied)
        // Note: we overwrite the args with their conversions;
        // We don't use the old arguments anymore
        data_.args = argsWithConversion(data_);
        ASSERT(!data_.genericInst);
        GenericDatatype genNode = GenericDatatype(data_.decl);
        data_.genericInst = canInstantiateGenericClassOrPackage(
                genNode.original(), genNode.instSet(), NodeRange(data_.args));
        if (!data_.genericInst && reportErrors) {
            REP_INFO(NOLOC, "Cannot instantiate generic class");
        }
        if (!data_.genericInst)
            return convNone;
    } else if (data_.type == CallableType::genericPackage) {
        // Check if we can instantiate the generic with the given arguments
        // (with conversions applied)
        // Note: we overwrite the args with their conversions;
        // We don't use the old arguments anymore
        data_.args = argsWithConversion(data_);
        ASSERT(!data_.genericInst);
        GenericPackage genNode = GenericPackage(data_.decl);
        data_.genericInst = canInstantiateGenericClassOrPackage(
                genNode.original(), genNode.instSet(), NodeRange(data_.args));
        if (!data_.genericInst && reportErrors) {
            REP_INFO(NOLOC, "Cannot instantiate generic package");
        }
        if (!data_.genericInst)
            return convNone;
    }

    return res;
}
ConversionType CallableImpl::canCall(CompilationContext* context, const Location& loc,
        const vector<Type>& argTypes, EvalMode evalMode, CustomCvtMode customCvtMode,
        bool reportErrors) {
    vector<Type> argTypes2;
    const vector<Type>* argTypesToUse = &argTypes;

    // If this callable requires an added this argument, add it
    if (data_.implicitArgType) {
        Type t = data_.implicitArgType;
        if (!Feather::isCategoryType(t))
            t = MutableType::get(data_.implicitArgType);

        argTypes2 = argTypes;
        argTypes2.insert(argTypes2.begin(), t);
        argTypesToUse = &argTypes2;
    }

    // Check argument count (including hidden params)
    size_t paramsCount = data_.params.size();
    if (paramsCount != argTypesToUse->size()) {
        if (reportErrors)
            REP_INFO(NOLOC, "Different number of parameters; args=%1%, params=%2%") %
                    argTypesToUse->size() % paramsCount;
        return convNone;
    }

    // Check evaluation mode
    if (!checkEvalMode(data_, *argTypesToUse, evalMode, reportErrors))
        return convNone;

    return canCall_common_types(
            data_, context, *argTypesToUse, evalMode, customCvtMode, reportErrors);
}

NodeHandle CallableImpl::generateCall(CompilationContext* context, const Location& loc) {
    NodeHandle res;

    // Regular function call case
    if (data_.type == CallableType::function) {
        if (!data_.decl.computeType())
            return {};

        // Get the arguments with conversions
        auto argsCvt = argsWithConversion(data_);

        // Check if the call is an intrinsic
        ASSERT(data_.decl.kind() == nkFeatherDeclFunction);
        res = handleIntrinsic(FunctionDecl(data_.decl), context, loc, argsCvt);
        if (res) {
            res.setContext(context);
            return res;
        } else {
            // Otherwise, generate a function call
            res = createFunctionCall(loc, context, data_.decl, NodeRange(argsCvt));
        }
    }

    // Generic call case
    if (data_.type == CallableType::genericFun) {
        ASSERT(data_.genericInst);
        res = callGenericFun(GenericFunction(data_.decl), loc, context, NodeRange(data_.args),
                data_.genericInst);
        res.setContext(context);
    } else if (data_.type == CallableType::genericClass) {
        ASSERT(data_.genericInst);
        res = callGenericClass(GenericDatatype(data_.decl), loc, context, NodeRange(data_.args),
                data_.genericInst);
        res.setContext(context);
    } else if (data_.type == CallableType::genericPackage) {
        ASSERT(data_.genericInst);
        res = callGenericPackage(
                GenericPackage(data_.decl), loc, context, NodeRange(data_.args), data_.genericInst);
        res.setContext(context);
    }

    // Concept check case
    if (data_.type == CallableType::concept) {
        ASSERT(data_.decl);

        // Get the argument, and compile it
        auto argsCvt = argsWithConversion(data_);
        ASSERT(argsCvt.size() == 1);
        NodeHandle arg = argsCvt.front();
        ASSERT(arg);
        if (!arg.semanticCheck())
            return {};

        // Check if the type of the argument fulfills the concept
        bool conceptFulfilled =
                g_ConceptsService->conceptIsFulfilled(ConceptDecl(data_.decl), arg.type());
        res = Feather_mkCtValueT(loc, StdDef::typeBool, &conceptFulfilled);
        res.setContext(context);
        res = res.semanticCheck();
    }

    // If this callable is a class-ctor, wrap the exiting result in a temp-var
    // construct
    if (data_.implicitArgType) {
        ASSERT(data_.tmpVar);

        res = createTempVarConstruct(loc, context, res, data_.tmpVar);
    }

    return res;
}

int CallableImpl::numParams() const { return data_.params.size(); }

Type CallableImpl::paramType(int idx) const { return getParamType(data_, idx, true); }

string CallableImpl::toString() const {
    ostringstream oss;
    oss << data_.decl.name() << "(";
    bool first = true;
    for (auto p : data_.params) {
        if (first)
            first = false;
        else
            oss << ", ";

        NodeHandle typeNode;
        StringRef name;
        auto var = p.kindCast<Feather::VarDecl>();
        if (var) {
            name = var.name();
            typeNode = var.typeNode();
        }
        auto param = p.kindCast<ParameterDecl>();
        if (param) {
            name = param.name();
            typeNode = param.typeNode();
        }
        Type type = typeNode ? tryGetTypeValue(typeNode) : Type();
        if (name)
            oss << name << ": ";
        if (type)
            oss << type;
        else
            oss << '?';
    }
    oss << ")";
    return oss.str();
}
} // namespace

int moreSpecialized(
        CompilationContext* context, const Callable& f1, const Callable& f2, bool noCustomCvt) {
    // Check parameter count
    size_t paramsCount = f1.numParams();
    if (paramsCount != f2.numParams())
        return 0;

    bool firstIsMoreSpecialized = false;
    bool secondIsMoreSpecialized = false;
    for (size_t i = 0; i < paramsCount; ++i) {
        Type t1 = f1.paramType(i);
        Type t2 = f2.paramType(i);
        // Ignore any params that are null
        // TODO (overloading): Fix this - we reach in this state for dependent params
        if (!t1 || !t2)
            continue;

        // Ignore parameters of same type
        if (t1 == t2)
            continue;

        ConversionFlags flags = noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
        ConversionResult c1 = g_ConvertService->checkConversion(context, t1, t2, flags);
        if (c1) {
            firstIsMoreSpecialized = true;
            if (secondIsMoreSpecialized)
                return 0;
        }
        ConversionResult c2 = g_ConvertService->checkConversion(context, t2, t1, flags);
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

Callables CallableService::getCallables(NodeRange decls, EvalMode evalMode,
        const std::function<bool(NodeHandle)>& pred, const char* ctorName) {
    Callables res;
    auto declsEx = expandDecls(decls, nullptr);

    for (NodeHandle d1 : declsEx) {
        NodeHandle node = d1;

        // If we have a resolved decl, get the callable for it
        if (node) {
            if (!node.computeType())
                continue;

            NodeHandle decl = resultingDecl(node);
            if (!decl)
                continue;

            // Is this a normal function call?
            auto funDecl = decl.kindCast<Feather::FunctionDecl>();
            if (funDecl && predIsSatisfied(funDecl, pred))
                res.callables_.push_back(mkFunCallable(funDecl));

            // Is this a generic?
            auto genFun = decl.kindCast<GenericFunction>();
            if (genFun && predIsSatisfied(genFun, pred))
                res.callables_.push_back(mkGenericFunCallable(genFun));
            auto genDatatype = decl.kindCast<GenericDatatype>();
            if (genDatatype && predIsSatisfied(genDatatype, pred))
                res.callables_.push_back(mkGenericClassCallable(genDatatype));
            auto genPackage = decl.kindCast<GenericPackage>();
            if (genPackage && predIsSatisfied(genPackage, pred))
                res.callables_.push_back(mkGenericPackageCallable(genPackage));

            // Is this a concept?
            auto concept = decl.kindCast<ConceptDecl>();
            if (concept && predIsSatisfied(concept, pred))
                res.callables_.push_back(mkConceptCallable(concept));

            // Is this a temporary object creation?
            auto structDecl = decl.kindCast<Feather::StructDecl>();
            if (structDecl) {
                getClassCtorCallables(structDecl, evalMode, res, pred, ctorName);
            }
        }
    }
    return res;
}

void setDefaultCallableService() {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_CallableService.reset(new CallableService);
}

} // namespace SprFrontend
