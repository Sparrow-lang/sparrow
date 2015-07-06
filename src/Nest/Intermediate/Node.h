#pragma once

#include "NodeVector.h"
#include <Nest/Intermediate/TypeRef.h>
#include <Nest/Frontend/Location.h>
#include <Nest/Common/Diagnostic.h>

#include <vector>
#include <boost/any.hpp>
#include <unordered_map>

FWD_CLASS1(Nest, Location);
FWD_CLASS1(Nest, CompilationContext);
FWD_CLASS1(Nest, Modifier);
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    enum PropertyKind
    {
        propInt,
        propString,
        propNode,
        propType,
    };
    union PropertyValue
    {
        int intValue_;
        string* stringValue_;
        Node* nodeValue_;
        TypeRef typeValue_;

        PropertyValue()
        {
            intValue_ = 0;
        }
        explicit PropertyValue(int val)
        {
            intValue_ = val;
        }
        explicit PropertyValue(string val)
        {
            stringValue_ = new string(move(val));
        }
        explicit PropertyValue(Node* val)
        {
            nodeValue_ = val;
        }
        explicit PropertyValue(TypeRef val)
        {
            typeValue_ = val;
        }
    };
    struct Property
    {
        PropertyKind kind_: 16;
        bool passToExpl_: 1;
        PropertyValue value_;

        Property() : kind_(propInt), passToExpl_(false), value_(0) {}
        Property(PropertyKind kind, PropertyValue value, bool passToExpl = false) : kind_(kind), passToExpl_(passToExpl), value_(value) {}
    };
    typedef unordered_map<string, Property> PropertyMap;
    typedef PropertyMap::value_type PropertyVal;

    /// Structure representing an AST node
    struct BasicNode
    {
        /// Node flags
        unsigned int nodeKind : 16;
        unsigned int nodeError : 1;
        unsigned int nodeSemanticallyChecked : 1;
        unsigned int computeTypeStarted : 1;
        unsigned int semanticCheckStarted : 1;

        /// The location corresponding to this node
        Location location;

        /// The children of this node
        NodeVector children;

        /// The nodes referred by this node
        NodeVector referredNodes;
        
        /// The properties of the node
        PropertyMap properties;

        /// The context of this node
        CompilationContext* context;
        
        /// The context of the children
        CompilationContext* childrenContext;
        
        /// The type of this node
        TypeRef type;
        
        /// The explanation of this node
        /// A node has explanation if its meaning can be explained with the help of another node (the explanation node)
        Node* explanation;

        /// The modifiers used to adjust the compilation process of this node
        vector<Modifier*> modifiers;
    };

    /// Creates a node of the given kind
    BasicNode* createNode(int nodeKind);

    /// Clone the given node - create one with the same characteristics
    /// We clear the compilation state of the new node when cloning
    BasicNode* cloneNode(BasicNode* node);

    /// Interface class for an intermediate code node
    class Node
    {
    public:
        explicit Node(int nodeKind, const Location& location, NodeVector children = {}, NodeVector referredNodes = {});
        Node(const Node& other);
        virtual ~Node() {}

        static Node* fromBasicNode(BasicNode* basicNode);

        static void* operator new(size_t size);
        static void operator delete(void* ptr);

        /// Clones the current node
        virtual Node* clone() const = 0;
        
        /// Returns the object kind ID
        virtual int nodeKind() const = 0;
        
        /// Returns the object kind name
        virtual const char* nodeKindName() const = 0;

        /// Method that returns a string representation of the current object
        virtual string toString() const;

        /// Dumps the information of this node into the given stream
        virtual void dump(ostream& os) const;
        virtual void dumpWithType(ostream& os) const;

    // Node properties
    public:
        /// Getter for the location associated with this node
        const Location& location() const;

        /// Setter for the location associated with this node
        void setLocation(const Location& loc);

        /// Getter for the children nodes of this node - think compositon
        const NodeVector& children() const { return basicNode_->children; }
        NodeVector& children() { return basicNode_->children; }
        
        /// Getter for the list of nodes referred by this node
        const NodeVector& referredNodes() const { return basicNode_->referredNodes; }
        NodeVector& referredNodes() { return basicNode_->referredNodes; }

        const vector<Modifier*>& modifiers() const { return basicNode_->modifiers; }
        vector<Modifier*>& modifiers() { return basicNode_->modifiers; }
        
        void setProperty(const char* name, int val, bool passToExpl = false);
        void setProperty(const char* name, string val, bool passToExpl = false);
        void setProperty(const char* name, Node* val, bool passToExpl = false);
        void setProperty(const char* name, TypeRef val, bool passToExpl = false);

        bool hasProperty(const char* name) const;
        const int* getPropertyInt(const char* name) const;
        const string* getPropertyString(const char* name) const;
        Node*const* getPropertyNode(const char* name) const;
        const TypeRef* getPropertyType(const char* name) const;
        
        int getCheckPropertyInt(const char* name) const;
        const string& getCheckPropertyString(const char* name) const;
        Node* getCheckPropertyNode(const char* name) const;
        TypeRef getCheckPropertyType(const char* name) const;
        

        /// Getter for the type of this node
        TypeRef type() const;
        
    // Node transformation
    public:
        /// Returns true if this node is explained with the use of another node
        bool isExplained() const;

        /// Getter for the current explanation of this node; can return null; will not get the explanation recursively
        Node* curExplanation();

        /// Getter for the explanation of this node, if it has one; otherwise returns this node
        Node* explanation();

        /// Setter for the children compilation context
        void setChildrenContext(CompilationContext* childrenContext);

    // Compilation processes
    public:
        /// Sets the compilation context for this node; with this we know where this node is put in the program
        void setContext(CompilationContext* context);

        /// Just computes the type, without performing all the semantic check actions; used for declarations
        void computeType(); // throws(CompilationError)

        /// Performs all the semantic-check actions
        void semanticCheck(); // throws(CompilationError)

        /// Reverts the compilation state; brings the node to un-compiled state
        void clearCompilationState(); // nothrow

        /// Add a modifier to this class; this modifier will be called before and after compilation
        void addModifier(Modifier* mod);

        /// This will return true if an error occurred at this node or in a child node
        bool hasError() const;

        /// Returns true if the node is semantically checked
        bool isSemanticallyChecked() const;

        /// Returns the compilation context of this node
        CompilationContext* context() const;

        /// Returns the compilation context for the children of this node
        CompilationContext* childrenContext() const;

    // Methods to be overridden by the subclasses to implement node functionality
    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();

    // Helpers
    public:

        /// Convert this node to the given node type
        template <typename T>
        T* as()
        {
            return this && nodeKind() == T::classNodeKind() ? static_cast<T*>(this) : nullptr;
        }

        /// Convert this node to the given node type; perform safety checks
        template <typename T>
        T* reinterpret()
        {
            if ( !this )
                REP_INTERNAL(NOLOC, "Expected AST node to reinterpret");
            if ( nodeKind() != T::classNodeKind() )
                REP_INTERNAL(basicNode_->location, "AST node is not of kind %1% (it's of kind %2%)") % T::classNodeKindName() % nodeKindName();
            return static_cast<T*>(this);
        }
        
    // Helpers
    protected:
        /// Set the explanation of this node.
        /// makes sure it has the right context, compiles it, and set the type of the current node to be the type of the
        /// explanation
        void setExplanation(Node* explanation);

    // General node attributes
    protected:
        /// The basic node underlying this Node
        BasicNode* basicNode_;

        // We use the following references, so that we don't change a lot of code at once
        Location& location_;
        NodeVector& children_;
        NodeVector& referredNodes_;
        PropertyMap& properties_;
        CompilationContext*& context_;
        CompilationContext*& childrenContext_;
        TypeRef& type_;
        Node*& explanation_;
        vector<Modifier*>& modifiers_;

        friend void save(const Node& obj, Common::Ser::OutArchive& ar);
        friend void load(Node& obj, Common::Ser::InArchive& ar);
    };

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const Node& n)
    {
        n.dump(os);
        return os;
    }

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const Node* n)
    {
        if ( n )
            n->dump(os);
        return os;
    }

    void save(const Node& obj, Common::Ser::OutArchive& ar);
    void load(Node& obj, Common::Ser::InArchive& ar);
}

#define DEFINE_NODE(className, kindId, kindName) \
    public: \
        static int classNodeKind() { return kindId; } \
        virtual int nodeKind() const { return kindId; } \
        static const char* classNodeKindName() { return kindName; } \
        virtual const char* nodeKindName() const { return kindName; } \
        virtual className* clone() const { \
            static_assert(sizeof(*this) == sizeof(Nest::Node), "Bad node size"); \
            return new className(*this); \
        } \
        static void* operator new(size_t size) { return Node::operator new(size); } \
        static void operator delete(void* ptr) { return Node::operator delete(ptr); } \
    private:
