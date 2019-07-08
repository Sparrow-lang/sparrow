#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/NodeUtils.h"

namespace Nest {

struct NodeRange;
struct NodeRangeM;

/**
 * @brief   A handle to a Sparrow node.
 *
 * This will not free the node on destructor.
 *
 * The purpose of this class is to add various utilities on the Node structure.
 *
 * @see     Nest_Node
 */
struct NodeHandle {
    //! The actual handle to the node
    Nest_Node* handle{nullptr};
    NodeHandle(){};
    //! Construct a node handle from an actual C-style Nest_Node pointer
    NodeHandle(Nest_Node* h)
        : handle(h) {}

    //! Automatic conversion to Nest_Node*
    operator Nest_Node*() const { return handle; }

    //! Checks if the current node is not null
    explicit operator bool() const { return handle != nullptr; }

    //! Makes a cast based of the kind.
    //! If the current node has the kind of the given node type, then make the cast. Otherwise,
    //! construct an empty result type.
    //! This is somehow similar to dynamic_cast, but based on statically available info.
    template <typename T> T kindCast() const {
        return handle && kind() == T::staticKind() ? T(handle) : T();
    }

    //!@{ Creation & cloning

    //! Creates a node of the given kind
    static NodeHandle create(int nodeKind, const Location& loc = Location{});

    //! Clone the given node - create one with the same characteristics
    //! We clear the compilation state of the new node when cloning
    NodeHandle clone();

    //!@}
    //!@{ Node attributes

    //! Getter for the kind of the node
    int kind() const { return handle->nodeKind; }

    //! Getter for the location of the node
    const Location& location() const { return handle->location; }
    //! Getter for the type of the node
    Type type() const { return handle->type; }
    //! Getter for the current (direct, firs-degree) explanation of this node
    NodeHandle curExplanation() const { return handle->explanation; }
    //! Getter for the context of the node
    CompilationContext* context() const { return handle->context; }

    //!@}
    //!@{ Compilation processes

    //! Sets the compilation context for this node; with this we know where this node is put in the
    //! program. This operation always succeeds. Setting the context for the children may fail; the
    //! error needs to be handled by the user
    void setContext(CompilationContext* context);

    //! Just computes the type, without performing all the semantic check actions; used for
    //! declarations Returns the type if succeeded, NULL if failure
    Type computeType();

    //! Performs all the semantic-check actions
    //! Returns the explanation if succeeded, NULL if failure
    NodeHandle semanticCheck();

    //! Reverts the compilation state; brings the node to un-compiled state
    void clearCompilationStateSimple();
    void clearCompilationState();

    //!@}
    //!@{ General node operations

    //! Returns a string description out of this
    const char* toString() const;
    //! Returns a more detailed string description out of this
    const char* toStringEx() const;
    //! Getter for the name of the kind of this node
    const char* kindName() const;

    //! Getter for the children of this node
    NodeRange children() const;
    //! Getter for the mutable version of the children of this node
    NodeRangeM childrenM() const;

    //! Getter for the list of nodes referred by this node
    NodeRange referredNodes() const;
    //! Getter for the list of nodes referred by this node -- mutable version
    NodeRangeM referredNodesM() const;

    //! Sets the children of the given node
    void setChildren(NodeRange children);
    //! Add a child to this node
    void addChild(NodeHandle child);
    //! Add the given children range to this node
    void addChildren(NodeRange children);

    //! Set the referred nodes for this node
    void setReferredNodes(NodeRange nodes);
    //! Add the given nodes to he referred nodes
    void addReferredNodes(NodeRange nodes);

    //!@}
    //!@{ Node properties

    void setProperty(Nest_NodeProperty prop);
    void setProperty(const char* name, int val);
    void setProperty(const char* name, StringRef val);
    void setProperty(const char* name, NodeHandle val);
    void setProperty(const char* name, Type val);
    void setProperty(const char* name, void* val);

    void setPropertyExpl(const char* name, int val);
    void setPropertyExpl(const char* name, StringRef val);
    void setPropertyExpl(const char* name, NodeHandle val);
    void setPropertyExpl(const char* name, Type val);
    void setPropertyExpl(const char* name, void* val);

    bool hasProperty(const char* name) const;
    const int* getPropertyInt(const char* name) const;
    const StringRef* getPropertyString(const char* name) const;
    StringRef getPropertyStringDeref(const char* name) const;
    const NodeHandle* getPropertyNode(const char* name) const;
    const Type* getPropertyType(const char* name) const;
    void* const* getPropertyPtr(const char* name) const;

    int getPropertyDefaultInt(const char* name, int defaultVal) const;
    StringRef getPropertyDefaultString(const char* name, StringRef defaultVal) const;
    NodeHandle getPropertyDefaultNode(const char* name, NodeHandle defaultVal) const;
    Type getPropertyDefaultType(const char* name, Type defaultVal) const;
    void* getPropertyDefaultPtr(const char* name, void* defaultVal) const;

    int getCheckPropertyInt(const char* name) const;
    StringRef getCheckPropertyString(const char* name) const;
    NodeHandle getCheckPropertyNode(const char* name) const;
    Type getCheckPropertyType(const char* name) const;
    void* getCheckPropertyPtr(const char* name) const;

    //!@}
    //!@{ Compilation processes misc

    //! Indicates if the node was marked with an error
    bool hasError() const;

    //! Tests if this node was successfully semantically checked
    bool isSemanticallyChecked() const;

    //! Add a modifier to this class; this modifier will be called before and after compilation
    void addModifier(Nest_Modifier* mod);

    //! Returns the compilation context for the children of this node
    CompilationContext* childrenContext() const;

    //! Returns true if we have a dedicated children context for this node
    bool hasDedicatedChildrenContext() const;

    //! Called to the set the children context for this node
    void setChildrenContext(CompilationContext* ctx);

    //! Getter for the explanation of this node, if it has one; otherwise returns this node
    NodeHandle explanation();

    //! Returns the additional nodes of this node
    NodeRange additionalNodes() const;

    //! Add the given node as an additional node for this node
    void addAdditionalNode(NodeHandle node);

    //!@}

protected:
    // NodeHandle semanticCheckImpl();  // doesn't have a default

    //! Default handler for computing the type of this node.
    static Type computeTypeImpl(NodeHandle node);
    //! Default handler for setting the context for children
    static void setContextForChildrenImpl(NodeHandle node);
    //! Default handler for transforming this into a string
    static const char* toStringImpl(NodeHandle node);

    //! Called to the set the type for this node
    void setType(Type t);
    //! Sets the explanation of this node.
    //! Typically called when setting the explanation early, inside computeType
    void setExplanation(NodeHandle expl);
};

ostream& operator<<(ostream& os, Nest::NodeHandle n);

} // namespace Nest
