#include <StdInc.h>
#include "SparrowNodes.h"
#include "IntMods.h"
#include "SprDebug.h"
#include <SparrowFrontendTypes.h>

#include <Helpers/CommonCode.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Generics.h>
#include <Helpers/StdDef.h>
#include <Helpers/Convert.h>
#include <Helpers/ForEachNodeInNodeList.h>

using namespace SprFrontend;
using namespace Nest;

namespace
{
    ////////////////////////////////////////////////////////////////////////////
    // Helpers for SprClass node
    //

    /// Get the fields from the symtab of the current class
    /// In the process it might compute the type of the SprVariables
    NodeVector getFields(SymTab* curSymTab)
    {
        // Check all the nodes registered in the children context so far to discover the fields
        NodeVector fields;
        for ( Node* n: Nest_symTabAllEntries(curSymTab) )
        {
            if ( n->nodeKind == nkFeatherDeclVar || n->nodeKind == nkSparrowDeclSprVariable )
                fields.push_back(n);
        }

        // Sort the fields by location - we need to add them in order
        sort(fields.begin(), fields.end(), [](Node* f1, Node* f2) { return Nest_compareLocations(&f1->location, &f2->location) < 0; });

        // Make sure we have only fields
        for ( Node*& field: fields )
        {
            if ( !Nest_computeType(field) )
                field = nullptr;
            field = Nest_explanation(field);
            if ( field->nodeKind != nkFeatherDeclVar || !isField(field) )
                field = nullptr;
        }

        // Remove all the nulls
        fields.erase(remove_if(fields.begin(), fields.end(), [](Node* n) { return n == nullptr; }), fields.end());

        return fields;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for SprFunction node
    //

    static const char* propIsMember = "spr.isMember";

    void handleStaticCtorDtor(Node* node, bool ctor)
    {
        ASSERT(Nest_nodeArraySize(node->children) == 4);
        Node* parameters = at(node->children, 0);

        // Make sure we don't have any parameters
        if ( parameters && Nest_nodeArraySize(parameters->children) != 0 )
        {
            REP_ERROR(node->location, "Static constructors and destructors cannot have parameters");
            return;
        }

        // Add a global construct / destruct action call to this
        Node* funCall = Feather_mkFunCall(node->location, node->explanation, {});
        Node* n = ctor ? Feather_mkGlobalConstructAction(node->location, funCall) : Feather_mkGlobalDestructAction(node->location, funCall);
        Nest_setContext(n, node->context);
        if ( Nest_semanticCheck(n) )
        {
            Nest_appendNodeToArray(&node->additionalNodes, n);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for SprVariable node
    //

    enum VarKind
    {
        varLocal,
        varField,
        varGlobal,
    };

    TypeRef computeVarType(Node* parent, CompilationContext* ctx, Node* typeNode, Node* init)
    {
        const Location& loc = parent->location;

        TypeRef t = nullptr;

        // If a type node was given, take the type from it
        if ( typeNode )
        {
            Nest_setPropertyExplInt(typeNode, propAllowDeclExp, 1);
            t = getType(typeNode);
            if ( !t )
                return nullptr;
        }
        else
        {
            // If no type node was given, maybe a type was given directly; if so, take it
            const TypeRef* givenType = Nest_getPropertyType(parent, "spr.givenType");
            t = givenType ? *givenType : nullptr;
        }

        // Should we get the type from the initialization expression?
        bool getTypeFromInit = nullptr == t;
        bool isRefAuto = false;
        if ( t && isConceptType(t, isRefAuto) )
            getTypeFromInit = true;
        if ( getTypeFromInit )
        {
            if ( !init )
                REP_ERROR_RET(nullptr, loc, "Initializer is required to deduce the type of the variable");

            if ( Nest_computeType(init) )
            {
                // If still have a type (i.e, auto type), check for conversion
                if ( t && !canConvert(init, t) )
                    REP_ERROR_RET(nullptr, init->location, "Initializer of the variable (%1%) cannot be converted to variable type (%2%)")
                    % init->type % t;

                t = getAutoType(init, isRefAuto);
            }
            else
                return nullptr;
        }

        // Make sure we have the right mode for our context
        t = Feather_adjustMode(t, ctx, loc);
        return t;
    }

}

void Package_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);

    // Set the context for all the children
    Node* content = at(node->children, 0);
    if ( content )
        Nest_setContext(content, node->childrenContext);
}
TypeRef Package_ComputeType(Node* node)
{
    // This can be computed without checking the children
    node->type = Feather_getVoidType(modeCt);

    // Compute the type for the children
    Node* content = at(node->children, 0);
    if ( content && !Nest_computeType(content) )
        return nullptr;
    node->explanation = content;
    if ( content )
        checkForAllowedNamespaceChildren(content);

    return node->type;
}
Node* Package_SemanticCheck(Node* node)
{
    if ( !Nest_computeType(node) )
        return nullptr;
    return Nest_semanticCheck(at(node->children, 0));
}

void SprClass_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, Feather_effectiveEvalMode(node));

    Nest_defaultFunSetContextForChildren(node);
}
TypeRef SprClass_ComputeType(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* parameters = at(node->children, 0);
    Node* children = at(node->children, 1);
    Node* ifClause = at(node->children, 2);

    // Is this a generic?
    if ( parameters && Nest_nodeArraySize(parameters->children) != 0 )
    {
        Node* generic = mkGenericClass(node, parameters, ifClause);
        Nest_setPropertyNode(node, propResultingDecl, generic);
        Nest_setContext(generic, node->context);
        if ( !Nest_semanticCheck(generic) )
            return nullptr;
        node->explanation = generic;
        return generic->type;
    }
    if ( ifClause )
        REP_ERROR_RET(nullptr, node->location, "If clauses must be applied only to generics; this is not a generic class");

    // Default class members
    if ( !Nest_hasProperty(node, propNoDefault) )
        Nest_addModifier(node, SprFe_getClassMembersIntMod());

    Node* resultingClass = nullptr;

    // Special case for Type class; re-use the existing StdDef class
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    if ( nativeName && *nativeName == "Type" )
    {
        resultingClass = StdDef::clsType;
    }

    // Create the resulting Feather.Class object
    if ( !resultingClass ) {
        resultingClass = Feather_mkClass(node->location, Feather_getName(node), {});
        copyAccessType(resultingClass, node);
    }
    Feather_setShouldAddToSymTab(resultingClass, 0);

    // Copy the "native" and "description" properties to the resulting class
    if ( nativeName )
    {
        Nest_setPropertyString(resultingClass, propNativeName, *nativeName);
    }
    const StringRef* description = Nest_getPropertyString(node, propDescription);
    if ( description )
    {
        Nest_setPropertyString(resultingClass, propDescription, *description);
    }

    Feather_setEvalMode(resultingClass, Feather_nodeEvalMode(node));
    resultingClass->childrenContext = node->childrenContext;
    Nest_setContext(resultingClass, node->context);
    Nest_setPropertyNode(node, propResultingDecl, resultingClass);

    node->explanation = resultingClass;

    // Check for Std classes
    checkStdClass(resultingClass);

    // We now have a type - from now on we can safely compute the types of the children
    node->type = Feather_getDataType(resultingClass, 0, modeRtCt);

    // Get the fields from the current class
    NodeVector fields = getFields(node->childrenContext->currentSymTab);
    Nest_appendNodesToArray(&resultingClass->children, all(fields));

    // Check all the children
    if ( children )
    {
        Nest_computeType(children); // Ignore local errors
        checkForAllowedNamespaceChildren(children, true);
    }

    // Take the fields and the methods
    forEachNodeInNodeList(children, [&] (Node* child) -> void
    {
        Node* p = Nest_explanation(child);
        if ( !isField(p) || Nest_hasProperty(node, propIsStatic) )
        {
            // Methods, generics
            Nest_appendNodeToArray(&node->additionalNodes, child);
        }
    });

    // Compute the type for the basic class
    if ( !Nest_computeType(resultingClass) )
        return nullptr;
    return node->type;
}
Node* SprClass_SemanticCheck(Node* node)
{
    if ( !Nest_computeType(node) )
        return nullptr;

    if ( !Nest_semanticCheck(node->explanation) )
        return nullptr;

    if ( node->explanation->nodeKind != nkFeatherDeclClass )
        return node->explanation; // This should be a generic; there is nothing else to do here

    // Semantic check all the children
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* children = at(node->children, 1);
    if ( children )
        Nest_semanticCheck(children);   // Ignore possible failures
    return node->explanation;
}


