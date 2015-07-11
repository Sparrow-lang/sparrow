#include <StdInc.h>
#include "StackAlloc.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/TypeTraits.h>


StackAlloc::StackAlloc(const Location& loc, DynNode* elemType, int numElements, size_t alignment)
    : DynNode(classNodeKind(), loc, {elemType})
{
    setProperty("numElements", numElements);
    setProperty("alignment", alignment);
}

TypeRef StackAlloc::elemType() const
{
    ASSERT(data_->children.size() == 1);
    return data_->children[0]->type();
}

size_t StackAlloc::numElements() const
{
    return getCheckPropertyInt("numElements");
}

size_t StackAlloc::alignment() const
{
    return getCheckPropertyInt("alignment");
}

void StackAlloc::dump(ostream& os) const
{
    os << "stackAlloc(" << elemType() << ", " << numElements() << ")";
}

void StackAlloc::doSemanticCheck()
{
    if ( numElements() == 0 )
        REP_ERROR(data_->location, "Cannot allocate 0 elements on the stack");

    // Set the resulting type
    ASSERT(data_->children.size() == 1);
    DynNode* elemTypeNode = data_->children[0];
    elemTypeNode->computeType();
    data_->type = adjustMode(getLValueType(elemTypeNode->type()), data_->context, data_->location);
}
