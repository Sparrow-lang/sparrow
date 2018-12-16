#pragma once

#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

namespace SprFrontend {

using Nest::Location;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::StringRef;
using Nest::Type;

/**
 * @brief      A for loop node
 *
 * Similar to a while loop, but tailored specifically for iterating over ranges.
 *
 * Expand the for statement of the form
 *      for ( <name>: <type> = <range> ) action;
 *
 * into:
 *      var $rangeVar: Range = <range>;
 *      while ( ! ($rangeVar isEmpty) ; $rangeVar popFront )
 *      {
 *          var <name>: <type> = $rangeVar front;
 *          action;
 *      }
 *
 * if <type> is not present, we will use '$rangeType.RetType'
 *
 * If CT modifier is present, this will iterate the range at compile-time.
 *
 * Preconditions:
 *     - range must be a valid range; must follow the Range concept
 *
 * Postconditions:
 *     - expands into a WhileStmt that iterates over the range
 *     - value semantics is used for creating a variable with the given name at each iteration
 */
struct ForStmt : NodeHandle {
    DECLARE_NODE_COMMON(ForStmt)

    /**
     * @brief      Creates a For statement node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the for variable
     * @param[in]  typeNode The type of the for variable; optional
     * @param[in]  range    The range we are iterating
     * @param[in]  action   The action to be executed each iteration
     *
     * @return     The desired node
     */
    static ForStmt create(const Location& loc, StringRef name, NodeHandle typeNode,
            NodeHandle range, NodeHandle action);

    //! Returns the node containing the module name we are referring to
    StringRef name() const { return getCheckPropertyString("name"); };

    //! Returns the node describing the type of the for variable
    NodeHandle typeNode() const { return children()[2]; };

    //! Returns the range we are iterating
    NodeHandle range() const { return children()[0]; };

    //! Returns the action to be executed for each value
    NodeHandle action() const { return children()[1]; };

private:
    static void setContextForChildrenImpl(ForStmt node);
    static Type computeTypeImpl(ForStmt node);
    static NodeHandle semanticCheckImpl(ForStmt node);
};

/**
 * @brief      A Sparrow return statement
 *
 * Similar to a Feather ReturnStmt, but also allows conversions when returning values.
 *
 * Preconditions:
 *     - If the function returns Void, no expression is given
 *     - If an expression is given, it must be convertible to the return type
 *
 * Postcondition:
 *     - always expands into a Feather::ReturnStmt
 *     - if expression is given, applies the right conversion towards the return type
 *     - handles return value as parameter for Feather::FunctionDecl
 */
struct ReturnStmt : NodeHandle {
    DECLARE_NODE_COMMON(ReturnStmt)

    /**
     * @brief      Creates a Return statement node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  exp       The expression to be returned
     *
     * @return     The desired node
     */
    static ReturnStmt create(const Location& loc, NodeHandle exp);

    //! Returns the expression to be returned
    NodeHandle exp() const { return children()[0]; };

private:
    static NodeHandle semanticCheckImpl(ReturnStmt node);
};

} // namespace SprFrontend