void SprFunction_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, Feather_effectiveEvalMode(node));

    Nest_defaultFunSetContextForChildren(node);
}
TypeRef SprFunction_ComputeType(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 4);
    Node* parameters = at(node->children, 0);
    Node* returnType = at(node->children, 1);
    Node* body = at(node->children, 2);
    Node* ifClause = at(node->children, 3);

    bool isStatic = Nest_hasProperty(node, propIsStatic);

    // Check if this is a member function
    Node* parentClass = Feather_getParentClass(node->context);
    bool isMember = nullptr != parentClass;
    if ( !isMember && isStatic )
        REP_ERROR_RET(nullptr, node->location, "Only functions inside classes can be static");
    if ( isMember )
        Nest_setPropertyInt(node, propIsMember, 1);

    // Is this a generic?
    if ( parameters )
    {
        Node* thisClass = isMember && !isStatic ? parentClass : nullptr;
        Node* generic = createGenericFun(node, parameters, ifClause, thisClass);
        if ( generic )
        {
            // TODO (explanation): explanation should be the result of semantic check
            node->explanation = generic;
            if ( !Nest_computeType(node->explanation) )
                return nullptr;
            Nest_setPropertyNode(node, propResultingDecl, generic);
            return node->explanation->type;
        }
    }
    if ( ifClause )
        REP_ERROR_RET(nullptr, node->location, "If clauses must be applied only to generics; this is not a generic function");

    StringRef funName = Feather_getName(node);

    // Special modifier for ctors & dtors
    if ( isMember && !isStatic && !Nest_hasProperty(node, propNoDefault) )
    {
        if ( funName == "ctor" )
            Nest_addModifier(node, SprFe_getCtorMembersIntMod());
        if ( funName == "dtor" )
            Nest_addModifier(node, SprFe_getDtorMembersIntMod());
    }

    EvalMode thisEvalMode = Feather_effectiveEvalMode(node);

    // Create the resulting function object
    Node* resultingFun = Feather_mkFunction(node->location, funName, nullptr, {}, body);
    Feather_setShouldAddToSymTab(resultingFun, 0);
    copyAccessType(resultingFun, node);

    // Copy the "native" and the "autoCt" properties
    const StringRef* nativeName = Nest_getPropertyString(node, propNativeName);
    if ( nativeName )
        Nest_setPropertyString(resultingFun, propNativeName, *nativeName);
    if ( Nest_hasProperty(node, propAutoCt) )
        Nest_setPropertyInt(resultingFun, propAutoCt, 1);
    if ( Nest_hasProperty(node, propNoInline) )
        Nest_setPropertyInt(resultingFun, propNoInline, 1);

    Feather_setEvalMode(resultingFun, thisEvalMode);
    resultingFun->childrenContext = node->childrenContext;
    Nest_setContext(resultingFun, node->context);
    Nest_setPropertyNode(node, propResultingDecl, resultingFun);

    // Compute the types of the parameters first
    if ( parameters && !Nest_computeType(parameters) )
        return nullptr;

    // If this is a non-static member function, add this as a parameter
    if ( isMember && !isStatic )
    {
        TypeRef thisType = Feather_getDataType(parentClass, 1, thisEvalMode);
        Node* thisParam = Feather_mkVar(node->location, fromCStr("$this"), Feather_mkTypeNode(node->location, thisType));
        Nest_setContext(thisParam, node->childrenContext);
        Feather_Function_addParameter(resultingFun, thisParam);
    }

    // Add the actual specified parameters
    if ( parameters )
    {
        for ( Node* n: parameters->children )
        {
            if ( !n )
                REP_ERROR_RET(nullptr, n->location, "Invalid node as parameter");
            if ( n->nodeError )
                return nullptr;

            Feather_Function_addParameter(resultingFun, n);
        }
    }

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the parameters
    TypeRef resType = returnType ? getType(returnType) : Feather_getVoidType(thisEvalMode);
    if ( !resType )
        REP_INTERNAL(node->location, "Cannot compute the function resulting type");
    resType = Feather_adjustModeBase(resType, thisEvalMode, node->childrenContext, node->location);

    // If the parameter is a non-reference class, not basic numeric, add result parameter; otherwise, normal result
    if ( resType->hasStorage && resType->numReferences == 0 && !Feather_isBasicNumericType(resType) )
    {
        Node* resParam = Feather_mkVar(returnType->location, fromCStr("_result"), Feather_mkTypeNode(returnType->location, Feather_addRef(resType)));
        Nest_setContext(resParam, node->childrenContext);
        Feather_Function_addParameterFirst(resultingFun, resParam);
        Nest_setPropertyNode(resultingFun, propResultParam, resParam);
        Feather_Function_setResultType(resultingFun, Feather_mkTypeNode(returnType->location, Feather_getVoidType(thisEvalMode)));
    }
    else
        Feather_Function_setResultType(resultingFun, Feather_mkTypeNode(node->location, resType));

    // TODO (explanation): explanation should be the result of semantic check
    node->explanation = resultingFun;
    node->type = Nest_computeType(node->explanation);

    // Check for Std functions
    checkStdFunction(node);

    return node->type;
}
Node* SprFunction_SemanticCheck(Node* node)
{
    if ( !Nest_computeType(node) )
        return nullptr;
    Node* resultingFun = node->explanation;

    ASSERT(resultingFun);
    if ( !Nest_semanticCheck(resultingFun) )
        return nullptr;

    // Check for static ctors & dtors
    if ( resultingFun && (!Nest_hasProperty(node, propIsMember) || Nest_hasProperty(node, propIsStatic)) )
    {
        StringRef funName = Feather_getName(node);

        if ( funName == "ctor" )
            handleStaticCtorDtor(node, true);
        if ( funName == "dtor" )
            handleStaticCtorDtor(node, false);
    }
    return resultingFun;
}

