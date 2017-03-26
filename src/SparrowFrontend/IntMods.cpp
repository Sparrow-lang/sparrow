#include <StdInc.h>
#include "IntMods.h"

#include <Nodes/Builder.h>
#include <Nodes/SparrowNodesAccessors.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/StdDef.h>
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/Modifier.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Diagnostic.hpp"

using namespace SprFrontend;
using namespace std;

namespace
{
    /// Search in the given class for a function with a specified name, taking the given type of parameter
    bool checkForMember(Node* cls, const string& funName, Node* paramClass)
    {
        NodeArray decls = Nest_symTabLookupCurrent(Nest_childrenContext(cls)->currentSymTab, funName.c_str());
        for ( Node* decl: decls )
        {
            decl = Nest_explanation(decl);
            if ( !decl )
                continue;
            if ( !decl || decl->nodeKind != nkFeatherDeclFunction )
                continue;

            // Make sure we only take in considerations operations of this class
            Node* cls2 = Feather_getParentClass(decl->context);
            if ( cls2 != Nest_explanation(cls) )
                continue;

            // Check 'this' and 'other' parameters
            size_t thisParamIdx = 0;
            size_t otherParamIdx = 1;
            if ( getResultParam(decl) )
            {
                ++thisParamIdx;
                ++otherParamIdx;
            }
            size_t numParams = Feather_Function_numParameters(decl);
            if ( numParams < 1+thisParamIdx )
                continue;
            if ( Feather_getName(Feather_Function_getParameter(decl, thisParamIdx)) != "this" )
                continue;

            if ( paramClass )
            {
                if ( numParams != 1+otherParamIdx )
                    continue;
                TypeRef paramType = Feather_Function_getParameter(decl, otherParamIdx)->type;
                if ( paramType->hasStorage )
                {
                    if ( Feather_classForType(paramType) == paramClass )
                    {
                        Nest_freeNodeArray(decls);
                        return true;
                    }
                }
            }
            else
            {
                if ( numParams == 1+thisParamIdx )
                {
                    Nest_freeNodeArray(decls);
                    return true;
                }
            }
        }
        Nest_freeNodeArray(decls);
        return false;
    }

    /// Checks if the class has a 'ctorFromCt' method
    bool checkForCtorFromCt(Node* cls)
    {
        NodeArray decls = Nest_symTabLookupCurrent(Nest_childrenContext(cls)->currentSymTab, "ctorFromCt");
        for ( Node* n: decls )
        {
            if ( Feather_effectiveEvalMode(n) == modeRt )
            {
                Nest_freeNodeArray(decls);
                return true;
            }
        }
        Nest_freeNodeArray(decls);
        return false;
    }

    /// Checks if the given class has reference fields
    bool hasReferences(Node* cls)
    {
        for ( Node* field: cls->children )
        {
            // Take in account only fields of the current class
            Node* cls2 = Feather_getParentClass(field->context);
            if ( cls2 != cls )
                continue;

            if ( field->type->numReferences > 0 )
                return true;
        }
        return false;
    }

    // Add to a local space an operator call
    void addOperatorCall(Node* dest, bool reverse, Node* operand1, const string& op, Node* operand2)
    {
        Node* call = mkOperatorCall(dest->location, operand1, fromString(op), operand2);
        if ( !reverse )
            Nest_appendNodeToArray(&dest->children, call);
        else
            Nest_insertNodeIntoArray(&dest->children, 0, call);
    }

    // Add a method with the given body and given arguments to the parent class
    Node* addMethod(Node* parent, const string& name, Node* body, vector<pair<TypeRef, string>> params, Node* resClass = nullptr, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location;
        loc.end = loc.start;

        // Construct the parameters list, return type node
        NodeVector sprParams;
        sprParams.reserve(params.size());
        for ( auto param: params )
        {
            sprParams.push_back(mkSprParameter(loc, fromString(param.second), param.first));
        }
        Node* parameters = sprParams.empty() ? nullptr : Feather_mkNodeList(loc, all(sprParams));
        Node* ret = resClass ? createTypeNode(Nest_childrenContext(parent), loc, Feather_getDataType(resClass, 0, modeRtCt)) : nullptr;

        // Add the function
        Node* m = mkSprFunction(loc, fromString(name), parameters, ret, body);
        Nest_setPropertyInt(m, propNoDefault, 1);
        copyAccessType(m, parent);
        Feather_setEvalMode(m, mode == modeUnspecified ? Feather_effectiveEvalMode(parent) : mode);
        Class_addChild(parent, m);
        if ( !Nest_computeType(m) )
            return nullptr;
        return m;
    }

