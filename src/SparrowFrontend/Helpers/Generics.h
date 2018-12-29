#pragma once

#include "SparrowFrontend/NodeCommonsH.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Generics.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"

namespace SprFrontend {

struct ConceptType;

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

/**
 * Checks if the given function is a generic, and creates a generic function node for it.
 *
 * If the given function contains any generic params (CT-only or concept), then we create a generic
 * function node for it.
 *
 * @param originalFun The original function (SprFunction) to be checked
 * @param params      The parameters of the original function
 * @param ifClause    The if clause of the function
 *
 * @return A GenericFunction node, if this is a generic; null otherwise
 */
GenericFunction checkCreateGenericFun(SprFunctionDecl originalFun, NodeRange params, NodeHandle ifClause);

// For other generic types, we know we need generic whenever we have parameters.


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
Instantiation searchInstantiation(InstantiationsSet instSet, NodeRange values);

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
Instantiation createNewInstantiation(InstantiationsSet instSet, NodeRange values, EvalMode evalMode);

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
 * @return The created bound variable. Can be either a Feather::VarDecl or a UsingDecl
 */
Feather::DeclNode createBoundVar(CompilationContext* context, ParameterDecl param, Type paramType, NodeHandle boundValue,
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
bool canInstantiate(Instantiation inst, InstantiationsSet instSet);

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
Instantiation canInstantiate(InstantiationsSet instSet, NodeRange values, EvalMode evalMode);

//! Given a generic param type and the corresponding bound value, determine if
//! the parameter is a concept parameter.
//! For concept parameters, we store the type as a bound value.
//! Used as a low-level primitive. Should not be called for CT-generics
bool isConceptParam(Type paramType, NodeHandle boundValue);

//! The interface for the service that deals with checking concepts.
//! Used so that we can easily mock and replace this service.
struct IConceptsService {
    virtual ~IConceptsService() {}

    //! Check if the given concept is fulfilled by the given type
    virtual bool conceptIsFulfilled(ConceptDecl concept, Type type) = 0;
    //! Check if the given type was generated from the given generic
    //! This will make generics behave like concepts
    virtual bool typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) = 0;

    //! Get the base concept type
    virtual ConceptType baseConceptType(ConceptDecl concept) = 0;
};

//! Implementation of the convert service
struct ConceptsService : IConceptsService {
    bool conceptIsFulfilled(ConceptDecl concept, Type type) final;
    bool typeGeneratedFromGeneric(GenericDatatype genericDatatype, Type type) final;
    ConceptType baseConceptType(ConceptDecl concept) final;
};

//! The convert service instance that we are using across the Sparrow compiler
extern unique_ptr<IConceptsService> g_ConceptsService;

//! Creates the default concepts service
void setDefaultConceptsService();

} // namespace SprFrontend
