#include <StdInc.h>
#include "OperatorCall.h"
#include <NodeCommonsCpp.h>
#include <Helpers/Overload.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/CommonCode.h>

#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/TypeTraits.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    string argTypeStr(DynNode* node)
    {
        return node && node->type() ? node->type()->description : "?";
    }
}

OperatorCall::OperatorCall(const Location& loc, DynNode* arg1, string op, DynNode* arg2)
    : DynNode(classNodeKind(), loc, {arg1, arg2})
{
    setProperty("spr.operation", move(op));
}

void OperatorCall::dump(ostream& os) const
{
    ASSERT(data_.children.size() == 2);
    Node* arg1 = data_.children[0];
    Node* arg2 = data_.children[1];
    const string& operation = getCheckPropertyString("spr.operation");

    if ( arg1 && arg2 )
        os << arg1 << ' ' << operation << ' ' << arg2;
    else if ( arg1 )
        os << arg1 << ' ' << operation;
    else
        os << operation << ' ' << arg2;
}

void OperatorCall::doSemanticCheck()
{
    ASSERT(data_.children.size() == 2);
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];
    const string& operation = getCheckPropertyString("spr.operation");

    if ( arg1 && arg2 )
    {
        if ( operation == "__dot__" )
        {
            handleDotExpr();
            return;
        }
        if ( operation == "===" )
        {
            handleRefEq();
            return;
        }
        else if ( operation == "!==" )
        {
            handleRefNe();
            return;
        }
        else if ( operation == ":=" )
        {
            handleRefAssign();
            return;
        }
    }
    if ( arg1 && operation == "__fapp__" )
    {
        handleFApp();
        return;
    }
    if ( arg2 && !arg1 && operation == "\\" )
    {
        handleFunPtr(); // TODO: this should ideally be defined in std lib
        return;
    }

    // Search for the operator
    DynNode* res = selectOperator(operation, arg1, arg2);

    // If nothing found try fall-back.
    if ( !res && arg1 && arg2 )
    {
        string op1, op2;
        DynNode* a1 = arg1;
        DynNode* a2 = arg2;

        if ( operation == "!=" )  // Transform '!=' into '=='
        {
            op1 = "==";
            op2 = "!";
        }
        else if ( operation == ">" )    // Transform '>' into '<'
        {
            op1 = "<";
            a1 = arg2;
            a2 = arg1;
        }
        else if ( operation == "<=" )   // Transform '<=' into '<'
        {
            op1 = "<";
            op2 = "!";
            a1 = arg2;
            a2 = arg1;
        }
        else if ( operation == ">=" ) // Transform '>=' into '<'
        {
            op1 = "<";
            op2 = "!";
        }

        // Check for <op>= operators
        else if ( operation.size() >= 2 && operation[operation.size()-1] == '=' )
        {
            string baseOperation = operation.substr(0, operation.size()-1);

            // Transform 'a <op>= b' into 'a = a <op> b'
            op1 = operation.substr(0, operation.size()-1);
            op2 = "=";
        }

        if ( !op1.empty() )
        {
            DynNode* r = selectOperator(op1, a1, a2);
            if ( r )
            {
                r->semanticCheck();
                if ( op2 == "!" )
                    res = selectOperator(op2, r, nullptr);
                else if ( op2 == "=" )
                    res = selectOperator(op2, a1, r);
                else if ( op2.empty() )
                    res = r;
            }
        }
    }

    if ( !res )
    {
        if ( arg1 && arg2 )
            REP_ERROR(data_.location, "Cannot find an overload for calling operator %2% %1% %3%") % operation % argTypeStr(arg1) % argTypeStr(arg2);
        else if ( arg1 )
            REP_ERROR(data_.location, "Cannot find an overload for calling operator %2% %1%") % operation % argTypeStr(arg1);
        else if ( arg2 )
            REP_ERROR(data_.location, "Cannot find an overload for calling operator %1% %2%") % operation % argTypeStr(arg2);
        else 
            REP_ERROR(data_.location, "Cannot find an overload for calling operator %1%") % operation;
    }

    setExplanation(res);
}