    // Add a method with the given body to the parent class
    Node* addMethod(Node* parent, const string& name, Node* body, TypeRef otherParam, Node* resClass = nullptr, EvalMode mode = modeUnspecified)
    {
        return addMethod(parent, name, body, otherParam ? vector<pair<TypeRef, string>>({ {otherParam, string("other")} }) : vector<pair<TypeRef, string>>({}), resClass, mode);
    }

    // Add an associated function with the given body and given arguments, near the parent class
    Node* addAssociatedFun(Node* parent, const string& name, Node* body, vector<pair<TypeRef, string>> params, Node* resClass = nullptr, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location;
        loc.end = loc.start;
        CompilationContext* ctx = parent->context;

        // Construct the parameters list, return type node
        NodeVector sprParams;
        sprParams.reserve(params.size());
        for ( auto param: params )
        {
            sprParams.push_back(mkSprParameter(loc, fromString(param.second), param.first));
        }
        Node* parameters = sprParams.empty() ? nullptr : Feather_mkNodeList(loc, all(sprParams));
        Node* ret = resClass ? createTypeNode(ctx, loc, Feather_getDataType(resClass, 0, modeRtCt)) : nullptr;

        // Add the function in the context of the parent
        Node* f = mkSprFunction(loc, fromString(name), parameters, ret, body);
        Nest_setPropertyInt(f, propNoDefault, 1);
        copyAccessType(f, parent);
        Feather_setEvalMode(f, mode == modeUnspecified ? Feather_effectiveEvalMode(parent) : mode);
        Nest_setContext(f, ctx);
        if ( !Nest_computeType(f) )
            return nullptr;
        Nest_queueSemanticCheck(f);
        Nest_appendNodeToArray(&parent->additionalNodes, f);
        return f;
    }

