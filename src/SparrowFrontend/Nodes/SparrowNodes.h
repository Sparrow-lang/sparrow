#pragma once

#include <Nest/Intermediate/EvalMode.h>
#include <Nest/Intermediate/NodeVector.h>
#include <SparrowFrontend/Nodes/DynNode.h>
#include <SparrowFrontend/Nodes/Decls/AccessType.h>

namespace SprFrontend
{
    using namespace Nest;

    enum SparrowNodeKinds
    {
        nkRelSparrowModifiersNode = 0,
        
        nkRelSparrowDeclSprCompilationUnit,
        nkRelSparrowDeclPackage,
        nkRelSparrowDeclSprClass,
        nkRelSparrowDeclSprFunction,
        nkRelSparrowDeclSprParameter,
        nkRelSparrowDeclSprVariable,
        nkRelSparrowDeclSprConcept,
        nkRelSparrowDeclGenericClass,
        nkRelSparrowDeclGenericFunction,
        nkRelSparrowDeclUsing,

        nkRelSparrowExpLiteral,
        nkRelSparrowExpThis,
        nkRelSparrowExpIdentifier,
        nkRelSparrowExpCompoundExp,
        nkRelSparrowExpFunApplication,
        nkRelSparrowExpOperatorCall,
        nkRelSparrowExpInfixExp,
        nkRelSparrowExpLambdaFunction,
        nkRelSparrowExpSprConditional,
        nkRelSparrowExpDeclExp,
        nkRelSparrowExpStarExp,
        
        nkRelSparrowStmtFor,
        nkRelSparrowStmtSprReturn,
        
        nkRelSparrowInnerInstantiation,
        nkRelSparrowInnerInstantiationsSet,
    };

    // The ID for the first Sparrow node kind
    extern int firstSparrowNodeKind;

    // The IDs for all the Sparrow node kinds
    extern int nkSparrowModifiersNode;
    
    extern int nkSparrowDeclSprCompilationUnit;
    extern int nkSparrowDeclPackage;
    extern int nkSparrowDeclSprClass;
    extern int nkSparrowDeclSprFunction;
    extern int nkSparrowDeclSprParameter;
    extern int nkSparrowDeclSprVariable;
    extern int nkSparrowDeclSprConcept;
    extern int nkSparrowDeclGenericClass;
    extern int nkSparrowDeclGenericFunction;
    extern int nkSparrowDeclUsing;

    extern int nkSparrowExpLiteral;
    extern int nkSparrowExpThis;
    extern int nkSparrowExpIdentifier;
    extern int nkSparrowExpCompoundExp;
    extern int nkSparrowExpFunApplication;
    extern int nkSparrowExpOperatorCall;
    extern int nkSparrowExpInfixExp;
    extern int nkSparrowExpLambdaFunction;
    extern int nkSparrowExpSprConditional;
    extern int nkSparrowExpDeclExp;
    extern int nkSparrowExpStarExp;
    
    extern int nkSparrowStmtFor;
    extern int nkSparrowStmtSprReturn;
    
    extern int nkSparrowInnerInstantiation;
    extern int nkSparrowInnerInstantiationsSet;

    /// Called to initialize the Feather node kinds
    void initSparrowNodeKinds();

    Node* mkModifiers(const Location& loc, Node* main, Node* mods);

    Node* mkSprCompilationUnit(const Location& loc, Node* package, Node* imports, Node* declarations);
    Node* mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType = publicAccess);
    Node* mkSprPackage(const Location& loc, string name, Node* children, AccessType accessType = publicAccess);
    Node* mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType = publicAccess);
    Node* mkSprVariable(const Location& loc, string name, TypeRef type, Node* init, AccessType accessType = publicAccess);
    Node* mkSprClass(const Location& loc, string name, Node* parameters, Node* baseClasses, Node* ifClause, Node* children, AccessType accessType = publicAccess);

    Node* mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType = publicAccess);

    Node* mkSprFunction(const Location& loc, string name, Node* parameters, Node* returnType, Node* body, Node* ifClause = nullptr, AccessType accessType = publicAccess);
    Node* mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init = nullptr);
    Node* mkSprParameter(const Location& loc, string name, TypeRef type, Node* init = nullptr);
    Node* mkSprAutoParameter(const Location& loc, string name);

    Node* mkLiteral(const Location& loc, string litType, string data);
    Node* mkThisExp(const Location& loc);
    Node* mkIdentifier(const Location& loc, string id);
    Node* mkCompoundExp(const Location& loc, Node* base, string id);
    Node* mkFunApplication(const Location& loc, Node* base, Node* arguments);
    Node* mkFunApplication(const Location& loc, Node* base, NodeVector arguments);
    Node* mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2);
    Node* mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2);
    Node* mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams);
    Node* mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2);
    Node* mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp = nullptr);
    Node* mkStarExp(const Location& loc, Node* base, const string& operName);

    Node* mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action);
    Node* mkReturnStmt(const Location& loc, Node* exp);

    Node* mkInstantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars);
    Node* mkInstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause);
}
