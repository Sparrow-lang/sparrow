#include <StdInc.h>
#include "TypeNode.h"
#include <Feather/FeatherNodeCommonsCpp.h>


TypeNode::TypeNode(const Location& loc, Type* type)
    : Node(loc)
{
    setProperty("givenType", type);
}

void TypeNode::dump(ostream& os) const
{
    os << "type(" << getCheckPropertyType("givenType") << ")";
}

void TypeNode::doSemanticCheck()
{
    type_ = getCheckPropertyType("givenType");
}