void SprParameter_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    Nest_defaultFunSetContextForChildren(node);
}
TypeRef SprParameter_ComputeType(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    Node* typeNode = at(node->children, 0);

    const TypeRef* givenType = Nest_getPropertyType(node, "spr.givenType");
    TypeRef t = givenType ? *givenType : getType(typeNode);
    if ( !t )
        return nullptr;

    Node* resultingParam = Feather_mkVar(node->location, Feather_getName(node), Feather_mkTypeNode(node->location, t));
    Feather_setEvalMode(resultingParam, Feather_effectiveEvalMode(node));
    Feather_setShouldAddToSymTab(resultingParam, 0);
    Nest_setContext(resultingParam, node->context);
    if ( !Nest_computeType(resultingParam) )
        return nullptr;
    Nest_setPropertyNode(node, propResultingDecl, resultingParam);
    node->explanation = resultingParam;
    return resultingParam->type;
}
Node* SprParameter_SemanticCheck(Node* node)
{
    if ( !Nest_computeType(node) )
        return nullptr;

    if ( !Nest_semanticCheck(node->explanation) )
        return nullptr;

    Node* init = at(node->children, 1);
    if ( init && !Nest_semanticCheck(init) )
        return nullptr;
    return node->explanation;
}


void SprVariable_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    // Create a new child compilation context if the mode has changed; otherwise stay in the same context
    EvalMode curEvalMode = Feather_nodeEvalMode(node);
    if ( curEvalMode != modeUnspecified && curEvalMode != node->context->evalMode )
        node->childrenContext = Nest_mkChildContext(node->context, curEvalMode);
    else
        node->childrenContext = node->context;

    Nest_defaultFunSetContextForChildren(node);
}
TypeRef SprVariable_ComputeType(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 2);
    Node* typeNode = at(node->children, 0);
    Node* init = at(node->children, 1);

    bool isStatic = Nest_hasProperty(node, propIsStatic);

    // Check the kind of the variable (local, global, field)
    VarKind varKind = varLocal;
    Node* parentFun = Feather_getParentFun(node->context);
    Node* parentClass = nullptr;
    if ( !parentFun )
    {
        // Check if this is a member function
        parentClass = Feather_getParentClass(node->context);
        if ( parentClass )
        {
            varKind = isStatic ? varGlobal : varField;
            if ( isStatic )
                parentClass = nullptr;
        }
        else
        {
            varKind = varGlobal;
            if ( isStatic )
                REP_ERROR(node->location, "Only variables inside classes can be static");
        }
    }

    // Get the type of the variable
    TypeRef t = computeVarType(node, node->childrenContext, typeNode, init);
    if ( !t )
        return nullptr;

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if ( t->mode == modeCt )
        Feather_setEvalMode(node, modeCt);

    // Create the resulting var
    Node* resultingVar = Feather_mkVar(node->location, Feather_getName(node), Feather_mkTypeNode(node->location, t));
    Feather_setEvalMode(resultingVar, Feather_effectiveEvalMode(node));
    Feather_setShouldAddToSymTab(resultingVar, 0);
    Nest_setPropertyNode(node, propResultingDecl, resultingVar);

    if ( varKind == varField )
    {
        Nest_setPropertyInt(resultingVar, propIsField, 1);
    }

    Nest_setContext(resultingVar, node->childrenContext);
    if ( !Nest_computeType(resultingVar) )
        return nullptr;

    // If this is a CT variable in a non-ct function, make this a global variable
    if ( varKind == varLocal && node->context->evalMode == modeRt && t->mode == modeCt )
        varKind = varGlobal;

    // If this is a CT variable in a non-ct function, make this a global variable

    bool isRef = t->numReferences > 0;

    // Generate the initialization and destruction calls
    Node* ctorCall = nullptr;
    Node* dtorCall = nullptr;
    Node* varRef = nullptr;
    if ( varKind != varField && (init || !isRef) )
    {
        ASSERT(resultingVar->type);

        varRef = Feather_mkVarRef(node->location, resultingVar);
        Nest_setContext(varRef, node->childrenContext);

        if ( !isRef )
        {
            // Create ctor and dtor
            ctorCall = createCtorCall(node->location, node->childrenContext, varRef, init);
            if ( resultingVar->type->mode != modeCt )
                dtorCall = createDtorCall(node->location, node->childrenContext, varRef);
        }
        else if ( init )   // Reference initialization
        {
            // Create an assignment operator
            ctorCall = mkOperatorCall(node->location, varRef, fromCStr(":="), init);
        }
    }

    // Set the explanation of this node
    Node* expl = nullptr;
    if ( varKind == varField )
    {
        // For fields, just explain this as the resulting var
        expl = resultingVar;
    }
    else
    {
        // For local and global variables take into consideration the ctor and dtor calls
        Node* resVar = resultingVar;
        if ( varKind == varLocal )
        {
            // For local variables, add the ctor & dtor actions in the node list, and make this as explanation
            dtorCall = dtorCall ? Feather_mkScopeDestructAction(node->location, dtorCall) : nullptr;
        }
        else
        {
            // For global variables, wrap ctor & dtor nodes in global ctor/dtor actions
            if ( ctorCall )
                ctorCall = Feather_mkGlobalConstructAction(node->location, ctorCall);
            if ( dtorCall )
                dtorCall = Feather_mkGlobalDestructAction(node->location, dtorCall);

            // This is a global var: don't include it into the function scope
            resVar = nullptr;
        }
        expl = Feather_mkNodeList(node->location, fromIniList({ resVar, ctorCall, dtorCall, Feather_mkNop(node->location) }));

        // If the variable was not added to the result, add it as an additional
        // (top-level) node to the result
        if ( !resVar )
            Nest_appendNodeToArray(&expl->additionalNodes, resultingVar);
    }

    ASSERT(expl);
    Nest_setContext(expl, node->childrenContext);
    node->type = Nest_computeType(expl);
    if ( !node->type )
        return nullptr;
    node->explanation = expl;

    Nest_setPropertyNode(node, "spr.resultingVar", resultingVar);

    // TODO (var): field initialization
    if ( init && varKind == varField )
        REP_ERROR(node->location, "Initializers for class attributes are not supported yet");

    return node->type;
}
Node* SprVariable_SemanticCheck(Node* node)
{
    if ( !Nest_computeType(node) )
        return nullptr;

    // Semantically check the resulting variable and explanation
    Node* resultingVar = Nest_getCheckPropertyNode(node, "spr.resultingVar");
    if ( !Nest_semanticCheck(resultingVar) )
        return nullptr;
    return Nest_semanticCheck(node->explanation);
}


