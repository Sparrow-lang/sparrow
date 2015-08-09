#pragma once

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/NodeVector.h>
#include <Nest/Intermediate/NodeKindRegistrar.h>


namespace SprFrontend
{
    using Nest::Node;
    using Nest::NodeVector;

    /// Interface class for an intermediate code node
    class DynNode
    {
    public:
        explicit DynNode(int nodeKind, const Nest::Location& location, NodeVector children = {}, NodeVector referredNodes = {});
        DynNode(const DynNode& other);

        static const DynNode* fromNode(const Nest::Node* node);
        static DynNode* fromNode(Nest::Node* node);

        Nest::Node* node() { return &data_; }
        const Nest::Node* node() const { return &data_; }

        static void* operator new(size_t size);
        static void operator delete(void* ptr);

        /// Clones the current node
        DynNode* clone() const;

        /// Returns the object kind ID
        int nodeKind() const { return data_.nodeKind; }
        
        /// Returns the object kind name
        const char* nodeKindName() const;

        /// Method that returns a string representation of the current object
        string toString() const;

        /// Dumps the information of this node into the given stream
        void dump(ostream& os) const;
        void dumpWithType(ostream& os) const;

    // DynNode properties
    public:
        /// Getter for the location associated with this node
        const Nest::Location& location() const;

        /// Setter for the location associated with this node
        void setLocation(const Nest::Location& loc);

        /// Getter for the children nodes of this node - think compositon
        const NodeVector& children() const { return data_.children; }
        NodeVector& children() { return data_.children; }
        
        /// Getter for the list of nodes referred by this node
        const NodeVector& referredNodes() const { return data_.referredNodes; }
        NodeVector& referredNodes() { return data_.referredNodes; }

        const vector<Nest::Modifier*>& modifiers() const { return data_.modifiers; }
        vector<Nest::Modifier*>& modifiers() { return data_.modifiers; }
        
        void setProperty(const char* name, int val, bool passToExpl = false);
        void setProperty(const char* name, string val, bool passToExpl = false);
        void setProperty(const char* name, Nest::Node* val, bool passToExpl = false);
        void setProperty(const char* name, Nest::TypeRef val, bool passToExpl = false);

        bool hasProperty(const char* name) const;
        const int* getPropertyInt(const char* name) const;
        const string* getPropertyString(const char* name) const;
        Nest::Node*const* getPropertyNode(const char* name) const;
        const Nest::TypeRef* getPropertyType(const char* name) const;
        
        int getCheckPropertyInt(const char* name) const;
        const string& getCheckPropertyString(const char* name) const;
        Nest::Node* getCheckPropertyNode(const char* name) const;
        Nest::TypeRef getCheckPropertyType(const char* name) const;
        

        /// Getter for the type of this node
        Nest::TypeRef type() const;
        
    // DynNode transformation
    public:
        /// Returns true if this node is explained with the use of another node
        bool isExplained() const;

        /// Getter for the current explanation of this node; can return null; will not get the explanation recursively
        Node* curExplanation();

        /// Getter for the explanation of this node, if it has one; otherwise returns this node
        Node* explanation();

        /// Setter for the children compilation context
        void setChildrenContext(Nest::CompilationContext* childrenContext);

    // Compilation processes
    public:
        /// Sets the compilation context for this node; with this we know where this node is put in the program
        void setContext(Nest::CompilationContext* context);

        /// Just computes the type, without performing all the semantic check actions; used for declarations
        void computeType(); // throws(CompilationError)

        /// Performs all the semantic-check actions
        void semanticCheck(); // throws(CompilationError)

        /// Reverts the compilation state; brings the node to un-compiled state
        void clearCompilationState(); // nothrow

        /// Add a modifier to this class; this modifier will be called before and after compilation
        void addModifier(Nest::Modifier* mod);

        /// This will return true if an error occurred at this node or in a child node
        bool hasError() const;

        /// Returns true if the node is semantically checked
        bool isSemanticallyChecked() const;

        /// Returns the compilation context of this node
        Nest::CompilationContext* context() const;

        /// Returns the compilation context for the children of this node
        Nest::CompilationContext* childrenContext() const;

    // Methods to be overridden by the subclasses to implement node functionality
    public:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();

    // Helpers
    protected:
        /// Set the explanation of this node.
        /// makes sure it has the right context, compiles it, and set the type of the current node to be the type of the
        /// explanation
        void setExplanation(Nest::Node* explanation);

    // General node attributes
    public:
        /// The basic node underlying this DynNode
        Nest::Node data_;

    protected:
        friend void save(const DynNode& obj, Nest::Common::Ser::OutArchive& ar);
        friend void load(DynNode& obj, Nest::Common::Ser::InArchive& ar);
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

    void save(const DynNode& obj, Nest::Common::Ser::OutArchive& ar);
    void load(DynNode& obj, Nest::Common::Ser::InArchive& ar);
}

#define DEFINE_NODE(className, kindId, kindName) \
    public: \
        static int& classNodeKindRef() { static int clsNodeKind = 0; return clsNodeKind; } \
        static int classNodeKind() { return classNodeKindRef(); } \
        int nodeKind() const { return data_.nodeKind; } \
        static const char* classNodeKindName() { return kindName; } \
        const char* nodeKindName() const { return Nest::getNodeKindName(classNodeKindRef()); } \
        className* clone() const { \
            static_assert(sizeof(*this) == sizeof(Nest::Node), "Bad node size"); \
            return new className(*this); \
        } \
        static void* operator new(size_t size) { return DynNode::operator new(size); } \
        static void operator delete(void* ptr) { return DynNode::operator delete(ptr); } \
    private: \
        static const char* toStringImpl(const Nest::Node* node) { ostringstream oss; static_cast<const className*>(DynNode::fromNode(node))->dump(oss); return strdup(oss.str().c_str()); } \
        static void setContextForChildrenImpl(Nest::Node* node) { static_cast<className*>(DynNode::fromNode(node))->doSetContextForChildren(); } \
        static TypeRef computeTypeImpl(Nest::Node* node) { className* thisNode = static_cast<className*>(DynNode::fromNode(node)); thisNode->doComputeType(); return thisNode->data_.type; } \
        static Nest::Node* semanticCheckImpl(Nest::Node* node) { className* thisNode = static_cast<className*>(DynNode::fromNode(node)); thisNode->doSemanticCheck(); return Nest::explanation(&thisNode->data_); } \
    public: \
        static void registerSelf() { \
            classNodeKindRef() = Nest::registerNodeKind(kindName, &semanticCheckImpl, &computeTypeImpl, &setContextForChildrenImpl, &toStringImpl); \
        } \
    private:
