#pragma once

#include "FeatherTypes.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

namespace Feather {

/**
 * @brief      A node that represents a no-operation
 *
 * Has a type of Void. Doesn't have any properties and doesn't check anything.
 */
struct Nop : Nest::NodeHandle {
    /**
     * @brief      Creates a Nop node
     *
     * @param[in]  loc       The location of the node
     *
     * @return     The desired Nop node
     */
    static Nop create(const Nest::Location& loc);

    Nop() = default;
    Nop(Nest::Node* n);
};

/**
 * @brief      A node that represents a type
 *
 * This node just returns the given type.
 */
struct TypeNode : Nest::NodeHandle {
    /**
     * @brief      Creates a TypeNode node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  type      The type this node should have
     *
     * @return     The desired TypeNode node
     */
    static TypeNode create(const Nest::Location& loc, TypeBase type);

    TypeNode() = default;
    TypeNode(Nest::Node* n);

    //! Returns the type given when created the node
    TypeBase givenType() const;
};

/**
 * @brief      A node that represents a type
 *
 * This node just returns the given type.
 */
struct BackendCode : Nest::NodeHandle {
    /**
     * @brief      Creates a BackendCode node
     *
     * @param[in]  loc   The location of the node
     * @param[in]  code  The backend code represented by this node
     * @param[in]  mode  The eval mode to be used
     *
     * @return     The desired BackendCode node
     */
    static BackendCode create(const Nest::Location& loc, Nest::StringRef code, EvalMode mode);

    BackendCode() = default;
    BackendCode(Nest::Node* n);

    //! Returns the code for this node
    Nest::StringRef code() const;

    //! Returns the mode given at creation
    EvalMode mode() const;
};

/**
 * @brief      A node representing a list of nodes
 *
 * There are two modes in which this list operates:
 *     - non void: the type of the last element in the list will become the type of the NodeList
 *     - void: the NodeList will always have the type Void.
 * The mode of the node will be adjusted according to the context.
 *
 * If no children is given to this, it will have the type Void (with the mode of the context).
 */
struct NodeList : Nest::NodeHandle {
    /**
     * @brief      Creates a NodeList node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  children  The children to be added to this NodeList
     * @param[in]  setVoid   If true, the type of this will be VoidType; otherwise the type of last
     *                       node
     *
     * @return     The desired NodeList
     */
    static NodeList create(
            const Nest::Location& loc, Nest::NodeRange children, bool setVoid = false);

    /**
     * @brief      Helper function to add a node to an already-existing NodeList
     *
     * The previous node can be null. In that case, this will create a NodeList with the given node,
     * and will make it a "void" NodeList -- its type will be Void.
     *
     * If not-null, this will modify the 'prev' node.
     *
     * @param[in]  prev  The previous NodeList; can be null
     * @param[in]  node  The node to be added
     *
     * @return     A NodeList containing all the nodes from the previous list, plus the new node.
     */
    static NodeList append(NodeList prev, NodeHandle node);

    /**
     * @brief      Helper function that concats two node lists
     *
     * If the given prev node is null, this will return the second NodeList.
     * One of the two lists should be not null.
     *
     * If not-null, this will modify the 'prev' node.
     *
     * @param[in]  prev     The previous NodeList; can be null
     * @param[in]  newNodes The new nodes to be appended
     *
     * @return     A new NodeList containing all the nodes from the two node lists
     */
    static NodeList append(NodeList prev, NodeList newNodes);

    NodeList() = default;
    NodeList(Nest::Node* n);

    //! The flag whether we yield a Void type regardless of the children.
    bool returnsVoid() const;
};

/**
 * @brief      A node representing a local space (inside a function body)
 *
 * This represents just a collection of instruction nodes corresponding to a scope in a function
 * body. The type of this node is always Void.
 *
 * We can have zero instructions in a LocalSpace (although this is useless)
 */
struct LocalSpace : Nest::NodeHandle {
    /**
     * @brief      Creates a LocalSpace node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  children  The children to be added to this LocalSpace
     *
     * @return     The desired LocalSpace
     */
    static LocalSpace create(const Nest::Location& loc, Nest::NodeRange children);

