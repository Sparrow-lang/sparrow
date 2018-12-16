#pragma once

#include "FeatherTypes.hpp"
#include "Feather/Api/Feather.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace Feather {

using Nest::Location;
using Nest::Node;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::StringRef;
using Nest::Type;
using Nest::TypeWithStorage;

/**
 * @brief      A node that represents a no-operation
 *
 * Has a type of Void. Doesn't have any properties and doesn't check anything.
 */
struct Nop : NodeHandle {
    DECLARE_NODE_COMMON(Nop)

    /**
     * @brief      Creates a Nop node
     *
     * @param[in]  loc       The location of the node
     *
     * @return     The desired Nop node
     */
    static Nop create(const Location& loc);

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node that represents a type
 *
 * This node just returns the given type.
 */
struct TypeNode : NodeHandle {
    DECLARE_NODE_COMMON(TypeNode)

    /**
     * @brief      Creates a TypeNode node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  type      The type this node should have
     *
     * @return     The desired TypeNode node
     */
    static TypeNode create(const Location& loc, Type type);

    //! Returns the type given when created the node
    Type givenType() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node that represents a type
 *
 * This node just returns the given type.
 */
struct BackendCode : NodeHandle {
    DECLARE_NODE_COMMON(BackendCode)

    /**
     * @brief      Creates a BackendCode node
     *
     * @param[in]  loc   The location of the node
     * @param[in]  code  The backend code represented by this node
     * @param[in]  mode  The eval mode to be used
     *
     * @return     The desired BackendCode node
     */
    static BackendCode create(const Location& loc, StringRef code, EvalMode mode);

    //! Returns the code for this node
    StringRef code() const;

    //! Returns the mode given at creation
    EvalMode mode() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
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
struct NodeList : NodeHandle {
    DECLARE_NODE_COMMON(NodeList)

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
    static NodeList create(const Location& loc, NodeRange children, bool setVoid = false);

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

    //! The flag whether we yield a Void type regardless of the children.
    bool returnsVoid() const;

private:
    static Type computeTypeImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a local space (inside a function body)
 *
 * This represents just a collection of instruction nodes corresponding to a scope in a function
 * body. The type of this node is always Void.
 *
 * We can have zero instructions in a LocalSpace (although this is useless)
 */
struct LocalSpace : NodeHandle {
    DECLARE_NODE_COMMON(LocalSpace)

    /**
     * @brief      Creates a LocalSpace node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  children  The children to be added to this LocalSpace
     *
     * @return     The desired LocalSpace
     */
    static LocalSpace create(const Location& loc, NodeRange children);

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static Type computeTypeImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
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
struct GlobalConstructAction : NodeHandle {
    DECLARE_NODE_COMMON(GlobalConstructAction)

    /**
     * @brief      Creates a GlobalConstructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at startup
     *
     * @return     The desired GlobalConstructAction
     */
    static GlobalConstructAction create(const Location& loc, NodeHandle action);

    //! Gets the action to be called
    NodeHandle action() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
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
struct GlobalDestructAction : NodeHandle {
    DECLARE_NODE_COMMON(GlobalDestructAction)

    /**
     * @brief      Creates a GlobalDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at program exit
     *
     * @return     The desired GlobalDestructAction
     */
    static GlobalDestructAction create(const Location& loc, NodeHandle action);

    //! Gets the action to be called
    NodeHandle action() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node describing an action that needed to be executed at the end of scope
 *
 * This node will translate in the backend in such a way that the action given will be called at
 * after all the instructions in this scope will be executed.
 *
 * The type of this node will be Void.
 */
struct ScopeDestructAction : NodeHandle {
    DECLARE_NODE_COMMON(ScopeDestructAction)

    /**
     * @brief      Creates a ScopeDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at scope exit
     *
     * @return     The desired ScopeDestructAction
     */
    static ScopeDestructAction create(const Location& loc, NodeHandle action);

    //! Gets the action to be called
    NodeHandle action() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node describing an action that needed to be executed at the end of cur instruction.
 *
 * This node will translate in the backend in such a way that the action given will be called after
 * the current instruction is executed -- action generated by temporary destructions.
 *
 * The type of this node will be Void.
 */
struct TempDestructAction : NodeHandle {
    DECLARE_NODE_COMMON(TempDestructAction)

    /**
     * @brief      Creates a TempDestructAction node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  action    The action to be executed at the end of instruction
     *
     * @return     The desired TempDestructAction
     */
    static TempDestructAction create(const Location& loc, NodeHandle action);

    //! Gets the action to be called
    NodeHandle action() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node that changes the mode to another node
 *
 * This node will ensure that the given node will be compiled in a context with the given mode.
 *
 * The type of this node will be Void.
 */
struct ChangeMode : NodeHandle {
    DECLARE_NODE_COMMON(ChangeMode)

