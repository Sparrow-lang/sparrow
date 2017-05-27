#include <StdInc.h>
#include "Generics.h"
#include "SprTypeTraits.h"
#include "SprDebug.h"

#include <SparrowFrontendTypes.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Ct.h>

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;
using namespace Nest;

// TODO
// namespace {

/*
Generics organization
---------------------

- we have an Instantiation, as a part of InstantiationSet, which is a part of a generic node
- the instantiation object will contain the following:
    - the list of bound variables (created at Inst node creation)
    - then, in the same context (but not as part of it), will add:
        - the if clause
        - the copy of the node that is instantiated

- terminology:
    - bound params: parameters that must be fixed to before creating the instantiation; used to
    differentiate between instantiations
    - non-bound params: all the parameter that are non-bound; RT parameters
    - final params: the params that are passed to the instantiated function; the RT params and the
    concept params
    - bound vars: the variables created to hold the values for the bound params
    - bound values: the CT values used to initialize the bound vars
    - generic params: the list of params with nulls in the positions of non-bound params
    - dependent param: a parameter for which it's type expression depends on the previous params

We distinguish between partial instantiation and full instantiation. The partial instantiation is
just a copy of the bound parameters/vars and the check of the if clause. In this partial
instantiation we don't copy/check the resulting decl. We use partial instantiations in the overload
procedure to check which generic decls can be called with the given set of arguments. Based on this
process, we may have multiple instantiations that are callable, but in the end they will not be
selected. We create full instantiations only for the selected callables. For a full instantiation
we also clone the original declaration, and semantic check it.
Any errors in compiling the partial instantiation, will make the generic not callable. Any errors in
the full instantiation (if partial instantiation succeeded) will be reported.

To check if we have dependent parameters, we check the type node of the param to contain an
identifier that refers to any of the previously seen parameters. If we detected that a parameter is
dependent, then we don't compute it's type. It will be added to the generic params, but with a null
type. A dependent param will be treated as if the type was 'AnyType', and we add an extra condition
for instantiation. A dependent param will therefore be treated as a concept param.

the instantiation process goes as following for generic functions:
- checkCreateGenericFun -- checks if the given function is generic, and prepare the generic fun
    - > check which parameter is dependent on another parameter
    - > compute the type of every non-dependent param
    - > if at least one of them is CT or concept, then we are seeing a generic fun
    - > gather all the concept & CT params; use them to create the generic fun node
    - > the generic params will be stored in the instSet(), while the original params in the node.
    - > the generic params will contain nulls for the non-bound params
- canInstantiateGenericFun -- checks if we can instantiate the generic
    - > the types of the parameters are already computed
    - getGenericFunBoundValues -- gets the bound values from the list of arguments
        - > for concept params, the bound value will be the actual type of the argument
        - > similar to the generic params, the values will have null gaps corresponding to non-bound
        parameters
    - canInstantiate: tries to perform the instantiation
        - searchInstantiation: searches if we already have the instantiation
            - > checks the instSet if we already have an instantiation (successful or not) with the
            same bound args
        - createNewInstantiation: create a new instantiation node
            - createBoundVariables: create the bound variables
                - > for each bound value, create a variables
                - > for CT param, we create a value just to create the value
                - > for concept params, we just create a variable of the right type, without
                initializing it; we use these in 'if clauses'
            - mkInstantiation: actually create the instantiation node
                - > the bound vars are stores as 'expandedInstantiation'
            - > semantically check the bound vars
            - > add this to the parent instSet
        - > clone the if clause, and evaluate it
        - > if the evaluation turns positive, this is a valid instantiation; regardless of the
            content/body of the generic we place in this instantiation
- callGenericFun: create the code to call the generic function
    - > if we don't already have an instantiated decl (from a previous instantiation attempt)
        - getGenericFunFinalParams: get the list of non-bound parameters
            - > clone all the RT parameter nodes
            - > for concept params, create a new param, with the corresponding instantiated type
        - createInstFn: create the actual instantiated function
            - > clones the original function
            - > replaces the parameters with the final-params
        - > compute the type of the instantiated fun, and queue it for semantic check
    - getGenericFunFinalArgs: Gets the args that need to be passed the final instantiated fun
        - > we select the args for RT params (nulls in generic params) or for concept parsms
    - createCallFn: create the node to actually call the function
        - compute the type of the instantiated function
        - create the call node

and for generic classes:
- canInstantiateGenericClass -- checks if we can instantiate the generic
    - getGenericClassBoundValues -- gets the bound values from the list of arguments
        - > all parameters need to be CT; errors are issues otherwise
    - canInstantiate: same as above
- callGenericClass: create the code to use the instantiated class
    - > if we don't already have an instantiated decl (from a previous instantiation attempt)
        - createInstantiatedClass: create the actual instantiated class
            - > clones the original class
            - > no params set to the instantiated class
        - > compute the type of the instantiated class, and queue it for semantic check
    - createTypeNode: Creates a type node to point to the instantiated class
 */