    LocalSpace() = default;
    LocalSpace(Nest::Node* n);
};

/**
 * @brief      A node describing an action that needed to be executed at program startup
 *
 * This node will translate in the backend in such a way that the action given will be called at
 * program startup.
 *
 * If the action is CT, it will be executed asap, and expand into a Nop.
 *
 * The type of this node will be Void.
 */
struct GlobalConstructAction : Nest::NodeHandle {
    /**
     * @brief      Creates a GlobalConstructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at startup
     *
     * @return     The desired GlobalConstructAction
     */
    static GlobalConstructAction create(const Nest::Location& loc, NodeHandle action);

    GlobalConstructAction() = default;
    GlobalConstructAction(Nest::Node* n);

    //! Gets the action to be called
    NodeHandle action() const;
};

/**
 * @brief      A node describing an action that needed to be executed at program exit
 *
 * This node will translate in the backend in such a way that the action given will be called at
 * program exit.
 *
 * If the action is CT, this expand into a Nop without executing the action ever.
 *
 * The type of this node will be Void.
 */
struct GlobalDestructAction : Nest::NodeHandle {
    /**
     * @brief      Creates a GlobalDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at program exit
     *
     * @return     The desired GlobalDestructAction
     */
    static GlobalDestructAction create(const Nest::Location& loc, NodeHandle action);

    GlobalDestructAction() = default;
    GlobalDestructAction(Nest::Node* n);

    //! Gets the action to be called
    NodeHandle action() const;
};

/**
 * @brief      A node describing an action that needed to be executed at the end of scope
 *
 * This node will translate in the backend in such a way that the action given will be called at
 * after all the instructions in this scope will be executed.
 *
 * The type of this node will be Void.
 */
struct ScopeDestructAction : Nest::NodeHandle {
    /**
     * @brief      Creates a ScopeDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at scope exit
     *
     * @return     The desired ScopeDestructAction
     */
    static ScopeDestructAction create(const Nest::Location& loc, NodeHandle action);

    ScopeDestructAction() = default;
    ScopeDestructAction(Nest::Node* n);

    //! Gets the action to be called
    NodeHandle action() const;
};

/**
 * @brief      A node describing an action that needed to be executed at the end of cur instruction.
 *
 * This node will translate in the backend in such a way that the action given will be called after
 * the current instruction is executed -- action generated by temporary destructions.
 *
 * The type of this node will be Void.
 */
struct TempDestructAction : Nest::NodeHandle {
    /**
     * @brief      Creates a TempDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at the end of instruction
     *
     * @return     The desired TempDestructAction
     */
    static TempDestructAction create(const Nest::Location& loc, NodeHandle action);

    TempDestructAction() = default;
    TempDestructAction(Nest::Node* n);

    //! Gets the action to be called
    NodeHandle action() const;
};

/**
 * @brief      A node that changes the mode to another node
 *
 * This node will ensure that the given node will be compiled in a context with the given mode.
 *
 * The type of this node will be Void.
 */
struct ChangeMode : Nest::NodeHandle {
    /**
     * @brief      Creates a ChangeMode node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  child     The node to change the mode for
     * @param[in]  mode      The mode we want to apply to 'child'
     *
     * @return     The desired ChangeMode node
     */
    static ChangeMode create(const Nest::Location& loc, NodeHandle child, EvalMode mode);

    ChangeMode() = default;
    ChangeMode(Nest::Node* n);

    //! Gets the child to change the node for
    NodeHandle child() const;

    //! The mode we want to set
    EvalMode mode() const;
};

/**
 * @brief      Base class for the declaration nodes
 *
 * Just provide some common functionality: getting the name and the eval mode
 */
struct DeclNode : Nest::NodeHandle {
    DeclNode() = default;
    DeclNode(Nest::Node* n);

    //! Gets the name of the declaration
    Nest::StringRef name() const;

