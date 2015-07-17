#include <StdInc.h>
#include "InfixExp.h"
#include "Nodes/Decls/Using.h"
#include "Helpers/Ct.h"
#include <NodeCommonsCpp.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    static const char* operPropName = "spr.operation";

//    void dumpInfixExp(DynNode* exp)
//    {
//        if ( !exp )
//        {
//            cerr << "NULL";
//        }
//        else
//        {
//            InfixExp* ie = exp->as<InfixExp>();
//            if ( ie )
//            {
//                cerr << "(";
//
//                const string& oper = ie->operation();
//                if ( oper == "__dot__" )
//                {
//                    dumpInfixExp(ie->arg1());
//                    cerr << " . ";
//                    dumpInfixExp(ie->arg2());
//                }
//                else if ( oper == "__fapp__" )
//                {
//                    dumpInfixExp(ie->arg1());
//                    cerr << " (...)";
//                }
//                else
//                {
//                    dumpInfixExp(ie->arg1());
//                    cerr << " " << ie->operation() << " ";
//                    dumpInfixExp(ie->arg2());
//                }
//
//                cerr << ")";
//            }
//            else
//                cerr << exp;
//        }
//    }
}

InfixExp::InfixExp(const Location& loc, string op, DynNode* arg1, DynNode* arg2)
    : DynNode(classNodeKind(), loc, {arg1, arg2})
{
    if ( op.empty() )
        REP_ERROR(data_.location, "Operation name must have at least one character");
    setProperty(operPropName, move(op));
}

const string& InfixExp::operation() const
{
    return getCheckPropertyString(operPropName);
}

DynNode* InfixExp::arg1() const
{
    ASSERT(data_.children.size() == 2);
    return (DynNode*) data_.children[0];
}

DynNode* InfixExp::arg2() const
{
    ASSERT(data_.children.size() == 2);
    return (DynNode*) data_.children[1];
}

void InfixExp::dump(ostream& os) const
{
    ASSERT(data_.children.size() == 2);
    Node* arg1 = data_.children[0];
    Node* arg2 = data_.children[1];

    os << arg1 << ' ' << operation() << ' ' << arg2;
}

void InfixExp::doSemanticCheck()
{
    ASSERT(data_.children.size() == 2);
    Node* arg1 = data_.children[0];
    Node* arg2 = data_.children[1];

    // This is constructed in such way that left most operations are applied first.
    // This way, we have a tree that has a lot of children on the left side and one children on the right side
    // When applying the precedence and associativity rules for the expressions in here, we will try to balance this
    // tree, and move some elements to the right side of the tree by performing some swaps

    handlePrecedence();
    handleAssociativity();

    arg1 = data_.children[0];
    arg2 = data_.children[1];

//    if ( data_.location.startLineNo() == 34 )
//    {
//        cerr << "Infix exp: ";
//        dumpInfixExp(this);
//        cerr << endl;
//    }

    setExplanation(mkOperatorCall(data_.location, (DynNode*) arg1, operation(), (DynNode*) arg2));
}

void InfixExp::handlePrecedence()
{
    // We go from left to right, trying to fix the precedence
    // Usually the tree is is deep on the left side, but it can contain some sub-trees on the right side too
    for ( ;/*ever*/; )
    {
        int rankCur = getPrecedence();

        // Check right wing first
        InfixExp* rightOp = DynNode::fromNode(data_.children[1])->as<InfixExp>();
        if ( rightOp )
        {
            int rankRight = rightOp->getPrecedence();
            // We never swap right if on the right we have a prefix operator (no 1st arg)
            if ( rankRight <= rankCur && rightOp->arg1() )
            {
                swapRight();
                continue;
            }
        }


        InfixExp* leftOp = DynNode::fromNode(data_.children[0])->as<InfixExp>();
        if ( leftOp )
        {
            leftOp->handlePrecedence();

            int rankLeft = leftOp->getPrecedence();
            if ( rankLeft < rankCur )
            {
                swapLeft();
                // leftOp is now on right of this node, containing the operation of this node
                // make sure the precedence is ok on the right side too
                leftOp->handlePrecedence();
                continue;
            }
        }

        // Nothing to do anymore
        break;
    }
}