    /// Generate a typical method with the given name, by calling 'op' for the base classes and fields
    void generateMethod(Node* parent, const string& name, const string& op, TypeRef otherParam, bool reverse = false, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location;
        loc.end = loc.start;
        Node* cls = Nest_explanation(parent);
        cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
        ASSERT(cls);

        Node* otherRef = nullptr;
        if ( otherParam )
        {
            otherRef = mkIdentifier(loc, fromCStr("other"));
            if ( otherParam->numReferences > 0 )
                otherRef = Feather_mkMemLoad(loc, otherRef);
        }

        // Construct the body
        Node* body = Feather_mkLocalSpace(loc, {});
        for ( Node* field: cls->children )
        {
            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("this"))), field);
            Node* otherFieldRef = otherParam ? Feather_mkFieldRef(loc, otherRef, field) : nullptr;

            string oper = op;
            if ( field->type->numReferences > 0 )
            {
                if ( op == "=" || op == "ctor" )
                {
                    oper = ":=";    // Transform into ref assignment
                    if ( !otherFieldRef )
                        otherFieldRef = buildNullLiteral(loc);
                }
                else if ( op == "dtor" )
                    continue;       // Nothing to destruct on references
            }
            addOperatorCall(body, reverse, fieldRef, oper, otherFieldRef);
        }

        addMethod(parent, name, body, otherParam, nullptr, mode);
    }

    /// Generate an associated function with the given name, by calling 'op' for the base classes and fields
    void generateAssociatedFun(Node* parent, const string& name, const string& op, TypeRef otherParam, bool reverse = false, EvalMode mode = modeUnspecified)
    {
        Location loc = parent->location;
        loc.end = loc.start;
        Node* cls = Nest_explanation(parent);
        cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
        ASSERT(cls);

        Node* destRef = mkIdentifier(loc, fromCStr("dest"));

        Node* otherRef = nullptr;
        if ( otherParam )
        {
            otherRef = mkIdentifier(loc, fromCStr("other"));
            if ( otherParam->numReferences > 0 )
                otherRef = Feather_mkMemLoad(loc, otherRef);
        }

        // Construct the body
        Node* body = Feather_mkLocalSpace(loc, {});
        for ( Node* field: cls->children )
        {
            // Take in account only fields of the current class
            Node* cls2 = Feather_getParentClass(field->context);
            if ( cls2 != cls )
                continue;

            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, destRef), field);
            Node* otherFieldRef = otherParam ? Feather_mkFieldRef(loc, otherRef, field) : nullptr;

            string oper = op;
            if ( field->type->numReferences > 0 )
            {
                if ( op == "=" || op == "ctor" )
                {
                    oper = ":=";    // Transform into ref assignment
                    if ( !otherFieldRef )
                        otherFieldRef = buildNullLiteral(loc);
                }
                else if ( op == "dtor" )
                    continue;       // Nothing to destruct on references
            }
            addOperatorCall(body, reverse, fieldRef, oper, otherFieldRef);
        }

        vector<pair<TypeRef, string>> params;
        params.reserve(2);
        params.push_back({Feather_addRef(cls->type), string("dest")});
        if ( otherParam )
            params.push_back({otherParam, string("other")});

        addAssociatedFun(parent, name, body, params, nullptr, mode);
    }

    /// Generate an empty, uninitialized ctor
    void generateUnititializedCtor(Node* parent)
    {
        Location loc = parent->location;
        loc.end = loc.start;

        Node* body = Feather_mkLocalSpace(loc, {});
        addMethod(parent, "ctor", body, StdDef::typeUninitialized);
    }
    // TODO (ctors): Check the generation of ctors -> prefer external ctors


    /// Generate an init ctor, that initializes all the members with data received as arguments
    void generateInitCtor(Node* parent)
    {
        vector<pair<TypeRef, string>> params;

        Location loc = parent->location;
        loc.end = loc.start;
        Node* cls = Nest_explanation(parent);
        cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
        ASSERT(cls);

        // Construct the body
        Node* body = Feather_mkLocalSpace(loc, {});
        for ( Node* field: cls->children )
        {
            // Take in account only fields of the current class
            Node* cls2 = Feather_getParentClass(field->context);
            if ( cls2 != cls )
                continue;

            TypeRef t = field->type;

            // Add a parameter for the base
            string paramName = "f"+toString(Feather_getName(field));
            params.push_back({t, paramName});
            Node* paramId = mkIdentifier(loc, fromString(paramName));
            if ( t->numReferences > 0 )
                paramId = Feather_mkMemLoad(loc, paramId);

            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("this"))), field);

            string oper = t->numReferences > 0 ? ":=" : "ctor";
            addOperatorCall(body, false, fieldRef, oper, paramId);
        }

        addMethod(parent, "ctor", body, params);
    }

    /// Generate the equality check method for the given class
    void generateEqualityCheckMethod(Node* parent)
    {
        Location loc = parent->location;
        loc.end = loc.start;
        Node* cls = Nest_explanation(parent);
        cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
        ASSERT(cls);

        // Construct the equality check expression
        Node* exp = nullptr;
        for ( Node* field: cls->children )
        {
            // Take in account only fields of the current class
            Node* cls2 = Feather_getParentClass(field->context);
            if ( cls2 != cls )
                continue;

            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("this"))), field);
            Node* otherFieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("other"))), field);

            const char* op = (field->type->numReferences == 0) ? "==" : "===";
            Node* curExp = mkOperatorCall(loc, fieldRef, fromCStr(op), otherFieldRef);
            if ( !exp )
                exp = curExp;
            else
                exp = mkOperatorCall(loc, exp, fromCStr("&&"), curExp);
        }
        if ( !exp )
            exp = buildBoolLiteral(loc, true);

        Node* body = Feather_mkLocalSpace(loc, {});
        Nest_appendNodeToArray(&body->children, mkReturnStmt(loc, exp));
        addMethod(parent, "==", body, Feather_getDataType(cls, 1, modeUnspecified), StdDef::clsBool);
    }

    /// Search the given body for a constructor with the given properties.
    ///
    /// This can search for constructors of given classes, constructors called on this, or called for the given field.
    ///
    /// It will search only the instructions directly inside the given local space, or in a child local space
    /// It will not search inside conditionals, or other instructions
    bool hasCtorCall(Node* inSpace, Node* ofClass, bool checkThis, Node* forField)
    {
        // Check all the items in the local space
        for ( Node* n: inSpace->children )
        {
            if ( !Nest_computeType(n) )
                continue;
            n = Nest_explanation(n);
            if ( !n )
                continue;

            // Check inner local spaces
            if ( n->nodeKind == nkFeatherLocalSpace )
            {
                if ( hasCtorCall(n, ofClass, checkThis, forField) )
                    return true;
                continue;
            }

            // We consider function calls for our checks
            if ( n->nodeKind != nkFeatherExpFunCall )
                continue;
            if ( Feather_getName(at(n->referredNodes, 0)) != "ctor" )
                continue;
            if ( Nest_nodeArraySize(n->children) == 0 )
                continue;
            Node* thisArg = at(n->children, 0);

            // If a class is given, check that the call is made to a function of that class
            if ( ofClass )
            {
                Node* parentCls = Feather_getParentClass(at(n->referredNodes, 0)->context);
                if ( parentCls != ofClass )
                    continue;
            }

            // Check for this to be passed as argument
            if ( checkThis )
            {
                // If we have a MemLoad, just ignore it
                if ( thisArg->nodeKind == nkFeatherExpMemLoad )
                    thisArg = Nest_explanation(at(thisArg->children, 0));

                if ( !thisArg || thisArg->nodeKind != nkFeatherExpVarRef
                    || Feather_getName(at(thisArg->referredNodes, 0)) != "this" )
                    continue;
            }

            // If a field is given, check that the this argument is a reference to our field
            if ( forField )
            {
                // If we have a Bitcast, just ignore it
                if ( thisArg->nodeKind == nkFeatherExpBitcast )
                    thisArg = Nest_explanation(at(thisArg->children, 0));

                if ( !thisArg || thisArg->nodeKind != nkFeatherExpFieldRef
                    || at(thisArg->referredNodes, 0) != forField )
                    continue;
            }

            return true;
        }
        return false;
    }
}

