#include <StdInc.h>
#include "Generics.h"
#include "SprTypeTraits.h"
#include "SprDebug.h"
#include "Nodes/Exp.hpp"

#include "Utils/cppif/SparrowFrontendTypes.hpp"
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>

#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;
using namespace Nest;

namespace SprFrontend {

unique_ptr<IConceptsService> g_ConceptsService;

namespace {

////////////////////////////////////////////////////////////////////////////
// InstantiationsSet
//

/**
 * Create all the bound variables for a (partial) instantiation.
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
 * @param context     The compilation context in which we create the bound vars
 * @param boundValues The list of bound values used in the creation of bound vars
 * @param params      The params of the instantiated set (may contain nulls for RT params)
 * @param isCtGeneric True if this is a CT-generic function
 *
 * @return A vector of all the bound variable
 */
NodeVector createAllBoundVariables(const Location& loc, CompilationContext* context,
        NodeRange boundValues, NodeRangeT<ParameterDecl> params, bool isCtGeneric) {
    // Create a variable for each bound parameter - put everything in a node list
    NodeVector nodes;
    size_t idx = 0;
    for (NodeHandle p : params) {
        NodeHandle boundValue = boundValues[idx++];
        if (!p || !boundValue)
            continue;
        Type paramType = p.type();
        if (!paramType)
            paramType = getType(boundValue); // Dependent param type case
        auto var = createBoundVar(context, ParameterDecl(p), paramType, boundValue, isCtGeneric);
        ASSERT(var);
        nodes.push_back(var);
    }
    return nodes;
}

////////////////////////////////////////////////////////////////////////////
// Generic function
//

using NamesVec = vector<StringRef>;

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
bool referencesSeenName(NodeHandle node, const NamesVec& seenNames) {
    if (!node)
        return false;
    Identifier ident = node.kindCast<Identifier>();
    if (ident) {
        // Check to see if the name matches one of our previously seen parameter names
        StringRef name = ident.name();
        for (StringRef seenName : seenNames) {
            if (name == seenName)
                return true;
        }
    } else {
        // For the rest of the nodes, check all the children
        for (NodeHandle child : node.children()) {
            if (referencesSeenName(child, seenNames))
                return true;
        }
    }
    return false;
}
} // namespace

GenericFunction checkCreateGenericFun(
        SprFunctionDecl originalFun, NodeRange params, NodeHandle ifClause) {
    auto numParams = Nest_nodeRangeSize(params);
    if (numParams == 0)
        return {}; // cannot be generic

    // If we are in a CT function, don't consider CT parameters
    bool isCtFun = originalFun.effectiveMode() == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = originalFun.hasProperty(propCtGeneric);

    // Check if we have some dependent type parameters
    vector<ParameterDecl> dependentParams(numParams, nullptr);
    NamesVec seenNames;
    seenNames.reserve(numParams);
    for (int i = 0; i < numParams; ++i) {
        ParameterDecl param = ParameterDecl(params[i]);
        NodeHandle paramType = param.typeNode();

        // Check if this parameter references a previously seen parameter
        if (i > 0 && referencesSeenName(paramType, seenNames)) {
            dependentParams[i] = param;
        }

        // Add this param name to the list of seen names
        seenNames.push_back(param.name());
    }
    ASSERT(dependentParams[0] == nullptr); // the first param cannot be dependent

    // Check if we have some CT or concept parameters
    // We compute the type of all the parameters, except the dependent ones
    vector<ParameterDecl> genericParams(numParams, nullptr);
    bool hasGenericParams = false;
    for (size_t i = 0; i < numParams; ++i) {
        ParameterDecl param = ParameterDecl(params[i]);
        if (dependentParams[i]) {
            genericParams[i] = param;
            hasGenericParams = true;
            continue; // Don't compute the type
        }
        if (!param.computeType())
            return {};

        ASSERT(param.type());
        bool isGeneric = isCtGeneric || (!isCtFun && param.type().mode() == modeCt) ||
                         isConceptType(param.type());

        if (isGeneric) {
            genericParams[i] = param;
            hasGenericParams = true;
        }
    }

    if (!hasGenericParams)
        return nullptr;

    // Actually create the generic
    auto res = GenericFunction::create(originalFun, params, genericParams, ifClause);
    copyOverloadPrio(originalFun, res);
    res.setContext(originalFun.context());
    return res;
}

Instantiation searchInstantiation(InstantiationsSet instSet, NodeRange values) {
    for (NodeHandle inst1 : instSet.instantiations()) {
        Instantiation inst = (Node*) inst1;
        auto boundValues = inst.boundValues();
        if (boundValues.size() < values.size())
            continue;

        bool argsMatch = true;
        for (size_t i = 0; i < values.size(); ++i) {
            NodeHandle boundVal = boundValues[i];
            NodeHandle val = values[i];
            if (!boundVal && !val)
                continue;
            if (!boundVal || !val || !ctValsEqual(val, boundVal)) {
                argsMatch = false;
                break;
            }
        }
        if (argsMatch)
            return inst;
    }
    return {};
}

Instantiation createNewInstantiation(InstantiationsSet instSet, NodeRange values, EvalMode evalMode) {
    ASSERT(instSet);
    const Location& loc = instSet.location();

    // Create a new context, but at the same level as the context of the parent node
    auto parentNode = instSet.parentNode();
    CompilationContext* context =
            Nest_mkChildContextWithSymTab(parentNode.context(), nullptr, evalMode);

    bool isCtGeneric = parentNode.hasProperty(propCtGeneric);

    // Create the instantiation
    auto boundVars = createAllBoundVariables(loc, context, values, instSet.params(), isCtGeneric);
    auto inst = Instantiation::create(loc, values, all(boundVars));
    // Add it to the parent instSet
    instSet.addInstantiation(inst);

    // Compile the bound vars for the newly created instantiation
    inst.boundVarsNode().setContext(context);
    if (!inst.boundVarsNode().semanticCheck())
        return {};

    return inst;
}

Feather::DeclNode createBoundVar(CompilationContext* context, ParameterDecl param, Type paramType,
        NodeHandle boundValue, bool isCtGeneric) {
    ASSERT(param);
    ASSERT(paramType);
    ASSERT(boundValue && boundValue.type());

    Location loc = param.location();
    bool isConcept = !isCtGeneric && isConceptParam(paramType, boundValue);

    StringRef name = param.name();
    Feather::DeclNode res;
    if (isConcept) {
        // Make sure to mimic the parameter/variable behavior when bounding the value
        Type t = getType(boundValue);
        auto var = Feather::VarDecl::create(loc, name, Feather::TypeNode::create(loc, t));
        if (t.mode() == modeCt)
            var.setMode(modeCt);
        Nest_symTabEnter(context->currentSymTab, name.begin, var);
        res = var;
    } else {
        // We are interested only in the value; a simple using is perfect
        auto usng = UsingDecl::create(loc, name, boundValue.clone());
        usng.setMode(modeCt);
        res = usng;
    }

    res.setContext(context);
    return res;
}

bool canInstantiate(Instantiation inst, InstantiationsSet instSet) {
    // If we already evaluated this, we already know the answer
    if (inst.isEvaluated())
        return inst.isValid();

    // From this point, we consider our instantiation evaluated
    // If we fail, then we will not set that it's valid
    inst.setEvaluated();

    // If we have an if clause, check if this CT evaluates to true
    NodeHandle ifClause = instSet.ifClause();
    if (ifClause) {
        // Always use a clone of the original node
        NodeHandle cond = ifClause.clone();
        cond.setContext(inst.boundVarsNode().childrenContext());

        // If the condition does not compile, we cannot instantiate
        bool isValid = false;
        int oldVal = Nest_isReportingEnabled();
        Nest_enableReporting(0);
        try {
            auto res = cond.semanticCheck();
            isValid = res != nullptr && Feather_isCt(res)    // We must have a value at CT
                      && Feather_isTestable(res)             // The value must be boolean
                      && Nest_getSuppressedErrorsNum() == 0; // No suppressed errors
        } catch (...) {
        }
        Nest_enableReporting(oldVal);
        if (!isValid)
            return false;

        // Evaluate the if clause condition and check the result
        if (!SprFrontend::getBoolCtValue(Nest_ctEval(cond)))
            return false;
    }

    inst.setValid();
    return true;
}

Instantiation canInstantiate(InstantiationsSet instSet, NodeRange values, EvalMode evalMode) {
    if (values.size() != instSet.params().size())
        return {};

    // Try to find an existing instantiation
    Instantiation inst = searchInstantiation(instSet, values);

    // If no instantiation is found, create a new instantiation
    if (!inst)
        inst = createNewInstantiation(instSet, values, evalMode);
    if (!inst)
        return {};

    return canInstantiate(inst, instSet) ? inst : nullptr;
}

bool isConceptParam(Type paramType, NodeHandle boundValue) {
    ASSERT(boundValue && boundValue.type());

    // Check the bound value type first; it should always be 'Type' (we store the type of the given
    // arg for concept types)
    // If it's not type, this is not a concept param
    if (boundValue.type() != StdDef::typeType || boundValue.type() == paramType)
        return false;

    // Dependent param case: if we passes the above check, treat this as a concept type
    if (!paramType)
        return true;

    // In a non-dependent parameter case, in which we have a proper paramType, check if this is a
    // concept or the param type is not CT
    ASSERT(isConceptType(paramType) || paramType == getType(boundValue));
    return isConceptType(paramType) || paramType.mode() != modeCt;
}

bool ConceptsService::conceptIsFulfilled(ConceptDecl concept, Type type) {
    ASSERT(concept);
    auto instSet = concept.instantiationsSet();

    // We only support data-like types to fulfil concepts
    // TODO (types): Expand this to all datatypes
    if (!Feather::isDataLikeType(type))
        return false;

    if (!concept.isSemanticallyChecked() || !instSet)
        REP_INTERNAL(concept.location(), "Invalid concept");
    ASSERT(instSet);

    NodeHandle typeValue = createTypeNode(concept.context(), concept.location(), type);
    if (!typeValue.semanticCheck())
        return false;

    Instantiation inst =
            canInstantiate(instSet, NodeRange({typeValue}), concept.context()->evalMode);
    return inst;
}

bool ConceptsService::typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) {
    ASSERT(genericDatatype);
    Feather::StructDecl structDecl = type.referredNode().kindCast<Feather::StructDecl>();
    if (!structDecl)
        return false;

    // Simple check: location & name is the same
    return 0 == Nest_compareLocations(&genericDatatype.location(), &structDecl.location()) &&
           genericDatatype.name() == structDecl.name();
}

ConceptType ConceptsService::baseConceptType(ConceptDecl concept) {
    auto baseConceptExp = concept.baseConcept();

    ConceptType res = baseConceptExp ? ConceptType(getType(baseConceptExp)) : ConceptType::get();
    return res;
}

void setDefaultConceptsService() {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    g_ConceptsService.reset(new ConceptsService);
}

} // namespace SprFrontend
