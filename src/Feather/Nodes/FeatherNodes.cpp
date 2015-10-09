#include <StdInc.h>
#include "FeatherNodes.h"
#include "FeatherNodeCommonsCpp.h"

#include <Feather/FeatherTypes.h>

#include <Util/Decl.h>
#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Context.h>
#include <Util/StringData.h>

#include <Nest/Common/Diagnostic.hpp>
#include <Nest/Common/Alloc.h>
#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/NodeKindRegistrar.h>
#include <Nest/Intermediate/Modifier.h>


using namespace Feather;

#define REQUIRE_NODE(loc, node) \
    if ( node ) ; else \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % ( #node )
#define REQUIRE_TYPE(loc, type) \
    if ( type ) ; else \
        REP_INTERNAL((loc), "Expected type (%1%)") % ( #type )

// namespace
// {

    void CtProcessMod_afterSemanticCheck(Modifier* mod, Node* node)
    {
        Nest_ctProcess(node);
    }
    Modifier ctProcessMod = { modTypeAfterSemanticCheck, &CtProcessMod_afterSemanticCheck };

    const char* propResultVoid = "nodeList.resultVoid";

    bool isField(Node* node)
    {
        if ( node->nodeKind != nkFeatherDeclVar )
            return false;
        
        // Check the direct parent is a class that contains the given node
        SymTab* st = node->context->currentSymTab;
        Node* parent = st ? st->node : nullptr;
        parent = parent ? Nest_explanation(parent) : nullptr;
        if ( parent && parent->nodeKind == nkFeatherDeclClass )
        {
            for ( Node* f: parent->children )
                if ( f == node )
                    return true;
        }
        return false;
    }


    Node* Nop_SemanticCheck(Node* node)
    {
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

    Node* TypeNode_SemanticCheck(Node* node)
    {
        node->type = Nest_getCheckPropertyType(node, "givenType");
        return node;
    }
    const char* TypeNode_toString(const Node* node)
    {
        ostringstream os;
        os << "type(" << Nest_getCheckPropertyType(node, "givenType") << ")";
        return dupString(os.str().c_str());
    }

    Node* BackendCode_SemanticCheck(Node* node)
    {
        EvalMode mode = BackendCode_getEvalMode(node);
        if ( !node->type )
            node->type = getVoidType(mode);

        if ( mode != modeRt )
        {
            // CT process this node right after semantic check
            Nest_addModifier(node, &ctProcessMod);
        }
        return node;
    }
    const char* BackendCode_toString(const Node* node)
    {
        ostringstream os;
        os << "backendCode(" << BackendCode_getCode(node).begin << ")";
        return dupString(os.str().c_str());
    }

    TypeRef NodeList_ComputeType(Node* node)
    {
        // Compute the type for all the children
        for ( Node* c: node->children )
        {
            if ( c && !Nest_computeType(c) )
                return NULL;
        }

        // Get the type of the last node
        unsigned numChildren = Nest_nodeArraySize(node->children);
        TypeRef res = ( Nest_hasProperty(node, propResultVoid) || numChildren==0 || !at(node->children, numChildren-1)->type )
            ? getVoidType(node->context->evalMode)
            : at(node->children, numChildren-1)->type;
        res = adjustMode(res, node->context, node->location);
        return res;
    }
    Node* NodeList_SemanticCheck(Node* node)
    {
        // Semantic check each of the children
        bool hasNonCtChildren = false;
        for ( Node* c: node->children )
        {
            if ( c )
            {
                if ( !Nest_semanticCheck(c) )
                    return NULL;
                hasNonCtChildren = hasNonCtChildren || !isCt(c);
            }
        }

        // Make sure the type is computed
        if ( !node->type )
        {
            // Get the type of the last node
            unsigned numChildren = Nest_nodeArraySize(node->children);
            TypeRef t = ( Nest_hasProperty(node, propResultVoid) || numChildren==0 || !at(node->children, numChildren-1)->type )
                ? getVoidType(node->context->evalMode)
                : at(node->children, numChildren-1)->type;
            t = adjustMode(t, node->context, node->location);
            node->type = t;
            checkEvalMode(node);
        }
        return node;
    }

    void LocalSpace_SetContextForChildren(Node* node)
    {
        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);
        Nest_defaultFunSetContextForChildren(node);
    }
    TypeRef LocalSpace_ComputeType(Node* node)
    {
        return getVoidType(node->context->evalMode);
    }
    Node* LocalSpace_SemanticCheck(Node* node)
    {
        // Compute type first
        Nest_computeType(node);

        // Semantic check each of the children
        for ( Node* c: node->children )
        {
            Nest_semanticCheck(c);  // Ignore possible errors
        }
        checkEvalMode(node);
        return node;
    }

    Node* GlobalConstructAction_SemanticCheck(Node* node)
    {
        Node* act = at(node->children, 0);
        if ( !Nest_semanticCheck(act) )
            return nullptr;

        node->type = getVoidType(node->context->evalMode);

        // For CT construct actions, evaluate them asap
        if ( isCt(act) )
        {
            Nest_ctEval(act);
            return mkNop(node->location);
        }
        return node;
    }

    Node* GlobalDestructAction_SemanticCheck(Node* node)
    {
        Node* act = at(node->children, 0);
        if ( !Nest_semanticCheck(act) )
            return nullptr;

        node->type = getVoidType(node->context->evalMode);

        // We never CT evaluate global destruct actions
        if ( isCt(act) )
        {
            return mkNop(node->location);
        }
        return node;
    }

    // Both for ScopeDestructAction and for TempDestructAction
    Node* ScopeTempDestructAction_SemanticCheck(Node* node)
    {
        Node* act = at(node->children, 0);
        if ( !Nest_semanticCheck(act) )
            return nullptr;
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

    void ChangeMode_SetContextForChildren(Node* node)
    {
        EvalMode curMode = (EvalMode) Nest_getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode;
        node->childrenContext = Nest_mkChildContext(node->context, newMode);
        Nest_defaultFunSetContextForChildren(node);
    }
    Node* ChangeMode_SemanticCheck(Node* node)
    {
        Node* exp = at(node->children, 0);

        // Make sure we are allowed to change the mode
        EvalMode baseMode = node->context->evalMode;
        EvalMode curMode = (EvalMode) Nest_getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode;
        if ( newMode == modeUnspecified )
            REP_INTERNAL(node->location, "Cannot change the mode to Unspecified");
        if ( newMode == modeRt && baseMode != modeRt )
            REP_ERROR_RET(nullptr, node->location, "Cannot change mode to RT in a non-RT context (%1%)") % baseMode;
        if ( newMode == modeRtCt && baseMode != modeRtCt )
            REP_ERROR_RET(nullptr, node->location, "Cannot change mode to RTCT in a non-RTCT context (%1%)") % baseMode;

        if ( !exp )
            REP_INTERNAL(node->location, "No node specified as child to a ChangeMode node");

        return Nest_semanticCheck(exp);
    }
    const char* ChangeMode_toString(const Node* node)
    {
        Node* exp = at(node->children, 0);
        ostringstream os;
        EvalMode curMode = (EvalMode) Nest_getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode;
        os << "changeMode(" << exp << ", " << newMode << ")";
        return dupString(os.str().c_str());
    }

    void Function_SetContextForChildren(Node* node)
    {
        // If we don't have a children context, create one
        if ( !node->childrenContext )
            node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, effectiveEvalMode(node));

        Nest_defaultFunSetContextForChildren(node);
        
        addToSymTab(node);
    }
    TypeRef Function_ComputeType(Node* node)
    {
        if ( size(getName(node)) == 0 )
            REP_ERROR_RET(nullptr, node->location, "No name given to function declaration");

        // We must have a result type
        Node* resultType = at(node->children, 0);
        TypeRef resType = Nest_computeType(resultType);
        if ( !resType )
            REP_ERROR_RET(nullptr, node->location, "No result type given to function %1%") % getName(node);

        NodeRange params = all(node->children);

        vector<TypeRef> subTypes;
        subTypes.reserve(Nest_nodeRangeSize(params)-1);
        subTypes.push_back(resType);

        // Get the type for all the parameters
        params.beginPtr += 2;
        for ( Node* param: params )
        {
            if ( !param )
                REP_ERROR_RET(nullptr, node->location, "Invalid parameter");
            if ( !Nest_computeType(param) )
                return nullptr;
            subTypes.push_back(param->type);
        }

        return getFunctionType(&subTypes[0], subTypes.size(), effectiveEvalMode(node));
    }
    Node* Function_SemanticCheck(Node* node)
    {
        // Make sure the type is computed
        if ( !Nest_computeType(node) )
            return nullptr;

        // Semantically check all the parameters
        NodeRange params = all(node->children);
        params.beginPtr += 2;
        for ( Node* param: params )
        {
            if ( !Nest_semanticCheck(param) )
                return nullptr;
        }

        // Semantically check the body, if we have one
        Node* body = at(node->children, 1);
        if ( body )
            Nest_semanticCheck(body);  // Ignore possible errors

        // TODO (function): Check that all the paths return a value
        return node;
    }
    const char* Function_toString(const Node* node)
    {
        ostringstream os;
        os << toString(getName(node));
        if ( node->type )
        {
            os << '(';
            bool hasResultParam = Nest_hasProperty(node, propResultParam);
            size_t startIdx = hasResultParam ? 3 : 2;
            for ( size_t i=startIdx; i<Nest_nodeArraySize(node->children); ++i )
            {
                if ( i > startIdx )
                    os << ", ";
                os << at(node->children, i)->type;
            }
            os << "): " << (hasResultParam ? removeRef(at(node->children, 2)->type) : at(node->children, 0)->type);
        }
        return dupString(os.str().c_str());
    }

    void Class_SetContextForChildren(Node* node)
    {
        // If we don't have a children context, create one
        if ( !node->childrenContext )
            node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, effectiveEvalMode(node));

        Nest_defaultFunSetContextForChildren(node);
        
        addToSymTab(node);
    }
    TypeRef Class_ComputeType(Node* node)
    {
        if ( size(getName(node)) == 0 )
            REP_ERROR_RET(nullptr, node->location, "No name given to class");

        // Compute the type for all the fields
        for ( Node* field: node->children )
        {
            // Ignore errors from children
            Nest_computeType(field);
        }
        return getDataType(node, 0, effectiveEvalMode(node));
    }
    Node* Class_SemanticCheck(Node* node)
    {
        if ( !Nest_computeType(node) )
            return nullptr;

        // Semantically check all the fields
        for ( Node* field: node->children )
        {
            Nest_semanticCheck(field);  // Ignore possible errors
        }
        return node;
    }

    void Var_SetContextForChildren(Node* node)
    {
        Nest_defaultFunSetContextForChildren(node);
        addToSymTab(node);
    }
    TypeRef Var_ComputeType(Node* node)
    {
        // Make sure the variable has a type
        ASSERT(Nest_nodeArraySize(node->children) == 1);
        Node* typeNode = at(node->children, 0);
        if ( !Nest_computeType(typeNode) )
            return nullptr;

        // Adjust the mode of the type
        return adjustMode(typeNode->type, node->context, node->location);
    }
    Node* Var_SemanticCheck(Node* node)
    {
        if ( !Nest_computeType(node) )
            return nullptr;

        // Make sure that the type has storage
        if ( !node->type->hasStorage )
            REP_ERROR_RET(nullptr, node->location, "Variable type has no storage (%1%") % node->type;

        Nest_computeType(classForType(node->type));           // Make sure the type of the class is computed
        return node;
    }

    Node* CtValue_SemanticCheck(Node* node)
    {
        // Check the type
        if ( !node->type )
            node->type = Nest_getCheckPropertyType(node, "valueType");
        if ( !node->type || !node->type->hasStorage || node->type->mode == modeRt )
            REP_ERROR_RET(nullptr, node->location, "Type specified for Ct Value cannot be used at compile-time");
        
        // Make sure data size matches the size reported by the type
        size_t valueSize = Nest_sizeOf(node->type);
        StringRef data = Nest_getCheckPropertyString(node, "valueData");
        if ( valueSize != size(data) )
        {
            REP_ERROR_RET(nullptr, node->location, "Read value size (%1%) differs from declared size of the value (%2%) - type: %3%")
                % size(data) % valueSize % node->type;
        }

        node->type = Feather::changeTypeMode(node->type, modeCt, node->location);
        return node;
    }
    const char* CtValue_toString(const Node* node)
    {
        if ( !node->type )
        {
            return "ctValue";
        }
        ostringstream os;
        os << "ctValue(" << node->type << ": ";

        StringRef valueDataStr = Nest_getCheckPropertyString(node, "valueData");
        const void* valueData = valueDataStr.begin;
        
        const StringRef* nativeName = node->type->hasStorage ? Feather::nativeName(node->type) : nullptr;
        if ( 0 == strcmp(node->type->description, "Type/ct") )
        {
            TypeRef t = *((TypeRef*) valueData);
            os << t;
        }
        else if ( nativeName && node->type->numReferences == 0 )
        {
            if ( *nativeName == "i1" || *nativeName == "u1" )
            {
                bool val = 0 != (*((unsigned char*) valueData));
                os << (val ? "true" : "false");
            }
            else if ( *nativeName == "i16" )
                os << *((const short*) valueData);
            else if (  *nativeName == "u16" )
                os << *((const unsigned short*) valueData);
            else if ( *nativeName == "i32" )
                os << *((const int*) valueData);
            else if (  *nativeName == "u32" )
                os << *((const unsigned int*) valueData);
            else if ( *nativeName == "i64" )
                os << *((const long long*) valueData);
            else if (  *nativeName == "u64" )
                os << *((const unsigned long long*) valueData);
            else if (  *nativeName == "StringRef" )
            {
                const StringData& s = *((const StringData*) valueData);
                os << "'" << s.toStdString() << "'";
            }
            else
                os << "'" << toString(valueDataStr) << "'";
        }
        else
            os << "'" << toString(valueDataStr) << "'";
        os << ")";
        return dupString(os.str().c_str());
    }

    Node* Null_SemanticCheck(Node* node)
    {
        ASSERT(Nest_nodeArraySize(node->children) == 1);
        Node* typeNode = at(node->children, 0);
        TypeRef t = Nest_computeType(typeNode);
        if ( !t )
            return nullptr;

        // Make sure that the type is a reference
        if ( !t->hasStorage )
            REP_ERROR_RET(nullptr, node->location, "Null node should have a type with storage (cur type: %1%") % t;
        if ( t->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Null node should have a reference type (cur type: %1%)") % t;

        node->type = adjustMode(t, node->context, node->location);
        return node;
    }
    const char* Null_toString(const Node* node)
    {
        ostringstream os;
        os << "null(" << node->type << ")";
        return dupString(os.str().c_str());
    }

    Node* VarRef_SemanticCheck(Node* node)
    {
        Node* var = at(node->referredNodes, 0);
        ASSERT(var);
        if ( var->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(node->location, "VarRef object needs to point to a Field (node kind: %1%)") % Nest_nodeKindName(var);
        if ( !Nest_computeType(var) )
            return nullptr;
        if ( isField(var) )
            REP_INTERNAL(node->location, "VarRef used on a field (%1%). Use FieldRef instead") % getName(var);
        if ( !var->type->hasStorage )
            REP_ERROR_RET(nullptr, node->location, "Variable type is doesn't have a storage type (type: %1%)") % var->type;
        node->type = adjustMode(getLValueType(var->type), node->context, node->location);
        checkEvalMode(node, var->type->mode);
        return node;
    }
    const char* VarRef_toString(const Node* node)
    {
        Node* var = at(node->referredNodes, 0);
        ostringstream os;
        os << "varRef(" << toString(getName(var)) << ")";
        return dupString(os.str().c_str());
    }

    Node* FieldRef_SemanticCheck(Node* node)
    {
        Node* obj = at(node->children, 0);
        Node* field = at(node->referredNodes, 0);
        ASSERT(obj);
        ASSERT(field);
        if ( field->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(node->location, "FieldRef object needs to point to a Field (node kind: %1%)") % Nest_nodeKindName(field);

        // Semantic check the object node - make sure it's a reference to a data type
        if ( !Nest_semanticCheck(obj) )
            return nullptr;
        ASSERT(obj->type);
        if ( !obj->type || !obj->type->hasStorage || obj->type->numReferences != 1 )
            REP_ERROR_RET(nullptr, node->location, "Field access should be done on a reference to a data type (type: %1%)") % obj->type;
        Node* cls = classForType(obj->type);
        ASSERT(cls);
        if ( !Nest_computeType(cls) )
            return nullptr;

        // Compute the type of the field
        if ( !Nest_computeType(field) )
            return nullptr;

        // Make sure that the type of a object is a data type that refers to a class the contains the given field
        bool fieldFound = false;
        for ( auto field: cls->children )
        {
            if ( &*field == field )
            {
                fieldFound = true;
                break;
            }
        }
        if ( !fieldFound )
            REP_ERROR_RET(nullptr, node->location, "Field '%1%' not found when accessing object") % getName(field);

        // Set the correct type for this node
        ASSERT(field->type);
        ASSERT(field->type->hasStorage);
        node->type = getLValueType(field->type);
        node->type = adjustMode(node->type, obj->type->mode, node->context, node->location);
        return node;
    }
    const char* FieldRef_toString(const Node* node)
    {
        Node* obj = at(node->children, 0);
        Node* field = at(node->referredNodes, 0);
        ostringstream os;
        os << "fieldRef(" << obj << ", " << field << ")";
        return dupString(os.str().c_str());
    }

    Node* FunRef_SemanticCheck(Node* node)
    {
        ASSERT(Nest_nodeArraySize(node->children) == 1);
        Node* resType = at(node->children, 0);
        Node* fun = at(node->referredNodes, 0);
        if ( !Nest_computeType(resType) )
            return nullptr;

        if ( !Nest_computeType(fun) )
            return nullptr;
        node->type = adjustMode(resType->type, node->context, node->location);
        return node;
    }
    const char* FunRef_toString(const Node* node)
    {
        Node* fun = at(node->referredNodes, 0);
        ostringstream os;
        os << "FunRef(" << fun << ")";
        return dupString(os.str().c_str());
    }

    Node* FunCall_SemanticCheck(Node* node)
    {
        Node* fun = at(node->referredNodes, 0);
        
        // Make sure the function declaration is has a valid type
        if ( !Nest_computeType(fun) )
            return nullptr;

        // Check argument count
        size_t numParameters = Function_numParameters(fun);
        if ( Nest_nodeArraySize(node->children) != numParameters )
            REP_ERROR_RET(nullptr, node->location, "Invalid function call: expecting %1% parameters, given %2%")
                % numParameters % Nest_nodeArraySize(node->children);

        // Semantic check the arguments
        // Also check that their type matches the corresponding type from the function decl
        bool allParamsAreCtAvailable = true;
        for ( size_t i=0; i<Nest_nodeArraySize(node->children); ++i )
        {
            Node* arg = at(node->children, i);

            // Semantically check the argument
            if ( !Nest_semanticCheck(arg) )
                return nullptr;
            if ( !isCt(arg) )
                allParamsAreCtAvailable = false;

            // Compare types
            TypeRef argType = arg->type;
            TypeRef paramType = Function_getParameter(fun, i)->type;
            if ( !isSameTypeIgnoreMode(argType, paramType) )
                REP_ERROR_RET(nullptr, arg->location, "Invalid function call: argument %1% is expected to have type %2% (actual type: %3%)")
                    % (i+1) % paramType % argType;
        }

        // CT availability checks
        EvalMode curMode = node->context->evalMode;
        EvalMode calledFunMode = effectiveEvalMode(fun);
        ASSERT(curMode != modeUnspecified);
        ASSERT(calledFunMode != modeUnspecified);
        if ( calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable )
        {
            REP_ERROR(node->location, "Not all arguments are compile-time, when calling a compile time function");
            REP_INFO(fun->location, "See called function");
            return NULL;
        }
        if ( curMode == modeRtCt && calledFunMode == modeRt )
        {
            REP_ERROR(node->location, "Cannot call RT functions from RTCT contexts");
            REP_INFO(fun->location, "See called function");
            return NULL;
        }
        if ( curMode == modeCt && calledFunMode == modeRt )
        {
            REP_ERROR(node->location, "Cannot call a RT function from a CT context");
            REP_INFO(fun->location, "See called function");
            return NULL;
        }

        // Get the type from the function decl
        node->type = Function_resultType(fun);

        // Handle autoCt case
        if ( allParamsAreCtAvailable && node->type->mode == modeRtCt && Nest_hasProperty(fun, propAutoCt) )
        {
            node->type = changeTypeMode(node->type, modeCt, node->location);
        }

        // Make sure we yield a type with the right mode
        node->type = adjustMode(node->type, node->context, node->location);

        checkEvalMode(node, calledFunMode);
        return node;
    }
    const char* FunCall_toString(const Node* node)
    {
        ostringstream os;
        os << "funCall-" << toString(getName(at(node->referredNodes, 0))) << "(";
        for ( size_t i=0; i<Nest_nodeArraySize(node->children); ++i )
        {
            if ( i != 0 )
                os << ", ";
            os << at(node->children, i);
        }
        os << ")";
        return dupString(os.str().c_str());
    }

    Node* MemLoad_SemanticCheck(Node* node)
    {
        Node* exp = at(node->children, 0);

        // Semantic check the argument
        if ( !Nest_semanticCheck(exp) )
            return nullptr;

        // Check if the type of the argument is a ref
        if ( !exp->type->hasStorage || exp->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "Cannot load from a non-reference (%1%, type: %2%)") % exp % exp->type;

        // Check flags
        AtomicOrdering ordering = (AtomicOrdering) Nest_getCheckPropertyInt(node, "atomicOrdering");
        if ( ordering == atomicRelease )
            REP_ERROR_RET(nullptr, node->location, "Cannot use atomic release with a load instruction");
        if ( ordering == atomicAcquireRelease )
            REP_ERROR_RET(nullptr, node->location, "Cannot use atomic acquire-release with a load instruction");

        // Remove the 'ref' from the type and get the base type
        node->type = removeRef(exp->type);
        node->type = adjustMode(node->type, node->context, node->location);
        return node;
    }

    Node* MemStore_SemanticCheck(Node* node)
    {
        Node* value = at(node->children, 0);
        Node* address = at(node->children, 1);
        ASSERT(value);
        ASSERT(address);

        // Semantic check the arguments
        if ( !Nest_semanticCheck(value) || !Nest_semanticCheck(address) )
            return nullptr;

        // Check if the type of the address is a ref
        if ( !address->type->hasStorage || address->type->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "The address of a memory store is not a reference, nor VarRef nor FieldRef (type: %1%)") % address->type;
        TypeRef baseAddressType = removeRef(address->type);

        // Check the equivalence of types
        if ( !isSameTypeIgnoreMode(value->type, baseAddressType) )
        {
            // Try again, getting rid of l-values
            TypeRef t1 = lvalueToRefIfPresent(value->type);
            if ( !isSameTypeIgnoreMode(t1, baseAddressType) )
                REP_ERROR_RET(nullptr, node->location, "The type of the value doesn't match the type of the address in a memory store (%1% != %2%)")
                    % value->type % baseAddressType;
        }

        // The resulting type is Void
        node->type = getVoidType(address->type->mode);
        return node;
    }

    Node* Bitcast_SemanticCheck(Node* node)
    {
        Node* exp = at(node->children, 0);
        Node* resType = at(node->children, 1);
        if ( !Nest_semanticCheck(exp) )
            return nullptr;            
        TypeRef tDest = Nest_computeType(resType);
        if ( !tDest )
            return nullptr;

        // Make sure both types have storage
        TypeRef srcType = exp->type;
        if ( !srcType->hasStorage )
            REP_ERROR_RET(nullptr, node->location, "The source of a bitcast is not a type with storage (%1%)") % srcType;
        if ( !tDest->hasStorage )
            REP_ERROR_RET(nullptr, node->location, "The destination type of a bitcast is not a type with storage (%1%)") % tDest;
        
        // Make sure both types are references
        if ( srcType->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "The source of a bitcast is not a reference (%1%)") % srcType;
        if ( tDest->numReferences == 0 )
            REP_ERROR_RET(nullptr, node->location, "The destination type of a bitcast is not a reference (%1%)") % tDest;

        node->type = adjustMode(tDest, node->context, node->location);
        return node;
    }
    const char* Bitcast_toString(const Node* node)
    {
        Node* exp = at(node->children, 0);
        Node* resType = at(node->children, 1);
        ostringstream os;
        if ( resType->type )
            os << "bitcast(" << resType->type << ", " << exp << ")";
        else
            os << "bitcast(type(" << resType << "), " << exp << ")";
        return dupString(os.str().c_str());
    }

    Node* Conditional_SemanticCheck(Node* node)
    {
        Node*& cond = at(node->children, 0);
        Node* alt1 = at(node->children, 1);
        Node* alt2 = at(node->children, 2);

        // Semantic check the condition
        if ( !Nest_semanticCheck(cond) )
            return nullptr;            

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(cond) )
            REP_ERROR_RET(nullptr, cond->location, "The condition of the conditional expression is not Testable");

        // Dereference the condition as much as possible
        while ( cond->type && cond->type->numReferences > 0 )
        {
            cond = mkMemLoad(cond->location, cond);
            Nest_setContext(cond, Nest_childrenContext(node));
            if ( !Nest_semanticCheck(cond) )
                return nullptr;
        }
        // TODO (conditional): This shouldn't be performed here

        // Semantic check the alternatives
        if ( !Nest_semanticCheck(alt1) || !Nest_semanticCheck(alt2) )
            return nullptr;            

        // Make sure the types of the alternatives are equal
        if ( !isSameTypeIgnoreMode(alt1->type, alt2->type) )
            REP_ERROR_RET(nullptr, node->location, "The types of the alternatives of a conditional must be equal (%1% != %2%)") % alt1->type % alt2->type;

        node->type = adjustMode(alt1->type, cond->type->mode, node->context, node->location);
        return node;
    }

    void If_SetContextForChildren(Node* node)
    {
        Node* condition = at(node->children, 0);
        Node* thenClause = at(node->children, 1);
        Node* elseClause = at(node->children, 2);

        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);

        Nest_setContext(condition, node->childrenContext);
        if ( thenClause )
            Nest_setContext(thenClause, node->childrenContext);
        if ( elseClause )
            Nest_setContext(elseClause, node->childrenContext);
    }
    Node* If_SemanticCheck(Node* node)
    {
        Node* condition = at(node->children, 0);
        Node* thenClause = at(node->children, 1);
        Node* elseClause = at(node->children, 2);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode);

        // Semantic check the condition
        if ( !Nest_semanticCheck(condition) )
            return nullptr;

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(condition) )
            REP_ERROR_RET(nullptr, condition->location, "The condition of the if is not Testable");

        // Dereference the condition as much as possible
        while ( condition->type && condition->type->numReferences > 0 )
        {
            condition = mkMemLoad(condition->location, condition);
            Nest_setContext(condition, Nest_childrenContext(node));
            if ( !Nest_semanticCheck(condition) )
                return nullptr;
        }
        at(node->children, 0) = condition;
        // TODO (if): Remove this dereference from here

        if ( nodeEvalMode(node) == modeCt )
        {
            if ( !isCt(condition) )
                REP_ERROR_RET(nullptr, condition->location, "The condition of the ct if should be available at compile-time (%1%)") % condition->type;

            // Get the CT value from the condition, and select an active branch
            Node* c = Nest_ctEval(condition);
            Node* selectedBranch = getBoolCtValue(c) ? thenClause : elseClause;

            // Expand only the selected branch
            if ( selectedBranch )
                return selectedBranch;
            else
                return mkNop(node->location);
        }

        // Semantic check the clauses
        if ( thenClause && !Nest_semanticCheck(thenClause) )
            return nullptr;            
        if ( elseClause && !Nest_semanticCheck(elseClause) )
            return nullptr;            
        return node;
    }

    void While_SetContextForChildren(Node* node)
    {
        Node* condition = at(node->children, 0);
        Node* step = at(node->children, 1);
        Node* body = at(node->children, 2);

        node->childrenContext = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);
        CompilationContext* condContext = nodeEvalMode(node) == modeCt ? Nest_mkChildContext(node->context, modeCt) : node->childrenContext;

        Nest_setContext(condition, condContext); // condition
        if ( step )
            Nest_setContext(step, condContext); // step
        if ( body )
            Nest_setContext(body, node->childrenContext); // body
        // Nest_defaultFunSetContextForChildren(node);
    }
    Node* While_SemanticCheck(Node* node)
    {
        Node* condition = at(node->children, 0);
        Node* step = at(node->children, 1);
        Node* body = at(node->children, 2);
        
        // Semantic check the condition
        if ( !Nest_semanticCheck(condition) )
            return nullptr;            
        if ( step && !Nest_computeType(step) )
            return nullptr;

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(condition) )
            REP_ERROR_RET(nullptr, condition->location, "The condition of the while is not Testable");

        if ( nodeEvalMode(node) == modeCt )
        {
            if ( !isCt(condition) )
                REP_ERROR_RET(nullptr, condition->location, "The condition of the ct while should be available at compile-time (%1%)") % condition->type;
            if ( step && !isCt(step) )
                REP_ERROR_RET(nullptr, step->location, "The step of the ct while should be available at compile-time (%1%)") % step->type;

            // Create a node-list that will be our explanation
            NodeArray result = Nest_allocNodeArray(0);

            // Do the while
            while ( true )
            {
                // CT-evaluate the condition; if the condition evaluates to false, exit the while
                if ( !getBoolCtValue(Nest_ctEval(condition)) )
                    break;

                // Put (a copy of) the body in the resulting node-list
                if ( body )
                {
                    Node* curBody = Nest_cloneNode(body);
                    Nest_setContext(curBody, node->context);
                    if ( !Nest_semanticCheck(curBody) )
                        return nullptr;
                    Nest_appendNodeToArray(&result, curBody);
                }

                // If we have a step, make sure to evaluate it
                if ( step )
                {
                    Nest_ctEval(step);    // We don't need the actual result
                }

                // Unfortunately, we don't treat 'break' and 'continue' instructions inside the ct while instructions
            }
            Nest_appendNodeToArray(&result, mkNop(node->location)); // Make sure our resulting type is Void

            // Set the explanation and exit
            Node* res = mkNodeList(node->location, all(result));
            Nest_freeNodeArray(result);
            return res;
        }

        // Semantic check the body and the step
        if ( body && !Nest_semanticCheck(body) )
            return nullptr;            
        if ( step && !Nest_semanticCheck(step) )
            return nullptr;            

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

    Node* Break_SemanticCheck(Node* node)
    {
        // Get the outer-most loop from the context
        Node* loop = getParentLoop(node->context);
        if ( !loop )
            REP_ERROR_RET(nullptr, node->location, "Break found outside any loop");
        Nest_setProperty(node, "loop", loop);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

    Node* Continue_SemanticCheck(Node* node)
    {
        // Get the outer-most loop from the context
        Node* loop = getParentLoop(node->context);
        if ( !loop )
            REP_ERROR_RET(nullptr, node->location, "Continue found outside any loop");
        Nest_setProperty(node, "loop", loop);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

    Node* Return_SemanticCheck(Node* node)
    {
        Node* exp = at(node->children, 0);

        // If we have an expression argument, semantically check it
        if ( exp && !Nest_semanticCheck(exp) )
            return nullptr;            

        // Get the parent function of this return
        Node* parentFun = getParentFun(node->context);
        if ( !parentFun )
            REP_ERROR_RET(nullptr, node->location, "Return found outside any function");
        TypeRef resultType = Function_resultType(parentFun);
        ASSERT(resultType);
        Nest_setProperty(node, "parentFun", parentFun);

        // If the return has an expression, check that has the same type as the function result type
        if ( exp )
        {
            if ( !isSameTypeIgnoreMode(exp->type, resultType) )
                REP_ERROR_RET(nullptr, node->location, "Returned expression's type is not the same as function's return type");
        }
        else
        {
            // Make sure that the function has a void return type
            if ( resultType->typeKind != typeKindVoid )
                REP_ERROR_RET(nullptr, node->location, "You must return something in a function that has non-Void result type");
        }

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode);
        return node;
    }

// }

int Feather::firstFeatherNodeKind = 0;

int Feather::nkFeatherNop = 0;
int Feather::nkFeatherTypeNode = 0;
int Feather::nkFeatherBackendCode = 0;
int Feather::nkFeatherNodeList = 0;
int Feather::nkFeatherLocalSpace = 0;
int Feather::nkFeatherGlobalConstructAction = 0;
int Feather::nkFeatherGlobalDestructAction = 0;
int Feather::nkFeatherScopeDestructAction = 0;
int Feather::nkFeatherTempDestructAction = 0;
int Feather::nkFeatherChangeMode = 0;

int Feather::nkFeatherDeclFunction = 0;
int Feather::nkFeatherDeclClass = 0;
int Feather::nkFeatherDeclVar = 0;

int Feather::nkFeatherExpCtValue = 0;
int Feather::nkFeatherExpNull = 0;
int Feather::nkFeatherExpVarRef = 0;
int Feather::nkFeatherExpFieldRef = 0;
int Feather::nkFeatherExpFunRef = 0;
int Feather::nkFeatherExpFunCall = 0;
int Feather::nkFeatherExpMemLoad = 0;
int Feather::nkFeatherExpMemStore = 0;
int Feather::nkFeatherExpBitcast = 0;
int Feather::nkFeatherExpConditional = 0;

int Feather::nkFeatherStmtIf = 0;
int Feather::nkFeatherStmtWhile = 0;
int Feather::nkFeatherStmtBreak = 0;
int Feather::nkFeatherStmtContinue = 0;
int Feather::nkFeatherStmtReturn = 0;

void Feather::initFeatherNodeKinds()
{
    nkFeatherNop = Nest_registerNodeKind("nop", &Nop_SemanticCheck);
    nkFeatherTypeNode = Nest_registerNodeKind("typeNode", &TypeNode_SemanticCheck, NULL, NULL, &TypeNode_toString);
    nkFeatherBackendCode = Nest_registerNodeKind("backendCode", &BackendCode_SemanticCheck, NULL, NULL, &BackendCode_toString);
    nkFeatherNodeList = Nest_registerNodeKind("nodeList", &NodeList_SemanticCheck, &NodeList_ComputeType, NULL, NULL);
    nkFeatherLocalSpace = Nest_registerNodeKind("localSpace", &LocalSpace_SemanticCheck, &LocalSpace_ComputeType, &LocalSpace_SetContextForChildren, NULL);
    nkFeatherGlobalConstructAction = Nest_registerNodeKind("globalConstructAction", &GlobalConstructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherGlobalDestructAction = Nest_registerNodeKind("globalDestructAction", &GlobalDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherScopeDestructAction = Nest_registerNodeKind("scopelDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherTempDestructAction = Nest_registerNodeKind("tempDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherChangeMode = Nest_registerNodeKind("changeMode", &ChangeMode_SemanticCheck, NULL, &ChangeMode_SetContextForChildren, &ChangeMode_toString);

    nkFeatherDeclFunction = Nest_registerNodeKind("fun", &Function_SemanticCheck, &Function_ComputeType, &Function_SetContextForChildren, &Function_toString);
    nkFeatherDeclClass = Nest_registerNodeKind("class", &Class_SemanticCheck, &Class_ComputeType, &Class_SetContextForChildren, NULL);
    nkFeatherDeclVar = Nest_registerNodeKind("var", &Var_SemanticCheck, &Var_ComputeType, &Var_SetContextForChildren, NULL);

    nkFeatherExpCtValue = Nest_registerNodeKind("ctValue", &CtValue_SemanticCheck, NULL, NULL, &CtValue_toString);
    nkFeatherExpNull = Nest_registerNodeKind("null", &Null_SemanticCheck, NULL, NULL, &Null_toString);
    nkFeatherExpVarRef = Nest_registerNodeKind("varRef", &VarRef_SemanticCheck, NULL, NULL, &VarRef_toString);
    nkFeatherExpFieldRef = Nest_registerNodeKind("fieldRef", &FieldRef_SemanticCheck, NULL, NULL, &FieldRef_toString);
    nkFeatherExpFunRef = Nest_registerNodeKind("funRef", &FunRef_SemanticCheck, NULL, NULL, &FunRef_toString);
    nkFeatherExpFunCall = Nest_registerNodeKind("funCall", &FunCall_SemanticCheck, NULL, NULL, &FunCall_toString);
    nkFeatherExpMemLoad = Nest_registerNodeKind("memLoad", &MemLoad_SemanticCheck, NULL, NULL, NULL);
    nkFeatherExpMemStore = Nest_registerNodeKind("memStore", &MemStore_SemanticCheck, NULL, NULL, NULL);
    nkFeatherExpBitcast = Nest_registerNodeKind("bitcast", &Bitcast_SemanticCheck, NULL, NULL, &Bitcast_toString);
    nkFeatherExpConditional = Nest_registerNodeKind("conditional", &Conditional_SemanticCheck, NULL, NULL, NULL);

    nkFeatherStmtIf = Nest_registerNodeKind("if", &If_SemanticCheck, NULL, &If_SetContextForChildren, NULL);
    nkFeatherStmtWhile = Nest_registerNodeKind("while", &While_SemanticCheck, NULL, &While_SetContextForChildren, NULL);
    nkFeatherStmtBreak = Nest_registerNodeKind("break", &Break_SemanticCheck, NULL, NULL, NULL);
    nkFeatherStmtContinue = Nest_registerNodeKind("continue", &Continue_SemanticCheck, NULL, NULL, NULL);
    nkFeatherStmtReturn = Nest_registerNodeKind("return", &Return_SemanticCheck, NULL, NULL, NULL);

    firstFeatherNodeKind = nkFeatherNop;
}


Node* Feather::mkNodeList(const Location& loc, NodeRange children, bool voidResult)
{
    Node* res = Nest_createNode(nkFeatherNodeList);
    res->location = loc;
    if ( voidResult )
        Nest_setProperty(res, propResultVoid, 1);
    Nest_nodeSetChildren(res, children);
    return res;
}
Node* Feather::addToNodeList(Node* prevList, Node* element)
{
    ASSERT(!prevList || prevList->nodeKind == nkFeatherNodeList);
    Node* res = prevList;
    if ( !res )
    {
        res = Nest_createNode(nkFeatherNodeList);
        if ( element )
            res->location = element->location;
        Nest_setProperty(res, propResultVoid, 1);    // voidResult == true
    }
    
    Nest_nodeAddChild(res, element);
    return res;
}
Node* Feather::appendNodeList(Node* list, Node* newNodes)
{
    if ( !list )
        return newNodes;
    if ( !newNodes )
        return list;
    
    ASSERT(list->nodeKind == nkFeatherNodeList);
    ASSERT(newNodes->nodeKind == nkFeatherNodeList);
    Node* res = list;
    Nest_nodeAddChildren(res, Nest_nodeChildren(newNodes));
    return res;
}


Node* Feather::mkNop(const Location& loc)
{
    Node* res = Nest_createNode(nkFeatherNop);
    res->location = loc;
    return res;
}
Node* Feather::mkTypeNode(const Location& loc, TypeRef type)
{
    Node* res = Nest_createNode(nkFeatherTypeNode);
    res->location = loc;
    Nest_setProperty(res, "givenType", type);
    return res;
}
Node* Feather::mkBackendCode(const Location& loc, StringRef code, EvalMode evalMode)
{
    Node* res = Nest_createNode(nkFeatherBackendCode);
    res->location = loc;
    Nest_setProperty(res, propCode, code);
    Nest_setProperty(res, propEvalMode, (int) evalMode);
    return res;
}
Node* Feather::mkLocalSpace(const Location& loc, NodeRange children)
{
    Node* res = Nest_createNode(nkFeatherLocalSpace);
    res->location = loc;
    Nest_nodeSetChildren(res, children);
    return res;
}
Node* Feather::mkGlobalConstructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = Nest_createNode(nkFeatherGlobalConstructAction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ action }));
    return res;
}
Node* Feather::mkGlobalDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = Nest_createNode(nkFeatherGlobalDestructAction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ action }));
    return res;
}
Node* Feather::mkScopeDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = Nest_createNode(nkFeatherScopeDestructAction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ action }));
    return res;
}
Node* Feather::mkTempDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = Nest_createNode(nkFeatherTempDestructAction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ action }));
    return res;
}


Node* Feather::mkFunction(const Location& loc, StringRef name, Node* resType, NodeRange params, Node* body, CallConvention callConv, EvalMode evalMode)
{
    Node* res = Nest_createNode(nkFeatherDeclFunction);
    res->location = loc;
    Nest_nodeAddChild(res, resType);
    Nest_nodeAddChild(res, body);
    Nest_nodeAddChildren(res, params);
    setName(res, move(name));
    Nest_setProperty(res, "callConvention", (int) callConv);
    setEvalMode(res, evalMode);

    // Make sure all the nodes given as parameters have the right kind
    for ( Node* param: params )
    {
        if ( Nest_explanation(param)->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(param->location, "Node %1% must be a parameter") % param;
    }

    return res;
}
Node* Feather::mkClass(const Location& loc, StringRef name, NodeRange fields, EvalMode evalMode)
{
    Node* res = Nest_createNode(nkFeatherDeclClass);
    res->location = loc;
    Nest_nodeSetChildren(res, fields);
    setName(res, move(name));
    setEvalMode(res, evalMode);

    // Make sure all the nodes given as parameters have the right kind
    for ( Node* field: fields )
    {
        if ( field->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(field->location, "Node %1% must be a field") % field;
    }

    return res;
}
Node* Feather::mkVar(const Location& loc, StringRef name, Node* typeNode, size_t alignment, EvalMode evalMode)
{
    REQUIRE_TYPE(loc, typeNode);
    Node* res = Nest_createNode(nkFeatherDeclVar);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ typeNode }));
    setName(res, move(name));
    Nest_setProperty(res, "alignment", alignment);
    setEvalMode(res, evalMode);
    return res;
}


Node* Feather::mkCtValue(const Location& loc, TypeRef type, StringRef data)
{
    REQUIRE_TYPE(loc, type);
    Node* res = Nest_createNode(nkFeatherExpCtValue);
    res->location = loc;
    Nest_setProperty(res, "valueType", type);
    Nest_setProperty(res, "valueData", move(data));
    return res;
}
Node* Feather::mkNull(const Location& loc, Node* typeNode)
{
    REQUIRE_NODE(loc, typeNode);
    Node* res = Nest_createNode(nkFeatherExpNull);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ typeNode }));
    return res;
}
Node* Feather::mkVarRef(const Location& loc, Node* varDecl)
{
    REQUIRE_NODE(loc, varDecl);
    if ( varDecl->nodeKind != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A VarRef node must be applied on a variable declaration (%1% given)") % Nest_nodeKindName(varDecl);
    Node* res = Nest_createNode(nkFeatherExpVarRef);
    res->location = loc;
    Nest_nodeSetReferredNodes(res, fromIniList({ varDecl }));
    return res;
}
Node* Feather::mkFieldRef(const Location& loc, Node* obj, Node* fieldDecl)
{
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    if ( fieldDecl->nodeKind != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A FieldRef node must be applied on a field declaration (%1% given)") % Nest_nodeKindName(fieldDecl);
    Node* res = Nest_createNode(nkFeatherExpFieldRef);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ obj }));
    Nest_nodeSetReferredNodes(res, fromIniList({ fieldDecl }));
    return res;
}
Node* Feather::mkFunRef(const Location& loc, Node* funDecl, Node* resType)
{
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    if ( funDecl->nodeKind != nkFeatherDeclFunction )
        REP_INTERNAL(loc, "A FunRef node must be applied on a function declaration (%1% given)") % Nest_nodeKindName(funDecl);
    Node* res = Nest_createNode(nkFeatherExpFunRef);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ resType }));
    Nest_nodeSetReferredNodes(res, fromIniList({ funDecl }));
    return res;
}
Node* Feather::mkFunCall(const Location& loc, Node* funDecl, NodeRange args)
{
    REQUIRE_NODE(loc, funDecl);
    if ( funDecl->nodeKind != nkFeatherDeclFunction )
        REP_INTERNAL(loc, "A FunCall node must be applied on a function declaration (%1% given)") % Nest_nodeKindName(funDecl);
    Node* res = Nest_createNode(nkFeatherExpFunCall);
    res->location = loc;
    Nest_nodeSetChildren(res, args);
    Nest_nodeSetReferredNodes(res, fromIniList({ funDecl }));
    return res;
}
Node* Feather::mkMemLoad(const Location& loc, Node* exp, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, exp);
    Node* res = Nest_createNode(nkFeatherExpMemLoad);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ exp }));
    Nest_setProperty(res, "alignment", alignment);
    Nest_setProperty(res, "volatile", isVolatile ? 1 : 0);
    Nest_setProperty(res, "atomicOrdering", (int) ordering);
    Nest_setProperty(res, "singleThreaded", singleThreaded ? 1 : 0);
    return res;
}
Node* Feather::mkMemStore(const Location& loc, Node* value, Node* address, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    Node* res = Nest_createNode(nkFeatherExpMemStore);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ value, address }));
    Nest_setProperty(res, "alignment", alignment);
    Nest_setProperty(res, "volatile", isVolatile ? 1 : 0);
    Nest_setProperty(res, "atomicOrdering", (int) ordering);
    Nest_setProperty(res, "singleThreaded", singleThreaded ? 1 : 0);
    return res;
}
Node* Feather::mkBitcast(const Location& loc, Node* destType, Node* exp)
{
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    Node* res = Nest_createNode(nkFeatherExpBitcast);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ exp, destType }));
    return res;
}
Node* Feather::mkConditional(const Location& loc, Node* condition, Node* alt1, Node* alt2)
{
    REQUIRE_NODE(loc, condition);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    Node* res = Nest_createNode(nkFeatherExpConditional);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ condition, alt1, alt2 }));
    return res;
}
Node* Feather::mkChangeMode(const Location& loc, Node* child, EvalMode mode)
{
    Node* res = Nest_createNode(nkFeatherChangeMode);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ child }));
    Nest_setProperty(res, propEvalMode, (int) mode);
    return res;
}


Node* Feather::mkIf(const Location& loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    Node* res = Nest_createNode(nkFeatherStmtIf);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ condition, thenClause, elseClause }));
    if ( isCt )
        setEvalMode(res, modeCt);
    return res;
}
Node* Feather::mkWhile(const Location& loc, Node* condition, Node* body, Node* step, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    Node* res = Nest_createNode(nkFeatherStmtWhile);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ condition, step, body }));
    if ( isCt )
        setEvalMode(res, modeCt);
    return res;
}
Node* Feather::mkBreak(const Location& loc)
{
    Node* res = Nest_createNode(nkFeatherStmtBreak);
    res->location = loc;
    Nest_setProperty(res, "loop", (Node*) nullptr);
    return res;
}
Node* Feather::mkContinue(const Location& loc)
{
    Node* res = Nest_createNode(nkFeatherStmtContinue);
    res->location = loc;
    Nest_setProperty(res, "loop", (Node*) nullptr);
    return res;
}
Node* Feather::mkReturn(const Location& loc, Node* exp)
{
    Node* res = Nest_createNode(nkFeatherStmtReturn);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ exp }));
    Nest_setProperty(res, "parentFun", (Node*) nullptr);
    return res;
}


StringRef Feather::BackendCode_getCode(const Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    return Nest_getCheckPropertyString(node, propCode);
}
EvalMode Feather::BackendCode_getEvalMode(Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    EvalMode curMode = (EvalMode) Nest_getCheckPropertyInt(node, propEvalMode);
    return curMode != modeUnspecified ? curMode : node->context->evalMode;
}

void Feather::ChangeMode_setChild(Node* node, Node* child)
{
    ASSERT(node);
    Nest_nodeSetChildren(node, fromIniList({ child }));

    if ( node->childrenContext )
        Nest_setContext(child, node->childrenContext);
}
EvalMode Feather::ChangeMode_getEvalMode(Node* node)
{
    EvalMode curMode = (EvalMode) Nest_getCheckPropertyInt(node, propEvalMode);
    return curMode != modeUnspecified ? curMode : node->context->evalMode;
}

void Feather::Function_addParameter(Node* node, Node* parameter, bool first)
{
    if ( Nest_explanation(parameter)->nodeKind != nkFeatherDeclVar )
        REP_INTERNAL(parameter->location, "Node %1% must be a parameter") % parameter;

    if ( first )
        Nest_insertNodeIntoArray(&node->children, 2, parameter);
    else
        Nest_appendNodeToArray(&node->children, parameter);
}
void Feather::Function_setResultType(Node* node, Node* resultType)
{
    at(node->children, 0) = resultType;
    Nest_setContext(resultType, node->childrenContext);
}
void Feather::Function_setBody(Node* node, Node* body)
{
    at(node->children, 1) = body;
}
size_t Feather::Function_numParameters(Node* node)
{
    return Nest_nodeArraySize(node->children)-2;
}
Node* Feather::Function_getParameter(Node* node, size_t idx)
{
    return at(node->children, idx+2);
}
TypeRef Feather::Function_resultType(Node* node)
{
    return at(node->children, 0)->type;
}
Node* Feather::Function_body(Node* node)
{
    return at(node->children, 1);
}
CallConvention Feather::Function_callConvention(Node* node)
{
    return (CallConvention) Nest_getCheckPropertyInt(node, "callConvention");
}