    /**
     * @brief      Creates a ChangeMode node from the given parameters
     *
     * @param[in]  loc       The location of the node
     * @param[in]  child     The node to change the mode for
     * @param[in]  mode      The mode we want to apply to 'child'
     *
     * @return     The desired ChangeMode node
     */
    static ChangeMode create(const Location& loc, NodeHandle child, EvalMode mode);

    //! Gets the child to change the node for
    NodeHandle child() const;

    //! The mode we want to set
    EvalMode mode() const;

    //! Changes the child of this node.
    //! If we have a compilation context, set it to the child
    void setChild(NodeHandle child);

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      Base class for the declaration nodes
 *
 * Just provide some common functionality: getting the name and the eval mode
 */
struct DeclNode : NodeHandle {
    DeclNode() = default;
    DeclNode(Node* n);

    //! Gets the name of the declaration
    StringRef name() const;

    //! Gets the eval mode for this declaration.
    //! This will always return the mode stored for the declaration, even if it's unspecified.
    EvalMode mode() const;

    //! Gets the effective eval mode for this declaration
    //! If we don't have an explicit mode set here, look in the context of this decl
    EvalMode effectiveMode() const;

    //! Helper function to set the mode of the declaration
    void setMode(EvalMode mode);

    //! Helper function to set the name and the mode of the declaration
    void setNameAndMode(StringRef name, EvalMode mode = modeUnspecified);
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
    DECLARE_NODE_COMMON(FunctionDecl)

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
    static FunctionDecl create(const Location& loc, StringRef name, NodeHandle resType,
            NodeRange params, NodeHandle body);

    //! Gets the result type node for the function
    NodeHandle resTypeNode() const;

    //! Gets the parameters of this function
    NodeRange parameters() const;

    //! Gets the body for the function
    NodeHandle body() const;

    //! Returns the call convention for this function decl
    CallConvention callConvention() const;

    /**
     * @brief      Adds a new parameter to the list of the parameters of this function
     *
     * Please note that the given parameter needs expand to a VarDecl node. That is, it doesn't
     * necessarily need to be a VarDecl node directly.
     *
     * @param[in]  param          The parameter declaration
     * @param[in]  insertInFront  True if this needs to be inserted before other params
     */
    void addParameter(DeclNode param, bool insertInFront = false);

    //! Sets the result type for this function declaration.
    void setResultType(NodeHandle resType);

    //! Sets the body for this function declaration.
    void setBody(NodeHandle body);

    //! @copydoc Nest::NodeHandle::type()
    FunctionType type() const { return FunctionType(DeclNode::type()); }

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static Type computeTypeImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A declaration that introduces a C-like struct
 *
 * This can have fields (variables) of different types inside it.
 *
 * The type of this node will be a DataType.
 */
struct StructDecl : DeclNode {
    DECLARE_NODE_COMMON(StructDecl)

    /**
     * @brief      Creates a struct node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the function
     * @param[in]  fields   The fields of the struc
     *
     * @return     The desired struct node
     */
    static StructDecl create(const Location& loc, StringRef name, NodeRange fields);

    //! Gets the fields of this struct
    NodeRange fields() const;

    //! @copydoc Nest::NodeHandle::type()
    DataType type() const { return DataType(DeclNode::type()); }

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static Type computeTypeImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
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
    DECLARE_NODE_COMMON(VarDecl)

    /**
     * @brief      Creates a variable node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the function
     * @param[in]  typeNode The node that describes the type of the variable
     *
     * @return     The desired variable node
     */
    static VarDecl create(const Location& loc, StringRef name, NodeHandle typeNode);

    //! Gets the type node of the variable
    NodeHandle typeNode() const;

    //! @copydoc Nest::NodeHandle::type()
    TypeWithStorage type() const { return TypeWithStorage(DeclNode::type()); }

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static Type computeTypeImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      An compile-time value
 *
 * Used to represent literals (integer, strings, etc). But can be used to represent complex
 * structures as well.
 *
 * Precondition:
 *     - given type has storage (of any mode)
 *     - the size of the value matches the number of bytes from given type
 *
 * Postconditions:
 *     - the type of the node matches the given type (ignoring mode)
 *     - output type is CT
 */
struct CtValueExp : NodeHandle {
    DECLARE_NODE_COMMON(CtValueExp)

    /**
     * @brief      Creates a CTValueExp node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  type     The type of this CT value
     * @param[in]  name     The actual data, in binary, for this value
     *
     * @return     The desired CT value node
     */
    static CtValueExp create(const Location& loc, TypeWithStorage type, StringRef data);

    //! Gets the type of this value
    TypeWithStorage valueType() const;

