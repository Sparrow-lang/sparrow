#pragma once

#include <Nest/Frontend/Location.h>

#include <string>
#include <vector>

namespace Feather
{
    using Nest::Location;

    /// Class that represent an AST node in the FSimple source code
    class SimpleAstNode
    {
    public:
        /// The AST node type
        enum NodeType
        {
            typeIdent,
            typeStringConst,
            typeIntConst,
        };

        ~SimpleAstNode();

        /// Static methods that create AST nodes
        static SimpleAstNode* identifier(const Location& loc, const string& name);
        static SimpleAstNode* stringConstant(const Location& loc, const string& val);
        static SimpleAstNode* intConstant(const Location& loc, int val);

        /// Getter for the location of this node
        const Location& location() const;

        bool isIdentifier() const;
        bool isStringConstant() const;
        bool isIntConstant() const;

        /// If the type of the node is identifier or string constant, this will return the associated value as string
        const string& stringValue() const;

        // If the type of the node is integer constant, this will return the integer value
        int intValue() const;

        /// Getter for the children nodes of this AST node
        const vector<SimpleAstNode*>& children() const;
        vector<SimpleAstNode*>& children();

        /// Adds a new child node to this node
        void addChild(SimpleAstNode* node);

    private:
        Location location_;
        NodeType type_;
        vector<SimpleAstNode*> children_;
        string stringValue_;
        int intValue_;

    private:
        SimpleAstNode(NodeType type, const Location& loc, const string& stringValue, int intValue = 0);
    };
}