    //! Gets the eval mode for this declaration
    EvalMode mode() const;

protected:
    //! Helper function to set the name and the mode of the declaration
    void setNameAndMode(Nest::StringRef name, EvalMode mode = modeUnspecified);
};

/**
 * @brief      A node that represents a Feather function declaration/definition
 *
 * This will provide basic function declaration/definition functionality, without any complex
 * features. It aims to translate directly to the backend code.
 *
 * The type of this node will be a FunctionType.
 */
struct FunctionDecl : DeclNode {
    /**
     * @brief      Creates a FunctionDecl node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the function
     * @param[in]  resType  The resulting type (as a node)
     * @param[in]  params   The parameters (expecting Var nodes)
     * @param[in]  body     The body of the function; can be null
     *
     * @return     The desired FunctionDecl node
     */
    static FunctionDecl create(const Nest::Location& loc, Nest::StringRef name, NodeHandle resType,
            Nest::NodeRange params, NodeHandle body);

    FunctionDecl() = default;
    FunctionDecl(Nest::Node* n);

    //! Gets the result type node for the function
    NodeHandle resType() const;

    //! Gets the parameters of this function
    Nest::NodeRange params() const;

    //! Gets the body for the function
    NodeHandle body() const;
};

/**
 * @brief      A declaration that introduces a C-like struct
 *
 * This can have fields (variables) of different types inside it.
 *
 * The type of this node will be a DataType.
 */
struct StructDecl : DeclNode {
    /**
     * @brief      Creates a struct node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the function
     * @param[in]  fields   The fields of the struc
     *
     * @return     The desired struct node
     */
    static StructDecl create(
            const Nest::Location& loc, Nest::StringRef name, Nest::NodeRange fields);

    StructDecl() = default;
    StructDecl(Nest::Node* n);

    //! Gets the fields of this struct
    Nest::NodeRange fields() const;
};

/**
 * @brief      A declaration that introduced a variable
 *
 * A variable can be:
 * - a global variable
 * - a local variable
 * - a parameter to a function
 * - a field in a struct node
 *
 * The type of this node will be the type of the variable.
 */
struct VarDecl : DeclNode {
    /**
     * @brief      Creates a variable node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the function
     * @param[in]  typeNode The node that describes the type of the variable
     *
     * @return     The desired variable node
     */
    static VarDecl create(const Nest::Location& loc, Nest::StringRef name, NodeHandle typeNode);

    VarDecl() = default;
    VarDecl(Nest::Node* n);

    //! Gets the type node of the variable
    NodeHandle typeNode() const;
};

/**
 * @brief      An compile-time value
 *
 * Used to represent literals (integer, strings, etc). But can be used to represent complex
 * structures as well.
 */
struct CtValueExp : Nest::NodeHandle {
    /**
     * @brief      Creates a CTValueExp node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  type     The type of this CT value
     * @param[in]  name     The actual data, in binary, for this value
     *
     * @return     The desired CT value node
     */
    static CtValueExp create(const Nest::Location& loc, TypeBase type, Nest::StringRef data);

    CtValueExp() = default;
    CtValueExp(Nest::Node* n);

    //! Gets the type of this value
    TypeBase valueType() const;

    //! Getter to the raw binary data associated with this CT value
    Nest::StringRef valueData() const;
};

/**
 * @brief      A node representing the Null value
 *
 * Can be of different types, as indicated at creation. Although we have the same value, we
 * distinguish between a null of an Int pointer and a null of a String pointer.
 */
struct NullExp : Nest::NodeHandle {
    /**
     * @brief      Creates a null expression node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  typeNode The actual type of the null value
     *
     * @return     The desired null value node
     */
    static NullExp create(const Nest::Location& loc, NodeHandle typeNode);

    NullExp() = default;
    NullExp(Nest::Node* n);

    //! Gets the type node
    NodeHandle typeNode() const;
};

/**
 * @brief      A node representing a reference to variable
 *
 * This node is used each time we refer to a variable, to get the reference to that variable. It
 * provides the means of interacting with the variable.
 */
struct VarRefExp : Nest::NodeHandle {
    /**
     * @brief      Creates a variable reference node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  varDecl  The declaration of the variable we are referring to
     *
     * @return     The desired variable ref node
     */
    static VarRefExp create(const Nest::Location& loc, VarDecl varDecl);