namespace
{
    DynNode* trySelectOperator(const string& operation, const DynNodeVector& args, CompilationContext* searchContext, bool searchOnlyGivenContext,
        CompilationContext* callContext, const Location& callLocation, EvalMode mode)
    {
        SymTab* sTab = searchContext->currentSymTab();
        DynNodeVector decls = searchOnlyGivenContext ? sTab->lookupCurrent(operation) : sTab->lookup(operation);
        if ( !decls.empty() )
            return selectOverload(callContext, callLocation, mode, move(decls), args, false, operation);

        return nullptr;
    }
}

#define CHECK_RET(expr) \
    { \
        DynNode* ret = expr; \
        if ( ret ) return ret; \
    }

DynNode* OperatorCall::selectOperator(const string& operation, DynNode* arg1, DynNode* arg2)
{
    DynNodeVector args;
    if ( arg1 )
        args.push_back(arg1);
    if ( arg2 )
        args.push_back(arg2);

    // If this is an unary operator, try using the operation prefix
    string opPrefix;
    if ( arg1 && !arg2 )
        opPrefix = "post_";
    if ( !arg1 && arg2 )
        opPrefix = "pre_";

    // Identifiy the first valid operand, so that we can search near it
    DynNode* base = nullptr;
    if ( arg1 )
        base = arg1;
    else if ( arg2 )
        base = arg2;

    DynNode* argClass = nullptr;
    if ( base )
    {
        base->semanticCheck();
        argClass = classForTypeRaw(base->type());
    }

    EvalMode mode;
    if ( argClass )
    {
        mode = base->type()->mode;

        // Step 1: Try to find an operator that match in the class of the base expression
        if ( !opPrefix.empty() )
            CHECK_RET(trySelectOperator(opPrefix + operation, args, argClass->childrenContext(), true, data_.context, data_.location, mode));
        CHECK_RET(trySelectOperator(operation, args, argClass->childrenContext(), true, data_.context, data_.location, mode));

        // Step 2: Try to find an operator that match in the near the class the base expression
        mode = data_.context->evalMode();
        if ( !opPrefix.empty() )
            CHECK_RET(trySelectOperator(opPrefix + operation, args, argClass->context(), true, data_.context, data_.location, mode));
        CHECK_RET(trySelectOperator(operation, args, argClass->context(), true, data_.context, data_.location, mode));
    }

    // Step 3: General search from the current context
    mode = data_.context->evalMode();
    if ( !opPrefix.empty() )
        CHECK_RET(trySelectOperator(opPrefix + operation, args, data_.context, false, data_.context, data_.location, mode));
    CHECK_RET(trySelectOperator(operation, args, data_.context, false, data_.context, data_.location, mode));

    return nullptr;
}

namespace
{
    DynNode* checkConvertNullToRefByte(DynNode* orig)
    {
        if ( isSameTypeIgnoreMode(orig->type(), StdDef::typeNull) )
        {
            DynNode* res = mkNull(orig->location(), mkTypeNode(orig->location(), StdDef::typeRefByte));
            res->setContext(orig->context());
            res->computeType();
            return res;
        }
        return orig;
    }
}

void OperatorCall::handleFApp()
{
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];

    if ( arg2 && arg2->nodeKind() != nkFeatherNodeList )
        REP_INTERNAL(arg2->location(), "Expected node list for function application; found %1%") % arg2;

    setExplanation(mkFunApplication(data_.location, arg1, (NodeList*) arg2));
}

void OperatorCall::handleDotExpr()
{
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];

    if ( arg2->nodeKind() != nkSparrowExpIdentifier )
        REP_INTERNAL(arg2->location(), "Expected identifier after dot; found %1%") % arg2;

    setExplanation(mkCompoundExp(data_.location, arg1, arg2->toString()));
}

