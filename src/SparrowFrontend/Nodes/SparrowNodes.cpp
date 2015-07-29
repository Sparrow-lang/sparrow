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
#include <Nodes/Decls/Instantiation.h>
#include <Nodes/Decls/InstantiationsSet.h>
#include <Nodes/Decls/GenericClass.h>
#include <Nodes/Decls/GenericFunction.h>
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

int SprFrontend::firstSparrowNodeKind = 0;

int SprFrontend::nkSparrowModifiersNode = 0;

int SprFrontend::nkSparrowDeclSprCompilationUnit = 0;
int SprFrontend::nkSparrowDeclPackage = 0;
int SprFrontend::nkSparrowDeclSprClass = 0;
int SprFrontend::nkSparrowDeclSprFunction = 0;
int SprFrontend::nkSparrowDeclSprParameter = 0;
int SprFrontend::nkSparrowDeclSprVariable = 0;
int SprFrontend::nkSparrowDeclSprConcept = 0;
int SprFrontend::nkSparrowDeclGenericClass = 0;
int SprFrontend::nkSparrowDeclGenericFunction = 0;
int SprFrontend::nkSparrowDeclUsing = 0;

int SprFrontend::nkSparrowExpLiteral = 0;
int SprFrontend::nkSparrowExpThis = 0;
int SprFrontend::nkSparrowExpIdentifier = 0;
int SprFrontend::nkSparrowExpCompoundExp = 0;
int SprFrontend::nkSparrowExpFunApplication = 0;
int SprFrontend::nkSparrowExpOperatorCall = 0;
int SprFrontend::nkSparrowExpInfixExp = 0;
int SprFrontend::nkSparrowExpLambdaFunction = 0;
int SprFrontend::nkSparrowExpSprConditional = 0;
int SprFrontend::nkSparrowExpDeclExp = 0;
int SprFrontend::nkSparrowExpStarExp = 0;

int SprFrontend::nkSparrowStmtFor = 0;
int SprFrontend::nkSparrowStmtSprReturn = 0;

int SprFrontend::nkSparrowInnerInstantiation = 0;
int SprFrontend::nkSparrowInnerInstantiationsSet = 0;

void SprFrontend::initSparrowNodeKinds()
{
    SprFrontend::ModifiersNode::registerSelf();
    
    SprFrontend::SprCompilationUnit::registerSelf();
    SprFrontend::Package::registerSelf();
    SprFrontend::SprClass::registerSelf();
    SprFrontend::SprFunction::registerSelf();
    SprFrontend::SprParameter::registerSelf();
    SprFrontend::SprVariable::registerSelf();
    SprFrontend::SprConcept::registerSelf();
    SprFrontend::GenericClass::registerSelf();
    SprFrontend::GenericFunction::registerSelf();
    SprFrontend::Using::registerSelf();

    SprFrontend::Literal::registerSelf();
    SprFrontend::This::registerSelf();
    SprFrontend::Identifier::registerSelf();
    SprFrontend::CompoundExp::registerSelf();
    SprFrontend::FunApplication::registerSelf();
    SprFrontend::OperatorCall::registerSelf();
    SprFrontend::InfixExp::registerSelf();
    SprFrontend::LambdaFunction::registerSelf();
    SprFrontend::SprConditional::registerSelf();
    SprFrontend::DeclExp::registerSelf();
    SprFrontend::StarExp::registerSelf();
    
    SprFrontend::For::registerSelf();
    SprFrontend::SprReturn::registerSelf();
    
    SprFrontend::Instantiation::registerSelf();
    SprFrontend::InstantiationsSet::registerSelf();

    nkSparrowModifiersNode =            ModifiersNode::classNodeKind();
    
    nkSparrowDeclSprCompilationUnit =   SprCompilationUnit::classNodeKind();
    nkSparrowDeclPackage =              Package::classNodeKind();
    nkSparrowDeclSprClass =             SprClass::classNodeKind();
    nkSparrowDeclSprFunction =          SprFunction::classNodeKind();
    nkSparrowDeclSprParameter =         SprParameter::classNodeKind();
    nkSparrowDeclSprVariable =          SprVariable::classNodeKind();
    nkSparrowDeclSprConcept =           SprConcept::classNodeKind();
    nkSparrowDeclGenericClass =         GenericClass::classNodeKind();
    nkSparrowDeclGenericFunction =      GenericFunction::classNodeKind();
    nkSparrowDeclUsing =                Using::classNodeKind();

    nkSparrowExpLiteral =               Literal::classNodeKind();
    nkSparrowExpThis =                  This::classNodeKind();
    nkSparrowExpIdentifier =            Identifier::classNodeKind();
    nkSparrowExpCompoundExp =           CompoundExp::classNodeKind();
    nkSparrowExpFunApplication =        FunApplication::classNodeKind();
    nkSparrowExpOperatorCall =          OperatorCall::classNodeKind();
    nkSparrowExpInfixExp =              InfixExp::classNodeKind();
    nkSparrowExpLambdaFunction =        LambdaFunction::classNodeKind();
    nkSparrowExpSprConditional =        SprConditional::classNodeKind();
    nkSparrowExpDeclExp =               DeclExp::classNodeKind();
    nkSparrowExpStarExp =               StarExp::classNodeKind();
    
    nkSparrowStmtFor =                  For::classNodeKind();
    nkSparrowStmtSprReturn =            SprReturn::classNodeKind();
    
    nkSparrowInnerInstantiation =       Instantiation::classNodeKind();
    nkSparrowInnerInstantiationsSet =   InstantiationsSet::classNodeKind();


    firstSparrowNodeKind = nkSparrowModifiersNode;
}


