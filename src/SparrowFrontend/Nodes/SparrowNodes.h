#pragma once

#include "Nest/Api/EvalMode.h"
#include "Nest/Api/NodeRange.h"

namespace SprFrontend {
enum SparrowNodeKinds {
    nkRelSparrowModifiersNode = 0,

    nkRelSparrowDeclModule,
    nkRelSparrowDeclImportName,
    nkRelSparrowDeclPackage,
    nkRelSparrowDeclSprDatatype,
    nkRelSparrowDeclSprField,
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
extern int nkSparrowDeclSprDatatype;
extern int nkSparrowDeclSprField;
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

Nest_Node* mkModifiers(const Nest_Location& loc, Nest_Node* main, Nest_Node* mods);

Nest_Node* mkModule(const Nest_Location& loc, Nest_Node* moduleName, Nest_Node* declarations);
Nest_Node* mkImportName(const Nest_Location& loc, Nest_Node* moduleName, Nest_Node* importedDeclNames,
        bool equals = false, Nest_StringRef alias = {0});
Nest_Node* mkSprUsing(const Nest_Location& loc, Nest_StringRef alias, Nest_Node* usingNode);
Nest_Node* mkSprPackage(const Nest_Location& loc, Nest_StringRef name, Nest_Node* children, Nest_Node* params = nullptr,
        Nest_Node* ifClause = nullptr);
Nest_Node* mkSprVariable(const Nest_Location& loc, Nest_StringRef name, Nest_Node* typeNode, Nest_Node* init);
Nest_Node* mkSprVariable(const Nest_Location& loc, Nest_StringRef name, Nest_TypeRef type, Nest_Node* init);
Nest_Node* mkSprDatatype(const Nest_Location& loc, Nest_StringRef name, Nest_Node* parameters, Nest_Node* underlyingData,
        Nest_Node* ifClause, Nest_Node* children);
Nest_Node* mkSprField(const Nest_Location& loc, Nest_StringRef name, Nest_Node* typeNode, Nest_Node* init);

Nest_Node* mkSprConcept(const Nest_Location& loc, Nest_StringRef name, Nest_StringRef paramName, Nest_Node* baseConcept,
        Nest_Node* ifClause);

Nest_Node* mkSprFunction(const Nest_Location& loc, Nest_StringRef name, Nest_Node* parameters, Nest_Node* returnType,
        Nest_Node* body, Nest_Node* ifClause = nullptr);
Nest_Node* mkSprParameter(const Nest_Location& loc, Nest_StringRef name, Nest_Node* typeNode, Nest_Node* init = nullptr);
Nest_Node* mkSprParameter(const Nest_Location& loc, Nest_StringRef name, Nest_TypeRef type, Nest_Node* init = nullptr);
Nest_Node* mkSprAutoParameter(const Nest_Location& loc, Nest_StringRef name);

Nest_Node* mkGenericPackage(Nest_Node* originalPackage, Nest_Node* parameters, Nest_Node* ifClause);
Nest_Node* mkGenericClass(Nest_Node* originalClass, Nest_Node* parameters, Nest_Node* ifClause);
Nest_Node* mkGenericFunction(Nest_Node* originalFun, Nest_NodeRange params, Nest_NodeRange genericParams,
        Nest_Node* ifClause, Nest_Node* thisClass = nullptr);

Nest_Node* mkLiteral(const Nest_Location& loc, Nest_StringRef litType, Nest_StringRef data);
Nest_Node* mkIdentifier(const Nest_Location& loc, Nest_StringRef id);
Nest_Node* mkCompoundExp(const Nest_Location& loc, Nest_Node* base, Nest_StringRef id);
Nest_Node* mkFunApplication(const Nest_Location& loc, Nest_Node* base, Nest_Node* arguments);
Nest_Node* mkFunApplication(const Nest_Location& loc, Nest_Node* base, Nest_NodeRange arguments);
Nest_Node* mkOperatorCall(const Nest_Location& loc, Nest_Node* arg1, Nest_StringRef op, Nest_Node* arg2);
Nest_Node* mkInfixOp(const Nest_Location& loc, Nest_StringRef op, Nest_Node* arg1, Nest_Node* arg2);
Nest_Node* mkLambdaExp(const Nest_Location& loc, Nest_Node* parameters, Nest_Node* returnType, Nest_Node* body,
        Nest_Node* bodyExp, Nest_Node* closureParams);
Nest_Node* mkConditionalExp(const Nest_Location& loc, Nest_Node* cond, Nest_Node* alt1, Nest_Node* alt2);
Nest_Node* mkDeclExp(const Nest_Location& loc, Nest_NodeRange decls, Nest_Node* baseExp = nullptr);
Nest_Node* mkStarExp(const Nest_Location& loc, Nest_Node* base, Nest_StringRef operName);
Nest_Node* mkModuleRef(const Nest_Location& loc, Nest_Node* module);

Nest_Node* mkForStmt(const Nest_Location& loc, Nest_StringRef name, Nest_Node* type, Nest_Node* range, Nest_Node* action);
Nest_Node* mkReturnStmt(const Nest_Location& loc, Nest_Node* exp);

Nest_Node* mkInstantiation(const Nest_Location& loc, Nest_NodeRange boundValues, Nest_NodeRange boundVars);
Nest_Node* mkInstantiationsSet(Nest_Node* parentNode, Nest_NodeRange params, Nest_Node* ifClause);
} // namespace SprFrontend
