#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"

namespace SprFrontend {

/*
There are 4 types of generics:
 - function generics
     - these are function that either have CT types as parameters, or have some concepts as
     parameters
     - when they are instantiated, some parameters may not be present for the instantiation
     - we distinguish between bound params (params that must be fixed before instantiation) and
     final params (params that are present in the final instantiation)
     - the concept parameters will become both bound and final params
     - we can have dependent types (a param type depends on the actual types of previous params)
 - class generics
     - all parameters must be CT
     - all parameters are considered to be bound params
     - there are no RT nor non-bound params
 - package generics
     - all parameters must be CT
     - all parameters are considered to be bound params
     - there are no RT nor non-bound params
 - concept declarations
     - the "type models concept" relation is implemented with generic instantiations
     - the concept will hold instantiations for each type that satisfies it

All concepts may have if-clauses. These are predicates that will be checked before instantiation.
IF they don't compile, or their (CT) evaluation return false, the instantiation fails.

Node organization:
- we have an Instantiation, as a part of InstantiationSet, which is a part of a generic node
- the instantiation object will contain the following:
    - the list of bound variables (created at Inst node creation)
    - then, in the same context (but not as part of it), will add:
        - the if clause
        - the copy of the node that is instantiated

Terminology:
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

For generic functions, we are building the partial instantiations iteratively. If we find some
errors, we may even leave the partial instantiations incomplete.

To check if we have dependent parameters, we check the type node of the param to contain an
identifier that refers to any of the previously seen parameters. If we detected that a parameter is
dependent, then we don't compute it's type. It will be added to the generic params, but with a null
type. A dependent param will be treated as if the type was 'AnyType', and we add an extra condition
for instantiation. A dependent param will therefore be treated as a concept param.

*/

struct InstNode {
    Node* node;

    InstNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowInnerInstantiation);
    }
    operator Node*() const { return node; }

    Node* boundVarsNode() const { return Nest::at(node->children, 0); }

    Nest_NodeRange boundValues() const { return Nest::all(node->referredNodes); }
    Nest_NodeRangeM boundValuesM() const { return Nest::allM(node->referredNodes); }

    bool isValid() const { return 0 != Nest_getCheckPropertyInt(node, "instIsValid"); }
    void setValid(bool valid = true) { Nest_setPropertyInt(node, "instIsValid", valid ? 1 : 0); }

    bool isEvaluated() const { return 0 != Nest_getCheckPropertyInt(node, "instIsEvaluated"); }
    void setEvaluated(bool evaluated = true) {
        Nest_setPropertyInt(node, "instIsEvaluated", evaluated ? 1 : 0);
    }

    Node* instantiatedDecl() const { return Nest_getCheckPropertyNode(node, "instantiatedDecl"); }
    void setInstantiatedDecl(Node* decl) {
        Nest_setPropertyNode(node, "instantiatedDecl", decl);
        Nest_appendNodeToArray(&boundVarsNode()->children, decl);
    }
};

struct InstSetNode {
    Node* node;

    InstSetNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowInnerInstantiationsSet);
    }
    operator Node*() const { return node; }

    Node* ifClause() const { return Nest::at(node->children, 0); }
    Nest_NodeArray& instantiations() const { return Nest::at(node->children, 1)->children; }

    Node* parentNode() const { return Nest::at(node->referredNodes, 0); }
    Nest_NodeRange params() const { return Nest::all(Nest::at(node->referredNodes, 1)->children); }
};

struct GenericFunNode {
    Node* node;

    GenericFunNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowDeclGenericFunction);
    }
    operator Node*() const { return node; }

    InstSetNode instSet() const { return Nest::at(node->children, 0); }

    Node* originalFun() const { return Nest::at(node->referredNodes, 0); }
    Nest_NodeRange originalParams() const { return Nest::all(Nest::at(node->referredNodes, 1)->children); }
};

struct GenericClassNode {
    Node* node;

    GenericClassNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowDeclGenericClass);
    }
    operator Node*() const { return node; }

    InstSetNode instSet() const { return Nest::at(node->children, 0); }

    Node* originalClass() const { return Nest::at(node->referredNodes, 0); }
};

struct GenericPackageNode {
    Node* node;

    GenericPackageNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowDeclGenericPackage);
    }
    operator Node*() const { return node; }

    InstSetNode instSet() const { return Nest::at(node->children, 0); }

    Node* originalPackage() const { return Nest::at(node->referredNodes, 0); }
};

struct ConceptNode {
    Node* node;

    ConceptNode(Node* n)
        : node(n) {
        ASSERT(!n || n->nodeKind == nkSparrowDeclSprConcept);
    }
    operator Node*() const { return node; }

    Node* baseConcept() const { return Nest::at(node->children, 0); }
    Node* ifClause() const { return Nest::at(node->children, 1); }
    InstSetNode instSet() const { return Nest::at(node->children, 2); }

