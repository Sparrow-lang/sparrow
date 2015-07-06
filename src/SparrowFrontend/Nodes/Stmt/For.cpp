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


For::For(const Location& loc, string name, Node* range, Node* action, Node* typeExpr, bool ct)
    : Node(classNodeKind(), loc, {range, action, typeExpr})
{
    setName(this, move(name));
    if ( ct )
        setEvalMode(this, modeCt);
}

void For::doSetContextForChildren()
{
    ASSERT(children_.size() == 3);
    Node* range = children_[0];
    Node* action = children_[1];
    Node* typeExpr = children_[2];

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(this) == modeCt ? new CompilationContext(context_, modeCt) : context_;
    if ( typeExpr )
        typeExpr->setContext(rangeContext);
    range->setContext(rangeContext);
    if ( action )
        action->setContext(context_);
}

void For::doComputeType()
{
    type_ = getVoidType(context_->evalMode());
}

void For::doSemanticCheck()
{
    ASSERT(children_.size() == 3);
    Node* range = children_[0];
    Node* action = children_[1];
    Node* typeExpr = children_[2];

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
    Node* rangeVar = mkSprVariable(loc, "$rangeVar", mkIdentifier(loc, "Range"), range);
    if ( ctFor )
        setEvalMode(rangeVar, modeCt);
    Node* rangeVarRef = mkIdentifier(loc, "$rangeVar");

    // while condition
    Node* base1 = mkCompoundExp(loc, rangeVarRef, "isEmpty");
    Node* whileCond = mkOperatorCall(loc, nullptr, "!", mkFunApplication(loc, base1, {}));

    // while step
    Node* base2 = mkCompoundExp(loc, rangeVarRef, "popFront");
    Node* whileStep = mkFunApplication(loc, base2, {});

    // while body
    Node* whileBody = nullptr;
    if ( action )
    {
        if ( !typeExpr )
            typeExpr = mkCompoundExp(loc, rangeVarRef, "RetType");

        // the iteration variable
        Node* base3 = mkCompoundExp(loc, rangeVarRef, "front");
        Node* init = mkFunApplication(loc, base3, {});

        Node* iterVar = mkSprVariable(location_, getName(this), typeExpr, init);
        if ( ctFor )
            setEvalMode(iterVar, modeCt);

        whileBody = mkLocalSpace(action->location(), { iterVar, action });
    }

    Node* whileStmt = mkWhile(loc, whileCond, whileBody, whileStep, ctFor);
    
    setExplanation(mkLocalSpace(location_, { rangeVar, whileStmt }));
}

