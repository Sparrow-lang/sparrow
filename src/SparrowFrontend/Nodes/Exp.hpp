#pragma once

#include "SparrowFrontend/Utils/cppif/SparrowFrontendTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

namespace SprFrontend {

using Feather::NodeList;
using Nest::Location;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::StringRef;
using Nest::Type;

/**
 * @brief      A literal in the Sparrow language
 *
 * This node is constructed for all types of literals in Sparrow, from integers to strings and type
 * literals.
 *
 * Preconditions:
 *     - literal type describes a known Sparrow type
 *     - the data passed to the construction must represent an object of the given type
 *
 * Postconditions:
 *     - expands into a CtValueExp
 */
struct Literal : NodeHandle {
    DECLARE_NODE_COMMON(Literal)

    /**
     * @brief      Creates a Literal node
     *
     * @param[in]  loc     The location of the node
     * @param[in]  litType The name of the type for this literal
     * @param[in]  data    The data (binary) for the value of the literal
     *
     * @return     The desired node
     */
    static Literal create(const Location& loc, StringRef litType, StringRef data);

    //! Returns the literal type string
    StringRef typeStr() const { return getCheckPropertyString(propSprLiteralType); };

    //! Returns the literal data as string
    StringRef dataStr() const { return getCheckPropertyString(propSprLiteralData); };

    //! Returns true if this is a string literal
    bool isString() const;

private:
    static NodeHandle semanticCheckImpl(Literal node);
};

/**
 * @brief      A simple Sparrow identifier
 *
 * Preconditions:
 *     - the given id must represent a declaration that can be found by name
 *
 * Postconditions:
 *     - if it points to a lest one FieldDecl, it will expand into a CompoundExp("this", id)
 *     - if this points to one variable, it expands to a VarRefExp
 *     - if this points to one decl that can represent a type construction, expand to a CtValueExp
 *     pointing to the corresponding type
 *     - otherwise, if decl-exps are accepted, epxand into a DeclExp
 *     - otherwise, try to expand into a fun application with zero args
 */
struct Identifier : NodeHandle {
    DECLARE_NODE_COMMON(Identifier)

    /**
     * @brief      Creates an identifier node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  id   The identifier name
     *
     * @return     The desired node
     */
    static Identifier create(const Location& loc, StringRef id);

    //! Returns the identifier name
    StringRef name() const { return getCheckPropertyString("name"); };

private:
    static NodeHandle semanticCheckImpl(Identifier node);
};

/**
 * @brief      A Sparrow compound expression (i.e., <base>.id)
 *
 * Preconditions:
 *     - the base expression is valid, and represents a data type
 *     - the identifier must point to a declaration that can be found from the base datatype
 *
 * Postconditions:
 *     - same as for Identifier, but cannot expand to a simple variable
 */
struct CompoundExp : NodeHandle {
    DECLARE_NODE_COMMON(CompoundExp)

    /**
     * @brief      Creates a compound expression node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  base The base expression
     * @param[in]  id   The name of the identifier to be found under base
     *
     * @return     The desired node
     */
    static CompoundExp create(const Location& loc, NodeHandle base, StringRef id);

    //! Returns the base of the compound expression
    NodeHandle base() const { return children()[0]; };

    //! Returns the compound expression name
    StringRef name() const { return getCheckPropertyString("name"); };

private:
    static NodeHandle semanticCheckImpl(CompoundExp node);
    static const char* toStringImpl(CompoundExp node);
};

/**
 * @brief      A Sparrow function application
 *
 * Examples:
 *     - f(a, b, c)
 *     - DataTypeName(0)
 *     - Vector(Int)
 *
 * Hardwired to handle some of the core Sparrow function (cast, ctEval, etc.)
 *
 * This can be similar to OperatorCall, but the OperatorCall has a fixed form, and the lookup rules
 * are a bit different.
 *
 * Preconditions:
 *     - the base node is valid, and must expand to one or more declarations to call, or to a type
 *     - an overload can be found to call the function
 *
 * Postconditions:
 *     - depending on the overload, expands into Feather FunCall, ctor call, generic instantiation,
 *       etc.
 */
struct FunApplication : NodeHandle {
    DECLARE_NODE_COMMON(FunApplication)

    /**
     * @brief      Creates a FunApplication node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  base      Base node from which we deduce the decl (function) to call
     * @param[in]  arguments The arguments of the calls
     *
     * @return     The desired node
     */
    static FunApplication create(const Location& loc, NodeHandle base, NodeList arguments);
    //! same as above, but using NodeRange for arguments.
    static FunApplication create(const Location& loc, NodeHandle base, NodeRange arguments);
    //! same as above, but using a string for the function name
    static FunApplication create(const Location& loc, StringRef fname, NodeRange arguments);

    //! Returns the base function-like object we are calling
    NodeHandle base() const { return children()[0]; };

    //! Returns the node-list of arguments
    NodeList arguments() const { return NodeList(children()[1]); };

private:
    static NodeHandle semanticCheckImpl(FunApplication node);
};

/**
 * @brief      An Sparrow operator call
 *
 * This handles expressions of the form 'arg1 oper arg2'.
 * Here, one of the arguments can be null.
 *
 * Hardwired to handle some core operator calls (ref equality, assign, etc.)
 *
 * Infix, prefix and postfix expressions expand to this.
 * This node does not handle associativity and precedence.
 *
 * Compared to FunApplication, the lookup rules are different: we try to search the operator from
 * the base argument first, and then we try an upward lookup from the current context
 *
 * Preconditions:
 *     - at least one of the arguments is valid
 *     - the operator can be found according to the lookup rules
 *     - the overload can be found for the given operator with the given arguments
 *
 * Postconditions:
 *     - expands into the result of the function overload procedure
 */
struct OperatorCall : NodeHandle {
    DECLARE_NODE_COMMON(OperatorCall)

    /**
     * @brief      Creates a OperatorCall node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  arg1 The left argument; may be null
     * @param[in]  op   The name of the operator to be called
     * @param[in]  arg2 The right argument; may be null
     *
     * @return     The desired node
     */
    static OperatorCall create(const Location& loc, NodeHandle arg1, StringRef op, NodeHandle arg2);

    //! Returns the first argument
    NodeHandle arg1() const { return children()[0]; };

    //! Returns the second argument
    NodeHandle arg2() const { return children()[1]; };

    //! Returns the operator we are using
    StringRef oper() const { return getCheckPropertyString(propSprOperation); };

private:
    static NodeHandle semanticCheckImpl(OperatorCall node);
};

/**
 * @brief      A Sparrow infix operation
 *
 * This has the form 'arg1 oper arg2'.
 * Similar to OperatorCall, but handles associativity and precedence.
 *
 * To handle associativity and precedence, it swaps content with InfixOp children
 *
 * Prerequisites:
 *     - both arguments need to be valid
 *
 * Postconditions:
 *     - the right precedence is applied for a chain of InfixOp (with different operations)
 *     - the right associativity is applied for a chain of Infix Op (with the same operations)
 *     - expands into an OperatorCall node
 */
struct InfixOp : NodeHandle {
    DECLARE_NODE_COMMON(InfixOp)

    /**
     * @brief      Creates an InfixOp node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  op   The name of the operator to be called
     * @param[in]  arg1 The left argument
     * @param[in]  arg2 The right argument
     *
     * @return     The desired node
     */
    static InfixOp create(const Location& loc, StringRef op, NodeHandle arg1, NodeHandle arg2);

    //! Returns the first argument
    NodeHandle arg1() const { return children()[0]; };

    //! Returns the second argument
    NodeHandle arg2() const { return children()[1]; };

    //! Returns the operator we are using
    StringRef oper() const { return getCheckPropertyString(propSprOperation); };

private:
    static NodeHandle semanticCheckImpl(InfixOp node);
};

/**
 * @brief      A lambda expression
 *
 * This is an expression of the following form:
 *     (fun.{closureParams} (param): returnType = bodyExp)
 *     (fun.{closureParams} (param): returnType {body})
 *     (fun.{closureParams} param = bodyExp)
 *     (fun.{closureParams} param {body})
 *
 * This will expand to:
 *
 * package $lambdaEnclosurePackage
 *      datatype $lambdaEnclosureData
 *          <closureParams> // as fields
 *      fun ()(this: @ $lambdaEnclosureData, <parameters>)
 *          <body>
 *
 * If a body expression is passed instead of a body, then:
 *     - the type of the function would be typeOf(bodyExp) -- it not explicitly given
 *     - the body becomes { return <bodyExp>; }
 *
 * Preconditions:
 *     - parameters is a NodeList of ParameterDecl
 *     - clossureParams is a NodeList of ParameterDecl
 *     - return type expands into something that represents a type
 *     - body/bodyExp can form a function (given parameters, closure parms and return type)
 *
 * Postconditions:
 *     - this generate a package, with a datatype and a call operator
 *     - it will expand into a FunApplication that constructs the closure class
 */
struct LambdaExp : NodeHandle {
    DECLARE_NODE_COMMON(LambdaExp)

