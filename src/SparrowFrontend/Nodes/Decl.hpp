#pragma once

#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

namespace SprFrontend {

using Nest::Node;
using Nest::Location;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::StringRef;
using Nest::Type;
using Nest::TypeWithStorage;

using Feather::NodeList;

struct InstantiationsSet;

/**
 * @brief      A modifier node
 *
 * Modifier nodes are used to modify the semantics of the nodes they apply to. For example, an
 * [autoCt] modifier applied to a function declaration makes the function to be auto-CT (call it as
 * CT if all the arguments are CT)
 *
 * Mostly it applies to decls, but not necessarily.
 */
struct ModifiersNode : NodeHandle {
    DECLARE_NODE_COMMON(ModifiersNode)

    /**
     * @brief      Creates a Modifiers node
     *
     * @param[in]  loc    The location of the node
     * @param[in]  target The node to which we apply the modifiers
     * @param[in]  mods   A modifier or a NodeList of modifiers to be applied to the node; each
     *                    modifier is an expression (identifier or function call)
     *
     * The 'mods' parameter is an expression or a node list of expressions
     *
     * @return     The desired node
     */
    static ModifiersNode create(const Location& loc, NodeHandle target, NodeHandle mods);

    //! Returns the target node to which we apply the modifier
    NodeHandle target() const { return children()[0]; };

    //! Returns the modifiers that we apply to the main node
    NodeHandle mods() const { return children()[1]; };

private:
    static void setContextForChildrenImpl(ModifiersNode node);
    static Type computeTypeImpl(ModifiersNode node);
    static NodeHandle semanticCheckImpl(ModifiersNode node);
};

/**
 * @brief      A using declaration
 *
 * A using declaration, has two forms:
 *     - using name = usedNode
 *     - using decl_exp
 *
 * For the first type of declaration, we introduce 'name' in our symtab. Then, people can refer to
 * the used node through it.
 *
 * In the second case, we find the given declaration, and we introduce its name into our symtab. The
 * effect is like having that decl inside our current symbol table.
 *
 * The used node can be an identifier, qualified identifier, or any kind of expression.
 * If name is absent, the used node must refer to a declaration.
 *
 * Preconditions:
 *     - the used node can be compiled in our current context
 *     - if the name is absent, used node refers to a declaration
 *
 * Postconditions
 *     - one entry is inserted in the current symtab
 *     - if the used node is CT, we replace it by its CT-evaluation
 *     - if the used node refers to one decl, we add it as "resulting decl" for us
 *     - this expands to Nop
 */
struct UsingDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(UsingDecl)

    /**
     * @brief      Creates a using decl node
     *
     * @param[in]  loc       The location of the node
     * @param[in]  name      The name for the using; can be empty
     * @param[in]  usedNode  The used node; required
     *
     * If name is absent, used node must refer to a decl.
     *
     * @return     The desired node
     */
    static UsingDecl create(const Location& loc, StringRef name, NodeHandle usedNode);

    //! Returns the used node
    NodeHandle usedNode() const { return children()[0]; };

private:
    static void setContextForChildrenImpl(UsingDecl node);
    static Type computeTypeImpl(UsingDecl node);
    static NodeHandle semanticCheckImpl(UsingDecl node);
};

/**
 * @brief      A package declaration
 *
 * A package is a way of hierarchically organizing declarations in the symtab. A package is always
 * introduced in a symtab, and a symtab is created for its body.
 *
 * Postconditions:
 *     - type is always Void
 *     - expands to its body
 */
struct PackageDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(PackageDecl)

    /**
     * @brief      Creates a package declaration node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the package
     * @param[in]  body     The content of the package; probably a NodeList
     * @param[in]  params   The parameters, in case of a generic package
     * @param[in]  ifClause The if clause, in case of a generic package
     *
     * Params and ifClause are used only for generic packages
     *
     * @return     The desired node
     */
    static PackageDecl create(const Location& loc, StringRef name, NodeHandle body,
            NodeList params = {}, NodeHandle ifClause = {});

    //! Returns the node containing the parameters
    NodeHandle body() const { return children()[0]; };

    //! Returns the node containing the parameters
    NodeList parameters() const { return NodeList(children()[1]); };

    //! Returns the if clause node
    NodeHandle ifClause() const { return children()[2]; };

private:
    static void setContextForChildrenImpl(PackageDecl node);
    static Type computeTypeImpl(PackageDecl node);
    static NodeHandle semanticCheckImpl(PackageDecl node);
};

