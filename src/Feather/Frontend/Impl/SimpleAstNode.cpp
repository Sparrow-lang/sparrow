#include <StdInc.h>
#include "SimpleAstNode.h"


using namespace Feather;

SimpleAstNode* SimpleAstNode::identifier(const Location& loc, const string& name)
{
    return new SimpleAstNode(typeIdent, loc, name);
}

SimpleAstNode* SimpleAstNode::stringConstant(const Location& loc, const string& val)
{
    return new SimpleAstNode(typeStringConst, loc, val);
}

SimpleAstNode* SimpleAstNode::intConstant(const Location& loc, int val)
{
    return new SimpleAstNode(typeIntConst, loc, string(), val);
}

SimpleAstNode::~SimpleAstNode()
{
    for ( auto child: children_ )
    {
        delete child;
    }
}

const Location& SimpleAstNode::location() const
{
    return location_;
}

bool SimpleAstNode::isIdentifier() const
{
    return type_ == typeIdent;
}

bool SimpleAstNode::isStringConstant() const
{
    return type_ == typeStringConst;
}

bool SimpleAstNode::isIntConstant() const
{
    return type_ == typeIntConst;
}

const string& SimpleAstNode::stringValue() const
{
    return stringValue_;
}

int SimpleAstNode::intValue() const
{
    return intValue_;
}

const vector<SimpleAstNode*>& SimpleAstNode::children() const
{
    return children_;
}

vector<SimpleAstNode*>& SimpleAstNode::children()
{
    return children_;
}

void SimpleAstNode::addChild(SimpleAstNode* node)
{
    children_.push_back(node);
}

SimpleAstNode::SimpleAstNode(NodeType type, const Location& loc, const string& stringValue, int intValue)
    : location_(loc), type_(type), stringValue_(stringValue), intValue_(intValue)
{
}