////////////////////////////////////////////////////////////////////////////
// InstantiationsSet
//

/**
 * Create a bound var for the given parameter / bound value
 *
 * We need bound variables for several reasons:
 *     - dependent param types might reference them
 *     - the if clause might reference them
 *     - the instantiated decl might reference them
 *
 * For the CT parameters that we have, we create a variable that is initialized with the given arg.
 * For concept params, we create a value of the appropriate type (the type is present as a bound
 * value), but we don't initialize the variable. The only thing we can do with these variables (for
 * concept params) is to deduce their type.
 *
 * @param param       The parameter for which we want to create a bound variable
 * @param boundValue  The bound value used for the type (and init) of the variable
 * @param insideClass True if we are inside a class; in this case, mark the variable as static.
 *
 * @return [description]
 */
Node* createBoundVar(Node* param, Node* boundValue, bool insideClass) {
    ASSERT(param);
    ASSERT(boundValue && boundValue->type);

    bool isConcept = !param->type || isConceptType(param->type);

    TypeRef t = isConcept ? getType(boundValue) : boundValue->type;
    Node* init = isConcept ? nullptr : Nest_cloneNode(boundValue);
    Node* var = mkSprVariable(param->location, Feather_getName(param), t, init);

    if (insideClass)
        Nest_setPropertyInt(var, propIsStatic, 1);
    if (!isConcept)
        Feather_setEvalMode(var, modeCt);
    return var;
}
// TODO (now): document
Node* createDependentBoundVar(Node* param, bool insideClass) {
    ASSERT(param);
    ASSERT(param->nodeKind == nkSparrowDeclSprParameter);

    Node* typeNode = Nest_cloneNode(at(param->children, 0)); // clone the original param type node
    Node* var = mkSprVariable(param->location, Feather_getName(param), typeNode, nullptr);

    if (insideClass)
        Nest_setPropertyInt(var, propIsStatic, 1);
    return var;
}

/**
 * Create the bound variables for a (partial) instantiation.
 *
 * We need these variables for two reasons:
 *     - the if clause might reference them
 *     - the instantiated decl might reference them
 *
 * For the CT parameters that we have, we create a variable that is initialized with the given arg.
 * For concept params, we create a value of the appropriate type (the type is present as a bound
 * value), but we don't initialize the variable. The only thing we can do with these variables (for
 * concept params) is to deduce their type.
 *
 * @param loc         The location in which we are creating the nodes
 * @param boundValues The list of bound values used in the creation of bound vars
 * @param params      The params of the instantiated set (may contain nulls for RT params)
 * @param insideClass True if the generic is inside a class; we mark the variables static if we are
 *                    inside classes
 *
 * @return A vector of all the bound variable
 */
NodeVector createBoundVariables(
        const Location& loc, NodeRange boundValues, NodeRange params, bool insideClass) {
    // Create a variable for each bound parameter - put everything in a node list
    NodeVector nodes;
    size_t idx = 0;
    for (Node* p : params) {
        Node* boundValue = at(boundValues, idx++);
        if (!p || !boundValue)
            continue;
        Node* var = createBoundVar(p, boundValue, insideClass);
        ASSERT(var);
        nodes.push_back(var);
    }
    // TODO (now): Remove this
    // nodes.push_back(Feather_mkNop(loc)); // Make sure the resulting type is Void
    return nodes;
}

/**
 * Search an instantiation in an instSet.
 *
 * Two instantiations are the same if the set of bound values are the same. This function checks all
 * the existing (partial) instantiations to see if they match a given set of bound values.
 *
 * We assume that the bound values will be consistent in their null values; null values are placed
 * exactly in the same place for all the instantiations.
 *
 * @note: We only check non-null values. This allows us to do partial matches.
 *
 * @param instSet The instantiations set to search into
 * @param values  The bound values that we are using to search for the instantiation.
 *
 * @return The instantiation node if found; null otherwise
 */