    //! Getter to the raw binary data associated with this CT value
    StringRef valueData() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    const char* toString();
};

/**
 * @brief      A node representing the Null value
 *
 * Can be of different types, as indicated at creation. Although we have the same value, we
 * distinguish between a null of an Int pointer and a null of a String pointer.
 *
 * Preconditions:
 *     - the type of the given node has storage
 *     - given type is can be dereferenced
 *
 * Postcondition:
 *     - the type of the node matches the type of the given node (ignoring mode)
 */
struct NullExp : NodeHandle {
    DECLARE_NODE_COMMON(NullExp)

    /**
     * @brief      Creates a null expression node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  typeNode The actual type of the null value
     *
     * @return     The desired null value node
     */
    static NullExp create(const Location& loc, NodeHandle typeNode);

    //! Gets the type node
    NodeHandle typeNode() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a reference to variable
 *
 * This node is used each time we refer to a variable, to get the reference to that variable. It
 * provides the means of interacting with the variable.
 *
 * Preconditions:
 *     - the given var decl node is not null
 *     - the given decl is not a field, and has storage
 *
 * Postcondition:
 *     - the resulting type will be a Mutable of the type of the var
 */
struct VarRefExp : NodeHandle {
    DECLARE_NODE_COMMON(VarRefExp)

    /**
     * @brief      Creates a variable reference node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  varDecl  The declaration of the variable we are referring to
     *
     * @return     The desired variable ref node
     */
    static VarRefExp create(const Location& loc, VarDecl varDecl);

    //! Get the variable declaration that this wants to access
    VarDecl varDecl() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a reference to a field
 *
 * This is essential the low-level node corresponding to the "." field access operator. Given an
 * expression of the base object, and a the field declaration, this provides a way for obtaining a
 * reference to the field data.
 */
struct FieldRefExp : NodeHandle {
    DECLARE_NODE_COMMON(FieldRefExp)

    /**
     * @brief      Creates a field reference node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  obj       The expression of the object containing the field
     * @param[in]  fieldDecl The actual declaration of the filed
     *
     * @return     The desired field ref node
     */
    static FieldRefExp create(const Location& loc, NodeHandle obj, VarDecl fieldDecl);

    //! Gets the object node
    NodeHandle object() const;

    //! Get the field declaration that this wants to access
    VarDecl fieldDecl() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
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
struct FunRefExp : NodeHandle {
    DECLARE_NODE_COMMON(FunRefExp)

    /**
     * @brief      Creates a function reference node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  funDecl  The actual declaration of the function
     * @param[in]  resTye   The expected result type of this expression
     *
     * @return     The desired function ref node
     */
    static FunRefExp create(const Location& loc, FunctionDecl funDecl, NodeHandle resType);

    //! Get the function declaration we are referring to
    FunctionDecl funDecl() const;

    //! Gets the node describing the result type of this node
    NodeHandle resTypeNode() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a function call expression
 *
 * Preconditions:
 *     - all input nodes are not null
 *     - the type of the arguments must match the types of the fun params (ignoring mode)
 *     - if the function is CT, all arguments must be CT
 *
 * Postcondition:
 *     - the resulting type will be the function result type (mode may vary)
 *     - if the function is 'autoCt' and all arguments are CT, the result type mode is CT
 *     - if some of the args are RT, then the result type should be RT
 */
struct FunCallExp : NodeHandle {
    DECLARE_NODE_COMMON(FunCallExp)

    /**
     * @brief      Creates a function call node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  funDecl  The declaration of the function to be called
     * @param[in]  args     The arguments used when calling the function
     *
     * @return     The desired function call node
     */
    static FunCallExp create(const Location& loc, FunctionDecl funDecl, NodeRange args);

    //! Get the function declaration we are referring to
    FunctionDecl funDecl() const;

    //! Gets the arguments used for the function call
    NodeRange arguments() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a memory load (dereference)
 *
 * Preconditions:
 *     - the given address node is not null
 *     - the type of the given address node has storage and can be dereferenced (at least 1 ref)
 *
 * Postcondition:
 *     - the type of the node has one reference less than the given type (adjusted to context)
 *     - the resulting type will not be a category type
 */
struct MemLoadExp : NodeHandle {
    DECLARE_NODE_COMMON(MemLoadExp)

    /**
     * @brief      Creates a mem load node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  address  The address we want to load from
     *
     * @return     The desired mem load node
     */
    static MemLoadExp create(const Location& loc, NodeHandle address);

