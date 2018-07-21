#include <StdInc.h>
#include "Generics.h"
#include "SprTypeTraits.h"
#include "SprDebug.h"

#include <SparrowFrontendTypes.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Ct.h>
#include <Helpers/StdDef.h>

#include "Feather/Utils/FeatherUtils.hpp"

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
        Nest_NodeRange boundValues, Nest_NodeRange params, bool isCtGeneric) {
    // Create a variable for each bound parameter - put everything in a node list
    NodeVector nodes;
    size_t idx = 0;
    for (Node* p : params) {
        Node* boundValue = at(boundValues, idx++);
        if (!p || !boundValue)
            continue;
        TypeRef paramType = p->type;
        if (!paramType)
            paramType = getType(boundValue); // Dependent param type case
        Node* var = createBoundVar(context, p, paramType, boundValue, isCtGeneric);
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
} // namespace

Node* checkCreateGenericFun(Node* originalFun, Node* parameters, Node* ifClause) {
    ASSERT(parameters);
    Nest_NodeRange params = all(parameters->children);
    auto numParams = Nest_nodeRangeSize(params);
    if (numParams == 0)
        return nullptr; // cannot be generic

    // If we are in a CT function, don't consider CT parameters
    bool isCtFun = Feather_effectiveEvalMode(originalFun) == modeCt;
    // For CT-generics, we consider all the parameters to be generic parameters
    bool isCtGeneric = Nest_hasProperty(originalFun, propCtGeneric);

    // Check if we have some dependent type parameters
    NodeVector dependentParams(numParams, nullptr);
    NamesVec seenNames;
    seenNames.reserve(numParams);
    // bool hasDependentParams = false;
    for (int i = 0; i < numParams; ++i) {
        Node* param = at(params, i);
        Node* paramType = at(param->children, 0);
        ASSERT(param->nodeKind == nkSparrowDeclSprParameter);

        // Check if this parameter references a previously seen parameter
        if (i > 0 && referencesSeenName(paramType, seenNames)) {
            dependentParams[i] = param;
            // hasDependentParams = true;
        }

        // Add this param name to the list of seen names
        seenNames.push_back(Feather_getName(param));
    }
    ASSERT(dependentParams[0] == nullptr); // the first param cannot be dependent

    // Dependent types debug info
    // if (hasDependentParams) {
    //     ostringstream oss;
    //     oss << "(null";
    //     for (int i = 1; i < numParams; i++) {
    //         oss << ", " << (dependentParams[i] ? NodeHandle(dependentParams[i]).toString() : "null");
    //     }
    //     oss << ")";
    //     REP_INFO(originalFun->location, "Dependent types: %1%") % oss.str();
    // }

    // Check if we have some CT or concept parameters
    // We compute the type of all the parameters, except the dependent ones
    NodeVector genericParams(numParams, nullptr);
    bool hasGenericParams = false;
    for (size_t i = 0; i < numParams; ++i) {
        Node* param = at(params, i);
        if (dependentParams[i]) {
            genericParams[i] = param;
            hasGenericParams = true;
            continue; // Don't compute the type
        }
        if (!Nest_computeType(param))
            return nullptr;

        ASSERT(param->type);
        bool isGeneric = isCtGeneric || (!isCtFun && param->type->mode == modeCt) ||
                         isConceptType(param->type);

        if (isGeneric) {
            genericParams[i] = param;
            hasGenericParams = true;
        }
    }

    if (!hasGenericParams)
        return nullptr;

    // Actually create the generic
    Node* res = mkGenericFunction(originalFun, params, all(genericParams), ifClause);
    copyOverloadPrio(originalFun, res);
    Feather_setEvalMode(res, Feather_effectiveEvalMode(originalFun));
    Nest_setContext(res, originalFun->context);
    return res;
}

Nest_NodeRange genericFunParams(Node* genericFun) { return GenericFunNode(genericFun).originalParams(); }
Nest_NodeRange genericClassParams(Node* genericClass) {
    return GenericClassNode(genericClass).instSet().params();
}
Nest_NodeRange genericPackageParams(Node* genericPackage) {
    return GenericPackageNode(genericPackage).instSet().params();
}

InstNode searchInstantiation(InstSetNode instSet, Nest_NodeRange values) {
    for (InstNode inst : instSet.instantiations()) {
        const auto& boundValues = inst.boundValues();
        if (size(boundValues) < size(values))
            continue;

        bool argsMatch = true;
        for (size_t i = 0; i < size(values); ++i) {
            Node* boundVal = at(boundValues, i);
            Node* val = at(values, i);
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
    return nullptr;
}

InstNode createNewInstantiation(InstSetNode instSet, Nest_NodeRange values, EvalMode evalMode) {
    ASSERT(instSet.node);
    const Location& loc = instSet.node->location;

    // Create a new context, but at the same level as the context of the parent node
    Node* parentNode = instSet.parentNode();
    CompilationContext* context =
            Nest_mkChildContextWithSymTab(parentNode->context, nullptr, evalMode);

    bool isCtGeneric = Nest_hasProperty(parentNode, propCtGeneric);

    // Create the instantiation
    auto boundVars = createAllBoundVariables(loc, context, values, instSet.params(), isCtGeneric);
    InstNode inst = mkInstantiation(loc, values, all(boundVars));
    // Add it to the parent instSet
    Nest_appendNodeToArray(&instSet.instantiations(), inst.node);

    // Compile the bound vars for the newly created instantiation
    Nest_setContext(inst.boundVarsNode(), context);
    if (!Nest_semanticCheck(inst.boundVarsNode()))
        return nullptr;

    return inst;
}

Node* createBoundVar(CompilationContext* context, Node* param, TypeRef paramType, Node* boundValue,
        bool isCtGeneric) {
    ASSERT(param);
    ASSERT(paramType);
    ASSERT(boundValue && boundValue->type);

    Location loc = param->location;
    bool isConcept = !isCtGeneric && isConceptParam(loc, paramType, boundValue);

    StringRef name = Feather_getName(param);
    Node* var = nullptr;
    if (isConcept) {
        TypeRef t = getType(boundValue);
        var = Feather_mkVar(loc, name, Feather_mkTypeNode(loc, t));
        if (t->mode == modeCt)
            Feather_setEvalMode(var, modeCt);
        Nest_symTabEnter(context->currentSymTab, name.begin, var);
    } else {
        var = mkSprUsing(loc, name, Nest_cloneNode(boundValue));
        Feather_setEvalMode(var, modeCt);
    }

    Nest_setContext(var, context);
    return var;
}

bool canInstantiate(InstNode inst, InstSetNode instSet) {
    // If we already evaluated this, we already know the answer
    if (inst.isEvaluated())
        return inst.isValid();

    // From this point, we consider our instantiation evaluated
    // If we fail, then we will not set that it's valid
    inst.setEvaluated();

    // If we have an if clause, check if this CT evaluates to true
    Node* ifClause = instSet.ifClause();
    if (ifClause) {
        // Always use a clone of the original node
        Node* cond = Nest_cloneNode(ifClause);
        Nest_setContext(cond, Nest_childrenContext(inst.boundVarsNode()));

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
            return false;

        // Evaluate the if clause condition and check the result
        if (!SprFrontend::getBoolCtValue(Nest_ctEval(cond)))
            return false;
    }

    inst.setValid();
    return true;
}

InstNode canInstantiate(InstSetNode instSet, Nest_NodeRange values, EvalMode evalMode) {
    // Try to find an existing instantiation
    InstNode inst = searchInstantiation(instSet, values);

    // If no instantiation is found, create a new instantiation
    if (!inst)
        inst = createNewInstantiation(instSet, values, evalMode);
    if (!inst)
        return nullptr;

    return canInstantiate(inst, instSet) ? inst : nullptr;
}

bool isConceptParam(Location paramLoc, TypeRef paramType, Node* boundValue) {
    ASSERT(boundValue && boundValue->type);

    // Check the bound value type first; it should always be 'Type' (we store the type of the given
    // arg for concept types)
    // If it's not type, this is not a concept param
    if (boundValue->type != StdDef::typeType || boundValue->type == paramType)
        return false;

    // Dependent param case: if we passes the above check, treat this as a concept type
    if (!paramType)
        return true;

    // In a non-dependent parameter case, in which we have a proper paramType, check if this is a
    // concept or the param type is not CT
    ASSERT(isConceptType(paramType) || paramType == getType(boundValue));
    return isConceptType(paramType) || paramType->mode != modeCt;
}

bool ConceptsService::conceptIsFulfilled(Node* concept1, TypeRef type) {
    ASSERT(concept1);
    ConceptNode concept = concept1;
    InstSetNode instSet = concept.instSet();

    // We only support datatype and lvalue types to fulfil concepts
    // TODO (types): Expand this to all datatypes
    int kind = type->typeKind;
    if (kind != Feather_getDataTypeKind() && kind != Feather_getLValueTypeKind())
        return false;

    if (!concept.node->nodeSemanticallyChecked || !instSet)
        REP_INTERNAL(concept.node->location, "Invalid concept");
    ASSERT(instSet);

    Node* typeValue = createTypeNode(concept.node->context, concept.node->location, type);
    if (!Nest_semanticCheck(typeValue))
        return false;

    InstNode inst =
            canInstantiate(instSet, fromIniList({typeValue}), concept.node->context->evalMode);
    return inst.node != nullptr;
}

bool ConceptsService::typeGeneratedFromGeneric(Node* genericClass, TypeRef type) {
    ASSERT(genericClass && genericClass->nodeKind == nkSparrowDeclGenericClass);
    Node* cls = Feather_classForType(type);
    if (!cls)
        return false;

    // Simple check: location & name is the same
    return 0 == Nest_compareLocations(&genericClass->location, &cls->location) &&
           Feather_getName(genericClass) == Feather_getName(cls);
}

TypeRef ConceptsService::baseConceptType(Node* concept) {
    Node* baseConcept = at(concept->children, 0);

    TypeRef res = baseConcept ? getType(baseConcept) : getConceptType();
    return res;
}

void setDefaultConceptsService() { g_ConceptsService.reset(new ConceptsService); }

} // namespace SprFrontend
