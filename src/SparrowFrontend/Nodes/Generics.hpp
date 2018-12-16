#pragma once

#include "SparrowFrontend/Nodes/Decl.hpp"

namespace SprFrontend {

using Nest::Location;
using Nest::Node;
using Nest::NodeRange;
using Nest::NodeRangeM;

struct InstantiationsSet;

/**
 * @brief      A Generic package node
 *
 * This is mainly a data node. The semantic check just expands to Nop.
 */
struct GenericPackage : Feather::DeclNode {
    DECLARE_NODE_COMMON(GenericPackage)

    /**
     * @brief      Creates a GenericPackage node
     *
     * @param[in]  original   The original Package node for which we create a generic
     * @param[in]  parameters The generic parameters applied to the package
     * @param[in]  ifClause   The if clause attached to the generic (optional)
     *
     * @return     The desired node
     */
    static GenericPackage create(PackageDecl original, NodeList parameters, NodeHandle ifClause);

    //! Returns the instantiations set node created for this generic
    InstantiationsSet instSet() const;

    //! Returns the original package
    PackageDecl original() const { return PackageDecl(referredNodes()[0]); }

private:
    static NodeHandle semanticCheckImpl2(GenericPackage node);
};

/**
 * @brief      A generic datatype node
 *
 * This is mainly a data node. The semantic check just expands to Nop.
 */
struct GenericDatatype : Feather::DeclNode {
    DECLARE_NODE_COMMON(GenericDatatype)

    /**
     * @brief      Creates a GenericDatatype node
     *
     * @param[in]  original   The original datatype decl node for which we create a generic
     * @param[in]  parameters The generic parameters applied to the datatype decl
     * @param[in]  ifClause   The if clause attached to the generic (optional)
     *
     * @return     The desired node
     */
    static GenericDatatype create(DataTypeDecl original, NodeList parameters, NodeHandle ifClause);

    //! Returns the instantiations set node created for this generic
    InstantiationsSet instSet() const;

    //! Returns the original datatype decl
    DataTypeDecl original() const { return DataTypeDecl(referredNodes()[0]); }

private:
    static NodeHandle semanticCheckImpl2(GenericDatatype node);
};

/**
 * @brief      A generic function declaration
 *
 * This is mainly a data node. The semantic check just expands to Nop.
 */
struct GenericFunction : Feather::DeclNode {
    DECLARE_NODE_COMMON(GenericFunction)

    /**
     * @brief      Creates a GenericFunction node
     *
     * @param[in]  original      The original function decl node for which we create a generic
     * @param[in]  params        All the parameters of the function
     * @param[in]  genericParams The generic parameters of the function
     * @param[in]  ifClause      The if clause attached to the generic (optional)
     *
     * @return     The desired node
     */
    static GenericFunction create(SprFunctionDecl original, NodeRange params,
            NodeRange genericParams, NodeHandle ifClause);

    //! Returns the instantiations set node created for this generic
    InstantiationsSet instSet() const;

    //! Returns the original function decl
    SprFunctionDecl original() const { return SprFunctionDecl(referredNodes()[0]); }

    //! Returns the original params of the function
    Nest_NodeRange originalParams() const { return referredNodes()[1].children(); }

private:
    static NodeHandle semanticCheckImpl2(GenericFunction node);
};

/**
 * @brief      Node representing a Sparrow generic instantiation
 *
 * This is mainly a data node. The semantic check just expands to Nop.
 *
 * Each time we attempt to instantiate a generic/concept, we create a node of this type. If the
 * instantiation succeeds, we mark this node appropriately. Next time we want to instantiate the
 * generic/concept with the same parameters, we will reuse the same Instantiation node.
 *
 * Preconditions:
 *     - boundValue are a NodeList of CT values
 *     - boundVars are the variables corresponding to the bound values
 *
 * Preconditions:
 *     - expands to Nop
 */
struct Instantiation : NodeHandle {
    DECLARE_NODE_COMMON(Instantiation)

    /**
     * @brief      Creates an Instantiation node
     *
     * @param[in]  loc         The location of the node
     * @param[in]  boundValues The bound values corresponding to the generic
     * @param[in]  boundVars   The variables created to hold the bound values
     *
     * @return     The desired node
     */
    static Instantiation create(const Location& loc, NodeRange boundValues, NodeRange boundVars);

    //! Returns the node list containing all the bound variables
    NodeList boundVarsNode() const { return NodeList(children()[0]); }

    //! Returns the bound values (immutable access)
    NodeRange boundValues() const { return referredNodes(); }
    //! Returns the bound values (mutable access)
    NodeRangeM boundValuesM() const { return referredNodesM(); }

    //! Checks if the instantiation is valid
    bool isValid() const { return 0 != getCheckPropertyInt("instIsValid"); }
    //! Marks the instantiation as valid or not
    void setValid(bool valid = true) { setProperty("instIsValid", valid ? 1 : 0); }

    //! Checks if the instantiation is evaluated
    bool isEvaluated() const { return 0 != getCheckPropertyInt("instIsEvaluated"); }
    //! Marks the instantiation as evaluated or not
    void setEvaluated(bool evaluated = true) { setProperty("instIsEvaluated", evaluated ? 1 : 0); }

    //! Gets the instantiated decl corresponding to this instantiation
    NodeHandle instantiatedDecl() const { return getCheckPropertyNode("instantiatedDecl"); }
    //! Sets a decl to be the instantiated decl -- only when instantiation succeeds
    void setInstantiatedDecl(NodeHandle decl) {
        setProperty("instantiatedDecl", decl);
        boundVarsNode().addChild(decl);
    }

private:
    static NodeHandle semanticCheckImpl2(Instantiation node);
};

/**
 * @brief      A node containing a set of instantiations
 *
 * This will be added to any generic/concept and keeps track of all the instantiations created for
 * the generic/concept.
 *
 * This is mainly a data node. The semantic check just expands to Nop.
 *
 * Preconditions:
 *     - params range will contain just parameters
 *     - every added instantiation will match the parameters of this node
 *
 * Preconditions:
 *     - expands to Nop
 */
struct InstantiationsSet : NodeHandle {
    DECLARE_NODE_COMMON(InstantiationsSet)

    /**
     * @brief      Creates a InstantiationsSet node
     *
     * @param[in]  loc        The location of the node
     * @param[in]  parentNode The parent decl, a generic or concept
     * @param[in]  params     The generic parameters we have
     * @param[in]  ifClause   The if clause of the generic
     *
     * @return     The desired node
     */
    static InstantiationsSet create(
            Feather::DeclNode parentNode, NodeRange params, NodeHandle ifClause);

    //! Returns the if clause of the node
    NodeHandle ifClause() const { return children()[0]; }
    //! Returns the range of instantiations
    NodeRange instantiations() const { return children()[1].children(); }
    //! Adds a new instantiation to this instantiations set
    void addInstantiation(Instantiation inst);

    //! Returns the parent node
    Feather::DeclNode parentNode() const { return Feather::DeclNode(referredNodes()[0]); }
    //! Returns the range of parameters for this instantiation set
    NodeRange params() const { return referredNodes()[1].children(); }

private:
    static NodeHandle semanticCheckImpl2(InstantiationsSet node);
};

} // namespace SprFrontend