InstNode searchInstantiation(InstSetNode instSet, NodeRange values) {
    for (InstNode inst : instSet.instantiations()) {
        const auto& boundValues = inst.boundValues();
        if (size(boundValues) != size(values))
            continue;

        bool argsMatch = true;
        for (size_t i = 0; i < size(values); ++i) {
            Node* boundVal = at(boundValues, i);
            Node* val = at(values, i);
            if (!val || !boundVal)
                continue;
            ASSERT(val->type);
            ASSERT(boundVal->type);
            if (!ctValsEqual(val, boundVal)) {
                argsMatch = false;
                break;
            }
        }
        if (argsMatch)
            return inst;
    }
    return nullptr;
}

/**
 * Create a new (partial) instantiation node.
 *
 * We will create a new instantiation node and add it to our instSet.
 * The instantiation will contain bound variables corresponding to the given bound values.
 * The bound values will be the 'expanded instantiation'.
 *
 * We fail if compiling the bound variables fail.
 *
 * @param instSet  The instSet where to place the new instantiation
 * @param values   The bound values corresponding to the new instantiation
 * @param evalMode The eval mode for the context where we place the instantiation
 *
 * @return The new instantiation node
 */
InstNode createNewInstantiation(InstSetNode instSet, NodeRange values, EvalMode evalMode) {
    ASSERT(instSet.node);
    const Location& loc = instSet.node->location;

    // Create a new context, but at the same level as the context of the parent node
    Node* parentNode = instSet.parentNode();
    CompilationContext* context =
            Nest_mkChildContextWithSymTab(parentNode->context, nullptr, evalMode);
    bool insideClass = nullptr != Feather_getParentClass(context);

    // Create the instantiation
    auto boundVars = createBoundVariables(loc, values, instSet.params(), insideClass);
    InstNode inst = mkInstantiation(loc, values, all(boundVars));
    // Add it to the parent instSet
    Nest_appendNodeToArray(&instSet.instantiations(), inst.node);

    // Compile the newly created instantiation
    Nest_setContext(inst.expandedInstantiation(), context);
    if (!Nest_semanticCheck(inst.expandedInstantiation()))
        return nullptr;

    return inst;
}

/**
 * Check if we can have an instantiation with the given bound values.
 *
 * The instantiations in an instSet are identified by the set of bound values that they have.
 * This function tries to search if we already have an instantiation with the given bound values. If
 * yes, then we will use it as a cache, to actually check if the instantiation is possible.
 *
 * If no instantiation exists with the given bound values, then we create a new instantiation.
 * After creating the new instantiation we place a clone of the if clause in the context of the
 * bound vars, and we try to compile it and evaluate it. If this fails, then we return null.
 *
 * Please note that at this level we only operate with partial instantiations. We don't care about
 * the actual instantiated decls. We do our logic on the bound values / bound vars.
 *
 * If all these succeed we return the instantiation node.
 *
 * @param instSet  The instSet in which we try to instantiate
 * @param values   The bound values that we want to instantiate for
 * @param evalMode The eval mode to be used by the instantiation.
 *
 * @return The inst node if the instantiation succeeds; null if it fails
 */
InstNode canInstantiate(InstSetNode instSet, NodeRange values, EvalMode evalMode) {
    // Try to find an existing instantiation
    InstNode inst = searchInstantiation(instSet, values);
    if (inst && inst.isEvaluated()) {
        // We already checked whether we can instantiate this
        return inst.isValid() ? inst : nullptr;
    }
    // ASSERT(inst); // As we construct it iteratively, we should always have one

    // If no instantiation is found, create a new instantiation
    // TODO: LucTeo: Remove this
    // TODO: Check the concepts instantiation case first
    if (!inst)
        inst = createNewInstantiation(instSet, values, evalMode);

    // From this point, we consider our instantiation evaluated
    // If we fail, then we will not set that it's valid
    inst.setEvaluated();

    // If we have an if clause, check if this CT evaluates to true
    Node* ifClause = instSet.ifClause();
    if (ifClause) {
        // Always use a clone of the original node
        Node* cond = Nest_cloneNode(ifClause);
        Nest_setContext(cond, Nest_childrenContext(inst.expandedInstantiation()));

        // If the condition does not compile, we cannot instantiate
        bool isValid = false;
        int oldVal = Nest_isReportingEnabled();
        Nest_enableReporting(0);
        try {
            Node* res = Nest_semanticCheck(cond);
            isValid = res != nullptr && Feather_isCt(res)    // We must have a value at CT
                      && Feather_isTestable(res)             // The value must be boolean
                      && Nest_getSuppressedErrorsNum() == 0; // No suppressed errors
        } catch (...) {
        }
        Nest_enableReporting(oldVal);
        if (!isValid)
            return nullptr;

        // Evaluate the if clause condition and check the result
        if (!SprFrontend::getBoolCtValue(Nest_ctEval(cond)))
            return nullptr;
    }

    inst.setValid();
    return inst;
}

