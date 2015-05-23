#pragma once

#include "NodeVector.h"
#include <Nest/Frontend/Location.h>
#include <Nest/Common/Diagnostic.h>

#include <vector>
#include <boost/any.hpp>
#include <unordered_map>

FWD_CLASS1(Nest, Location);
FWD_CLASS1(Nest, CompilationContext);
FWD_CLASS1(Nest, Type);
FWD_CLASS1(Nest, Modifier);
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    /// Interface class for an intermediate code node
    class Node
    {
    public:
        explicit Node(const Location& location, NodeVector children = {}, NodeVector referredNodes = {});
        Node(const Node& other);
        virtual ~Node() {}

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
        const NodeVector& children() const { return children_; }
        NodeVector& children() { return children_; }
        
        /// Getter for the list of nodes referred by this node
        const NodeVector& referredNodes() const { return referredNodes_; }
        NodeVector& referredNodes() { return referredNodes_; }

        const vector<Modifier*>& modifiers() const { return modifiers_; }
        vector<Modifier*>& modifiers() { return modifiers_; }
        
        void setProperty(const char* name, int val, bool passToExpl = false);
        void setProperty(const char* name, string val, bool passToExpl = false);
        void setProperty(const char* name, Node* val, bool passToExpl = false);
        void setProperty(const char* name, Type* val, bool passToExpl = false);

        bool hasProperty(const char* name) const;
        const int* getPropertyInt(const char* name) const;
        const string* getPropertyString(const char* name) const;
        Node*const* getPropertyNode(const char* name) const;
        Type*const* getPropertyType(const char* name) const;
        
        int getCheckPropertyInt(const char* name) const;
        const string& getCheckPropertyString(const char* name) const;
        Node* getCheckPropertyNode(const char* name) const;
        Type* getCheckPropertyType(const char* name) const;
        

        /// Getter for the type of this node
        Type* type() const;
        
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

        /// Convert this node to the given node type; perfrom safety checks
        template <typename T>
        T* reinterpret()
        {
            if ( !this )
                REP_INTERNAL(NOLOC, "Expected AST node to reinterpret");
            if ( nodeKind() != T::classNodeKind() )
                REP_INTERNAL(location_, "AST node is not of kind %1% (it's of kind %2%)") % T::classNodeKindName() % nodeKindName();
            return static_cast<T*>(this);
        }
        
    // Helpers
    protected:
        /// Set the explanation of this node.
        /// makes sure it has the right context, compiles it, and set the type of the current node to be the type of the
        /// explanation
        void setExplanation(Node* explanation);

    // Serialization
    public:
        void save(Common::Ser::OutArchive& ar) const;
        void load(Common::Ser::InArchive& ar);

    // General node attributes
    protected:
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
            Type* typeValue_;

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
            explicit PropertyValue(Type* val)
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

        /// Node flags
        struct NodeFlags
        {
            unsigned int nodeError : 1;
            unsigned int nodeSemanticallyChecked : 1;

            unsigned int computeTypeStarted : 1;
            unsigned int semanticCheckStarted : 1;
        } flags_;

        /// The location corresponding to this node
        Location location_;

        /// The children of this node
        NodeVector children_;

        /// The nodes referred by this node
        NodeVector referredNodes_;
        
        /// The properties of the node
        PropertyMap properties_;

        /// The context of this node
        CompilationContext* context_;
        
        /// The context of the children
        CompilationContext* childrenContext_;
        
        /// The type of this node
        Type* type_;
        
        /// The explanation of this node
        /// A node has explanation if its meaning can be explained with the help of another node (the explanation node)
        Node* explanation_;

        /// The modifiers used to adjust the compilation process of this node
        vector<Modifier*> modifiers_;
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