DynNode* SprFrontend::mkModifiers(const Location& loc, DynNode* main, DynNode* mods)
{
    REQUIRE_NODE(loc, main);
    return mods ? new ModifiersNode(loc, main, mods) : main;
}

DynNode* SprFrontend::mkSprCompilationUnit(const Location& loc, DynNode* package, NodeList* imports, NodeList* declarations)
{
    return new SprCompilationUnit(loc, package, imports, declarations);
}

DynNode* SprFrontend::mkSprUsing(const Location& loc, string alias, DynNode* usingNode, AccessType accessType)
{
    return new Using(loc, move(alias), usingNode, accessType);
}

DynNode* SprFrontend::mkSprPackage(const Location& loc, string name, NodeList* children, AccessType accessType)
{
    return new Package(loc, move(name), children, accessType);
}

DynNode* SprFrontend::mkSprVariable(const Location& loc, string name, DynNode* typeNode, DynNode* init, AccessType accessType)
{
    return new SprVariable(loc, move(name), typeNode, init, accessType);
}

DynNode* SprFrontend::mkSprVariable(const Location& loc, string name, TypeRef type, DynNode* init, AccessType accessType)
{
    return new SprVariable(loc, move(name), type, init, accessType);
}

DynNode* SprFrontend::mkSprClass(const Location& loc, string name, NodeList* parameters, NodeList* baseClasses, DynNode* ifClause, NodeList* children, AccessType accessType)
{
    return new SprClass(loc, move(name), parameters, baseClasses, children, ifClause, accessType);
}

DynNode* SprFrontend::mkSprConcept(const Location& loc, string name, string paramName, DynNode* baseConcept, DynNode* ifClause, AccessType accessType)
{
    return new SprConcept(loc, move(name), move(paramName), baseConcept, ifClause, accessType);
}

DynNode* SprFrontend::mkSprFunction(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause, AccessType accessType)
{
    return new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType);
}

DynNode* SprFrontend::mkSprFunctionExp(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* bodyExp, DynNode* ifClause, AccessType accessType)
{
    const Location& loc2 = bodyExp->location();
    DynNode* body = mkBlockStmt(loc2, (NodeList*) Feather::mkNodeList(loc2, { mkReturnStmt(loc2, bodyExp)->node() }));
    if ( !returnType )
        returnType = mkFunApplication(loc2, mkIdentifier(loc2, "typeOf"), (NodeList*) Feather::mkNodeList(loc2, { bodyExp->node() }));

    return new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType);
}