    Node* originalClass() const { return Nest::at(node->referredNodes, 0); }
};

/**
 * Checks if the given function is a generic, and creates a generic function node for it.
 *
 * If the given function contains any generic params (CT-only or concept), then we create a generic
 * function node for it.
 *
 * @param originalFun The original function (SprFunction) to be checked
 * @param parameters  The parameters of the original function
 * @param ifClause    The if clause of the function
 *
 * @return A GenericFunction node, if this is a generic; null otherwise
 */
Node* checkCreateGenericFun(Node* originalFun, Node* parameters, Node* ifClause);

// The generic classes are created whenever we have class parameters
// In that case, we call mkGenericClass directly

/**
 * Returns the parameters that the caller needs to fill to instantiate/call the generic.
 *
 * This returns the original list of parameters, as all parameters need to be filled.
 *
 * @param genericFun The generic function node to get the parameters for
 *
 * @return The params for the generic function.
 */
Nest_NodeRange genericFunParams(Node* genericFun);
/**
 * Returns the parameters that the caller needs to fill to instantiate/call the generic.
 *
 * This returns all the parameters of the class.
 *
 * @param genericClass The generic class node to get the parameters for
 *
 * @return The params for the generic class.
 */
Nest_NodeRange genericClassParams(Node* genericClass);
/**
 * Returns the parameters that the caller needs to fill to instantiate/call the generic.
 *
 * This returns all the parameters of the package.
 *
 * @param genericPackage The generic package node to get the parameters for
 *
 * @return The params for the generic package.
 */
Nest_NodeRange genericPackageParams(Node* genericPackage);

/**
 * Search an instantiation in an instSet.
 *
 * Two instantiations are the same if the set of bound values are the same. This function checks all
 * the existing (partial) instantiations to see if they match a given set of bound values.
 *
 * @note: We allow 'values' to have a smaller size than the bound values of other instantiations.
 * @note: We may have erroneous instantiations with lower number of bound values
 *
 * @param instSet The instantiations set to search into
 * @param values  The bound values that we are using to search for the instantiation.
 *
 * @return The instantiation node if found; null otherwise
 *
 * Called only for generic functions.
 */
InstNode searchInstantiation(InstSetNode instSet, Nest_NodeRange values);

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
InstNode createNewInstantiation(InstSetNode instSet, Nest_NodeRange values, EvalMode evalMode);

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
 * @param context     The compilation context in which we are adding the bound variable
 * @param param       The parameter for which we want to create a bound variable
 * @param paramType   The the of the parameter; used when cannot deduce it directly from 'param'
 * @param boundValue  The bound value used for the type (and init) of the variable
 * @param isCtGeneric True if this is a CT-generic function
 *
 * @return The created bound variable.
 */
Node* createBoundVar(CompilationContext* context, Node* param, TypeRef paramType, Node* boundValue,
        bool isCtGeneric);

/**
 * Check if the given instantiation is valid.
 *
 * This basically checks if the if clause is valid for the given instantiation. We assume a properly
 * constructed instantiation.
 *
 * If we already checked if we can instantiate this, reuse the old result.
 *
 * @param inst    The instantiation we check if we can instantiate.
 * @param instSet The instantiations set from which inst is a part of
 *
 * @return True if the instantiation can be made
 */
bool canInstantiate(InstNode inst, InstSetNode instSet);

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
InstNode canInstantiate(InstSetNode instSet, Nest_NodeRange values, EvalMode evalMode);

/// Given a generic param type and the corresponding bound value, determine if
/// the parameter is a concept parameter.
/// For concept parameters, we store the type as a bound value.
/// Used as a low-level primitive. Should not be called for CT-generics
bool isConceptParam(Location paramLoc, TypeRef paramType, Node* boundValue);

//! The interface for the service that deals with checking concepts.
//! Used so that we can easily mock and replace this service.
struct IConceptsService {
    virtual ~IConceptsService() {}

    //! Check if the given concept is fulfilled by the given type
    virtual bool conceptIsFulfilled(Node* concept, TypeRef type) = 0;
    //! Check if the given type was generated from the given generic
    //! This will make generics behave like concepts
    virtual bool typeGeneratedFromGeneric(Node* genericDatatype, TypeRef type) = 0;

    //! Get the base concept type
    virtual TypeRef baseConceptType(Node* concept) = 0;
};

//! Implementation of the convert service
struct ConceptsService : IConceptsService {
    bool conceptIsFulfilled(Node* concept, TypeRef type) final;
    bool typeGeneratedFromGeneric(Node* genericDatatype, TypeRef type) final;
    TypeRef baseConceptType(Node* concept) final;
};

//! The convert service instance that we are using across the Sparrow compiler
extern unique_ptr<IConceptsService> g_ConceptsService;

//! Creates the default concepts service
void setDefaultConceptsService();

} // namespace SprFrontend