void _IntModClassMembers_afterComputeType(Modifier*, Node* node)
{
    /// Check to apply only to classes
    if ( node->nodeKind != nkSparrowDeclSprClass )
        REP_INTERNAL(node->location, "IntModClassMembers modifier can be applied only to classes");
    Node* cls = node;
    if ( !cls->type )
        REP_INTERNAL(node->location, "Type was not computed for %1% when applying IntModClassMembers") % Feather_getName(node);

    Node* basicClass = Nest_explanation(node);
    basicClass = basicClass && basicClass->nodeKind == nkFeatherDeclClass ? basicClass : nullptr;
    ASSERT(basicClass);
    TypeRef paramType = Feather_getDataType(basicClass, 1, modeRtCt);

    // Default ctor
    if ( !checkForMember(cls, "ctor", nullptr) )
        generateMethod(cls, "ctor", "ctor", nullptr);

    // Uninitialized ctor
    if ( !checkForMember(cls, "ctor", StdDef::clsUninitialized) )
        generateUnititializedCtor(cls);

    // Copy ctor
    if ( !checkForMember(cls, "ctor", basicClass) ) {
        generateMethod(cls, "ctor", "ctor", paramType);
        // generateAssociatedFun(cls, "construct", "construct", paramType);
    }

    // Initialization ctor
    if ( Nest_hasProperty(cls, propGenerateInitCtor) )
        generateInitCtor(cls);

    // CT to RT ctor
    if ( !checkForCtorFromCt(cls) && !hasReferences(basicClass) )
        generateMethod(cls, "ctorFromCt", "ctor", Feather_checkChangeTypeMode(Feather_getDataType(basicClass, 0, modeRtCt), modeCt, node->location), false, modeRt);

    // Dtor
    if ( !checkForMember(cls, "dtor", nullptr) ) {
        generateMethod(cls, "dtor", "dtor", nullptr, true);
        // generateAssociatedFun(cls, "destruct", "dtor", nullptr);
    }

    // Assignment operator
    if ( !checkForMember(cls, "=", basicClass) )
        generateMethod(cls, "=", "=", paramType);

    // Equality test operator
    if ( !checkForMember(cls, "==", basicClass) )
        generateEqualityCheckMethod(cls);
}

