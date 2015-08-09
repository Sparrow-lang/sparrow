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
    : DynNode(classNodeKind(), loc, {range, action, typeExpr})
{
    setName(node(), move(name));
    if ( ct )
        setEvalMode(node(), modeCt);
}

void For::doSetContextForChildren()
{
    ASSERT(data_.children.size() == 3);
    Node* range = data_.children[0];
    Node* action = data_.children[1];
    Node* typeExpr = data_.children[2];

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(node()) == modeCt ? new CompilationContext(data_.context, modeCt) : data_.context;
    if ( typeExpr )
        Nest::setContext(typeExpr, rangeContext);
    Nest::setContext(range, rangeContext);
    if ( action )
        Nest::setContext(action, data_.context);
}

void For::doComputeType()
{
    data_.type = getVoidType(data_.context->evalMode());
}

void For::doSemanticCheck()
{
    ASSERT(data_.children.size() == 3);
    Node* range = data_.children[0];
    Node* action = data_.children[1];
    Node* typeExpr = data_.children[2];

    bool ctFor = nodeEvalMode(node()) == modeCt;

    if ( typeExpr )
        Nest::semanticCheck(typeExpr);
    Nest::semanticCheck(range);

    const Location& loc = range->location;

    if ( ctFor && !isCt(range->type) )
        REP_ERROR(loc, "Range must be available at CT, for a CT for (range type: %1%)") % range->type;

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
    Node* whileCond = mkOperatorCall(loc, nullptr, "!", mkFunApplication(loc, base1, nullptr));

    // while step
    Node* base2 = mkCompoundExp(loc, rangeVarRef, "popFront");
    Node* whileStep = mkFunApplication(loc, base2, nullptr);

    // while body
    Node* whileBody = nullptr;
    if ( action )
    {
        if ( !typeExpr )
            typeExpr = mkCompoundExp(loc, rangeVarRef, "RetType");

        // the iteration variable
        Node* base3 = mkCompoundExp(loc, rangeVarRef, "front");
        Node* init = mkFunApplication(loc, base3, nullptr);

        Node* iterVar = mkSprVariable(data_.location, getName(node()), typeExpr, init);
        if ( ctFor )
            setEvalMode(iterVar, modeCt);

        whileBody = mkLocalSpace(action->location, { iterVar, action });
    }

    Node* whileStmt = mkWhile(loc, whileCond, whileBody, whileStep, ctFor);
    
    setExplanation(mkLocalSpace(data_.location, { rangeVar, whileStmt }));
}

