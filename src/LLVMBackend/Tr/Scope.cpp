#include <StdInc.h>
#include "Scope.h"
#include "TrContext.h"
#include "Instruction.h"
#include "TrLocal.h"
#include "DebugInfo.h"
#include "Module.h"

#include <Nest/Common/Diagnostic.h>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;

Scope::Scope(TrContext& context, const Location& loc)
    : context_(context)
    , location_(loc)
{
    // Register self to the context
    context_.scopesStack().push_back(this);

    // If we are emitting debug information, emit lexical block start
    if ( context_.module().debugInfo() )
        context_.module().debugInfo()->emitLexicalBlockStart(context_.builder(), location_);
}

Scope::~Scope()
{
    // Output the scope destruct actions
    ASSERT(!context_.scopesStack().empty());
    ASSERT(context_.scopesStack().back() == this);
    outputDestructActions();

    // If we are emitting debug information, emit lexical block start
    if ( context_.module().debugInfo() )
        context_.module().debugInfo()->emitLexicalBlockEnd(context_.builder(), location_);

    // De-register itself from the scope
    context_.scopesStack().pop_back();
}

TrContext& Scope::context() const
{
    return context_;
}

const vector<Instruction*>& Scope::instructionsStack() const
{
    return instructionsStack_;
}

vector<Instruction*>& Scope::instructionsStack()
{
    return instructionsStack_;
}

void Scope::addScopeDestructAction(DynNode* destructAction)
{
    destructActions_.push_back(destructAction);
}

void Scope::outputDestructActions()
{
    // If we have some instructions on the stack, write their temp destruct actions
    for ( Instruction* i: boost::adaptors::reverse(instructionsStack_) )
    {
        i->outputDestructActions();
    }

    // Now write the scope destruct actions
    for ( DynNode* n: boost::adaptors::reverse(destructActions_) )
    {
        translateNode(n, context_);
    }
}
