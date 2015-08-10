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
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Convert.h>
#include <Helpers/CommonCode.h>


#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Context.h>

#include <Nest/Common/Diagnostic.h>


using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

#define UNUSED(x) (void)(x)

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


////////////////////////////////////////////////////////////////////////////////
// For
//

void For_SetContextForChildren(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* range = node->children[0];
    Node* action = node->children[1];
    Node* typeExpr = node->children[2];

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(node) == modeCt ? new CompilationContext(node->context, modeCt) : node->context;
    if ( typeExpr )
        Nest::setContext(typeExpr, rangeContext);
    Nest::setContext(range, rangeContext);
    if ( action )
        Nest::setContext(action, node->context);
}

TypeRef For_ComputeType(Node* node)
{
    return getVoidType(node->context->evalMode());
}

Node* For_SemanticCheck(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* range = node->children[0];
    Node* action = node->children[1];
    Node* typeExpr = node->children[2];

    bool ctFor = nodeEvalMode(node) == modeCt;

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

        Node* iterVar = mkSprVariable(node->location, getName(node), typeExpr, init);
        if ( ctFor )
            setEvalMode(iterVar, modeCt);

        whileBody = mkLocalSpace(action->location, { iterVar, action });
    }

    Node* whileStmt = mkWhile(loc, whileCond, whileBody, whileStep, ctFor);
    
    return mkLocalSpace(node->location, { rangeVar, whileStmt });
}


////////////////////////////////////////////////////////////////////////////////
// SprReturn
//

Node* SprReturn_SemanticCheck(Node* node)
{
    ASSERT(node->children.size() == 1);
    Node* exp = node->children[0];

    // Get the parent function of this return
    Node* parentFun = getParentFun(node->context);
    if ( !parentFun )
        REP_ERROR(node->location, "Return found outside any function");

    // Compute the result type of the function
    TypeRef resType = nullptr;
    Node* resultParam = getResultParam(parentFun);
    if ( resultParam ) // Does this function have a result param?
    {
        resType = removeRef(resultParam->type);
        ASSERT(!Function_resultType(parentFun)->hasStorage); // The function should have void result
    }
    else
    {
        resType = Function_resultType(parentFun);
    }
    ASSERT(resType);

    // Check match between return expression and function result type
    ConversionResult cvt = convNone;
    if ( exp )
    {
        Nest::semanticCheck(exp);
        if ( !resType->hasStorage && exp->type == resType )
        {
            return mkNodeList(node->location, { exp, mkReturn(node->location) });
        }
        else
        {
            cvt = canConvert(exp, resType);
        }
        if ( !cvt )
            REP_ERROR(exp->location, "Cannot convert return expression (%1%) to %2%") % exp->type % resType;
    }
    else
    {
        if ( Function_resultType(parentFun)->typeKind != typeKindVoid )
            REP_ERROR(node->location, "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if ( resultParam )
    {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location;
        Node* thisArg = mkMemLoad(l, mkVarRef(l, resultParam));
        Nest::setContext(thisArg, node->context);
        Node* action = createCtorCall(l, node->context, thisArg, exp);
        if ( !action )
            REP_ERROR(exp->location, "Cannot construct return type object %1% from %2%") % exp->type % resType;

        return mkNodeList(node->location, { action, mkReturn(node->location, nullptr)});
    }
    else
    {
        exp = exp ? cvt.apply(node->context, exp) : nullptr;
        return mkReturn(node->location, exp);
    }
}


////////////////////////////////////////////////////////////////////////////////
// Node kind variables
//

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

////////////////////////////////////////////////////////////////////////////////
// Functions from the header
//

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
    
    nkSparrowStmtFor = registerNodeKind("spr.for", &For_SemanticCheck, &For_ComputeType, &For_SetContextForChildren, NULL);
    nkSparrowStmtSprReturn = registerNodeKind("spr.return", &SprReturn_SemanticCheck, NULL, NULL, NULL);

    nkSparrowInnerInstantiation = registerNodeKind("spr.instantiation", &Instantiation_SemanticCheck, NULL, NULL, NULL);
    nkSparrowInnerInstantiationsSet = registerNodeKind("spr.instantiationSet", &InstantiationsSet_SemanticCheck, NULL, NULL, NULL);

    SprFrontend::For::classNodeKindRef() = nkSparrowStmtFor;
    SprFrontend::SprReturn::classNodeKindRef() = nkSparrowStmtSprReturn;

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


Node* SprFrontend::mkLiteral(const Location& loc, string litType, string data)
{
    return (new Literal(loc, move(litType), move(data)))->node();
}

Node* SprFrontend::mkThisExp(const Location& loc)
{
    return (new This(loc))->node();
}

Node* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    return (new Identifier(loc, move(id)))->node();
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, string id)
{
    return (new CompoundExp(loc, base, move(id)))->node();
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

Node* SprFrontend::mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2)
{
    return (new InfixExp(loc, move(op), arg1, arg2))->node();
}

Node* SprFrontend::mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams)
{
    if ( bodyExp )
    {
        ASSERT(!body);
        const Location& loc = bodyExp->location;
        body = Feather::mkLocalSpace(loc, { mkReturnStmt(loc, bodyExp) });
        if ( !returnType )
            returnType = mkFunApplication(loc, mkIdentifier(loc, "typeOf"), Feather::mkNodeList(loc, { bodyExp }));
    }
    return (new LambdaFunction(loc, parameters, returnType, body, closureParams))->node();
}

Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2)
{
    return (new SprConditional(loc, cond, alt1, alt2))->node();
}

Node* SprFrontend::mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp)
{
    return (new DeclExp(loc, move(decls), baseExp))->node();
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR(loc, "Expected '*' in expression; found '%1%'") % operName;
    return (new StarExp(loc, base))->node();
}


Node* SprFrontend::mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action)
{
    return (new For(loc, move(name), range, action, type))->node();
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp)
{
    return (new SprReturn(loc, exp))->node();
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
