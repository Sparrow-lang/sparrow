#pragma once

namespace SprFrontend {

// There are 3 types of generics:
//  - function generics
//      - these are function that either have CT types as parameters, or have some concepts as
//      parameters
//      - when they are instantiated, some parameters may not be present for the instantiation
//      - we distinguish between bound params (params that must be fixed before instantiation) and
//      non-bound params (params that are present in the final instantiation)
//      - the concept parameters will become both bound and non-bound params
//  - class generics
//      - all parameters must be CT
//      - all parameters are considered to be bound params
//      - there are no RT nor non-bound params
//  - concept declarations
//      - the "type models concept" relation is implemented with generic instantiations
//      - the concept will hold instantiations for each type that satisfies it
//
// All concepts may have if-clauses. These are predicates that will be checked before instantiation.
// IF they return false, the instantiation fails.

/**
 * Checks if the given function is a generic, and creates a generic function node for it.
 *
 * If the given function contains any generic params (CT-only or concept), then we create a generic
 * function node for it.
 *
 * @param originalFun The original function (SprFunction) to be checked
 * @param parameters  The parameters of the original function
 * @param ifClause    The if clause of the function
 * @param thisClass   The this class for the function, if applicable
 *
 * @return A GenericFunction node, if this is a generic; null otherwise
 */
Node* checkCreateGenericFun(
        Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass = nullptr);

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
NodeRange genericFunParams(Node* genericFun);
/**
 * Returns the parameters that the caller needs to fill to instantiate/call the generic.
 *
 * This returns all the parameters of the class.
 *
 * @param genericClass The generic class node to get the parameters for
 *
 * @return The params for the generic class.
 */
NodeRange genericClassParams(Node* genericClass);

/**
 * Check if we can instantiate the generic function with the given arguments.
 *
 * This will create a (partial) instantiation for the generic. If the instantiation is valid, then
 * it will return the instantiation node. If there are errors creating the partial instantiation, or
 * if evaluating the if clause yields false, this will return null.
 *
 * @param node The generic function we want to check if we can instantiate
 * @param args The list of arguments that we want to instantiate the function with.
 *
 * @return The instantiation node, or null if the instantiation is not valid
 */
Node* canInstantiateGenericFun(Node* node, NodeRange args);
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
Node* canInstantiateGenericClass(Node* node, NodeRange args);

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
Node* callGenericFun(Node* node, const Location& loc, CompilationContext* context, NodeRange args,
        Node* instantiation);
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
Node* callGenericClass(Node* node, const Location& loc, CompilationContext* context, NodeRange args,
        Node* instantiation);

/// Check if the given concept is fulfilled by the given type
bool conceptIsFulfilled(Node* concept, TypeRef type);
/// Check if the given type was generated from the given generic
/// This will make generics behave like concepts
bool typeGeneratedFromGeneric(Node* genericClass, TypeRef type);

/// Get the base concept type
TypeRef baseConceptType(Node* concept);
}
