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

#include "Mods/ModStatic.h"
#include "Mods/ModCt.h"
#include "Mods/ModRt.h"
#include "Mods/ModRtCt.h"
#include "Mods/ModAutoCt.h"
#include "Mods/ModCtGeneric.h"
#include "Mods/ModNative.h"
#include "Mods/ModConvert.h"
#include "Mods/ModNoDefault.h"
#include "Mods/ModInitCtor.h"
#include "Mods/ModMacro.h"
#include "Mods/ModNoInline.h"

#include <Helpers/ForEachNodeInNodeList.h>



#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Common/Diagnostic.h>


using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

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

////////////////////////////////////////////////////////////////////////////////
// ModifiersNode
//

void applyModifier(Node* base, Node* modNode)
{
    Nest::Modifier* mod = nullptr;
    
    if ( modNode->nodeKind == nkSparrowExpIdentifier )
    {
        const string& name = getCheckPropertyString(modNode, "name");
        if ( name == "static" )
            mod = new ModStatic;
        else if ( name == "ct" )
            mod = new ModCt;
        else if ( name == "rt" )
            mod = new ModRt;
        else if ( name == "rtct" )
            mod = new ModRtCt;
        else if ( name == "autoCt" )
            mod = new ModAutoCt;
        else if ( name == "ctGeneric" )
            mod = new ModCtGeneric;
        else if ( name == "convert" )
            mod = new ModConvert;
        else if ( name == "noDefault" )
            mod = new ModNoDefault;
        else if ( name == "initCtor" )
            mod = new ModInitCtor;
        else if ( name == "macro" )
            mod = new ModMacro;
        else if ( name == "noInline" )
            mod = new ModNoInline;
    }
    else
    {
        // check for: native("name")
        if ( modNode->nodeKind == nkSparrowExpInfixExp )    // fun application
        {
            Node* fbase = modNode->children[0];
            Node* fargs = modNode->children[1];
            if ( fbase->nodeKind == nkSparrowExpIdentifier && getCheckPropertyString(fbase, "name") == "native" )
            {
                // one argument: a literal
                if ( fargs && fargs->nodeKind == Feather::nkFeatherNodeList && fargs->children.size() == 1
                && fargs->children.front()->nodeKind == nkSparrowExpLiteral )
                {
                    Node* arg = fargs->children.front();
                    const string& type = getCheckPropertyString(arg, "spr.literalType"); 
                    if ( type == "StringRef" )
                    {
                        const string& dataStr = getCheckPropertyString(arg, "spr.literalData");
                        mod = new ModNative(dataStr);
                    }
                }
            }
        }
    }
    
    // If we recognized a modifier, add it to the base node; otherwise raise an error
    if ( mod )
        Nest::addModifier(base, mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found: %1%") % modNode;
}
void ModifiersNode_SetContextForChildren(Node* node)
{
    Node* base = node->children[0];
    Node* modifierNodes = node->children[1];

    // Set the context of the modifiers
    if ( modifierNodes )
        Nest::setContext(modifierNodes, node->context);

    // Interpret the modifiers here - as much as possible
    if ( modifierNodes )
    {
        if ( modifierNodes->nodeKind == Feather::nkFeatherNodeList )
        {
            // Add the modifiers to the base node
            forEachNodeInNodeList(modifierNodes, [&] (Node* modNode)
            {
                applyModifier(base, modNode);
            });
        }
        else
        {
            applyModifier(base, modifierNodes);
        }
    }

    // Set the context for the base
    if ( base )
        Nest::setContext(base, node->context);
}
TypeRef ModifiersNode_ComputeType(Node* node)
{
    Node* base = node->children[0];
    computeType(base);
    return base->type;
}
Node* ModifiersNode_SemanticCheck(Node* node)
{
    return node->children[0];
}


////////////////////////////////////////////////////////////////////////////////
// Instantiation
//

Node* Instantiation_SemanticCheck(Node* node)
{
    return mkNop(node->location);
}


////////////////////////////////////////////////////////////////////////////////
// InstantiationSet
//

Node* InstantiationsSet_SemanticCheck(Node* node)
{
    return mkNop(node->location);
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
    nkSparrowModifiersNode = registerNodeKind("spr.modifiers", &ModifiersNode_SemanticCheck, &ModifiersNode_ComputeType, &ModifiersNode_SetContextForChildren, NULL);

    SprFrontend::ModifiersNode::classNodeKindRef() = nkSparrowModifiersNode;
    
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
    
    nkSparrowInnerInstantiation = registerNodeKind("spr.instantiation", &Instantiation_SemanticCheck, NULL, NULL, NULL);
    nkSparrowInnerInstantiationsSet = registerNodeKind("spr.instantiationSet", &InstantiationsSet_SemanticCheck, NULL, NULL, NULL);

    SprFrontend::Instantiation::classNodeKindRef() = nkSparrowInnerInstantiation;
    SprFrontend::InstantiationsSet::classNodeKindRef() = nkSparrowInnerInstantiationsSet;

    firstSparrowNodeKind = nkSparrowModifiersNode;
}


Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods)
{
    REQUIRE_NODE(loc, main);
    if ( !mods )
        return main;

    Node* res = createNode(nkSparrowModifiersNode);
    res->location = loc;
    res->children = { main, mods };
    return res;
}