DynNode* SprFrontend::mkSprParameter(const Location& loc, string name, DynNode* typeNode, DynNode* init)
{
    return new SprParameter(loc, move(name), typeNode, init);
}

DynNode* SprFrontend::mkSprParameter(const Location& loc, string name, TypeRef type, DynNode* init)
{
    return new SprParameter(loc, move(name), type, init);
}

DynNode* SprFrontend::mkSprAutoParameter(const Location& loc, string name)
{
    return new SprParameter(loc, move(name), mkIdentifier(loc, "AnyType"), nullptr);
}

DynNode* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    return new Identifier(loc, move(id));
}

DynNode* SprFrontend::mkCompoundExp(const Location& loc, DynNode* base, string id)
{
    return new CompoundExp(loc, base, move(id));
}

DynNode* SprFrontend::mkStarExp(const Location& loc, DynNode* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR(loc, "Expected '*' in expression; found '%1%'") % operName;
    return new StarExp(loc, base);
}

DynNode* SprFrontend::mkPostfixOp(const Location& loc, string op, DynNode* base)
{
    return new OperatorCall(loc, base, move(op), nullptr);
}

DynNode* SprFrontend::mkInfixOp(const Location& loc, string op, DynNode* arg1, DynNode* arg2)
{
    return new InfixExp(loc, move(op), arg1, arg2);
}

DynNode* SprFrontend::mkPrefixOp(const Location& loc, string op, DynNode* base)
{
    return new InfixExp(loc, move(op), nullptr, base);
}

DynNode* SprFrontend::mkFunApplication(const Location& loc, DynNode* base, NodeList* arguments)
{
    return new FunApplication(loc, base, arguments);
}
DynNode* SprFrontend::mkFunApplication(const Location& loc, DynNode* base, DynNodeVector arguments)
{
    return new FunApplication(loc, base, move(arguments));
}
DynNode* SprFrontend::mkOperatorCall(const Location& loc, DynNode* arg1, string op, DynNode* arg2)
{
    return new OperatorCall(loc, arg1, move(op), arg2);
}


DynNode* SprFrontend::mkConditionalExp(const Location& loc, DynNode* cond, DynNode* alt1, DynNode* alt2)
{
    return new SprConditional(loc, cond, alt1, alt2);
}

DynNode* SprFrontend::mkThisExp(const Location& loc)
{
    return new This(loc);
}

DynNode* SprFrontend::mkParenthesisExp(const Location& loc, DynNode* exp)
{
    return (DynNode*) Feather::mkNodeList(loc, {exp->node()}, false);
}

DynNode* SprFrontend::mkIntLiteral(const Location& loc, int value)
{
    return new Literal(loc, "Int", toStrData(value));;
}

DynNode* SprFrontend::mkUIntLiteral(const Location& loc, unsigned int value)
{
    return new Literal(loc, "UInt", toStrData(value));;
}

DynNode* SprFrontend::mkLongLiteral(const Location& loc, long value)
{
    return new Literal(loc, "Long", toStrData(value));;
}

DynNode* SprFrontend::mkULongLiteral(const Location& loc, unsigned long value)
{
    return new Literal(loc, "ULong", toStrData(value));;
}

DynNode* SprFrontend::mkFloatLiteral(const Location& loc, float value)
{
    return new Literal(loc, "Float", toStrData(value));;
}

DynNode* SprFrontend::mkDoubleLiteral(const Location& loc, double value)
{
    return new Literal(loc, "Double", toStrData(value));;
}

DynNode* SprFrontend::mkCharLiteral(const Location& loc, char value)
{
    return new Literal(loc, "Char", toStrData(value));;
}

DynNode* SprFrontend::mkStringLiteral(const Location& loc, string value)
{
    return new Literal(loc, "StringRef", move(value));
}

