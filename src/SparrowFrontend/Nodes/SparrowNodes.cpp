#include <StdInc.h>
#include "SparrowNodes.h"

#include "Module.hpp"
#include "Decl.hpp"
#include "Generics.hpp"
#include "Exp.hpp"
#include "Stmt.hpp"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

////////////////////////////////////////////////////////////////////////////////
// Node kind variables
//

int SprFrontend::firstSparrowNodeKind = 0;

int SprFrontend::nkSparrowModifiersNode = 0;

int SprFrontend::nkSparrowDeclModule = 0;
int SprFrontend::nkSparrowDeclImportName = 0;
int SprFrontend::nkSparrowDeclPackage = 0;
int SprFrontend::nkSparrowDeclSprDatatype = 0;
int SprFrontend::nkSparrowDeclSprFunction = 0;
int SprFrontend::nkSparrowDeclSprParameter = 0;
int SprFrontend::nkSparrowDeclSprField = 0;
int SprFrontend::nkSparrowDeclSprVariable = 0;
int SprFrontend::nkSparrowDeclSprConcept = 0;
int SprFrontend::nkSparrowDeclGenericPackage = 0;
int SprFrontend::nkSparrowDeclGenericDatatype = 0;
int SprFrontend::nkSparrowDeclGenericFunction = 0;
int SprFrontend::nkSparrowDeclUsing = 0;

int SprFrontend::nkSparrowExpLiteral = 0;
int SprFrontend::nkSparrowExpIdentifier = 0;
int SprFrontend::nkSparrowExpCompoundExp = 0;
int SprFrontend::nkSparrowExpFunApplication = 0;
int SprFrontend::nkSparrowExpOperatorCall = 0;
int SprFrontend::nkSparrowExpInfixExp = 0;
int SprFrontend::nkSparrowExpLambdaFunction = 0;
int SprFrontend::nkSparrowExpSprConditional = 0;
int SprFrontend::nkSparrowExpDeclExp = 0;
int SprFrontend::nkSparrowExpStarExp = 0;
int SprFrontend::nkSparrowExpModuleRef = 0;

int SprFrontend::nkSparrowStmtFor = 0;
int SprFrontend::nkSparrowStmtSprReturn = 0;

int SprFrontend::nkSparrowInnerInstantiation = 0;
int SprFrontend::nkSparrowInnerInstantiationsSet = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions from the header
//

void SprFrontend::initSparrowNodeKinds() {
    nkSparrowModifiersNode = ModifiersNode::registerNodeKind();

    nkSparrowDeclModule = Module::registerNodeKind();
    nkSparrowDeclImportName = ImportName::registerNodeKind();

    nkSparrowDeclPackage = PackageDecl::registerNodeKind();
    nkSparrowDeclSprDatatype = DataTypeDecl::registerNodeKind();
    nkSparrowDeclSprField = FieldDecl::registerNodeKind();
    nkSparrowDeclSprFunction = SprFunctionDecl::registerNodeKind();
    nkSparrowDeclSprParameter = ParameterDecl::registerNodeKind();
    nkSparrowDeclSprVariable = VariableDecl::registerNodeKind();
    nkSparrowDeclSprConcept = ConceptDecl::registerNodeKind();

    nkSparrowDeclGenericPackage = GenericPackage::registerNodeKind();
    nkSparrowDeclGenericDatatype = GenericDatatype::registerNodeKind();
    nkSparrowDeclGenericFunction = GenericFunction::registerNodeKind();

    nkSparrowDeclUsing = UsingDecl::registerNodeKind();

    nkSparrowExpLiteral = Literal::registerNodeKind();
    nkSparrowExpIdentifier = Identifier::registerNodeKind();
    nkSparrowExpCompoundExp = CompoundExp::registerNodeKind();
    nkSparrowExpFunApplication = FunApplication::registerNodeKind();
    nkSparrowExpOperatorCall = OperatorCall::registerNodeKind();
    nkSparrowExpInfixExp = InfixOp::registerNodeKind();
    nkSparrowExpLambdaFunction = LambdaExp::registerNodeKind();
    nkSparrowExpSprConditional = ConditionalExp::registerNodeKind();
    nkSparrowExpDeclExp = DeclExp::registerNodeKind();
    nkSparrowExpStarExp = StarExp::registerNodeKind();
    nkSparrowExpModuleRef = ModuleRef::registerNodeKind();

    nkSparrowStmtFor = ForStmt::registerNodeKind();
    nkSparrowStmtSprReturn = ReturnStmt::registerNodeKind();

    nkSparrowInnerInstantiation = Instantiation::registerNodeKind();
    nkSparrowInnerInstantiationsSet = InstantiationsSet::registerNodeKind();

    firstSparrowNodeKind = nkSparrowModifiersNode;
}

Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods) {
    return ModifiersNode::create(loc, main, mods);
}

Node* SprFrontend::mkModule(const Location& loc, Node* moduleName, Node* declarations) {
    return Module::create(loc, moduleName, declarations);
}