    VarRefExp() = default;
    VarRefExp(Nest::Node* n);

    //! Get the variable declaration that this wants to access
    VarDecl varDecl() const;
};

/**
 * @brief      A node representing a reference to a field
 *
 * This is essential the low-level node corresponding to the "." field access operator. Given an
 * expression of the base object, and a the field declaration, this provides a way for obtaining a
 * reference to the field data.
 */
struct FieldRefExp : Nest::NodeHandle {
    /**
     * @brief      Creates a field reference node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  obj       The expression of the object containing the field
     * @param[in]  fieldDecl The actual declaration of the filed
     *
     * @return     The desired field ref node
     */
    static FieldRefExp create(const Nest::Location& loc, NodeHandle obj, VarDecl fieldDecl);

    FieldRefExp() = default;
    FieldRefExp(Nest::Node* n);

    //! Gets the object node
    NodeHandle object() const;

    //! Get the field declaration that this wants to access
    VarDecl fieldDecl() const;
};

/**
 * @brief      A node representing a reference to a function
 *
 * This is equivalent to taking the pointer to a function in C.
 * This is the way for which we can operate with function pointers; the only way we can use
 * functions inside expressions.
 *
 * As we always assign datatypes to function references, the type we want this to have is given at
 * node creation.
 *
 * @warning     There is no enforced correlation between the type of the function declaration and
 *              the result type given to this expression.
 */
struct FunRefExp : Nest::NodeHandle {
    /**
     * @brief      Creates a function reference node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  funDecl  The actual declaration of the function
     * @param[in]  resTye   The expected result type of this expression
     *
     * @return     The desired function ref node
     */
    static FunRefExp create(const Nest::Location& loc, FunctionDecl funDecl, NodeHandle resType);

    FunRefExp() = default;
    FunRefExp(Nest::Node* n);

    //! Get the function declaration we are referring to
    FunctionDecl funDecl() const;

    //! Gets the node describing the result type of this node
    NodeHandle resTypeNode() const;
};

/**
 * @brief      A node representing a function call expression
 *
 * TODO
 */
struct FunCallExp : Nest::NodeHandle {
    /**
     * @brief      Creates a function call node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  funDecl  The declaration of the function to be called
     * @param[in]  args     The arguments used when calling the function
     *
     * @return     The desired function call node
     */
    static FunCallExp create(const Nest::Location& loc, FunctionDecl funDecl, Nest::NodeRange args);

    FunCallExp() = default;
    FunCallExp(Nest::Node* n);

    //! Get the function declaration we are referring to
    FunctionDecl funDecl() const;

    //! Gets the arguments used for the function call
    Nest::NodeRange arguments() const;
};

/**
 * @brief      A node representing a memory load (dereference)
 *
 * TODO
 */
struct MemLoadExp : Nest::NodeHandle {
    /**
     * @brief      Creates a mem load node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  address  The address we want to load from
     *
     * @return     The desired mem load node
     */
    static MemLoadExp create(const Nest::Location& loc, NodeHandle address);

    MemLoadExp() = default;
    MemLoadExp(Nest::Node* n);

    //! Get the expression we want this node to dereference
    NodeHandle address() const;
};

/**
 * @brief      A node representing a memory store
 *
 * TODO
 */
struct MemStoreExp : Nest::NodeHandle {
    /**
     * @brief      Creates a mem store node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  value    The value we want to store at the given address
     * @param[in]  address  The address we want to store to
     *
     * @return     The desired mem store node
     */
    static MemStoreExp create(const Nest::Location& loc, NodeHandle value, NodeHandle address);

    MemStoreExp() = default;
    MemStoreExp(Nest::Node* n);

