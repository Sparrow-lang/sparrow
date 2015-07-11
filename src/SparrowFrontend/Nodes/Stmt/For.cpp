#include <StdInc.h>
#include "For.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Feather;


For::For(const Location& loc, string name, DynNode* range, DynNode* action, DynNode* typeExpr, bool ct)
    : DynNode(classNodeKind(), loc, {range, action, typeExpr})
{
    setName(this, move(name));
    if ( ct )
        setEvalMode(this, modeCt);
}

void For::doSetContextForChildren()
{
    ASSERT(data_.children.size() == 3);
    DynNode* range = data_.children[0];
    DynNode* action = data_.children[1];
    DynNode* typeExpr = data_.children[2];

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(this) == modeCt ? new CompilationContext(data_.context, modeCt) : data_.context;
    if ( typeExpr )
        typeExpr->setContext(rangeContext);
    range->setContext(rangeContext);
    if ( action )
        action->setContext(data_.context);
}

void For::doComputeType()
{
    data_.type = getVoidType(data_.context->evalMode());
}

void For::doSemanticCheck()
{
    ASSERT(data_.children.size() == 3);
    DynNode* range = data_.children[0];
    DynNode* action = data_.children[1];
    DynNode* typeExpr = data_.children[2];

    bool ctFor = nodeEvalMode(this) == modeCt;

    if ( typeExpr )
        typeExpr->semanticCheck();
    range->semanticCheck();

    const Location& loc = range->location();

    if ( ctFor && !isCt(range->type()) )
        REP_ERROR(loc, "Range must be available at CT, for a CT for (range type: %1%)") % range->type();

    // Expand the for statement of the form
    //      for ( <name>: <type> = <range> ) action;
    //
    // into:
    //      var $rangeVar: Range = <range>;
    //      while ( ! $rangeVar.isEmpty() ; $rangeVar.popFront() )
    //      {
    //          var <name>: <type> = $rangeVar.front();
    //          action;
    //      }
    //
    // if <type> is not present, we will use '$rangeType.RetType'

    // Variable to hold the range - initialize it with the range node
    DynNode* rangeVar = mkSprVariable(loc, "$rangeVar", mkIdentifier(loc, "Range"), range);
    if ( ctFor )
        setEvalMode(rangeVar, modeCt);
    DynNode* rangeVarRef = mkIdentifier(loc, "$rangeVar");

    // while condition
    DynNode* base1 = mkCompoundExp(loc, rangeVarRef, "isEmpty");
    DynNode* whileCond = mkOperatorCall(loc, nullptr, "!", mkFunApplication(loc, base1, {}));

    // while step
    DynNode* base2 = mkCompoundExp(loc, rangeVarRef, "popFront");
    DynNode* whileStep = mkFunApplication(loc, base2, {});

    // while body
    DynNode* whileBody = nullptr;
    if ( action )
    {
        if ( !typeExpr )
            typeExpr = mkCompoundExp(loc, rangeVarRef, "RetType");

        // the iteration variable
        DynNode* base3 = mkCompoundExp(loc, rangeVarRef, "front");
        DynNode* init = mkFunApplication(loc, base3, {});

        DynNode* iterVar = mkSprVariable(data_.location, getName(this), typeExpr, init);
        if ( ctFor )
            setEvalMode(iterVar, modeCt);

        whileBody = mkLocalSpace(action->location(), { iterVar, action });
    }

    DynNode* whileStmt = mkWhile(loc, whileCond, whileBody, whileStep, ctFor);
    
    setExplanation(mkLocalSpace(data_.location, { rangeVar, whileStmt }));
}