void InfixExp::handleAssociativity()
{
    // Normally, we should only check the left children for right associativity.
    // But, as we apply precedence, we must check for associativity in both directions

    const string& curOp = operation();
    bool isRightAssoc = isRightAssociativity();
    if ( !isRightAssoc )
    {
        for ( ;/*ever*/; )
        {
            DynNode* arg2 = (DynNode*) data_.children[1];
            InfixExp* rightOp = arg2->as<InfixExp>();
            if ( !rightOp )
                break;

            rightOp->handleAssociativity();
            // We never swap right if on the right we have a prefix operator (no 1st arg)
            if ( rightOp && curOp == rightOp->operation() && rightOp->arg1() )
                swapRight();
            else
                break;
        }
    }
    else
    {
        for ( ;/*ever*/; )
        {
            DynNode* arg1 = (DynNode*) data_.children[0];
            InfixExp* leftOp = arg1->as<InfixExp>();
            if ( !leftOp )
                break;

            leftOp->handleAssociativity();
            if ( leftOp && curOp == leftOp->operation() )
                swapLeft();
            else
                break;
        }
    }
}

int InfixExp::getPrecedence()
{
    DynNode* arg1 = (DynNode*) data_.children[0];

    // For prefix operator, search with a special name
    string oper = arg1 ? operation() : "__pre__";

    string precedenceName = "oper_precedence_" + oper;
    string defaultPrecedenceName = "oper_precedence_default";

    // Perform a name lookup for the actual precedence name
    int res = getIntValue(toDyn(data_.context->currentSymTab()->lookup(precedenceName)), -1);
    if ( res > 0 )
        return res;

    // Search the default precedence name
    res = getIntValue(toDyn(data_.context->currentSymTab()->lookup(defaultPrecedenceName)), -1);
    if ( res > 0 )
        return res;

    return 0;
}

bool InfixExp::isRightAssociativity()
{
    string assocName = "oper_assoc_" + operation();

    // Perform a name lookup for the actual associativity name
    int res = getIntValue(toDyn(data_.context->currentSymTab()->lookup(assocName)), 1);
    return res < 0;
}

int InfixExp::getIntValue(const DynNodeVector& decls, int defaultVal)
{
    // If no declarations found, return the default value
    if ( decls.empty() )
        return defaultVal;

    // If more than one declarations found, issue a warning
    if ( decls.size() > 1 )
    {
        REP_WARNING(data_.location, "Multple precedence declarations found for '%1%'") % operation();
        for ( DynNode* decl: decls )
            if ( decl )
                REP_INFO(decl->location(), "See alternative");
    }

    // Just one found. Evaluate its value
    DynNode* node = decls.front();
    node->semanticCheck();
    if ( node->nodeKind() == nkSparrowDeclUsing )
        node = static_cast<Using*>(node)->source();

    return getIntCtValue(node->explanation());
}



// Visual explanation:
//
//     X     <- this        Y     <- keep this pointing here
//    / \                  / \
//   Y   R           =>   L   X
//  / \                      / \
// L   M                    M   R
//
// We must keep the this pointer in the same position, but we must switch the arguments & operations
// The left argument of this will become the right argument of this
void InfixExp::swapLeft()
{
    InfixExp* other = DynNode::fromNode(data_.children[0])->as<InfixExp>();
    ASSERT(other);

    this->data_.children[0] = other->data_.children[0];
    other->data_.children[0] = other->data_.children[1];
    other->data_.children[1] = this->data_.children[1];
    this->data_.children[1] = other->node();

    string otherOper = other->operation();
    other->setProperty(operPropName, operation());
    setProperty(operPropName, move(otherOper));
}

// Visual explanation:
//
//     X     <- this        Y     <- keep this pointing here
//    / \                  / \
//   L   Y           =>   X   R
//      / \              / \
//     M   R            L   M    
//
// We must keep the this pointer in the same position, but we must switch the arguments & operations
// The right argument of this will become the left argument of this
//
// NOTE: this function will move 'M' from left-side to right-side
//       it cannot be applied if 'Y' is a unary operator that contains a null 'M'
void InfixExp::swapRight()
{
    InfixExp* other = DynNode::fromNode(data_.children[1])->as<InfixExp>();
    ASSERT(other);

    this->data_.children[1] = other->data_.children[1];
    other->data_.children[1] = other->data_.children[0];
    other->data_.children[0] = this->data_.children[0];
    this->data_.children[0] = other->node();

    string otherOper = other->operation();
    other->setProperty(operPropName, operation());
    setProperty(operPropName, move(otherOper));
}