Node* SprFrontend::mkImportName(const Location& loc, Node* moduleName, Node* importedDeclNames,
        bool equals, Nest_StringRef alias) {
    return ImportName::create(loc, moduleName, importedDeclNames, equals ? alias : StringRef{});
}

Node* SprFrontend::mkSprUsing(const Location& loc, Nest_StringRef alias, Node* usingNode) {
    return UsingDecl::create(loc, alias, usingNode);
}

Node* SprFrontend::mkSprPackage(
        const Location& loc, Nest_StringRef name, Node* children, Node* params, Node* ifClause) {
    return PackageDecl::create(loc, name, children, params, ifClause);
}

Node* SprFrontend::mkSprDatatype(const Location& loc, Nest_StringRef name, Node* parameters,
        Node* underlyingData, Node* ifClause, Node* children) {
    return DataTypeDecl::create(loc, name, parameters, underlyingData, ifClause, children);
}

Node* SprFrontend::mkSprField(
        const Location& loc, Nest_StringRef name, Node* typeNode, Node* init) {
    return FieldDecl::create(loc, name, typeNode, init);
}

Node* SprFrontend::mkSprConcept(const Location& loc, Nest_StringRef name, Nest_StringRef paramName,
        Node* baseConcept, Node* ifClause) {
    return ConceptDecl::create(loc, name, paramName, baseConcept, ifClause);
}

Node* SprFrontend::mkSprFunction(const Location& loc, Nest_StringRef name, Node* parameters,
        Node* returnType, Node* body, Node* ifClause) {
    return SprFunctionDecl::create(loc, name, parameters, returnType, body, ifClause);
}

Node* SprFrontend::mkSprParameter(
        const Location& loc, Nest_StringRef name, Node* typeNode, Node* init) {
    return ParameterDecl::create(loc, name, typeNode, init);
}

Node* SprFrontend::mkSprParameter(
        const Location& loc, Nest_StringRef name, TypeRef type, Node* init) {
    return ParameterDecl::create(loc, name, type, init);
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, Nest_StringRef name) {
    return ParameterDecl::create(loc, name);
}

Node* SprFrontend::mkGenericPackage(Node* originalPackage, Node* parameters, Node* ifClause) {
    return GenericPackage::create(originalPackage, parameters, ifClause);
}

Node* SprFrontend::mkGenericClass(Node* originalClass, Node* parameters, Node* ifClause) {
    return GenericDatatype::create(originalClass, parameters, ifClause);
}

Node* SprFrontend::mkGenericFunction(
        Node* originalFun, Nest_NodeRange params, Nest_NodeRange genericParams, Node* ifClause) {
    return GenericFunction::create(originalFun, params, genericParams, ifClause);
}

Node* SprFrontend::mkLiteral(const Location& loc, Nest_StringRef litType, Nest_StringRef data) {
    return Literal::create(loc, litType, data);
}

Node* SprFrontend::mkIdentifier(const Location& loc, Nest_StringRef id) {
    return Identifier::create(loc, id);
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, Nest_StringRef id) {
    return CompoundExp::create(loc, base, id);
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Node* arguments) {
    return FunApplication::create(loc, base, arguments);
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Nest_NodeRange arguments) {
    return FunApplication::create(loc, base, arguments);
}

Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, Nest_StringRef op, Node* arg2) {
    return OperatorCall::create(loc, arg1, op, arg2);
}

Node* SprFrontend::mkInfixOp(const Location& loc, Nest_StringRef op, Node* arg1, Node* arg2) {
    return InfixOp::create(loc, op, arg1, arg2);
}

Node* SprFrontend::mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body,
        Node* bodyExp, Node* closureParams) {
    return LambdaExp::create(loc, parameters, returnType, body, bodyExp, closureParams);
}

Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2) {
    return ConditionalExp::create(loc, cond, alt1, alt2);
}

Node* SprFrontend::mkDeclExp(const Location& loc, Nest_NodeRange decls, Node* baseExp) {
    return DeclExp::create(loc, decls, baseExp);
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, Nest_StringRef operName) {
    return StarExp::create(loc, base, operName);
}

Node* SprFrontend::mkModuleRef(const Location& loc, Node* module) {
    return ModuleRef::create(loc, module);
}

Node* SprFrontend::mkForStmt(
        const Location& loc, Nest_StringRef name, Node* type, Node* range, Node* action) {
    return ForStmt::create(loc, name, type, range, action);
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp) {
    return ReturnStmt::create(loc, exp);
}

Node* SprFrontend::mkInstantiation(
        const Location& loc, Nest_NodeRange boundValues, Nest_NodeRange boundVars) {
    return Instantiation::create(loc, boundValues, boundVars);
}

Node* SprFrontend::mkInstantiationsSet(Node* parentNode, Nest_NodeRange params, Node* ifClause) {
    return InstantiationsSet::create(parentNode, params, ifClause);
}
