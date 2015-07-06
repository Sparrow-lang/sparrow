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
        DynNode* nodeValue_;
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
        explicit PropertyValue(DynNode* val)
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
    struct Node
    {
        /// DynNode flags
        unsigned int nodeKind : 16;
        unsigned int nodeError : 1;
        unsigned int nodeSemanticallyChecked : 1;
        unsigned int computeTypeStarted : 1;
        unsigned int semanticCheckStarted : 1;

        /// The location corresponding to this node
        Location location;

        /// The children of this node
        DynNodeVector children;

        /// The nodes referred by this node
        DynNodeVector referredNodes;
        
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
        DynNode* explanation;

        /// The modifiers used to adjust the compilation process of this node
        vector<Modifier*> modifiers;
    };

    /// Creates a node of the given kind
    Node* createNode(int nodeKind);

    /// Clone the given node - create one with the same characteristics
    /// We clear the compilation state of the new node when cloning
    Node* cloneNode(Node* node);
}