////////////////////////////////////////////////////////////////////////////
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
    if (hasCtOnlyArgs && hasRtOnlyArgs)
        REP_ERROR(loc, "Cannot instantiate generic with both RT-only and "
                       "CT-only arguments");
    if (mainEvalMode == modeCt && hasRtOnlyArgs)
        REP_ERROR(loc, "Cannot use RT-only arguments in a CT generic");
    // if (mainEvalMode == modeRt && hasCtOnlyArgs)
    //     REP_ERROR(loc, "Cannot use CT-only arguments in a RT generic");
    // TODO (now): check this

    if (hasCtOnlyArgs)
        return modeCt;
    if (hasRtOnlyArgs)
        return modeRt;
    return mainEvalMode;
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

////////////////////////////////////////////////////////////////////////////
// Generic function
//

/**
 * Get the bound value to be used to instantiate a generic function.
 *
 * For every CT param that we have, we add the arg value as a bound value.
 * For every concept param that we have, we add the type of the arg as a bound value.
 *
 * The params that have a null type are dependent params, so we treat them as concept params.
 *
 * These bound values are the 'signature' used to instantiate the generic function for these args.
 *
 * @param context       The context in which we should create any type nodes
 * @param args          The arguments passed when calling the function
 * @param genericParams The generic params
 *
 * @return All the bound values to be used to instantiate the function with the given args
 */
