#include <StdInc.h>
#include "Nop.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Type/Void.h>


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
        type_ = Void::get(context_->evalMode());
}