/**
 * @brief      A Sparrow variable declaration
 *
 * Represents constructs of the given form:
 *     var name: type = initializer
 *
 * This can represent both a function-local variable or a global variable. The explanation of the
 * node is different based on the kind of variable.
 *
 * The explanation will contain code to call the constructor & destructor for the variable (if
 * appropriate).
 *
 * Preconditions:
 *     - either type node or initializer node is non-null
 *
 * Postconditions:
 *     - the node is explained in terms of a Feather VarDecl
 *     - if the type is not a reference, constructor & destructor calls will be generated
 *     - if the variable is local and CT, it will be transformed into a global variable
 */
struct VariableDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(VariableDecl)

    /**
     * @brief      Creates a Sparrow variable node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the variable
     * @param[in]  typeNode Node describing the type of the variable -- can be null
     * @param[in]  init     Optional initialization value for the variable
     *
     * typeNode or init can be null. But one of them needs to be valid. If the given typeNode is
     * null, the type of the variable is deduced from the initializer.
     *
     * @return     The desired node
     */
    static VariableDecl create(
            const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init);
    //! Same as the other create, but uses a direct type instead of a type node
    static VariableDecl create(
            const Location& loc, StringRef name, TypeWithStorage type, NodeHandle init);

    //! Returns the type node given for the variable
    NodeHandle typeNode() const { return children()[0]; };
    //! Returns the initializer of the variable
    NodeHandle init() const { return children()[1]; };

    //! Returns the resulting Feather VarDecl object.
    //! A Sparrow variable will always be implemented in terms for a Feather VarDecl node
    Feather::VarDecl resultingVar() const {
        return Feather::VarDecl(getCheckPropertyNode("spr.resultingVar"));
    }

private:
    static void setContextForChildrenImpl(VariableDecl node);
    static Type computeTypeImpl(VariableDecl node);
    static NodeHandle semanticCheckImpl(VariableDecl node);
};

/**
 * @brief      A datatype declaration (structure, class)
 *
 * This represents the two Sparrow constructs:
 *     - datatype <name> { <body> }
 *     - datatype <name> = <underlyingData>
 *
 * The second form, is reduced to the first one, in the following way:
 *     - datatype <name> { data: <underlyingData> }
 *
 * This can also be created with generic parameters & if clause. In this case, the datatype decl
 * explains itself into a generic datatype node.
 *
 * Preconditions:
 *     - body is a NodeList
 *     - body can contain only fields and using decls
 *
 * Postconditions:
 *     - type will be always a DataType with zero references
 *     - (if not generic) it will expand into a Feather StructDecl
 *     - adds ClassMembersIntMod modifier
 */
struct DataTypeDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(DataTypeDecl)

    /**
     * @brief      Creates a datatype declaration node
     *
     * @param[in]  loc       The location of the node
     *
     * @return     The desired node
     */
    static DataTypeDecl create(const Location& loc, StringRef name, NodeList parameters,
            NodeHandle underlyingData, NodeHandle ifClause, NodeList body);

    //! Returns the body of the data type (fields + using decls)
    NodeList body() const { return NodeList(children()[1]); };

    //! Returns the parameters given to the data type declarations (for generic datatypes)
    NodeList parameters() const { return NodeList(children()[0]); };

    //! Returns the if clause for the data type declaration (for generic datatypes)
    NodeHandle ifClause() const { return children()[2]; };

private:
    static void setContextForChildrenImpl(DataTypeDecl node);
    static Type computeTypeImpl(DataTypeDecl node);
    static NodeHandle semanticCheckImpl(DataTypeDecl node);
};

/**
 * @brief      A field declaration, part of a datatype
 *
 * Preconditions:
 *     - either type node or initializer node is non-null
 *
 * Postconditions:
 *     - the node is explained in terms of a Feather VarDecl
 */
struct FieldDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(FieldDecl)

    /**
     * @brief      Creates a field declaration node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  typeNode Node representing the type of the field
     * @param[in]  init     The initializer to be used for the field (when generating ctors)
     *
     * @return     The desired node
     */
    static FieldDecl create(
            const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init = {});

    //! Returns the type node of the field
    NodeHandle typeNode() const { return children()[0]; };

    //! Returns the initializer of the field
    NodeHandle init() const { return children()[1]; };

private:
    static void setContextForChildrenImpl(FieldDecl node);
    static Type computeTypeImpl(FieldDecl node);
    static NodeHandle semanticCheckImpl(FieldDecl node);
};

