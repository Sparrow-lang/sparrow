#pragma once

#include "NodeVector.h"
#include <Nest/Intermediate/TypeRef.h>
#include <Nest/Frontend/Location.h>
#include <Nest/Common/Diagnostic.hpp>

// #include <vector>
// #include <boost/any.hpp>

FWD_CLASS3(Nest,Common,Ser, OutArchive);
FWD_CLASS3(Nest,Common,Ser, InArchive);

typedef struct Nest_Node Node;
typedef struct Nest_Modifier Modifier;
typedef struct Nest_CompilationContext CompilationContext;

enum Nest_PropertyKind
{
    propInt,
    propString,
    propNode,
    propType,
};
typedef enum Nest_PropertyKind PropertyKind;

union Nest_PropertyValue
{
    int intValue_;
    string* stringValue_;
    Node* nodeValue_;
    TypeRef typeValue_;

    Nest_PropertyValue()
    {
        intValue_ = 0;
    }
    explicit Nest_PropertyValue(int val)
    {
        intValue_ = val;
    }
    explicit Nest_PropertyValue(string val)
    {
        stringValue_ = new string(move(val));
    }
    explicit Nest_PropertyValue(Node* val)
    {
        nodeValue_ = val;
    }
    explicit Nest_PropertyValue(TypeRef val)
    {
        typeValue_ = val;
    }
};
typedef union Nest_PropertyValue PropertyValue;

struct Nest_Property
{
    PropertyKind kind_: 16;
    bool passToExpl_: 1;
    PropertyValue value_;

    Nest_Property() : kind_(propInt), passToExpl_(false), value_(0) {}
    Nest_Property(PropertyKind kind, PropertyValue value, bool passToExpl = false) : kind_(kind), passToExpl_(passToExpl), value_(value) {}
};
typedef struct Nest_Property Property;
typedef unordered_map<string, Property> PropertyMap;
typedef PropertyMap::value_type PropertyVal;

/// Structure representing an AST node
struct Nest_Node
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

typedef struct Nest_Node Nest_Node;
typedef struct Nest_Node Node;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General Node operations
//

/// Creates a node of the given kind
Node* Nest_createNode(int nodeKind);

/// Clone the given node - create one with the same characteristics
/// We clear the compilation state of the new node when cloning
Node* Nest_cloneNode(Node* node);

// Temporary methods:
void Nest_initNode(Node* node, int nodeKind);
void Nest_initCopyNode(Node* node, const Node* srcNode);

/// Returns a string description out of the given node
const char* Nest_toString(const Node* node);

/// Getter for the name of the node kind for the given node
const char* Nest_nodeKindName(const Node* node);




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node properties
//

void Nest_setProperty(Node* node, const char* name, int val, bool passToExpl = false);
void Nest_setProperty(Node* node, const char* name, string val, bool passToExpl = false);
void Nest_setProperty(Node* node, const char* name, Node* val, bool passToExpl = false);
void Nest_setProperty(Node* node, const char* name, TypeRef val, bool passToExpl = false);

bool Nest_hasProperty(const Node* node, const char* name);
const int* Nest_getPropertyInt(const Node* node, const char* name);
const string* Nest_getPropertyString(const Node* node, const char* name);
Node*const* Nest_getPropertyNode(const Node* node, const char* name);
const TypeRef* Nest_getPropertyType(const Node* node, const char* name);

int Nest_getCheckPropertyInt(const Node* node, const char* name);
const string& Nest_getCheckPropertyString(const Node* node, const char* name);
Node* Nest_getCheckPropertyNode(const Node* node, const char* name);
TypeRef Nest_getCheckPropertyType(const Node* node, const char* name);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation processes
//

/// Sets the compilation context for this node; with this we know where this node is put in the program
void Nest_setContext(Node* node, CompilationContext* context);

/// Just computes the type, without performing all the semantic check actions; used for declarations
void Nest_computeType(Node* node);

/// Performs all the semantic-check actions
void Nest_semanticCheck(Node* node);

/// Reverts the compilation state; brings the node to un-compiled state
void Nest_clearCompilationState(Node* node);

/// Add a modifier to this class; this modifier will be called before and after compilation
void Nest_addModifier(Node* node, Modifier* mod);

/// Returns the compilation context for the children of this node
CompilationContext* Nest_childrenContext(const Node* node);

/// Set the explanation of this node.
/// makes sure it has the right context, compiles it, and set the type of the current node to be the type of the
/// explanation
void Nest_setExplanation(Node* node, Node* explanation);

/// Getter for the explanation of this node, if it has one; otherwise returns this node
Node* Nest_explanation(Node* node);


template <typename T>
basic_ostream<T>& operator << (basic_ostream<T>& os, const Node* n)
{
    if ( n )
        os << Nest_toString(n);
    return os;
}

/// Returns 'src' if the given node is of the specified type; null otherwise
Node* Nest_ofKind(Node* src, int desiredNodeKind);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default implementation for node-specific functions
//

/// Returns a string description of the given node
const char* Nest_defaultFunToString(const Node* node);

/// Sets the context for all the children of the given node
void Nest_defaultFunSetContextForChildren(Node* node);

/// Computes the type of the node - calls semantic check to compute the type
TypeRef Nest_defaultFunComputeType(Node* node);
