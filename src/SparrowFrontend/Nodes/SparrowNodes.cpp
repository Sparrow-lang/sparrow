#include <StdInc.h>
#include "SparrowNodes.h"

#include <Nodes/ModifiersNode.h>
#include <Nodes/Decls/SprCompilationUnit.h>
#include <Nodes/Decls/SprClass.h>
#include <Nodes/Decls/SprConcept.h>
#include <Nodes/Decls/Package.h>
#include <Nodes/Decls/SprFunction.h>
#include <Nodes/Decls/SprParameter.h>
#include <Nodes/Decls/SprVariable.h>
#include <Nodes/Decls/Using.h>
#include <Nodes/Exp/Literal.h>
#include <Nodes/Exp/Identifier.h>
#include <Nodes/Exp/CompoundExp.h>
#include <Nodes/Exp/StarExp.h>
#include <Nodes/Exp/This.h>
#include <Nodes/Exp/FunApplication.h>
#include <Nodes/Exp/OperatorCall.h>
#include <Nodes/Exp/InfixExp.h>
#include <Nodes/Exp/SprConditional.h>
#include <Nodes/Exp/LambdaFunction.h>
#include <Nodes/Exp/DeclExp.h>
#include <Nodes/Stmt/For.h>
#include <Nodes/Stmt/SprReturn.h>

#include <Feather/Nodes/NodeList.h>
#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Common/Diagnostic.h>


using namespace SprFrontend;

#define UNUSED(x) (void)(x)

namespace
{
    template <typename T>
    string toStrData(T val)
    {
        const T* ptr = &val;
        const T* end = ptr + 1;
        return string(reinterpret_cast<const char*>(ptr), reinterpret_cast<const char*>(end));
    }
}


Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods)
{
    REQUIRE_NODE(loc, main);
    return mods ? new ModifiersNode(loc, main, mods) : main;
}

Node* SprFrontend::mkSprCompilationUnit(const Location& loc, Node* package, NodeList* imports, NodeList* declarations)
{
    return new SprCompilationUnit(loc, package, imports, declarations);
}

Node* SprFrontend::mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType)
{
    return new Using(loc, move(alias), usingNode, accessType);
}

Node* SprFrontend::mkSprPackage(const Location& loc, string name, NodeList* children, AccessType accessType)
{
    return new Package(loc, move(name), children, accessType);
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType)
{
    return new SprVariable(loc, move(name), typeNode, init, accessType);
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, Type* type, Node* init, AccessType accessType)
{
    return new SprVariable(loc, move(name), type, init, accessType);
}

Node* SprFrontend::mkSprClass(const Location& loc, string name, NodeList* parameters, NodeList* baseClasses, Node* ifClause, NodeList* children, AccessType accessType)
{
    return new SprClass(loc, move(name), parameters, baseClasses, children, ifClause, accessType);
}

Node* SprFrontend::mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType)
{
    return new SprConcept(loc, move(name), move(paramName), baseConcept, ifClause, accessType);
}

Node* SprFrontend::mkSprFunction(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType)
{
    return new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType);
}

Node* SprFrontend::mkSprFunctionExp(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* bodyExp, Node* ifClause, AccessType accessType)
{
    const Location& loc2 = bodyExp->location();
    Node* body = mkBlockStmt(loc2, (NodeList*) Feather::mkNodeList(loc2, { mkReturnStmt(loc2, bodyExp) }));
    if ( !returnType )
        returnType = mkFunApplication(loc2, mkIdentifier(loc2, "typeOf"), (NodeList*) Feather::mkNodeList(loc2, { bodyExp }));

    return new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType);
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init)
{
    return new SprParameter(loc, move(name), typeNode, init);
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, Type* type, Node* init)
{
    return new SprParameter(loc, move(name), type, init);
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, string name)
{
    return new SprParameter(loc, move(name), mkIdentifier(loc, "AnyType"), nullptr);
}

Node* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    return new Identifier(loc, move(id));
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, string id)
{
    return new CompoundExp(loc, base, move(id));
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR(loc, "Expected '*' in expression; found '%1%'") % operName;
    return new StarExp(loc, base);
}

Node* SprFrontend::mkPostfixOp(const Location& loc, string op, Node* base)
{
    return new OperatorCall(loc, base, move(op), nullptr);
}

Node* SprFrontend::mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2)
{
    return new InfixExp(loc, move(op), arg1, arg2);
}

Node* SprFrontend::mkPrefixOp(const Location& loc, string op, Node* base)
{
    return new InfixExp(loc, move(op), nullptr, base);
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, NodeList* arguments)
{
    return new FunApplication(loc, base, arguments);
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, NodeVector arguments)
{
    return new FunApplication(loc, base, move(arguments));
}
Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2)
{
    return new OperatorCall(loc, arg1, move(op), arg2);
}


Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2)
{
    return new SprConditional(loc, cond, alt1, alt2);
}

