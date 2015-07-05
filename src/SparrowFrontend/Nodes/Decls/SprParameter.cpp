#include <StdInc.h>
#include "SprParameter.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>

#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;

SprParameter::SprParameter(const Location& loc, string name, Node* typeNode, Node* init)
    : Node(loc, {typeNode, init})
{
    Feather::setName(this, move(name));
}

SprParameter::SprParameter(const Location& loc, string name, TypeRef type, Node* init)
    : Node(loc, {nullptr, init})
{
    Feather::setName(this, move(name));
    setProperty("spr.givenType", type);
}

Node* SprParameter::initValue() const
{
    ASSERT(children_.size() == 2);
    return children_[1];
}

void SprParameter::dump(ostream& os) const
{
    os << Feather::getName(this) << ": ";
    if ( type_ )
        os << type_;
    else
    {
        const TypeRef* givenType = getPropertyType("spr.givenType");
        if ( givenType )
            os << *givenType;
        else
            os << children_[0];
    }
    if ( children_[1] )
        os << " = " << children_[1];
}

void SprParameter::doSetContextForChildren()
{
    Feather::addToSymTab(this);

    Node::doSetContextForChildren();
}

void SprParameter::doComputeType()
{
    ASSERT(children_.size() == 2);
    Node* typeNode = children_[0];

    const TypeRef* givenType = getPropertyType("spr.givenType");
    TypeRef t = givenType ? *givenType : getType(typeNode);

    Node* resultingParam = Feather::mkVar(location_, Feather::getName(this), Feather::mkTypeNode(location_, t), 0, Feather::effectiveEvalMode(this));
    Feather::setShouldAddToSymTab(resultingParam, false);
    resultingParam->setContext(context_);
    resultingParam->computeType();
    explanation_ = resultingParam;
    type_ = resultingParam->type();
    setProperty(Feather::propResultingDecl, resultingParam);
}

void SprParameter::doSemanticCheck()
{
    computeType();

    explanation_->semanticCheck();

    Node* init = initValue();
    if ( init )
        init->semanticCheck();
}