Node* SprFrontend::mkSprCompilationUnit(const Location& loc, Node* package, Node* imports, Node* declarations)
{
    return (new SprCompilationUnit(loc, package, imports, declarations))->node();
}

Node* SprFrontend::mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType)
{
    return (new Using(loc, move(alias), usingNode, accessType))->node();
}

Node* SprFrontend::mkSprPackage(const Location& loc, string name, Node* children, AccessType accessType)
{
    return (new Package(loc, move(name), children, accessType))->node();
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType)
{
    return (new SprVariable(loc, move(name), typeNode, init, accessType))->node();
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, TypeRef type, Node* init, AccessType accessType)
{
    return (new SprVariable(loc, move(name), type, init, accessType))->node();
}

Node* SprFrontend::mkSprClass(const Location& loc, string name, Node* parameters, Node* baseClasses, Node* ifClause, Node* children, AccessType accessType)
{
    return (new SprClass(loc, move(name), parameters, baseClasses, children, ifClause, accessType))->node();
}

Node* SprFrontend::mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType)
{
    return (new SprConcept(loc, move(name), move(paramName), baseConcept, ifClause, accessType))->node();
}

Node* SprFrontend::mkSprFunction(const Location& loc, string name, Node* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType)
{
    return (new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType))->node();
}

Node* SprFrontend::mkSprFunctionExp(const Location& loc, string name, Node* parameters, Node* returnType, Node* bodyExp, Node* ifClause, AccessType accessType)
{
    const Location& loc2 = bodyExp->location;
    Node* body = mkBlockStmt(loc2, Feather::mkNodeList(loc2, { mkReturnStmt(loc2, bodyExp) }));
    if ( !returnType )
        returnType = mkFunApplication(loc2, mkIdentifier(loc2, "typeOf"), Feather::mkNodeList(loc2, { bodyExp }));

    return (new SprFunction(loc, move(name), parameters, returnType, body, ifClause, accessType))->node();
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init)
{
    return (new SprParameter(loc, move(name), typeNode, init))->node();
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, TypeRef type, Node* init)
{
    return (new SprParameter(loc, move(name), type, init))->node();
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, string name)
{
    return (new SprParameter(loc, move(name), mkIdentifier(loc, "AnyType"), nullptr))->node();
}

Node* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    return (new Identifier(loc, move(id)))->node();
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, string id)
{
    return (new CompoundExp(loc, base, move(id)))->node();
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR(loc, "Expected '*' in expression; found '%1%'") % operName;
    return (new StarExp(loc, base))->node();
}

Node* SprFrontend::mkPostfixOp(const Location& loc, string op, Node* base)
{
    return (new OperatorCall(loc, base, move(op), nullptr))->node();
}

Node* SprFrontend::mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2)
{
    return (new InfixExp(loc, move(op), arg1, arg2))->node();
}

Node* SprFrontend::mkPrefixOp(const Location& loc, string op, Node* base)
{
    return (new InfixExp(loc, move(op), nullptr, base))->node();
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Node* arguments)
{
    return (new FunApplication(loc, base, arguments))->node();
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, NodeVector arguments)
{
    return (new FunApplication(loc, base, move(arguments)))->node();
}
Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2)
{
    return (new OperatorCall(loc, arg1, move(op), arg2))->node();
}


Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2)
{
    return (new SprConditional(loc, cond, alt1, alt2))->node();
}