    /**
     * @brief      Creates a LambdaExp node
     *
     * @param[in]  loc           The location of the node
     * @param[in]  parameters    The parameters of the lambda fun
     * @param[in]  returnType    The return type of the lambda (optional)
     * @param[in]  body          The body of the lambda
     * @param[in]  bodyExp       Expression to be used as the body of the lambda function
     * @param[in]  closureParams The closure params for the lambda
     *
     * Either body or bodyExp is missing, but one of them needs to be present
     *
     * @return     The desired node
     */
    static LambdaExp create(const Location& loc, NodeList parameters, NodeHandle returnType,
            NodeHandle body, NodeHandle bodyExp, NodeList closureParams);

    //! Returns the parameters
    NodeList parameters() const { return NodeList(referredNodes()[0]); };

    //! Returns the return type of the lambda
    NodeHandle returnType() const { return referredNodes()[1]; };

    //! Returns the body of the lambda
    NodeHandle body() const { return referredNodes()[2]; };

    //! Returns the closure params
    NodeList closureParams() const { return NodeList(referredNodes()[3]); };

private:
    static NodeHandle semanticCheckImpl(LambdaExp node);
};

/**
 * @brief      A Sparrow conditional expression
 *
 * Same as a Feather conditional, but also applied conversions.
 *
 * Preconditions:
 *     - all nodes are valid
 *     - condition must be a Bool type (native type: i1)
 *     - alt1 and alt2 must have a common type
 *
 *
 * Postcondition:
 *     - conversions are applied to alt1 and alt2 to get them to the common type
 *     - this expands into a Feather ConditionalExp
 */
struct ConditionalExp : NodeHandle {
    DECLARE_NODE_COMMON(ConditionalExp)

    /**
     * @brief      Creates a ConitionalExp node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  cond The condition that determines the alternative to use
     * @param[in]  alt1 The first alternative (taken if condition is true)
     * @param[in]  alt2 The second alternative (taken if condition is false)
     *
     * @return     The desired node
     */
    static ConditionalExp create(
            const Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2);

    //! Returns the condition
    NodeHandle cond() const { return children()[0]; };

    //! Returns the alternative to be used if the condition is true
    NodeHandle alt1() const { return children()[1]; };

    //! Returns the alternative to be used if the condition is false
    NodeHandle alt2() const { return children()[2]; };

private:
    static NodeHandle semanticCheckImpl(ConditionalExp node);
};

/**
 * @brief      An expression that points to one ore more declarations
 *
 * Used internally for thinks like function application names.
 * This is mostly used to store information required in different cases.
 *
 * Preconditions:
 *     - all nodes are declarations
 *
 * Postconditions:
 *     - expands to Void
 */
struct DeclExp : NodeHandle {
    DECLARE_NODE_COMMON(DeclExp)

    /**
     * @brief      Creates a DeclExp node
     *
     * @param[in]  loc     The location of the node
     * @param[in]  decls   The decls that this node refers to
     * @param[in]  baseExp Optionally, a base expression to be used with the declarations
     *
     * @return     The desired node
     */
    static DeclExp create(const Location& loc, NodeRange decls, NodeHandle baseExp = {});

    //! Returns the base expression
    NodeHandle baseExp() const { return referredNodes()[0]; };

    //! Returns the range of decls referred by this node
    NodeRange referredDecls() const;

private:
    static NodeHandle semanticCheckImpl(DeclExp node);
};

/**
 * @brief      A Sparrow star expression
 *
 * This is used for using A.* constructions.
 *
 * Preconditions:
 *     - the operator name must be "*"
 *     - base expression must point to declarations
 *
 * Postconditions:
 *     - expands to a DeclExp pointing to all the declarations in the given decls.
 */
struct StarExp : NodeHandle {
    DECLARE_NODE_COMMON(StarExp)

    /**
     * @brief      Creates a StarExp node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  baseExp   The base expression that points to declarations
     * @param[in]  operName  Should be "*"
     *
     * @return     The desired node
     */
    static StarExp create(const Location& loc, NodeHandle baseExp, StringRef operName);

    //! Returns the base expression
    NodeHandle baseExp() const { return children()[0]; };

    //! Returns the operator name
    // StringRef oper() const { return children()[0]; };

private:
    static NodeHandle semanticCheckImpl(StarExp node);
};

} // namespace SprFrontend
