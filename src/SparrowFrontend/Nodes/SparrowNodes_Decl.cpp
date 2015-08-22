#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"

#include "IntMods/IntModClassMembers.h"
#include "IntMods/IntModCtorMembers.h"
#include "IntMods/IntModDtorMembers.h"

#include <SparrowFrontendTypes.h>

#include <Helpers/CommonCode.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/Generics.h>
#include <Helpers/StdDef.h>
#include <Helpers/Convert.h>
#include <Helpers/QualifiedId.h>
#include <Helpers/ForEachNodeInNodeList.h>

#include <Feather/Util/Decl.h>
#include <Feather/Util/Context.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;
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
        for ( Node* n: curSymTab->allEntries() )
        {
            if ( n->nodeKind == nkFeatherDeclVar || n->nodeKind == nkSparrowDeclSprVariable )
                fields.push_back(n);
        }

        // Sort the fields by location - we need to add them in order
        sort(fields.begin(), fields.end(), [](Node* f1, Node* f2) { return Nest_compareLocations(&f1->location, &f2->location) < 0; });

        // Make sure we have only fields
        for ( Node*& field: fields )
        {
            Nest::computeType(field);
            field = Nest::explanation(field);
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
        ASSERT(node->children.size() == 4);
        Node* parameters = node->children[0];

        // Make sure we don't have any parameters
        if ( parameters && !parameters->children.empty() )
            REP_ERROR(node->location, "Static constructors and destructors cannot have parameters");

        // Add a global construct / destruct action call to this
        Node* funCall = mkFunCall(node->location, node->explanation, {});
        Node* n = ctor ? mkGlobalConstructAction(node->location, funCall) : mkGlobalDestructAction(node->location, funCall);
        Nest::setContext(n, node->context);
        Nest::semanticCheck(n);
        ASSERT(node->context->sourceCode());
        node->context->sourceCode()->addAdditionalNode(n);
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
            t = getType(typeNode);
            if ( !t )
                REP_ERROR(loc, "Invalid type for variable");
        }
        else
        {
            // If no type node was given, maybe a type was given directly; if so, take it
            const TypeRef* givenType = getPropertyType(parent, "spr.givenType");
            t = givenType ? *givenType : nullptr;
        }

        // Should we get the type from the initiailization expression?
        bool getTypeFromInit = nullptr == t;
        bool isRefAuto = false;
        if ( t && isConceptType(t, isRefAuto) )
            getTypeFromInit = true;
        if ( getTypeFromInit )
        {
            if ( !init )
                REP_ERROR(loc, "Initializer is requrired to deduce the type of the variable");

            Nest::computeType(init);

            // If still have a type (i.e, auto type), check for conversion
            if ( t && !canConvert(init, t) )
                REP_ERROR(init->location, "Initializer of the variable (%1%) cannot be converted to variable type (%2%)")
                % init->type % t;
            
            t = getAutoType(init, isRefAuto);
        }

        // Make sure we have the right mode for our context
        t = adjustMode(t, ctx, loc);
        return t;
    }
}

void SprCompilationUnit_SetContextForChildren(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* packageName = node->children[0];
    Node* imports = node->children[1];
    Node* declarations = node->children[2];
    if ( packageName )
        Nest::setContext(packageName, node->context);

    // Handle package name - create namespaces for it
    if ( packageName )
    {
        // Get the parts of the package name
        vector<string> names;
        interpretQualifiedId(packageName, names);
        ASSERT(!names.empty() && !names.back().empty());

        for ( int i=0; i<(int)names.size(); ++i )
        {
            // Try to find an existing package in the current symbol table
            NodeVector decls = node->context->currentSymTab()->lookupCurrent(names[i]);
            if ( decls.size() == 1 )
            {
                node->context = Nest::childrenContext(decls.front());
                continue;
            }

            // We didn't find the package part. From now on create new namespaces
            for ( int j=(int)names.size()-1; j>=i; --j )
            {
                Node* pk = mkSprPackage(packageName->location, move(names[j]), declarations);
                declarations = Feather::mkNodeList(packageName->location, {pk}, true);
                node->children[2] = declarations;
            }
            break;
        }
    }

    // If we don't have a children context, create one
    node->childrenContext = node->context;
    //if ( !node->childrenContext )
    //    node->childrenContext = node->context->createChildContext(node, evalMode());

    // Set the context for all the children
    if ( declarations )
        Nest::setContext(declarations, node->childrenContext);
    if ( imports )
        Nest::setContext(imports, node->childrenContext);

    // Handle imports
    if ( imports )
    {
        const Nest::SourceCode* sourceCode = (Nest::SourceCode*) node->location.sourceCode;
        for ( Node* i: imports->children )
        {
            Node* lit = ofKind(i, nkSparrowExpLiteral);
            if ( lit && Literal_isString(lit) )
            {
                Nest::theCompiler().addSourceCodeByFilename(sourceCode, Literal_getData(lit));
            }
            else
            {
                vector<string> qid;
                interpretQualifiedId(i, qid);
                Nest::theCompiler().addSourceCodeByQid(sourceCode, qid);
            }
        }
    }
}
TypeRef SprCompilationUnit_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* declarations = node->children[2];

    // Compute the type for the children
    if ( declarations )
    {
        Nest::computeType(declarations);
        checkForAllowedNamespaceChildren(declarations);
    }

    return Feather::getVoidType(Feather::modeCt);
}
Node* SprCompilationUnit_SemanticCheck(Node* node)
{
    Nest::computeType(node);

    ASSERT(node->children.size() == 3);
    Node* declarations = node->children[2];

    return declarations ? declarations : Feather::mkNop(node->location);
}

void Package_SetContextForChildren(Node* node)
{
    // TODO (now): Try to remove this
    Feather::addToSymTab(node);

    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = node->context->createChildContext(node);

    // Set the context for all the children
    Nest::setContext(node->children[0], node->childrenContext);
}
TypeRef Package_ComputeType(Node* node)
{
    // Compute the type for the children
    Nest::computeType(node->children[0]);
    node->explanation = node->children[0];
    checkForAllowedNamespaceChildren(node->children[0]);

    return Feather::getVoidType(modeCt);
}
Node* Package_SemanticCheck(Node* node)
{
    Nest::computeType(node);
    Nest::semanticCheck(node->children[0]);
    return node->children[0];
}

void SprClass_SetContextForChildren(Node* node)
{
    addToSymTab(node);
    
    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = node->context->createChildContext(node, effectiveEvalMode(node));

    defaultFunSetContextForChildren(node);
}
TypeRef SprClass_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 4);
    Node* parameters = node->children[0];
    Node* baseClasses = node->children[1];
    Node* children = node->children[2];
    Node* ifClause = node->children[3];

    // Is this a generic?
    if ( parameters && !parameters->children.empty() )
    {
        Node* generic = mkGenericClass(node, parameters, ifClause);
        setProperty(node, propResultingDecl, generic);
        Nest::setContext(generic, node->context);
        Nest::semanticCheck(generic);
        node->explanation = generic;
        return generic->type;
    }
    if ( ifClause )
        REP_ERROR(node->location, "If clauses must be applied only to generics; this is not a generic class");

    // Default class members
    if ( !hasProperty(node, propNoDefault) )
        addModifier(node, new IntModClassMembers);
    
    Node* resultingClass = nullptr;

    // Special case for Type class; re-use the existing StdDef class
    const string* nativeName = getPropertyString(node, propNativeName);
    if ( nativeName && *nativeName == "Type" )
    {
        resultingClass = StdDef::clsType;
    }

    // Create the resulting Feather.Class object
    if ( !resultingClass )
        resultingClass = Feather::mkClass(node->location, getName(node), {});
    setShouldAddToSymTab(resultingClass, false);

    // Copy the "native" and "description" properties to the resulting class
    if ( nativeName )
    {
        Nest::setProperty(resultingClass, Feather::propNativeName, *nativeName);
    }
    const string* description = getPropertyString(node, propDescription);
    if ( description )
    {
        Nest::setProperty(resultingClass, propDescription, *description);
    }

    setEvalMode(resultingClass, nodeEvalMode(node));
    resultingClass->childrenContext = node->childrenContext;
    Nest::setContext(resultingClass, node->context);
    setProperty(node, propResultingDecl, resultingClass);

    node->explanation = resultingClass;

    // Check for Std classes
    checkStdClass(resultingClass);
    
    // First check all the base classes
    if ( baseClasses )
    {
        for ( auto& bcName: baseClasses->children )
        {
            // Make sure the type refers to a class
            TypeRef bcType = getType(bcName);
            if ( !bcType || !bcType->hasStorage )
                REP_ERROR(node->location, "Invalid base class");
            Node* baseClass = classForType(bcType);
            
            // Compute the type of the base class
            Nest::computeType(baseClass);

            // Add the fields of the base class to the resulting basic class
            resultingClass->children.insert(resultingClass->children.end(), baseClass->children.begin(), baseClass->children.end());

            // Copy the symbol table entries of the base to this class
            SymTab* ourSymTab = childrenContext(node)->currentSymTab();
            SymTab* baseSymTab = Nest::childrenContext(baseClass)->currentSymTab();
            ourSymTab->copyEntries(baseSymTab);
        }
    }

    // We now have a type - from now on we can safely compute the types of the children
    node->type = getDataType(resultingClass);

    // Get the fields from the current class
    NodeVector fields = getFields(node->childrenContext->currentSymTab());
    resultingClass->children.insert(resultingClass->children.end(), fields.begin(), fields.end());

    // Check all the children
    if ( children )
    {
        Nest::computeType(children);
        checkForAllowedNamespaceChildren(children, true);
    }

    // Take the fields and the methods
    forEachNodeInNodeList(children, [&] (Node* child) -> void
    {
        Node* p = Nest::explanation(child);
        if ( !isField(p) )
        {
            // Methods, generics
            ASSERT(node->context->sourceCode());
            node->context->sourceCode()->addAdditionalNode(child);
        }
    });

    // Compute the type for the basic class
    Nest::computeType(resultingClass);
    return node->type;
}
Node* SprClass_SemanticCheck(Node* node)
{
    Nest::computeType(node);

    Nest::semanticCheck(node->explanation);

    if ( node->explanation->nodeKind != nkFeatherDeclClass )
        return node->explanation; // This should be a generic; there is nothing else to do here

    // Semantic check all the children
    ASSERT(node->children.size() == 4);
    Node* children = node->children[2];
    if ( children )
        Nest::semanticCheck(children);
    return node->explanation;
}


void SprFunction_SetContextForChildren(Node* node)
{
    addToSymTab(node);

    // If we don't have a children context, create one
    if ( !node->childrenContext )
        node->childrenContext = node->context->createChildContext(node, effectiveEvalMode(node));

    defaultFunSetContextForChildren(node);
}
TypeRef SprFunction_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 4);
    Node* parameters = node->children[0];
    Node* returnType = node->children[1];
    Node* body = node->children[2];
    Node* ifClause = node->children[3];

    bool isStatic = hasProperty(node, propIsStatic);

    // Check if this is a member function
    Node* parentClass = Feather::getParentClass(node->context);
    bool isMember = nullptr != parentClass;
    if ( !isMember && isStatic )
        REP_ERROR(node->location, "Only functions inside classes can be static");
    if ( isMember )
        setProperty(node, propIsMember, 1);

    // Is this a generic?
    if ( parameters )
    {
        Node* thisClass = isMember && !isStatic ? parentClass : nullptr;
        Node* generic = createGenericFun(node, parameters, ifClause, thisClass);
        if ( generic )
        {
            // TODO (explanation): explanation should be the result of semantic check
            node->explanation = generic;
            Nest::computeType(node->explanation);
            setProperty(node, propResultingDecl, generic);
            return node->explanation->type;
        }
    }
    if ( ifClause )
        REP_ERROR(node->location, "If clauses must be applied only to generics; this is not a generic function");

    const string& funName = getName(node);

    // Special modifier for ctors & dtors
    if ( isMember && !isStatic && !hasProperty(node, propNoDefault) )
    {
        if ( funName == "ctor" )
            addModifier(node, new IntModCtorMembers);
        if ( funName == "dtor" )
            addModifier(node, new IntModDtorMembers);
    }

    EvalMode thisEvalMode = effectiveEvalMode(node);

    // Create the resulting function object
    Node* resultingFun = mkFunction(node->location, funName, nullptr, {}, body);
    setShouldAddToSymTab(resultingFun, false);

    // Copy the "native" and the "autoCt" properties
    const string* nativeName = getPropertyString(node, propNativeName);
    if ( nativeName )
        Nest::setProperty(resultingFun, Feather::propNativeName, *nativeName);
    if ( hasProperty(node, propAutoCt) )
        Nest::setProperty(resultingFun, propAutoCt, 1);
    if ( hasProperty(node, propNoInline) )
        Nest::setProperty(resultingFun, propNoInline, 1);

    setEvalMode(resultingFun, thisEvalMode);
    resultingFun->childrenContext = node->childrenContext;
    Nest::setContext(resultingFun, node->context);
    setProperty(node, propResultingDecl, resultingFun);

    // Compute the types of the parameters first
    if ( parameters )
        Nest::computeType(parameters);

    // If this is a non-static member function, add this as a parameter
    if ( isMember && !isStatic )
    {
        TypeRef thisType = getDataType(parentClass, 1, thisEvalMode);
        Node* thisParam = Feather::mkVar(node->location, "$this", mkTypeNode(node->location, thisType));
        Nest::setContext(thisParam, node->childrenContext);
        Function_addParameter(resultingFun, thisParam);
    }

    // Add the actual specified parameters
    if ( parameters )
    {
        for ( Node* n: parameters->children )
        {
            if ( !n )
                REP_ERROR(n->location, "Invalid node as parameter");

            Function_addParameter(resultingFun, n);
        }
    }

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the parameters
    TypeRef resType = returnType ? getType(returnType) : getVoidType(thisEvalMode);
    resType = adjustMode(resType, thisEvalMode, node->childrenContext, node->location);

    // If the parameter is a non-reference class, not basic numeric, add result parameter; otherwise, normal result
    if ( resType->hasStorage && resType->numReferences == 0 && !isBasicNumericType(resType) )
    {
        Node* resParam = Feather::mkVar(returnType->location, "_result", mkTypeNode(returnType->location, addRef(resType)));
        Nest::setContext(resParam, node->childrenContext);
        Function_addParameter(resultingFun, resParam, true);
        Nest::setProperty(resultingFun, propResultParam, resParam);
        Function_setResultType(resultingFun, mkTypeNode(returnType->location, getVoidType(thisEvalMode)));
    }
    else
        Function_setResultType(resultingFun, mkTypeNode(node->location, resType));

    // TODO (explanation): explanation should be the result of semantic check
    node->explanation = resultingFun;
    Nest::computeType(node->explanation);
    node->type = node->explanation->type;

    // Check for Std functions
    checkStdFunction(node);

    return node->type;
}
Node* SprFunction_SemanticCheck(Node* node)
{
    Nest::computeType(node);
    Node* resultingFun = node->explanation;

    ASSERT(resultingFun);
    Nest::semanticCheck(resultingFun);

    // Check for static ctors & dtors
    if ( resultingFun && (!hasProperty(node, propIsMember) || hasProperty(node, propIsStatic)) )
    {
        const string& funName = getName(node);
        
        if ( funName == "ctor" )
            handleStaticCtorDtor(node, true);
        if ( funName == "dtor" )
            handleStaticCtorDtor(node, false);
    }
    return resultingFun;
}

void SprParameter_SetContextForChildren(Node* node)
{
    Feather::addToSymTab(node);

    defaultFunSetContextForChildren(node);
}
TypeRef SprParameter_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 2);
    Node* typeNode = node->children[0];

    const TypeRef* givenType = getPropertyType(node, "spr.givenType");
    TypeRef t = givenType ? *givenType : getType(typeNode);

    Node* resultingParam = Feather::mkVar(node->location, Feather::getName(node), Feather::mkTypeNode(node->location, t), 0, Feather::effectiveEvalMode(node));
    Feather::setShouldAddToSymTab(resultingParam, false);
    Nest::setContext(resultingParam, node->context);
    Nest::computeType(resultingParam);
    Nest::setProperty(node, Feather::propResultingDecl, resultingParam);
    node->explanation = resultingParam;
    return resultingParam->type;
}
Node* SprParameter_SemanticCheck(Node* node)
{
    Nest::computeType(node);

    Nest::semanticCheck(node->explanation);

    Node* init = node->children[1];
    if ( init )
        Nest::semanticCheck(init);
    return node->explanation;
}


void SprVariable_SetContextForChildren(Node* node)
{
    addToSymTab(node);

    // Create a new child compilation context if the mode has changed; otherwise stay in the same context
    EvalMode curEvalMode = nodeEvalMode(node);
    if ( curEvalMode != modeUnspecified && curEvalMode != node->context->evalMode() )
        node->childrenContext = new CompilationContext(node->context, curEvalMode);
    else
        node->childrenContext = node->context;

    defaultFunSetContextForChildren(node);
}
TypeRef SprVariable_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 2);
    Node* typeNode = node->children[0];
    Node* init = node->children[1];

    bool isStatic = hasProperty(node, propIsStatic);

    // Check the kind of the variable (local, global, field)
    VarKind varKind = varLocal;
    Node* parentFun = Feather::getParentFun(node->context);
    Node* parentClass = nullptr;
    if ( !parentFun )
    {
        // Check if this is a member function
        parentClass = Feather::getParentClass(node->context);
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

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if ( t->mode == modeCt )
        setEvalMode(node, modeCt);

    // Create the resulting var
    Node* resultingVar = mkVar(node->location, getName(node), mkTypeNode(node->location, t));
    setEvalMode(resultingVar, effectiveEvalMode(node));
    setShouldAddToSymTab(resultingVar, false);
    setProperty(node, propResultingDecl, resultingVar);

    if ( varKind == varField )
    {
        Nest::setProperty(resultingVar, propIsField, 1);
    }

    Nest::setContext(resultingVar, node->childrenContext);
    Nest::computeType(resultingVar);

    // If this is a CT variable in a non-ct function, make this a global variable
    if ( varKind == varLocal && node->context->evalMode() == modeRt && isCt(t) )
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

        varRef = mkVarRef(node->location, resultingVar);
        Nest::setContext(varRef, node->childrenContext);

        if ( !isRef )
        {
            // Create ctor and dtor
            ctorCall = createCtorCall(node->location, node->childrenContext, varRef, init);
            if ( !Feather::isCt(resultingVar->type) )
                dtorCall = createDtorCall(node->location, node->childrenContext, varRef);
        }
        else if ( init )   // Reference initialization
        {
            // Create an assignment operator
            ctorCall = mkOperatorCall(node->location, varRef, ":=", init);
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
            dtorCall = dtorCall ? mkScopeDestructAction(node->location, dtorCall) : nullptr;
        }
        else
        {
            // Add the variable at the top level
            ASSERT(node->context->sourceCode());
            node->context->sourceCode()->addAdditionalNode(resultingVar);
            resVar = nullptr;

            // For global variables, add the ctor & dtor actions as top level actions
            if ( ctorCall )
                ctorCall = mkGlobalConstructAction(node->location, ctorCall);
            if ( dtorCall )
                dtorCall = mkGlobalDestructAction(node->location, dtorCall);
        }
        expl = mkNodeList(node->location, { resVar, ctorCall, dtorCall, mkNop(node->location) });
    }

    ASSERT(expl);
    Nest::setContext(expl, node->childrenContext);
    Nest::computeType(expl);
    node->explanation = expl;
    node->type = expl->type;

    setProperty(node, "spr.resultingVar", resultingVar);

    // TODO (var): field initialization
    if ( init && varKind == varField )
        REP_ERROR(node->location, "Initializers for class attributes are not supported yet");

    return node->type;
}
Node* SprVariable_SemanticCheck(Node* node)
{
    Nest::computeType(node);

    // Semantically check the resulting variable and explanation
    Node* resultingVar = getCheckPropertyNode(node, "spr.resultingVar");
    Nest::semanticCheck(resultingVar);
    Nest::semanticCheck(node->explanation);
    return node->explanation;
}


