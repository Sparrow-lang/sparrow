#pragma once

#include "Node.h"
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
    /// Interface class for an intermediate code node
    class DynNode
    {
    public:
        explicit DynNode(int nodeKind, const Location& location, DynNodeVector children = {}, DynNodeVector referredNodes = {});
        DynNode(const DynNode& other);
        virtual ~DynNode() {}

        static DynNode* fromBasicNode(Node* basicNode);

        static void* operator new(size_t size);
        static void operator delete(void* ptr);

        /// Clones the current node
        virtual DynNode* clone() const = 0;
        
        /// Returns the object kind ID
        virtual int nodeKind() const = 0;
        
        /// Returns the object kind name
        virtual const char* nodeKindName() const = 0;

        /// Method that returns a string representation of the current object
        virtual string toString() const;

        /// Dumps the information of this node into the given stream
        virtual void dump(ostream& os) const;
        virtual void dumpWithType(ostream& os) const;

    // DynNode properties
    public:
        /// Getter for the location associated with this node
        const Location& location() const;

        /// Setter for the location associated with this node
        void setLocation(const Location& loc);

        /// Getter for the children nodes of this node - think compositon
        const DynNodeVector& children() const { return basicNode_->children; }
        DynNodeVector& children() { return basicNode_->children; }
        
        /// Getter for the list of nodes referred by this node
        const DynNodeVector& referredNodes() const { return basicNode_->referredNodes; }
        DynNodeVector& referredNodes() { return basicNode_->referredNodes; }

        const vector<Modifier*>& modifiers() const { return basicNode_->modifiers; }
        vector<Modifier*>& modifiers() { return basicNode_->modifiers; }
        
        void setProperty(const char* name, int val, bool passToExpl = false);
        void setProperty(const char* name, string val, bool passToExpl = false);
        void setProperty(const char* name, DynNode* val, bool passToExpl = false);
        void setProperty(const char* name, TypeRef val, bool passToExpl = false);

        bool hasProperty(const char* name) const;
        const int* getPropertyInt(const char* name) const;
        const string* getPropertyString(const char* name) const;
        DynNode*const* getPropertyNode(const char* name) const;
        const TypeRef* getPropertyType(const char* name) const;
        
        int getCheckPropertyInt(const char* name) const;
        const string& getCheckPropertyString(const char* name) const;
        DynNode* getCheckPropertyNode(const char* name) const;
        TypeRef getCheckPropertyType(const char* name) const;
        

        /// Getter for the type of this node
        TypeRef type() const;
        
    // DynNode transformation
    public:
        /// Returns true if this node is explained with the use of another node
        bool isExplained() const;

        /// Getter for the current explanation of this node; can return null; will not get the explanation recursively
        DynNode* curExplanation();

        /// Getter for the explanation of this node, if it has one; otherwise returns this node
        DynNode* explanation();

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
        void setExplanation(DynNode* explanation);

    // General node attributes
    protected:
        /// The basic node underlying this DynNode
        Node* basicNode_;

        // We use the following references, so that we don't change a lot of code at once
        Location& location_;
        DynNodeVector& children_;
        DynNodeVector& referredNodes_;
        PropertyMap& properties_;
        CompilationContext*& context_;
        CompilationContext*& childrenContext_;
        TypeRef& type_;
        DynNode*& explanation_;
        vector<Modifier*>& modifiers_;

        friend void save(const DynNode& obj, Common::Ser::OutArchive& ar);
        friend void load(DynNode& obj, Common::Ser::InArchive& ar);
    };

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const DynNode& n)
    {
        n.dump(os);
        return os;
    }

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const DynNode* n)
    {
        if ( n )
            n->dump(os);
        return os;
    }

    void save(const DynNode& obj, Common::Ser::OutArchive& ar);
    void load(DynNode& obj, Common::Ser::InArchive& ar);
}

#define DEFINE_NODE(className, kindId, kindName) \
    public: \
        static int classNodeKind() { return kindId; } \
        virtual int nodeKind() const { return kindId; } \
        static const char* classNodeKindName() { return kindName; } \
        virtual const char* nodeKindName() const { return kindName; } \
        virtual className* clone() const { \
            static_assert(sizeof(*this) == sizeof(Nest::DynNode), "Bad node size"); \
            return new className(*this); \
        } \
        static void* operator new(size_t size) { return DynNode::operator new(size); } \
        static void operator delete(void* ptr) { return DynNode::operator delete(ptr); } \
    private:
