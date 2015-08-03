#include <StdInc.h>
#include "IntModClassMembers.h"

#include <Nodes/Decls/SprClass.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/StdDef.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;
using namespace Feather;
using namespace std;

namespace
{
    /// Search in the given class for a function with a specified name, taking the given type of parameter
    bool checkForMember(DynNode* cls, const string& funName, Class* paramClass)
    {
        NodeVector decls = cls->childrenContext()->currentSymTab()->lookupCurrent(funName);
        for ( Node* decl: decls )
        {
            decl = explanation(decl);
            if ( !decl )
                continue;
            if ( !decl || decl->nodeKind != nkFeatherDeclFunction )
                continue;

            // Make sure we only take in considerations operations of this class
            Class* cls2 = getParentClass(decl->context);
            if ( cls2 != cls->explanation() )
                continue;

            // Check 'this' and 'other' parameters
            size_t thisParamIdx = 0;
            size_t otherParamIdx = 1;
            if ( getResultParam(decl) )
            {
                ++thisParamIdx;
                ++otherParamIdx;
            }
            size_t numParams = Function_numParameters(decl);
            if ( numParams < 1+thisParamIdx )
                continue;
            if ( getName(Function_getParameter(decl, thisParamIdx)) != "$this" )
                continue;

            if ( paramClass )
            {
                if ( numParams != 1+otherParamIdx )
                    continue;
                TypeRef paramType = Function_getParameter(decl, otherParamIdx)->type;
                if ( paramType->hasStorage )
                {
                    if ( classForType(paramType) == paramClass )
                        return true;
                }
            }
            else
            {
                if ( numParams == 1+thisParamIdx )
                    return true;
            }
        }
        return false;
    }

    /// Checks if the class has a 'ctorFromCt' method
    bool checkForCtorFromCt(DynNode* cls)
    {
        NodeVector decls = cls->childrenContext()->currentSymTab()->lookupCurrent("ctorFromCt");
        for ( Node* n: decls )
        {
            if ( effectiveEvalMode(n) == modeRt )
                return true;
        }
        return false;
    }

    /// Checks if the given class has reference fields
    bool hasReferences(Class* cls)
    {
        for ( DynNode* field: cls->fields() )
        {
            // Take in account only fields of the current class
            Class* cls2 = getParentClass(field->context());
            if ( cls2 != cls )
                continue;

            if ( field->type()->numReferences > 0 )
                return true;
        }
        return false;
    }

    // Add to a local space an operator call
    void addOperatorCall(Node* dest, bool reverse, DynNode* operand1, const string& op, DynNode* operand2)
    {
        Node* call = mkOperatorCall(dest->location, operand1->node(), op, operand2->node());
        if ( !reverse )
            dest->children.push_back(call);
        else
            dest->children.insert(dest->children.begin(), call);
    }

    // Add a method with the given body and given atguments to the parent class
    DynNode* addMethod(SprClass* parent, const string& name, Node* body, vector<pair<TypeRef, string>> params, Class* resClass = nullptr, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location();
        loc.setAsStartOf(loc);

        // Construct the parameters list, return type node
        NodeVector sprParams;
        sprParams.reserve(params.size());
        for ( auto param: params )
        {
            sprParams.push_back(mkSprParameter(loc, param.second, param.first));
        }
        Node* parameters = sprParams.empty() ? nullptr : mkNodeList(loc, move(sprParams));
        Node* ret = resClass ? createTypeNode(parent->childrenContext(), loc, getDataType(resClass->node()))->node() : nullptr;
        
        // Add the function
        Node* m = mkSprFunction(loc, name, parameters, ret, body);
        Nest::setProperty(m, propNoDefault, 1);
        setEvalMode(m, mode == modeUnspecified ? effectiveEvalMode(parent->node()) : mode);
        parent->addChild((DynNode*) m);
        Nest::computeType(m);
        return (DynNode*) m;
    }
    
    // Add a method with the given body to the parent class
    DynNode* addMethod(SprClass* parent, const string& name, Node* body, TypeRef otherParam, Class* resClass = nullptr, EvalMode mode = modeUnspecified)
    {
        return addMethod(parent, name, body, otherParam ? vector<pair<TypeRef, string>>({ {otherParam, string("other")} }) : vector<pair<TypeRef, string>>({}), resClass, mode);
    }
    
    /// Generate a typical method with the given name, by calling 'op' for the base classes and fields
    void generateMethod(SprClass* parent, const string& name, const string& op, TypeRef otherParam, bool reverse = false, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location();
        loc.setAsStartOf(loc);
        Class* cls = parent->explanation()->as<Class>();
        ASSERT(cls);

        Node* otherRef = nullptr;
        if ( otherParam )
        {
            otherRef = mkIdentifier(loc, "other");
            if ( otherParam->numReferences > 0 )
                otherRef = mkMemLoad(loc, otherRef);
        }

        // Construct the body
        Node* body = mkLocalSpace(loc, {});
        for ( DynNode* field: cls->fields() )
        {
            // Take in account only fields of the current class
            Class* cls2 = getParentClass(field->context());
            if ( cls2 != cls )
                continue;

            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field->node());
            Node* otherFieldRef = otherParam ? mkFieldRef(loc, otherRef, field->node()) : nullptr;

            string oper = op;
            if ( field->type()->numReferences > 0 )
            {
                if ( op == "=" || op == "ctor" )
                {
                    oper = ":=";    // Transform into ref assignment
                    if ( !otherFieldRef )
                        otherFieldRef = mkNullLiteral(loc);
                }
                else if ( op == "dtor" )
                    continue;       // Nothing to destruct on references
            }
            addOperatorCall(body, reverse, (DynNode*) fieldRef, oper, (DynNode*) otherFieldRef);
        }