void SprConcept_SetContextForChildren(Node* node)
{
    addToSymTab(node);

    if ( !node->childrenContext )
        node->childrenContext = node->context->createChildContext(node, effectiveEvalMode(node));

    defaultFunSetContextForChildren(node);
}

Node* SprConcept_SemanticCheck(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* baseConcept = node->children[0];
    Node* ifClause = node->children[1];
    Node*& instantiationsSet = node->children[2];
    const string& paramName = getCheckPropertyString(node, "spr.paramName");

    // Compile the base concept node; make sure it's ct
    if ( baseConcept )
    {
        Nest::semanticCheck(baseConcept);
        if ( !isCt(baseConcept) )
            REP_ERROR(baseConcept->location, "Base concept type needs to be compile-time (type=%1%)") % baseConcept->type;
    }

    Node* param = baseConcept
        ? mkSprParameter(node->location, paramName, baseConcept)
        : mkSprParameter(node->location, paramName, getConceptType());
    Nest::setContext(param, node->childrenContext);
    Nest::computeType(param);       // But not semanticCheck, as it will complain of instantiating a var of type auto

    delete instantiationsSet;
    instantiationsSet = mkInstantiationsSet(node, { (Node*) param }, ifClause);
    return Feather::mkNop(node->location);
}

Node* Generic_SemanticCheck(Node* node)
{
    return mkNop(node->location);
}

void Using_SetContextForChildren(Node* node)
{
    if ( Feather::hasName(node) )
        Feather::addToSymTab(node);

    defaultFunSetContextForChildren(node);
}
TypeRef Using_ComputeType(Node* node)
{
    ASSERT(node->children.size() == 1);
    Node* usingNode = node->children[0];
    const string* alias = getPropertyString(node, "name");

    // Compile the using name
    Nest::semanticCheck(usingNode);
    
    if ( !alias || alias->empty() )
    {
        // Make sure that this node refers to one or more declaration
        Node* baseExp;
        NodeVector decls = getDeclsFromNode(usingNode, baseExp);
        if ( decls.empty() )
            REP_ERROR(usingNode->location, "Invalid using name - no declarations can be found");

        // Add references in the current symbol tab
        for ( Node* decl: decls )
        {
            node->context->currentSymTab()->enter(Feather::getName(decl), decl);
        }
    }
    else
    {
        // We added this node to the current sym tab, as we set shouldAddToSymTab_ to true
    }

    node->explanation = Feather::mkNop(node->location);
    Nest::setContext(node->explanation, node->context);
    Nest::semanticCheck(node->explanation);
    return node->explanation->type;
}
Node* Using_SemanticCheck(Node* node)
{
    Nest::computeType(node);
    return node->explanation;
}