NodeVector getGenericFunBoundValues(
        CompilationContext* context, NodeRange args, NodeRange genericParams) {
    NodeVector boundValues;
    boundValues.resize(size(args), nullptr);

    for (size_t i = 0; i < size(args); ++i) {
        Node* arg = at(args, i);
        Node* param = at(genericParams, i);
        if (!param)
            continue;

        bool isRefAuto;
        if (!param->type || isConceptType(param->type, isRefAuto)) {
            // Create a CtValue with the type of the argument corresponding to
            // the auto parameter
            if (!Nest_computeType(arg))
                return {};
            TypeRef t = getAutoType(arg, isRefAuto);
            Node* typeNode = createTypeNode(context, param->location, t);
            if (!Nest_computeType(typeNode))
                return {};
            boundValues[i] = typeNode;
        } else {
            // Evaluate the node and add the resulting CtValue as a bound
            // argument
            if (!Nest_computeType(arg))
                return {};
            if (!Feather_isCt(arg))
                return {}; // This argument must be CT in order to instantiate
                           // the generic
            Node* n = Nest_ctEval(arg);
            if (!n || n->nodeKind != nkFeatherExpCtValue)
                REP_INTERNAL(arg->location, "Invalid argument %1% when instantiating generic") %
                        (i + 1);
            boundValues[i] = n;
        }
        ASSERT(boundValues[i] && boundValues[i]->type);
    }
    return boundValues;
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
    if (hasCtOnlyArgs && hasRtOnlyArgs)
        REP_ERROR(loc, "Cannot instantiate generic with both RT-only and "
                       "CT-only arguments");
    if (mainEvalMode == modeCt && hasRtOnlyArgs)
        REP_ERROR(loc, "Cannot use RT-only arguments in a CT generic");
    // if (mainEvalMode == modeRt && hasCtOnlyArgs)
    //     REP_ERROR(loc, "Cannot use CT-only arguments in a RT generic");
    // TODO (now): Check this

    if (hasCtOnlyArgs)
        return modeCt;
    if (hasRtOnlyArgs)
        return modeRt;
    return mainEvalMode;
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

typedef vector<StringRef> NamesVec;

/**
 * Checks if the given node references any of the given seen names.
 *
 * Recursively digs deep in the given node to find any identifier that references to the given set
 * of names.
 *
 * We use this to find whether we have dependent types: one type depending on the actual
 * instantiation of another.
 *
 * @param node      The node that we want to check
 * @param seenNames The list of seen names we compare against
 *
 * @return True if this node references any of the given names
 */
bool referencesSeenName(Node* node, const NamesVec& seenNames) {
    if (!node)
        return false;
    if (node->nodeKind == nkSparrowExpIdentifier) {
        // Check to see if the name matches one of our previously seen parameter names
        StringRef name = Feather_getName(node);
        for (StringRef seenName : seenNames) {
            if (name == seenName)
                return true;
        }
    } else {
        // For the rest of the nodes, check all the children
        for (Node* child : node->children) {
            if (referencesSeenName(child, seenNames))
                return true;
        }
    }
    return false;
}
// }

Node* SprFrontend::checkCreateGenericFun(
        Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass) {
    ASSERT(parameters);
    NodeRange params = all(parameters->children);
    auto numParams = Nest_nodeRangeSize(params);
    if (numParams == 0)
        return nullptr;   // cannot be generic

    // If we are in a CT function, don't consider CT parameters
    bool inCtFun = Feather_effectiveEvalMode(originalFun) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = Nest_hasProperty(originalFun, propCtGeneric);

    // Check if we have some dependent type parameters
    NodeVector dependentParams(numParams, nullptr);
    NamesVec seenNames;
    seenNames.reserve(numParams);
    bool hasDependentParams = false;
    for (int i = 0; i < numParams; ++i) {
        Node* param = at(params, i);
        Node* paramType = at(param->children, 0);
        ASSERT(param->nodeKind == nkSparrowDeclSprParameter);

        // Check if this parameter references a previously seen parameter
        if (i>0 && referencesSeenName(paramType, seenNames)) {
            dependentParams[i] = param;
            hasDependentParams = true;
        }

        // Add this param name to the list of seen names
        seenNames.push_back(Feather_getName(param));
    }
    ASSERT(dependentParams[0] == nullptr);  // the first param cannot be dependent

    // TODO: properly implement dependent params
    if (hasDependentParams) {
        ostringstream oss;
        oss << "(null";
        for (int i=1; i<numParams; i++) {
            oss << ", " << (dependentParams[i] ? Nest_toString(dependentParams[i]) : "null");
        }
        oss << ")";
        REP_INFO(originalFun->location, "Dependent types: %1%") % oss.str();
    }


    // Check if we have some CT or concept parameters
    // We compute the type of all the parameters, except the dependent ones
    NodeVector genericParams(numParams, nullptr);
    bool hasGenericParams = false;
    for (size_t i = 0; i < numParams; ++i) {
        Node* param = at(params, i);
        if (dependentParams[i]) {
            genericParams[i] = param;
            hasGenericParams = true;
            continue;   // Don't compute the type
        }
        if (!Nest_computeType(param))
            return nullptr;

        if (isConceptType(param->type)) {
            genericParams[i] = param;
            hasGenericParams = true;
        }
        if ((!inCtFun || isCtGeneric) && Feather_isCt(param)) {
            genericParams[i] = param;
            hasGenericParams = true;
        }
    }

    if (!hasGenericParams)
        return nullptr;

    // If a 'this' class is passed, add an extra parameter for this
    NodeVector paramsWithThis;
    if (thisClass) {
        TypeRef thisType =
                Feather_getDataType(thisClass, 1, Feather_effectiveEvalMode(originalFun));
        Node* thisParam = mkSprParameter(originalFun->location, fromCStr("this"), thisType);
        Nest_setContext(thisParam, Nest_childrenContext(originalFun));
        if (!Nest_computeType(thisParam))
            return nullptr;
        genericParams.insert(genericParams.begin(), nullptr);

        // Add 'this' param to our range of parameters
        paramsWithThis = NodeVector(params.beginPtr, params.endPtr);
        paramsWithThis.insert(paramsWithThis.begin(), thisParam);
        params = all(paramsWithThis);
    }

    // Actually create the generic
    Node* res = mkGenericFunction(originalFun, params, all(genericParams), ifClause);
    Feather_setEvalMode(res, Feather_effectiveEvalMode(originalFun));
    Nest_setContext(res, originalFun->context);
    return res;
}

NodeRange SprFrontend::genericFunParams(Node* genericFun) {
    return GenericFunNode(genericFun).originalParams();
}
NodeRange SprFrontend::genericClassParams(Node* genericClass) {
    return GenericClassNode(genericClass).instSet().params();
}

Node* SprFrontend::canInstantiateGenericFun(Node* node1, NodeRange args) {
    GenericFunNode node = node1;

    Node* originalFun = at(node.node->referredNodes, 0);
    NodeVector boundValues =
            getGenericFunBoundValues(originalFun->context, args, node.instSet().params());

    EvalMode resultingEvalMode =
            Nest_hasProperty(originalFun, propCtGeneric)
                    ? modeCt // If we have a CT generic, the resulting eval mode is
                             // always CT
                    : getGenericFunResultingEvalMode(originalFun->location,
                              Feather_effectiveEvalMode(originalFun), args,
                              node.instSet().params());

    InstSetNode instSet = node.instSet();
    InstNode inst = canInstantiate(instSet, all(boundValues), resultingEvalMode);
    return inst.node;
}
Node* SprFrontend::canInstantiateGenericClass(Node* node1, NodeRange args) {
    GenericClassNode node = node1;
    NodeVector boundValues = getGenericClassBoundValues(args);
    Node* originalClass = node.originalClass();
    EvalMode resultingEvalMode = getGenericClassResultingEvalMode(
            originalClass->location, Feather_effectiveEvalMode(originalClass), all(boundValues));
    InstNode inst = canInstantiate(node.instSet(), all(boundValues), resultingEvalMode);
    return inst.node;
}

Node* SprFrontend::callGenericFun(Node* node1, const Location& loc, CompilationContext* context,
        NodeRange args, Node* inst1) {
    ASSERT(inst1);
    InstNode inst = inst1;
    GenericFunNode node = node1;

    // If not already created, create the actual instantiation declaration
    Node* instDecl = inst.instantiatedDecl();
    Node* expandedInst = inst.expandedInstantiation();
    if (!instDecl) {
        Node* originalFun = node.originalFun();
        ASSERT(originalFun->nodeKind == nkSparrowDeclSprFunction);
        NodeVector finalParams = getGenericFunFinalParams(
                inst, originalFun, node.originalParams(), node.instSet().params());

        // Create the actual instantiation declaration
        CompilationContext* ctx = Nest_childrenContext(expandedInst);
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
Node* SprFrontend::callGenericClass(Node* node1, const Location& loc, CompilationContext* context,
        NodeRange args, Node* inst1) {
    ASSERT(inst1);
    InstNode inst = inst1;
    GenericClassNode node = node1;

    // If not already created, create the actual instantiation declaration
    Node* instDecl = inst.instantiatedDecl();
    Node* expandedInst = inst.expandedInstantiation();
    if (!instDecl) {
        Node* originalClass = Nest_ofKind(node.originalClass(), nkSparrowDeclSprClass);
        string description = getGenericClassDescription(originalClass, inst);

        // Create the actual instantiation declaration
        CompilationContext* ctx = Nest_childrenContext(expandedInst);
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
        Nest_appendNodeToArray(&node.node->additionalNodes, expandedInst);
        if (node.node->explanation && node.node->explanation != node)
            Nest_appendNodeToArray(&node.node->explanation->additionalNodes, expandedInst);
    }

    // Now actually create the call object: a Type CT value
    Node* cls = Nest_ofKind(Nest_explanation(instDecl), nkFeatherDeclClass);
    ASSERT(cls);
    return createTypeNode(node.node->context, loc, Feather_getDataType(cls, 0, modeRtCt));
}

bool SprFrontend::conceptIsFulfilled(Node* concept1, TypeRef type) {
    ASSERT(concept1);
    ConceptNode concept = concept1;
    InstSetNode instSet = concept.instSet();

    if (!concept.node->nodeSemanticallyChecked || !instSet)
        REP_INTERNAL(concept.node->location, "Invalid concept");

    Node* typeValue = createTypeNode(concept.node->context, concept.node->location, type);
    if (!Nest_semanticCheck(typeValue))
        return false;

    InstNode inst =
            canInstantiate(instSet, fromIniList({typeValue}), concept.node->context->evalMode);
    return inst.node != nullptr;
}

bool SprFrontend::typeGeneratedFromGeneric(Node* genericClass, TypeRef type) {
    ASSERT(genericClass && genericClass->nodeKind == nkSparrowDeclGenericClass);
    Node* cls = Feather_classForType(type);
    if (!cls)
        return false;

    // Simple check: location & name is the same
    return 0 == Nest_compareLocations(&genericClass->location, &cls->location) &&
           Feather_getName(genericClass) == Feather_getName(cls);
}

TypeRef SprFrontend::baseConceptType(Node* concept) {
    Node* baseConcept = at(concept->children, 0);

    TypeRef res = baseConcept ? getType(baseConcept) : getConceptType();
    return res;
}