        addMethod(parent, name, body, otherParam, nullptr, mode);
    }

    /// Generate an empty, uninitialized ctor
    void generateUnititializedCtor(SprClass* parent)
    {
        Location loc = parent->location();
        loc.setAsStartOf(loc);

        Node* body = mkLocalSpace(loc, {});
        addMethod(parent, "ctor", body, StdDef::typeUninitialized);
    }

    
    /// Generate an init ctor, that initializes all the members with data received as arguments
    void generateInitCtor(SprClass* parent)
    {
        vector<pair<TypeRef, string>> params;
        
        Location loc = parent->location();
        loc.setAsStartOf(loc);
        Class* cls = parent->explanation()->as<Class>();
        ASSERT(cls);

        // Construct the body
        Node* body = mkLocalSpace(loc, {});
        for ( DynNode* field: cls->fields() )
        {
            // Take in account only fields of the current class
            Class* cls2 = getParentClass(field->context());
            if ( cls2 != cls )
                continue;
            
            TypeRef t = field->type();
            
            // Add a parameter for the base
            string paramName = "f"+getName(field->node());
            params.push_back({t, paramName});
            Node* paramId = mkIdentifier(loc, move(paramName));
            if ( t->numReferences > 0 )
                paramId = mkMemLoad(loc, paramId);
            
            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field->node());
            
            string oper = t->numReferences > 0 ? ":=" : "ctor";
            addOperatorCall(body, false, (DynNode*) fieldRef, oper, (DynNode*) paramId);
        }
        
        addMethod(parent, "ctor", body, params);
    }

    /// Generate the equality check method for the given class
    void generateEqualityCheckMethod(SprClass* parent)
    {
        Location loc = parent->location();
        loc.setAsStartOf(loc);
        Class* cls = parent->explanation()->as<Class>();
        ASSERT(cls);

        // Construct the equality check expression
        Node* exp = nullptr;
        for ( DynNode* field: cls->fields() )
        {
            // Take in account only fields of the current class
            Class* cls2 = getParentClass(field->context());
            if ( cls2 != cls )
                continue;

            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field->node());
            Node* otherFieldRef = mkFieldRef(loc, mkMemLoad(loc, mkIdentifier(loc, "other")), field->node());

            const char* op = (field->type()->numReferences == 0) ? "==" : "===";
            Node* curExp = mkOperatorCall(loc, fieldRef, op, otherFieldRef);
            if ( !exp )
                exp = curExp;
            else
                exp = mkOperatorCall(loc, exp, "&&", curExp);
        }
        if ( !exp )
            exp = mkBoolLiteral(loc, true);

        Node* body = mkLocalSpace(loc, {});
        body->children.push_back(mkReturnStmt(loc, exp));
        addMethod(parent, "==", body, getDataType(cls->node(), 1), StdDef::clsBool);
    }
}

void IntModClassMembers::afterComputeType(Node* n)
{
    DynNode* node = (DynNode*) n;
    
    /// Check to apply only to classes
    SprClass* cls = node->as<SprClass>();
    if ( !cls )
        REP_INTERNAL(node->location(), "IntModClassMembers modifier can be applied only to classes");
    if ( !cls->type() )
        REP_INTERNAL(node->location(), "Type was not computed for %1% when applying IntModClassMembers") % getName(cls->node());

    Class* basicClass = cls->explanation()->as<Class>();
    ASSERT(basicClass);
    TypeRef paramType = getDataType(basicClass->node(), 1);

    // Default ctor
    if ( !checkForMember(cls, "ctor", nullptr) )
        generateMethod(cls, "ctor", "ctor", nullptr);

    // Uninitialized ctor
    if ( !checkForMember(cls, "ctor", StdDef::clsUninitialized) )
        generateUnititializedCtor(cls);

    // Copy ctor
    if ( !checkForMember(cls, "ctor", basicClass) )
        generateMethod(cls, "ctor", "ctor", paramType);

    // Initialization ctor
    if ( cls->hasProperty(propGenerateInitCtor) )
        generateInitCtor(cls);
    
    // CT to RT ctor
    if ( !checkForCtorFromCt(cls) && !hasReferences(basicClass) )
        generateMethod(cls, "ctorFromCt", "ctor", changeTypeMode(getDataType(basicClass->node(), 0), modeCt, node->location()), false, modeRt);

    // Dtor
    if ( !checkForMember(cls, "dtor", nullptr) )
        generateMethod(cls, "dtor", "dtor", nullptr, true);

    // Assignment operator
    if ( !checkForMember(cls, "=", basicClass) )
        generateMethod(cls, "=", "=", paramType);

    // Equality test operator
    if ( !checkForMember(cls, "==", basicClass) )
        generateEqualityCheckMethod(cls);
}