    //! Get the value we want to store at the given address
    NodeHandle value() const;
    //! Get the address we are storing into
    NodeHandle address() const;
};

/**
 * @brief      A node representing a bitcast (change the type)
 *
 * TODO
 */
struct BitcastExp : Nest::NodeHandle {
    /**
     * @brief      Creates a bitcast node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  destType The new type of the expression
     * @param[in]  exp      The expression we want to change the type
     *
     * @return     The desired bitcast node
     */
    static BitcastExp create(const Nest::Location& loc, NodeHandle destType, NodeHandle exp);

    BitcastExp() = default;
    BitcastExp(Nest::Node* n);

    //! Get the value we want to store at the given address
    NodeHandle destTypeNode() const;
    //! Get the expression we want to change the type
    NodeHandle expression() const;
};

/**
 * @brief      A node representing a conditional (ternary) expression
 *
 * TODO
 */
struct ConditionalExp : Nest::NodeHandle {
    /**
     * @brief      Creates a conditional node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  cond     The node representing the condition
     * @param[in]  alt1     The first alternative node
     * @param[in]  alt2     The seond alternative node
     *
     * @return     The desired conditional node
     */
    static ConditionalExp create(
            const Nest::Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2);

    ConditionalExp() = default;
    ConditionalExp(Nest::Node* n);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the first alternative node
    NodeHandle alt1() const;
    //! Get the second alternative node
    NodeHandle alt2() const;
};

/**
 * @brief      A node representing an 'if' statement
 *
 * TODO
 */
struct IfStmt : Nest::NodeHandle {
    /**
     * @brief      Creates a if statement node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  cond     The node representing the condition
     * @param[in]  thenC    The 'then' clause
     * @param[in]  elseC    The 'else' clause; can be null
     *
     * @return     The desired if statement node
     */
    static IfStmt create(
            const Nest::Location& loc, NodeHandle cond, NodeHandle thenC, NodeHandle elseC);

    IfStmt() = default;
    IfStmt(Nest::Node* n);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the 'then' clause node
    NodeHandle thenClause() const;
    //! Get the 'else' clause node
    NodeHandle elseClause() const;
};

/**
 * @brief      A node representing a while statement
 *
 * TODO
 */
struct WhileStmt : Nest::NodeHandle {
    /**
     * @brief      Creates a while statement node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  cond     The condition of the while loop
     * @param[in]  body     The body of the while loop
     * @param[in]  step     The 'step' -- action to be executed between iteration; can be null
     *
     * @return     The desired while statement node
     */
    static WhileStmt create(
            const Nest::Location& loc, NodeHandle cond, NodeHandle body, NodeHandle step);

    WhileStmt() = default;
    WhileStmt(Nest::Node* n);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the body of the while loop
    NodeHandle body() const;
    //! Get the step node of the while loop
    NodeHandle step() const;
};

/**
 * @brief      A node representing a break statement
 *
 * TODO
 */
struct BreakStmt : Nest::NodeHandle {
    /**
     * @brief      Creates a break statement node
     *
     * @param[in]  loc      The location of the node
     *
     * @return     The desired break statement node
     */
    static BreakStmt create(const Nest::Location& loc);

    BreakStmt() = default;
    BreakStmt(Nest::Node* n);
};

/**
 * @brief      A node representing a continue statement
 *
 * TODO
 */
struct ContinueStmt : Nest::NodeHandle {
    /**
     * @brief      Creates a continue statement node
     *
     * @param[in]  loc      The location of the node
     *
     * @return     The desired continue statement node
     */
    static ContinueStmt create(const Nest::Location& loc);

    ContinueStmt() = default;
    ContinueStmt(Nest::Node* n);
};

/**
 * @brief      A node representing a return statement
 *
 * TODO
 */
struct ReturnStmt : Nest::NodeHandle {
    /**
     * @brief      Creates a return statement node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  exp      The expression we are returning; can be null
     *
     * @return     The desired return statement node
     */
    static ReturnStmt create(const Nest::Location& loc, NodeHandle exp);

    ReturnStmt() = default;
    ReturnStmt(Nest::Node* n);

    //! Get the expression we are returning; can be null
    NodeHandle expression() const;
};

} // namespace Feather