void IntModCtorMembers_beforeSemanticCheck(Modifier*, Node* fun)
{
    /// Check to apply only to non-static constructors
    if ( fun->nodeKind != nkSparrowDeclSprFunction || Feather_getName(fun) != "ctor" )
        REP_INTERNAL(fun->location, "IntModCtorMembers modifier can be applied only to constructors");
    if ( !funHasThisParameters(fun) )
        REP_INTERNAL(fun->location, "IntModCtorMembers cannot be applied to static constructors");

    // If we have a body, make sure it's a local space
    Node* body = at(fun->children, 2);
    if ( !body )
        return; // nothing to do
    if ( body->nodeKind != nkFeatherLocalSpace )
        REP_INTERNAL(fun->location, "Constructor body is not a local space (needed by IntModCtorMembers)");

    // Get the class
    Node* cls = Feather_getParentClass(fun->context);
    CHECK(fun->location, cls);

    // If we are calling other constructor of this class, don't add any initialization
    if ( hasCtorCall(body, cls, true, nullptr) )
        return;

    // Generate the ctor calls in the order of the fields; add them to the body of the constructor
    const Location& loc = body->location;
    for ( int i = Nest_nodeArraySize(cls->children)-1; i>=0; --i )
    {
        Node* field = at(cls->children, i);

        // Make sure we initialize only fields of the current class
        Node* cls2 = Feather_getParentClass(field->context);
        if ( cls2 != cls )
            continue;

        if ( !hasCtorCall(body, nullptr, false, field) )
        {
            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("this"))), field);
            Node* call = nullptr;
            if ( field->type->numReferences == 0 )
            {
                call = mkOperatorCall(loc, fieldRef, fromCStr("ctor"), nullptr);
            }
            else
            {
                call = mkOperatorCall(loc, fieldRef, fromCStr(":="), buildNullLiteral(loc));
            }
            Nest_setContext(call, Nest_childrenContext(body));
            Nest_insertNodeIntoArray(&body->children, 0, call);
        }
    }
}

void IntModDtorMembers_beforeSemanticCheck(Modifier*, Node* fun)
{
    /// Check to apply only to non-static destructors
    if ( fun->nodeKind != nkSparrowDeclSprFunction || Feather_getName(fun) != "dtor" )
        REP_INTERNAL(fun->location, "IntModDtorMembers modifier can be applied only to destructors");
    if ( !funHasThisParameters(fun) )
        REP_INTERNAL(fun->location, "IntModDtorMembers cannot be applied to static destructors");

    // If we have a body, make sure it's a local space
    Node* body = at(fun->children, 2);
    if ( !body )
        return; // nothing to do
    if ( body->nodeKind != nkFeatherLocalSpace )
        REP_INTERNAL(fun->location, "Destructor body is not a local space (needed by IntModDtorMembers)");

    // Get the class
    Node* cls = Feather_getParentClass(fun->context);
    CHECK(fun->location, cls);

    // Generate the dtor calls in reverse order of the fields; add them to the body of the destructor
    CompilationContext* context = Nest_childrenContext(body);
    const Location& loc = body->location;
    for ( int i = Nest_nodeArraySize(cls->children)-1; i>=0; --i )
    {
        Node* field = at(cls->children, i);

        // Make sure we destruct only fields of the current class
        Node* cls2 = Feather_getParentClass(field->context);
        if ( cls2 != cls )
            continue;

        if ( field->type->numReferences == 0 )
        {
            Node* fieldRef = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, mkIdentifier(loc, fromCStr("this"))), field);
            Nest_setContext(fieldRef, context);
            Node* call = mkOperatorCall(loc, fieldRef, fromCStr("dtor"), nullptr);
            Nest_setContext(call, context);
            Nest_appendNodeToArray(&body->children, call);
        }

    }
}

Modifier _classMembersIntMod = { modTypeAfterComputeType, &_IntModClassMembers_afterComputeType };
Modifier _ctorMembersIntMod = { modTypeBeforeSemanticCheck, &IntModCtorMembers_beforeSemanticCheck };
Modifier _dtorMembersIntMod = { modTypeBeforeSemanticCheck, &IntModDtorMembers_beforeSemanticCheck };

Modifier* SprFe_getClassMembersIntMod()
{
    return &_classMembersIntMod;
}
Modifier* SprFe_getCtorMembersIntMod()
{
    return &_ctorMembersIntMod;
}
Modifier* SprFe_getDtorMembersIntMod()
{
    return &_dtorMembersIntMod;
}
