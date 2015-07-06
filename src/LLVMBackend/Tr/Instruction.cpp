#include <StdInc.h>
#include "Instruction.h"
#include "Scope.h"
#include "TrContext.h"
#include "TrLocal.h"

#include <Nest/Common/Diagnostic.h>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;

namespace
{
    Scope& scopeFromContext(TrContext& context)
    {
        ASSERT(!context.scopesStack().empty());
        return *context.scopesStack().back();
    }
}

Instruction::Instruction(TrContext& context)
    : scope_(scopeFromContext(context))
{
    scope_.instructionsStack().push_back(this);
}

Instruction::~Instruction()
{
    ASSERT(!scope_.instructionsStack().empty());
    ASSERT(scope_.instructionsStack().back() == this);
    outputDestructActions();
    scope_.instructionsStack().pop_back();
}

void Instruction::addTempDestructAction(DynNode* destructAction)
{
    destructActions_.push_back(destructAction);
}

void Instruction::outputDestructActions()
{
    for ( DynNode* n: boost::adaptors::reverse(destructActions_) )
    {
        translateNode(n, scope_.context());
    }
}

Nest::DynNodeVector Instruction::stealDestructActions()
{
    Nest::DynNodeVector res;
    res.swap(destructActions_);
    return res;
}