Node* SprFrontend::mkThisExp(const Location& loc)
{
    return new This(loc);
}

Node* SprFrontend::mkParenthesisExp(const Location& loc, Node* exp)
{
    return Feather::mkNodeList(loc, {exp}, false);
}

Node* SprFrontend::mkIntLiteral(const Location& loc, int value)
{
    return new Literal(loc, "Int", toStrData(value));;
}

Node* SprFrontend::mkUIntLiteral(const Location& loc, unsigned int value)
{
    return new Literal(loc, "UInt", toStrData(value));;
}

Node* SprFrontend::mkLongLiteral(const Location& loc, long value)
{
    return new Literal(loc, "Long", toStrData(value));;
}

Node* SprFrontend::mkULongLiteral(const Location& loc, unsigned long value)
{
    return new Literal(loc, "ULong", toStrData(value));;
}

Node* SprFrontend::mkFloatLiteral(const Location& loc, float value)
{
    return new Literal(loc, "Float", toStrData(value));;
}

Node* SprFrontend::mkDoubleLiteral(const Location& loc, double value)
{
    return new Literal(loc, "Double", toStrData(value));;
}

Node* SprFrontend::mkCharLiteral(const Location& loc, char value)
{
    return new Literal(loc, "Char", toStrData(value));;
}

Node* SprFrontend::mkStringLiteral(const Location& loc, string value)
{
    return new Literal(loc, "StringRef", move(value));
}

Node* SprFrontend::mkNullLiteral(const Location& loc)
{
    return new Literal(loc, "Null", string());;
}

Node* SprFrontend::mkBoolLiteral(const Location& loc, bool value)
{
    return new Literal(loc, "Bool", toStrData(value));;
}

Node* SprFrontend::mkLambdaExp(const Location& loc, NodeList* parameters, Node* returnType, Node* body, Node* bodyExp, NodeList* closureParams)
{
    if ( bodyExp )
    {
        ASSERT(!body);
        const Location& loc = bodyExp->location();
        body = mkBlockStmt(loc, (NodeList*) Feather::mkNodeList(loc, { mkReturnStmt(loc, bodyExp) }));
        if ( !returnType )
            returnType = mkFunApplication(loc, mkIdentifier(loc, "typeOf"), (NodeList*) Feather::mkNodeList(loc, { bodyExp }));
    }
    return new LambdaFunction(loc, parameters, returnType, body, closureParams);
}


Node* SprFrontend::mkExpressionStmt(const Location& /*loc*/, Node* exp)
{
    return exp;
}

Node* SprFrontend::mkBlockStmt(const Location& loc, NodeList* statements)
{
    return Feather::mkLocalSpace(loc, statements ? statements->children() : NodeVector());
}

Node* SprFrontend::mkIfStmt(const Location& loc, Node* cond, Node* thenClause, Node* elseClause)
{
    return Feather::mkIf(loc, cond, thenClause, elseClause);
}

Node* SprFrontend::mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action)
{
    return new For(loc, move(name), range, action, type);
}

Node* SprFrontend::mkWhileStmt(const Location& loc, Node* cond, Node* step, Node* action)
{
    return Feather::mkWhile(loc, cond, action, step);
}

Node* SprFrontend::mkBreakStmt(const Location& loc)
{
    return Feather::mkBreak(loc);
}

Node* SprFrontend::mkContinueStmt(const Location& loc)
{
    return Feather::mkContinue(loc);
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp)
{
    return new SprReturn(loc, exp);
}

//Node* SprFrontend::mkThrowStmt(const Location& loc, Node* exp)
//{
//    UNUSED(loc); UNUSED(exp);
//    return NULL; // TODO
//}
//
//Node* SprFrontend::mkTryStmt(const Location& loc, Node* actions, NodeList* catches, Node* finallyBlock)
//{
//    UNUSED(loc); UNUSED(actions); UNUSED(catches); UNUSED(finallyBlock);
//    return NULL; // TODO
//}
//
//Node* SprFrontend::mkCatchBlock(const Location& loc, Node* formal, Node* action)
//{
//    UNUSED(loc); UNUSED(formal); UNUSED(action);
//    return NULL; // TODO
//}
//
//Node* SprFrontend::mkFinallyBlock(const Location& loc, Node* action)
//{
//    UNUSED(loc); UNUSED(action);
//    return NULL; // TODO
//}

Node* SprFrontend::mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp)
{
    return new DeclExp(loc, move(decls), baseExp);
}