void SprConcept_SetContextForChildren(Node* node)
{
    Feather_addToSymTab(node);

    if ( !node->childrenContext )
        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, Feather_effectiveEvalMode(node));

    Nest_defaultFunSetContextForChildren(node);
}

Node* SprConcept_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* baseConcept = at(node->children, 0);
    Node* ifClause = at(node->children, 1);
    Node*& instantiationsSet = at(node->children, 2);
    StringRef paramName = Nest_getCheckPropertyString(node, "spr.paramName");

    // Compile the base concept node; make sure it's ct
    if ( baseConcept )
    {
        if ( !Nest_semanticCheck(baseConcept) )
            return nullptr;
        if ( !Feather_isCt(baseConcept) )
            REP_ERROR_RET(nullptr, baseConcept->location, "Base concept type needs to be compile-time (type=%1%)") % baseConcept->type;
    }

    Node* param = baseConcept
        ? mkSprParameter(node->location, paramName, baseConcept)
        : mkSprParameter(node->location, paramName, getConceptType());
    Nest_setContext(param, node->childrenContext);
    if ( !Nest_computeType(param))        // But not semanticCheck, as it will complain of instantiating a var of type auto
        return nullptr;

    instantiationsSet = mkInstantiationsSet(node, fromIniList({ param }), ifClause);
    return Feather_mkNop(node->location);
}