/**
 * @brief      A Sparrow function declaration
 *
 * Preconditions:
 *     - name must be valid
 *     - ifClause can only be added if this is a generic
 *
 * Postconditions:
 *     - if this is a generic, it will expand into a GenericFunctionDecl node
 *     - otherwise, it will be expanded in a Feather FunctionDecl
 *     - if the function is 'ctor' and we don't have noDefault, we add 'CtorMembersIntMod' modifier
 *     - if the function is 'dtor' and we don't have noDefault, we add 'DtorMembersIntMod' modifier
 *     - all parameter nodes need to be ParameterDecl
 *     - if we have a non-trivial result type, add a reference parameter in the explanation fun
 *     - if this is a static ctor/dtor, create a global construct/destruct action
 */
struct SprFunctionDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(SprFunctionDecl)

    /**
     * @brief      Creates a Sparrow function node
     *
     * @param[in]  loc        The location of the node
     * @param[in]  name       The name of the function
     * @param[in]  parameters The list of parameters of the function
     * @param[in]  returnType The return type of the function
     * @param[in]  body       The body of the function
     * @param[in]  ifClause   The if clause of the function
     *
     * @return     The desired node
     */
    static SprFunctionDecl create(const Location& loc, StringRef name, NodeList parameters,
            NodeHandle returnType, NodeHandle body, NodeHandle ifClause = {});

    //! Returns the parameters of the function
    NodeList parameters() const { return NodeList(children()[0]); };

    //! Returns the return type of the function (if any)
    NodeHandle returnType() const { return children()[1]; };

    //! Returns the body of the function
    NodeHandle body() const { return children()[2]; };

    //! Returns the if clause of the function, if any
    NodeHandle ifClause() const { return children()[3]; };

private:
    static void setContextForChildrenImpl(SprFunctionDecl node);
    static Type computeTypeImpl(SprFunctionDecl node);
    static NodeHandle semanticCheckImpl(SprFunctionDecl node);
};

/**
 * @brief      A parameter declaration
 *
 * Preconditions:
 *     - name must be valid
 *     - we provide a valid type (either as a node or direct)
 *
 * Postconditions:
 *     - the node is explained in terms of a Feather VarDecl
 */
struct ParameterDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(ParameterDecl)

    /**
     * @brief      Creates a parameter declaration node
     *
     * @param[in]  loc      The location of the node
     * @param[in]  name     The name of the parameter
     * @param[in]  typeNode The node indicating the type of the node
     * @param[in]  init     The initializer for the parameter
     *
     * @return     The desired node
     */
    static ParameterDecl create(
            const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init = {});
    /**
     * @brief      Creates a parameter declaration node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  name The name of the parameter
     * @param[in]  type The type of the parameter
     * @param[in]  init The initializer for the parameter
     *
     * @return     The desired node
     */
    static ParameterDecl create(
            const Location& loc, StringRef name, TypeWithStorage type, NodeHandle init = {});

    /**
     * @brief      Creates an AnyType generic parameter node
     *
     * @param[in]  loc  The location of the node
     * @param[in]  name The name of the parameter
     *
     * @return     The desired node
     */
    static ParameterDecl create(const Location& loc, StringRef name);

    //! Returns the type node of the parameter
    NodeHandle typeNode() const { return children()[0]; };

    //! Returns the initializer of the parameter
    NodeHandle init() const { return children()[1]; };

private:
    static void setContextForChildrenImpl(ParameterDecl node);
    static Type computeTypeImpl(ParameterDecl node);
    static NodeHandle semanticCheckImpl(ParameterDecl node);
};

/**
 * @brief      A concept declaration
 *
 * Postconditions:
 *     - an instantiations set node is created to implement the type/concept mapping
 */
struct ConceptDecl : Feather::DeclNode {
    DECLARE_NODE_COMMON(ConceptDecl)

    /**
     * @brief      Creates a TODO node
     *
     * @param[in]  loc         The location of the node
     * @param[in]  name        The name of the concept
     * @param[in]  paramName   The name of the parameter to be used in defining the concept
     * @param[in]  baseConcept The base concept
     * @param[in]  ifClause    The if clause that defines the concept
     *
     * @return     The desired node
     */
    static ConceptDecl create(const Location& loc, StringRef name, StringRef paramName,
            NodeHandle baseConcept, NodeHandle ifClause);

    StringRef paramName() const;

    //! Returns the base concept node
    //! This is an expression that will point to a concept
    NodeHandle baseConcept() const { return children()[0]; };

    //! Returns the if clause of the concept
    NodeHandle ifClause() const { return children()[1]; };

    //! Returns the instantiations set node created for this concept
    InstantiationsSet instantiationsSet() const;

private:
    static void setContextForChildrenImpl(ConceptDecl node);
    static NodeHandle semanticCheckImpl(ConceptDecl node);
};

} // namespace SprFrontend
