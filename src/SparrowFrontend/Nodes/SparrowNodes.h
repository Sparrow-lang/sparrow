#pragma once

#include "Nest/Api/EvalMode.h"
#include "Nest/Api/NodeRange.h"

namespace SprFrontend
{
    enum SparrowNodeKinds
    {
        nkRelSparrowModifiersNode = 0,

        nkRelSparrowDeclModule,
        nkRelSparrowDeclImportName,
        nkRelSparrowDeclPackage,
        nkRelSparrowDeclSprClass,
        nkRelSparrowDeclSprFunction,
        nkRelSparrowDeclSprParameter,
        nkRelSparrowDeclSprVariable,
        nkRelSparrowDeclSprConcept,
        nkRelSparrowDeclGenericPackage,
        nkRelSparrowDeclGenericClass,
        nkRelSparrowDeclGenericFunction,
        nkRelSparrowDeclUsing,

        nkRelSparrowExpLiteral,
        nkRelSparrowExpIdentifier,
        nkRelSparrowExpCompoundExp,
        nkRelSparrowExpFunApplication,
        nkRelSparrowExpOperatorCall,
        nkRelSparrowExpInfixExp,
        nkRelSparrowExpLambdaFunction,
        nkRelSparrowExpSprConditional,
        nkRelSparrowExpDeclExp,
        nkRelSparrowExpStarExp,
        nkRelSparrowExpModuleRef,

        nkRelSparrowStmtFor,
        nkRelSparrowStmtSprReturn,

        nkRelSparrowInnerInstantiation,
        nkRelSparrowInnerInstantiationsSet,
    };

    // The ID for the first Sparrow node kind
    extern int firstSparrowNodeKind;

    // The IDs for all the Sparrow node kinds
    extern int nkSparrowModifiersNode;

    extern int nkSparrowDeclModule;
    extern int nkSparrowDeclImportName;
    extern int nkSparrowDeclPackage;
    extern int nkSparrowDeclSprClass;
    extern int nkSparrowDeclSprFunction;
    extern int nkSparrowDeclSprParameter;
    extern int nkSparrowDeclSprVariable;
    extern int nkSparrowDeclSprConcept;
    extern int nkSparrowDeclGenericPackage;
    extern int nkSparrowDeclGenericClass;
    extern int nkSparrowDeclGenericFunction;
    extern int nkSparrowDeclUsing;

    extern int nkSparrowExpLiteral;
    extern int nkSparrowExpIdentifier;
    extern int nkSparrowExpCompoundExp;
    extern int nkSparrowExpFunApplication;
    extern int nkSparrowExpOperatorCall;
    extern int nkSparrowExpInfixExp;
    extern int nkSparrowExpLambdaFunction;
    extern int nkSparrowExpSprConditional;
    extern int nkSparrowExpDeclExp;
    extern int nkSparrowExpStarExp;
    extern int nkSparrowExpModuleRef;

    extern int nkSparrowStmtFor;
    extern int nkSparrowStmtSprReturn;

    extern int nkSparrowInnerInstantiation;
    extern int nkSparrowInnerInstantiationsSet;

    /// Called to initialize the Feather node kinds
    void initSparrowNodeKinds();

    Node* mkModifiers(const Location& loc, Node* main, Node* mods);

    Node* mkModule(const Location& loc, Node* moduleName, Node* declarations);
    Node* mkImportName(const Location& loc, Node* moduleName, Node* importedDeclNames, bool equals = false, StringRef alias = {0});
    Node* mkSprUsing(const Location& loc, StringRef alias, Node* usingNode);
    Node* mkSprPackage(const Location& loc, StringRef name, Node* children, Node* params = nullptr, Node* ifClause = nullptr);
    Node* mkSprVariable(const Location& loc, StringRef name, Node* typeNode, Node* init);
    Node* mkSprVariable(const Location& loc, StringRef name, TypeRef type, Node* init);
    Node* mkSprClass(const Location& loc, StringRef name, Node* parameters, Node* underlyingData, Node* ifClause, Node* children);

    Node* mkSprConcept(const Location& loc, StringRef name, StringRef paramName, Node* baseConcept, Node* ifClause);

    Node* mkSprFunction(const Location& loc, StringRef name, Node* parameters, Node* returnType, Node* body, Node* ifClause = nullptr);
    Node* mkSprParameter(const Location& loc, StringRef name, Node* typeNode, Node* init = nullptr);
    Node* mkSprParameter(const Location& loc, StringRef name, TypeRef type, Node* init = nullptr);
    Node* mkSprAutoParameter(const Location& loc, StringRef name);

    Node* mkGenericPackage(Node* originalPackage, Node* parameters, Node* ifClause);
    Node* mkGenericClass(Node* originalClass, Node* parameters, Node* ifClause);
    Node* mkGenericFunction(Node* originalFun, NodeRange params, NodeRange genericParams, Node* ifClause, Node* thisClass = nullptr);

    Node* mkLiteral(const Location& loc, StringRef litType, StringRef data);
    Node* mkIdentifier(const Location& loc, StringRef id);
    Node* mkCompoundExp(const Location& loc, Node* base, StringRef id);
    Node* mkFunApplication(const Location& loc, Node* base, Node* arguments);
    Node* mkFunApplication(const Location& loc, Node* base, NodeRange arguments);
    Node* mkOperatorCall(const Location& loc, Node* arg1, StringRef op, Node* arg2);
    Node* mkInfixOp(const Location& loc, StringRef op, Node* arg1, Node* arg2);
    Node* mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams);
    Node* mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2);
    Node* mkDeclExp(const Location& loc, NodeRange decls, Node* baseExp = nullptr);
    Node* mkStarExp(const Location& loc, Node* base, StringRef operName);
    Node* mkModuleRef(const Location& loc, Node* module);

    Node* mkForStmt(const Location& loc, StringRef name, Node* type, Node* range, Node* action);
    Node* mkReturnStmt(const Location& loc, Node* exp);

    Node* mkInstantiation(const Location& loc, NodeRange boundValues, NodeRange boundVars);
    Node* mkInstantiationsSet(Node* parentNode, NodeRange params, Node* ifClause);
}