Node* Generic_SemanticCheck(Node* node)
{
    Node* res = Feather_mkNop(node->location);

    // Copy all the additional nodes
    for ( Node* n: all(node->additionalNodes) )
        Nest_appendNodeToArray(&node->additionalNodes, n);

    return res;
}

void Using_SetContextForChildren(Node* node)
{
    bool hasAlias = Nest_hasProperty(node, "name");
    if ( hasAlias )
        Feather_addToSymTab(node);
    else
        Nest_symTabAddToCheckNode(node->context->currentSymTab, node);

    Nest_defaultFunSetContextForChildren(node);
}
TypeRef Using_ComputeType(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 1);
    Node* usingNode = at(node->children, 0);
    const StringRef* alias = Nest_getPropertyString(node, "name");

    // Compile the using name
    Nest_setPropertyExplInt(usingNode, propAllowDeclExp, 1);
    if ( !Nest_semanticCheck(usingNode) )
        return nullptr;

    if ( !alias || size(*alias) == 0 )
    {
        // Make sure that this node refers to one or more declaration
        Node* baseExp;
        NodeVector decls = getDeclsFromNode(usingNode, baseExp);
        if ( decls.empty() && !Nest_hasProperty(node, propNoWarnIfNoDeclFound) )
            REP_WARNING(usingNode->location, "No declarations can be found for using");

        // Add references in the current symbol tab
        for ( Node* decl: decls )
        {
            Nest_symTabEnter(node->context->currentSymTab, Feather_getName(decl).begin, decl);
        }
    }
    else
    {
        // We already added this node to the current sym tab, as we set
        // shouldAddToSymTab_ to true.

        // If this points to another decl (and only one), set the resulting decl
        // for this node.
        if ( usingNode->nodeKind == nkSparrowExpDeclExp && size(usingNode->referredNodes) == 2 ) {
            // at position 0 we find 'baseExp'
            Node* decl = at(usingNode->referredNodes, 1);
            Nest_setPropertyNode(node, propResultingDecl, decl);
        }
    }

    node->explanation = Feather_mkNop(node->location);
    Nest_setContext(node->explanation, node->context);
    if ( !Nest_semanticCheck(node->explanation) )
        return nullptr;
    return node->explanation->type;
}
Node* Using_SemanticCheck(Node* node)
{
    return Nest_computeType(node) ? node->explanation : nullptr;
}
