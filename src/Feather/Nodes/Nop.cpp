#include <StdInc.h>
#include "Nop.h"
#include <Feather/FeatherNodeCommonsCpp.h>


Nop::Nop(const Location& loc)
    : Node(loc)
{
}

void Nop::dump(ostream& os) const
{
    os << "nop";
}

void Nop::doSemanticCheck()
{
    if ( !type_ )
        type_ = getVoidType(context_->evalMode());
}