DynNode* SprFrontend::mkNullLiteral(const Location& loc)
{
    return new Literal(loc, "Null", string());;
}

DynNode* SprFrontend::mkBoolLiteral(const Location& loc, bool value)
{
    return new Literal(loc, "Bool", toStrData(value));;
}

DynNode* SprFrontend::mkLambdaExp(const Location& loc, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* bodyExp, NodeList* closureParams)
{
    if ( bodyExp )
    {
        ASSERT(!body);
        const Location& loc = bodyExp->location();
        body = mkBlockStmt(loc, (NodeList*) Feather::mkNodeList(loc, { mkReturnStmt(loc, bodyExp)->node() }));
        if ( !returnType )
            returnType = mkFunApplication(loc, mkIdentifier(loc, "typeOf"), (NodeList*) Feather::mkNodeList(loc, { bodyExp->node() }));
    }
    return new LambdaFunction(loc, parameters, returnType, body, closureParams);
}


DynNode* SprFrontend::mkExpressionStmt(const Location& /*loc*/, DynNode* exp)
{
    return exp;
}

DynNode* SprFrontend::mkBlockStmt(const Location& loc, NodeList* statements)
{
    return (DynNode*) Feather::mkLocalSpace(loc, statements ? statements->data_.children : NodeVector());
}

DynNode* SprFrontend::mkIfStmt(const Location& loc, DynNode* cond, DynNode* thenClause, DynNode* elseClause)
{
    return (DynNode*) Feather::mkIf(loc, cond->node(), thenClause->node(), elseClause->node());
}

DynNode* SprFrontend::mkForStmt(const Location& loc, string name, DynNode* type, DynNode* range, DynNode* action)
{
    return new For(loc, move(name), range, action, type);
}

DynNode* SprFrontend::mkWhileStmt(const Location& loc, DynNode* cond, DynNode* step, DynNode* action)
{
    return (DynNode*) Feather::mkWhile(loc, cond->node(), action->node(), step->node());
}

DynNode* SprFrontend::mkBreakStmt(const Location& loc)
{
    return (DynNode*) Feather::mkBreak(loc);
}

DynNode* SprFrontend::mkContinueStmt(const Location& loc)
{
    return (DynNode*) Feather::mkContinue(loc);
}

DynNode* SprFrontend::mkReturnStmt(const Location& loc, DynNode* exp)
{
    return new SprReturn(loc, exp);
}

//DynNode* SprFrontend::mkThrowStmt(const Location& loc, DynNode* exp)
//{
//    UNUSED(loc); UNUSED(exp);
//    return NULL; // TODO
//}
//
//DynNode* SprFrontend::mkTryStmt(const Location& loc, DynNode* actions, NodeList* catches, DynNode* finallyBlock)
//{
//    UNUSED(loc); UNUSED(actions); UNUSED(catches); UNUSED(finallyBlock);
//    return NULL; // TODO
//}
//
//DynNode* SprFrontend::mkCatchBlock(const Location& loc, DynNode* formal, DynNode* action)
//{
//    UNUSED(loc); UNUSED(formal); UNUSED(action);
//    return NULL; // TODO
//}
//
//DynNode* SprFrontend::mkFinallyBlock(const Location& loc, DynNode* action)
//{
//    UNUSED(loc); UNUSED(action);
//    return NULL; // TODO
//}

DynNode* SprFrontend::mkDeclExp(const Location& loc, DynNodeVector decls, DynNode* baseExp)
{
    return new DeclExp(loc, move(decls), baseExp);
}

DynNode* SprFrontend::mkInstantiation(const Location& loc, DynNodeVector boundValues, DynNodeVector boundVars)
{
    return new Instantiation(loc, boundValues, boundVars);
}

DynNode* SprFrontend::mkInstantiationsSet(DynNode* parentNode, DynNodeVector params, DynNode* ifClause)
{
    return new InstantiationsSet(parentNode, params, ifClause);
}