Node* SprFrontend::mkThisExp(const Location& loc)
{
    return (new This(loc))->node();
}

Node* SprFrontend::mkParenthesisExp(const Location& loc, Node* exp)
{
    return Feather::mkNodeList(loc, {exp}, false);
}

Node* SprFrontend::mkIntLiteral(const Location& loc, int value)
{
    return (new Literal(loc, "Int", toStrData(value)))->node();
}

Node* SprFrontend::mkUIntLiteral(const Location& loc, unsigned int value)
{
    return (new Literal(loc, "UInt", toStrData(value)))->node();
}

Node* SprFrontend::mkLongLiteral(const Location& loc, long value)
{
    return (new Literal(loc, "Long", toStrData(value)))->node();
}

Node* SprFrontend::mkULongLiteral(const Location& loc, unsigned long value)
{
    return (new Literal(loc, "ULong", toStrData(value)))->node();
}

Node* SprFrontend::mkFloatLiteral(const Location& loc, float value)
{
    return (new Literal(loc, "Float", toStrData(value)))->node();
}

Node* SprFrontend::mkDoubleLiteral(const Location& loc, double value)
{
    return (new Literal(loc, "Double", toStrData(value)))->node();
}

Node* SprFrontend::mkCharLiteral(const Location& loc, char value)
{
    return (new Literal(loc, "Char", toStrData(value)))->node();
}

Node* SprFrontend::mkStringLiteral(const Location& loc, string value)
{
    return (new Literal(loc, "StringRef", move(value)))->node();
}

Node* SprFrontend::mkNullLiteral(const Location& loc)
{
    return (new Literal(loc, "Null", string()))->node();
}

Node* SprFrontend::mkBoolLiteral(const Location& loc, bool value)
{
    return (new Literal(loc, "Bool", toStrData(value)))->node();
}

Node* SprFrontend::mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams)
{
    if ( bodyExp )
    {
        ASSERT(!body);
        const Location& loc = bodyExp->location;
        body = mkBlockStmt(loc, Feather::mkNodeList(loc, { mkReturnStmt(loc, bodyExp) }));
        if ( !returnType )
            returnType = mkFunApplication(loc, mkIdentifier(loc, "typeOf"), Feather::mkNodeList(loc, { bodyExp }));
    }
    return (new LambdaFunction(loc, parameters, returnType, body, closureParams))->node();
}


Node* SprFrontend::mkExpressionStmt(const Location& /*loc*/, Node* exp)
{
    return exp;
}

Node* SprFrontend::mkBlockStmt(const Location& loc, Node* statements)
{
    return Feather::mkLocalSpace(loc, statements ? statements->children : NodeVector());
}

Node* SprFrontend::mkIfStmt(const Location& loc, Node* cond, Node* thenClause, Node* elseClause)
{
    return Feather::mkIf(loc, cond, thenClause, elseClause);
}

Node* SprFrontend::mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action)
{
    return (new For(loc, move(name), range, action, type))->node();
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
    return (new SprReturn(loc, exp))->node();
}

//Node* SprFrontend::mkThrowStmt(const Location& loc, Node* exp)
//{
//    UNUSED(loc); UNUSED(exp);
//    return NULL; // TODO
//}
//
//Node* SprFrontend::mkTryStmt(const Location& loc, Node* actions, Node* catches, Node* finallyBlock)
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
    return (new DeclExp(loc, move(decls), baseExp))->node();
}

Node* SprFrontend::mkInstantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars)
{
    Node* res = createNode(nkSparrowInnerInstantiation);
    res->location = loc;
    res->children = { Feather::mkNodeList(loc, move(boundVars)) };
    res->referredNodes = move(boundValues);
    setProperty(res, "instIsValid", 0);
    setProperty(res, "instantiatedDecl", (Node*) nullptr);
    return res;
}

Node* SprFrontend::mkInstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause)
{
    Location loc = parentNode->location;
    Node* res = createNode(nkSparrowInnerInstantiationsSet);
    res->location = loc;
    res->children = { ifClause, Feather::mkNodeList(loc, {}) };
    res->referredNodes = { parentNode, mkNodeList(loc, move(params)) };
    return res;
}