void OperatorCall::handleRefEq()
{
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];

    arg1->semanticCheck();
    arg2->semanticCheck();

    // If we have null as arguments, convert them to "RefByte"
    arg1 = checkConvertNullToRefByte(arg1);
    arg2 = checkConvertNullToRefByte(arg2);

    // Make sure that both the arguments are references
    if ( arg1->type()->numReferences == 0 )
        REP_ERROR(data_.location, "Left operand of a reference equality operator is not a reference (%1%)") % arg1->type();
    if ( arg2->type()->numReferences == 0 )
        REP_ERROR(data_.location, "Right operand of a reference equality operator is not a reference (%1%)") % arg2->type();

    DynNode* arg1Cvt = nullptr;
    DynNode* arg2Cvt = nullptr;
    doDereference1(arg1, arg1Cvt);             // Dereference until the last reference
    doDereference1(arg2, arg2Cvt);
    arg1Cvt = mkBitcast(data_.location, mkTypeNode(data_.location, StdDef::typeRefByte), arg1Cvt);
    arg2Cvt = mkBitcast(data_.location, mkTypeNode(data_.location, StdDef::typeRefByte), arg2Cvt);

    setExplanation(mkFunCall(data_.location, StdDef::opRefEq, {arg1Cvt, arg2Cvt}));
}

void OperatorCall::handleRefNe()
{
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];

    arg1->semanticCheck();
    arg2->semanticCheck();

    // If we have null as arguments, convert them to "RefByte"
    arg1 = checkConvertNullToRefByte(arg1);
    arg2 = checkConvertNullToRefByte(arg2);

    // Make sure that both the arguments are references
    if ( arg1->type()->numReferences == 0 )
        REP_ERROR(data_.location, "Left operand of a reference equality operator is not a reference (%1%)") % arg1->type();
    if ( arg2->type()->numReferences == 0 )
        REP_ERROR(data_.location, "Right operand of a reference equality operator is not a reference (%1%)") % arg2->type();

    DynNode* arg1Cvt = nullptr;
    DynNode* arg2Cvt = nullptr;
    doDereference1(arg1, arg1Cvt);             // Dereference until the last reference
    doDereference1(arg2, arg2Cvt);
    arg1Cvt = mkBitcast(data_.location, mkTypeNode(data_.location, StdDef::typeRefByte), arg1Cvt);
    arg2Cvt = mkBitcast(data_.location, mkTypeNode(data_.location, StdDef::typeRefByte), arg2Cvt);

    setExplanation(mkFunCall(data_.location, StdDef::opRefNe, {arg1Cvt, arg2Cvt}));
}

void OperatorCall::handleRefAssign()
{
    DynNode* arg1 = (DynNode*) data_.children[0];
    DynNode* arg2 = (DynNode*) data_.children[1];

    arg1->semanticCheck();
    arg2->semanticCheck();

    // Make sure the first argument is a reference reference
    if ( arg1->type()->numReferences < 2 )
        REP_ERROR(data_.location, "Left operand of a reference assign operator is not a reference reference (%1%)") % arg1->type();
    TypeRef arg1BaseType = Feather::removeRef(arg1->type());

    // Check the second type to be null or a reference
    TypeRef arg2Type = arg2->type();
    if ( !Feather::isSameTypeIgnoreMode(arg2Type, StdDef::typeNull) )
    {
        if ( arg2Type->numReferences == 0 )
            REP_ERROR(data_.location, "Right operand of a reference assign operator is not a reference (%1%)") % arg2->type();
    }

    // Check for a conversion from the second argument to the first argument
    ConversionResult c = canConvert(arg2, arg1BaseType);
    if ( !c )
        REP_ERROR(data_.location, "Cannot convert from %1% to %2%") % arg2->type() % arg1BaseType;
    DynNode* cvt = c.apply(arg2);

    // Return a memstore operator
    setExplanation(mkMemStore(data_.location, cvt, arg1));
}

void OperatorCall::handleFunPtr()
{
    DynNode* funNode = (DynNode*) data_.children[1];

    setExplanation(createFunPtr(funNode));
}
