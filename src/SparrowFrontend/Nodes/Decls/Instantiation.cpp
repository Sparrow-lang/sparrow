#include <StdInc.h>
#include "Instantiation.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

Instantiation::Instantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars)
    : DynNode(classNodeKind(), loc, { Feather::mkNodeList(loc, move(boundVars)) }, move(boundValues))
{
    setProperty("instIsValid", 0);
    setProperty("instantiatedDecl", (Node*) nullptr);

}

const NodeVector& Instantiation::boundValues() const
{
    return reinterpret_cast<const NodeVector&>(data_.referredNodes);
}

Node*& Instantiation::expandedInstantiation()
{
    return data_.children[0];
}

Node* Instantiation::instantiatedDecl()
{
    return getCheckPropertyNode("instantiatedDecl");
}

void Instantiation::setInstantiatedDecl(Node* decl)
{
    setProperty("instantiatedDecl", decl);
    expandedInstantiation()->children.push_back(decl);
}

bool Instantiation::isValid() const
{
    return 0 != getCheckPropertyInt("instIsValid");
}

void Instantiation::setValid(bool valid)
{
    setProperty("instIsValid", valid ? 1 : 0);
}


void Instantiation::doSemanticCheck()
{
    setExplanation(mkNop(data_.location));
}
