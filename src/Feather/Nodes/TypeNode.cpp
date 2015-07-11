#include <StdInc.h>
#include "TypeNode.h"
#include <Feather/FeatherNodeCommonsCpp.h>


TypeNode::TypeNode(const Location& loc, TypeRef type)
    : DynNode(classNodeKind(), loc)
{
    setProperty("givenType", type);
}

void TypeNode::dump(ostream& os) const
{
    os << "type(" << getCheckPropertyType("givenType") << ")";
}

void TypeNode::doSemanticCheck()
{
    data_->type = getCheckPropertyType("givenType");
}