    //! Get the expression we want this node to dereference
    NodeHandle address() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a memory store
 *
 * This is equivalent to the following expression in C:
 *     *address = value
 *
 * Preconditions:
 *     - the given nodes are not null
 *     - the type of the given address node has storage and can be dereferenced (at least 1 ref)
 *     - the type of the given value is the same type of the address, minus one ref; if the value
 *         type is cat type, also try removing the category when comparing
 *
 * Postcondition:
 *     - the resulting type will be Void (adjusted to context)
 */
struct MemStoreExp : NodeHandle {
    DECLARE_NODE_COMMON(MemStoreExp)

    /**
     * @brief      Creates a mem store node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  value    The value we want to store at the given address
     * @param[in]  address  The address we want to store to
     *
     * @return     The desired mem store node
     */
    static MemStoreExp create(const Location& loc, NodeHandle value, NodeHandle address);

    //! Get the value we want to store at the given address
    NodeHandle value() const;
    //! Get the address we are storing into
    NodeHandle address() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a bitcast (change the type)
 *
 * This is equivalent to the following expression in C:
 *     (destType) exp  // pointers only
 *
 * Preconditions:
 *     - the given nodes are not null
 *     - type(destType) has storage and can be dereferenced
 *     - type(exp) has storage and can be dereferenced
 *
 * Postcondition:
 *     - the resulting type will be type(destType) (adjusted to context)
 */
struct BitcastExp : NodeHandle {
    DECLARE_NODE_COMMON(BitcastExp)

    /**
     * @brief      Creates a bitcast node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  destType The new type of the expression
     * @param[in]  exp      The expression we want to change the type
     *
     * @return     The desired bitcast node
     */
    static BitcastExp create(const Location& loc, NodeHandle destType, NodeHandle exp);

    //! Get the value we want to store at the given address
    NodeHandle destTypeNode() const;
    //! Get the expression we want to change the type
    NodeHandle expression() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
    static const char* toStringImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a conditional (ternary) expression
 *
 * This is equivalent to the following expression in C:
 *     cond ? alt1 : alt2
 *
 * Preconditions:
 *     - the given nodes are not null
 *     - type(cond) has a type that points to a decl with "i1" native name
 *     - type(alt1) == type(alt2) (ignoring mode)
 *
 * Postcondition:
 *     - the resulting type will be type(alt1) (possible with a different mode)
 */
struct ConditionalExp : NodeHandle {
    DECLARE_NODE_COMMON(ConditionalExp)

    /**
     * @brief      Creates a conditional node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  cond     The node representing the condition
     * @param[in]  alt1     The first alternative node
     * @param[in]  alt2     The second alternative node
     *
     * @return     The desired conditional node
     */
    static ConditionalExp create(
            const Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the first alternative node
    NodeHandle alt1() const;
    //! Get the second alternative node
    NodeHandle alt2() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing an 'if' statement
 *
 * TODO
 */
struct IfStmt : NodeHandle {
    DECLARE_NODE_COMMON(IfStmt)

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
    static IfStmt create(const Location& loc, NodeHandle cond, NodeHandle thenC, NodeHandle elseC);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the 'then' clause node
    NodeHandle thenClause() const;
    //! Get the 'else' clause node
    NodeHandle elseClause() const;

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a while statement
 *
 * TODO
 */
struct WhileStmt : NodeHandle {
    DECLARE_NODE_COMMON(WhileStmt)

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
    static WhileStmt create(const Location& loc, NodeHandle cond, NodeHandle body, NodeHandle step);

    //! Get the condition node
    NodeHandle condition() const;
    //! Get the body of the while loop
    NodeHandle body() const;
    //! Get the step node of the while loop
    NodeHandle step() const;

private:
    static void setContextForChildrenImpl(ThisNodeType node);
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a break statement
 *
 * TODO
 */
struct BreakStmt : NodeHandle {
    DECLARE_NODE_COMMON(BreakStmt)

    /**
     * @brief      Creates a break statement node
     *
     * @param[in]  loc      The location of the node
     *
     * @return     The desired break statement node
     */
    static BreakStmt create(const Location& loc);

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a continue statement
 *
 * TODO
 */
struct ContinueStmt : NodeHandle {
    DECLARE_NODE_COMMON(ContinueStmt)

    /**
     * @brief      Creates a continue statement node
     *
     * @param[in]  loc      The location of the node
     *
     * @return     The desired continue statement node
     */
    static ContinueStmt create(const Location& loc);

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

/**
 * @brief      A node representing a return statement
 *
 * TODO
 */
struct ReturnStmt : NodeHandle {
    DECLARE_NODE_COMMON(ReturnStmt)

    /**
     * @brief      Creates a return statement node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  exp      The expression we are returning; can be null
     *
     * @return     The desired return statement node
     */
    static ReturnStmt create(const Location& loc, NodeHandle exp);

    //! Get the expression we are returning; can be null
    NodeHandle expression() const;

private:
    static NodeHandle semanticCheckImpl(ThisNodeType node);
};

} // namespace Feather
