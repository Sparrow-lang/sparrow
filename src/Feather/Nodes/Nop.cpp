#include <StdInc.h>
#include "Nop.h"
#include <Feather/FeatherNodeCommonsCpp.h>


Nop::Nop(const Location& loc)
    : DynNode(classNodeKind(), loc)
{
}

void Nop::dump(ostream& os) const
{
    os << "nop";
}

void Nop::doSemanticCheck()
{
    if ( !data_->type )
        data_->type = getVoidType(data_->context->evalMode());
}
