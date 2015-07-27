#include <StdInc.h>
#include "FeatherNodes.h"
#include "Nop.h"
#include "TypeNode.h"
#include "BackendCode.h"
#include "NodeList.h"
#include "LocalSpace.h"
#include "GlobalConstructAction.h"
#include "GlobalDestructAction.h"
#include "ScopeDestructAction.h"
#include "TempDestructAction.h"
#include "FeatherNodeCommonsCpp.h"

#include "Decls/Function.h"
#include "Decls/Class.h"
#include "Decls/Var.h"

#include "Exp/CtValue.h"
#include "Exp/Null.h"
#include "Exp/StackAlloc.h"
#include "Exp/VarRef.h"
#include "Exp/FieldRef.h"
#include "Exp/FunRef.h"
#include "Exp/FunCall.h"
#include "Exp/MemLoad.h"
#include "Exp/MemStore.h"
#include "Exp/Bitcast.h"
#include "Exp/Conditional.h"
#include "ChangeMode.h"

#include "Stmt/If.h"
#include "Stmt/While.h"
#include "Stmt/Break.h"
#include "Stmt/Continue.h"
#include "Stmt/Return.h"

#include <Feather/FeatherTypes.h>

#include <Util/Decl.h>
#include <Util/TypeTraits.h>
#include <Util/Ct.h>
#include <Util/Context.h>
#include <Util/StringData.h>

#include <Nest/Common/Diagnostic.h>
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
    class CtProcessMod : public Nest::Modifier
    {
    public:
        virtual void afterSemanticCheck(Node* node)
        {
            theCompiler().ctProcess(node);
        };
    };

    const char* propResultVoid = "nodeList.resultVoid";

    bool isField(Node* node)
    {
        if ( node->nodeKind != nkFeatherDeclVar )
            return false;
        
        // Check the direct parent is a class that contains the given node
        Nest::SymTab* st = node->context->currentSymTab();
        Node* parent = st ? st->node() : nullptr;
        parent = parent ? explanation(parent) : nullptr;
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
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    Node* TypeNode_SemanticCheck(Node* node)
    {
        node->type = Nest::getCheckPropertyType(node, "givenType");
        return node;
    }
    const char* TypeNode_toString(const Node* node)
    {
        ostringstream os;
        os << "type(" << getCheckPropertyType(node, "givenType") << ")";
        return strdup(os.str().c_str());
    }

    Node* BackendCode_SemanticCheck(Node* node)
    {
        EvalMode mode = BackendCode_getEvalMode(node);
        if ( !node->type )
            node->type = getVoidType(mode);

        if ( mode != modeRt )
        {
            // CT process this node right after semantic check
            addModifier(node, new CtProcessMod);
        }
        return node;
    }
    const char* BackendCode_toString(const Node* node)
    {
        ostringstream os;
        os << "backendCode(" << BackendCode_getCode(node) << ")";
        return strdup(os.str().c_str());
    }

    TypeRef NodeList_ComputeType(Node* node)
    {
        // Compute the type for all the children
        for ( Node* c: node->children )
        {
            if ( c )
                Nest::computeType(c);
        }

        // Get the type of the last node
        TypeRef res = ( hasProperty(node, propResultVoid) || node->children.empty() || !node->children.back()->type ) ? getVoidType(node->context->evalMode()) : node->children.back()->type;
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
                Nest::semanticCheck(c);
                hasNonCtChildren = hasNonCtChildren || !isCt(c);
            }
        }

        // Make sure the type is computed
        if ( !node->type )
        {
            // Get the type of the last node
            TypeRef t = ( hasProperty(node, propResultVoid) || node->children.empty() || !node->children.back()->type ) ? getVoidType(node->context->evalMode()) : node->children.back()->type;
            t = adjustMode(t, node->context, node->location);
            node->type = t;
            checkEvalMode(node);
        }
        return node;
    }

    void LocalSpace_SetContextForChildren(Node* node)
    {
        node->childrenContext = node->context->createChildContext(node);
        Nest::defaultFunSetContextForChildren(node);
    }
    TypeRef LocalSpace_ComputeType(Node* node)
    {
        return getVoidType(node->context->evalMode());
    }
    Node* LocalSpace_SemanticCheck(Node* node)
    {
        // Compute type first
        computeType(node);

        // Semantic check each of the children
        for ( Node* c: node->children )
        {
            try
            {
                Nest::semanticCheck(c);
            }
            catch(...)
            {
                // Don't pass errors upwards
            }
        }
        checkEvalMode(node);
        return node;
    }

    Node* GlobalConstructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());

        // For CT construct actions, evaluate them asap
        if ( isCt(node->children[0]) )
        {
            theCompiler().ctEval(node->children[0]);
            return mkNop(node->location)->node();
        }
        return node;
    }

    Node* GlobalDestructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());

        // We never CT evaluate global destruct actions
        if ( isCt(node->children[0]) )
        {
            return mkNop(node->location)->node();
        }
        return node;
    }

    // Both for ScopeDestructAction and for TempDestructAction
    Node* ScopeTempDestructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    void Function_SetContextForChildren(Node* node)
    {
        // If we don't have a children context, create one
        if ( !node->childrenContext )
            node->childrenContext = node->context->createChildContext(node, effectiveEvalMode(node));

        Nest::defaultFunSetContextForChildren(node);
        
        addToSymTab(node);
    }
    TypeRef Function_ComputeType(Node* node)
    {
        if ( getName(node).empty() )
            REP_ERROR(node->location, "No name given to function declaration");

        // We must have a result type
        Node* resultType = node->children[0];
        computeType(resultType);
        TypeRef resType = resultType->type;
        if ( !resType )
            REP_ERROR(node->location, "No result type given to function %1%") % getName(node);

        // Compute the type for all the parameters
        auto it = node->children.begin()+2;
        auto ite = node->children.end();
        for ( ; it!=ite; ++it )
        {
            Node* param = *it;
            if ( !param )
                REP_ERROR(node->location, "Invalid param");
            computeType(param);
        }

        // Set the type for this node
        vector<TypeRef> subTypes;
        subTypes.reserve(node->children.size()-1);
        subTypes.push_back(resType);
        it = node->children.begin()+2;
        for ( ; it!=ite; ++it )
        {
            Node* param = *it;
            subTypes.push_back(param->type);
        }
        return getFunctionType(&subTypes[0], subTypes.size(), effectiveEvalMode(node));
    }
    Node* Function_SemanticCheck(Node* node)
    {
        // Make sure the type is computed
        computeType(node);

        // Semantically check all the parameters
        auto it = node->children.begin()+2;
        auto ite = node->children.end();
        for ( ; it!=ite; ++it )
        {
            Node* param = *it;
            semanticCheck(param);
        }

        // Semantically check the body, if we have one
        try
        {
            if ( node->children[1] )
                semanticCheck(node->children[1]);

            // TODO (function): Check that all the paths return a value
        }
        catch (const exception&)
        {
            // Don't propagate errors from the body
        }
        return node;
    }
    const char* Function_toString(const Node* node)
    {
        ostringstream os;
        os << getName(node);
        if ( node->type )
        {
            os << '(';
            bool hasResultParam = hasProperty(node, propResultParam);
            size_t startIdx = hasResultParam ? 3 : 2;
            for ( size_t i=startIdx; i<node->children.size(); ++i )
            {
                if ( i > startIdx )
                    os << ", ";
                os << node->children[i]->type;
            }
            os << "): " << (hasResultParam ? removeRef(node->children[2]->type) : node->children[0]->type);
        }
        return strdup(os.str().c_str());
    }

    void Class_SetContextForChildren(Node* node)
    {
        // If we don't have a children context, create one
        if ( !node->childrenContext )
            node->childrenContext = node->context->createChildContext(node, effectiveEvalMode(node));

        Nest::defaultFunSetContextForChildren(node);
        
        addToSymTab(node);
    }
    TypeRef Class_ComputeType(Node* node)
    {
        if ( getName(node).empty() )
            REP_ERROR(node->location, "No name given to class");

        // Compute the type for all the fields
        for ( Node* field: node->children )
        {
            try
            {
                Nest::computeType(field);
            }
            catch (const exception&)
            {
                // Don't propagate the errors upward
            }
        }
        return getDataType(node, 0, effectiveEvalMode(node));
    }
    Node* Class_SemanticCheck(Node* node)
    {
        computeType(node);

        // Semantically check all the fields
        for ( Node* field: node->children )
        {
            try
            {
                Nest::semanticCheck(field);
            }
            catch (const exception&)
            {
                // Don't propagate the errors upward
            }
        }
        return node;
    }

    void Var_SetContextForChildren(Node* node)
    {
        Nest::defaultFunSetContextForChildren(node);
        addToSymTab(node);
    }
    TypeRef Var_ComputeType(Node* node)
    {
        // Make sure the variable has a type
        ASSERT(node->children.size() == 1);
        Node* typeNode = node->children[0];
        computeType(typeNode);

        // Adjust the mode of the type
        return adjustMode(typeNode->type, node->context, node->location);
    }
    Node* Var_SemanticCheck(Node* node)
    {
        computeType(node);

        // Make sure that the type has storage
        if ( !node->type->hasStorage )
            REP_ERROR(node->location, "Variable type has no storage (%1%") % node->type;

        Nest::computeType(classForTypeRaw(node->type));           // Make sure the type of the class is computed
        return node;
    }

    Node* CtValue_SemanticCheck(Node* node)
    {
        // Check the type
        if ( !node->type )
            node->type = getCheckPropertyType(node, "valueType");
        if ( !node->type || !node->type->hasStorage || node->type->mode == modeRt )
            REP_ERROR(node->location, "Type specified for Ct Value cannot be used at compile-time");
        
        // Make sure data size matches the size reported by the type
        size_t valueSize = theCompiler().sizeOf(node->type);
        const string& data = getCheckPropertyString(node, "valueData");
        if ( valueSize != data.size() )
        {
            REP_ERROR(node->location, "Read value size (%1%) differs from declared size of the value (%2%) - type: %3%")
                % data.size() % valueSize % node->type;
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

        const string& valueDataStr = getCheckPropertyString(node, "valueData");
        const void* valueData = valueDataStr.c_str();
        
        const string* nativeName = node->type->hasStorage ? Feather::nativeName(node->type) : nullptr;
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
                os << "'" << valueDataStr << "'";
        }
        else
            os << "'" << valueDataStr << "'";
        os << ")";
        return strdup(os.str().c_str());
    }

    Node* Null_SemanticCheck(Node* node)
    {
        ASSERT(node->children.size() == 1);
        DynNode* typeNode = (DynNode*) node->children[0];
        typeNode->computeType();

        // Make sure that the type is a reference
        TypeRef t = typeNode->type();
        if ( !t->hasStorage )
            REP_ERROR(node->location, "Null node should have a type with storage (cur type: %1%") % t;
        if ( t->numReferences == 0 )
            REP_ERROR(node->location, "Null node should have a reference type (cur type: %1%)") % t;

        node->type = adjustMode(t, node->context, node->location);
        return node;
    }
    const char* Null_toString(const Node* node)
    {
        ostringstream os;
        os << "null(" << node->type << ")";
        return strdup(os.str().c_str());
    }

    Node* StackAlloc_SemanticCheck(Node* node)
    {
        if ( getCheckPropertyInt(node, "numElements") == 0 )
            REP_ERROR(node->location, "Cannot allocate 0 elements on the stack");

        // Set the resulting type
        ASSERT(node->children.size() == 1);
        DynNode* elemTypeNode = (DynNode*) node->children[0];
        elemTypeNode->computeType();
        node->type = adjustMode(getLValueType(elemTypeNode->type()), node->context, node->location);
        return node;
    }
    const char* StackAlloc_toString(const Node* node)
    {
        ostringstream os;
        os << "stackAlloc(" << node->children[0]->type << ", " << getCheckPropertyInt(node, "numElements") << ")";
        return strdup(os.str().c_str());
    }

    Node* VarRef_SemanticCheck(Node* node)
    {
        DynNode* var = (DynNode*) node->referredNodes[0];
        ASSERT(var);
        if ( var->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(node->location, "VarRef object needs to point to a Field (node kind: %1%)") % var->nodeKindName();
        var->computeType();
        if ( isField(var->node()) )
            REP_INTERNAL(node->location, "VarRef used on a field (%1%). Use FieldRef instead") % getName(var->node());
        if ( !var->type()->hasStorage )
            REP_ERROR(node->location, "Variable type is doesn't have a storage type (type: %1%)") % var->type();
        node->type = adjustMode(getLValueType(var->type()), node->context, node->location);
        checkEvalMode(node, var->type()->mode);
        return node;
    }
    const char* VarRef_toString(const Node* node)
    {
        ostringstream os;
        os << "varRef(" << getName(node->referredNodes[0]) << ")";
        return strdup(os.str().c_str());
    }

    Node* FieldRef_SemanticCheck(Node* node)
    {
        DynNode* obj = (DynNode*) node->children[0];
        DynNode* field = (DynNode*) node->referredNodes[0];
        ASSERT(obj);
        ASSERT(field);
        if ( field->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(node->location, "FieldRef object needs to point to a Field (node kind: %1%)") % field->nodeKindName();

        // Semantic check the object node - make sure it's a reference to a data type
        obj->semanticCheck();
        ASSERT(obj->type());
        if ( !obj->type() || !obj->type()->hasStorage || obj->type()->numReferences != 1 )
            REP_ERROR(node->location, "Field access should be done on a reference to a data type (type: %1%)") % obj->type();
        Class* cls = classForType(obj->type());
        ASSERT(cls);
        cls->computeType();

        // Compute the type of the field
        field->computeType();

        // Make sure that the type of a object is a data type that refers to a class the contains the given field
        bool fieldFound = false;
        for ( auto field: cls->fields() )
        {
            if ( &*field == field )
            {
                fieldFound = true;
                break;
            }
        }
        if ( !fieldFound )
            REP_ERROR(node->location, "Field '%1%' not found when accessing object") % getName(field->node());

        // Set the correct type for this node
        ASSERT(field->type());
        ASSERT(field->type()->hasStorage);
        node->type = getLValueType(field->type());
        node->type = adjustMode(node->type, obj->type()->mode, node->context, node->location);
        return node;
    }
    const char* FieldRef_toString(const Node* node)
    {
        ostringstream os;
        os << "fieldRef(" << node->children[0] << ", " << node->referredNodes[0] << ")";
        return strdup(os.str().c_str());
    }

    Node* FunRef_SemanticCheck(Node* node)
    {
        ASSERT(node->children.size() == 1);
        DynNode* resType = (DynNode*) node->children[0];
        resType->computeType();

        computeType(node->referredNodes[0]);
        node->type = adjustMode(resType->type(), node->context, node->location);
        return node;
    }
    const char* FunRef_toString(const Node* node)
    {
        ostringstream os;
        os << "FunRef(" << node->referredNodes[0] << ")";
        return strdup(os.str().c_str());
    }

    Node* FunCall_SemanticCheck(Node* node)
    {
        Function* fun = (Function*) node->referredNodes[0];
        
        // Make sure the function declaration is has a valid type
        fun->computeType();

        // Check argument count
        if ( node->children.size() != fun->numParameters() )
            REP_ERROR(node->location, "Invalid function call: expecting %1% parameters, given %2%")
                % fun->numParameters() % node->children.size();

        // Semantic check the arguments
        // Also check that their type matches the corresponding type from the function decl
        bool allParamsAreCtAvailable = true;
        for ( size_t i=0; i<node->children.size(); ++i )
        {
            // Semantically check the argument
            Nest::semanticCheck(node->children[i]);
            if ( !isCt(node->children[i]) )
                allParamsAreCtAvailable = false;

            // Compare types
            TypeRef argType = node->children[i]->type;
            TypeRef paramType = fun->getParameter(i)->type();
            if ( !isSameTypeIgnoreMode(argType, paramType) )
                REP_ERROR(node->children[i]->location, "Invalid function call: argument %1% is expected to have type %2% (actual type: %3%)")
                    % (i+1) % paramType % argType;
        }

        // CT availability checks
        EvalMode curMode = node->context->evalMode();
        EvalMode calledFunMode = effectiveEvalMode(fun->node());
        ASSERT(curMode != Nest::modeUnspecified);
        ASSERT(calledFunMode != Nest::modeUnspecified);
        if ( calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable )
        {
            REP_ERROR_NOTHROW(node->location, "Not all arguments are compile-time, when calling a compile time function");
            REP_INFO(fun->location(), "See called function");
            REP_ERROR_THROW("Bad mode");
        }
        if ( curMode == modeRtCt && calledFunMode == modeRt )
        {
            REP_ERROR_NOTHROW(node->location, "Cannot call RT functions from RTCT contexts");
            REP_INFO(fun->location(), "See called function");
            REP_ERROR_THROW("Bad mode");
        }
        if ( curMode == modeCt && calledFunMode == modeRt )
        {
            REP_ERROR_NOTHROW(node->location, "Cannot call a RT function from a CT context");
            REP_INFO(fun->location(), "See called function");
            REP_ERROR_THROW("Bad mode");
        }

        // Get the type from the function decl
        node->type = fun->resultType();

        // Handle autoCt case
        if ( allParamsAreCtAvailable && node->type->mode == modeRtCt && fun->hasProperty(propAutoCt) )
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
        os << "funCall-" << getName(node->referredNodes[0]) << "(";
        for ( size_t i=0; i<node->children.size(); ++i )
        {
            if ( i != 0 )
                os << ", ";
            os << node->children[i];
        }
        os << ")";
        return strdup(os.str().c_str());
    }

    Node* MemLoad_SemanticCheck(Node* node)
    {
        ASSERT(node->children[0]);

        // Semantic check the argument
        Nest::semanticCheck(node->children[0]);

        // Check if the type of the argument is a ref
        if ( !node->children[0]->type->hasStorage || node->children[0]->type->numReferences == 0 )
            REP_ERROR(node->location, "Cannot load from a non-reference (%1%, type: %2%)") % node->children[0] % node->children[0]->type;

        // Check flags
        AtomicOrdering ordering = (AtomicOrdering) getCheckPropertyInt(node, "atomicOrdering");
        if ( ordering == atomicRelease )
            REP_ERROR(node->location, "Cannot use atomic release with a load instruction");
        if ( ordering == atomicAcquireRelease )
            REP_ERROR(node->location, "Cannot use atomic acquire-release with a load instruction");

        // Remove the 'ref' from the type and get the base type
        node->type = removeRef(node->children[0]->type);
        node->type = adjustMode(node->type, node->context, node->location);
        return node;
    }

    Node* MemStore_SemanticCheck(Node* node)
    {
        DynNode* value = (DynNode*) node->children[0];
        DynNode* address = (DynNode*) node->children[1];
        ASSERT(value);
        ASSERT(address);

        // Semantic check the arguments
        value->semanticCheck();
        address->semanticCheck();

        // Check if the type of the address is a ref
        if ( !address->type()->hasStorage || address->type()->numReferences == 0 )
            REP_ERROR(node->location, "The address of a memory store is not a reference, nor VarRef nor FieldRef (type: %1%)") % address->type();
        TypeRef baseAddressType = removeRef(address->type());

        // Check the equivalence of types
        if ( !isSameTypeIgnoreMode(value->type(), baseAddressType) )
        {
            // Try again, getting rid of l-values
            TypeRef t1 = lvalueToRefIfPresent(value->type());
            if ( !isSameTypeIgnoreMode(t1, baseAddressType) )
                REP_ERROR(node->location, "The type of the value doesn't match the type of the address in a memory store (%1% != %2%)")
                    % value->type() % baseAddressType;
        }

        // The resulting type is Void
        node->type = getVoidType(address->type()->mode);
        return node;
    }

    Node* Bitcast_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        Nest::computeType(node->children[1]);
        TypeRef tDest = node->children[1]->type;

        // Make sure both types have storage
        TypeRef srcType = node->children[0]->type;
        if ( !srcType->hasStorage )
            REP_ERROR(node->location, "The source of a bitcast is not a type with storage (%1%)") % srcType;
        if ( !tDest->hasStorage )
            REP_ERROR(node->location, "The destination type of a bitcast is not a type with storage (%1%)") % tDest;
        
        // Make sure both types are references
        if ( srcType->numReferences == 0 )
            REP_ERROR(node->location, "The source of a bitcast is not a reference (%1%)") % srcType;
        if ( tDest->numReferences == 0 )
            REP_ERROR(node->location, "The destination type of a bitcast is not a reference (%1%)") % tDest;

        node->type = adjustMode(tDest, node->context, node->location);
        return node;
    }
    const char* Bitcast_toString(const Node* node)
    {
        ostringstream os;
        if ( node->children[1]->type )
            os << "bitcast(" << node->children[1]->type << ", " << node->children[0] << ")";
        else
            os << "bitcast(type(" << node->children[1] << "), " << node->children[0] << ")";
        return strdup(os.str().c_str());
    }

    Node* Conditional_SemanticCheck(Node* node)
    {
        // Semantic check the condition
        Nest::semanticCheck(node->children[0]);

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(node->children[0]) )
            REP_ERROR(node->children[0]->location, "The condition of the conditional expression is not Testable");

        // Dereference the condition as much as possible
        while ( node->children[0]->type && node->children[0]->type->numReferences > 0 )
        {
            node->children[0] = mkMemLoad(node->children[0]->location, (DynNode*) node->children[0])->node();
            Nest::setContext(node->children[0], childrenContext(node));
            Nest::semanticCheck(node->children[0]);
        }
        // TODO (conditional): This shouldn't be performed here

        // Semantic check the alternatives
        Nest::semanticCheck(node->children[1]);
        Nest::semanticCheck(node->children[2]);

        // Make sure the types of the alternatives are equal
        if ( !isSameTypeIgnoreMode(node->children[1]->type, node->children[2]->type) )
            REP_ERROR(node->location, "The types of the alternatives of a conditional must be equal (%1% != %2%)") % node->children[1]->type % node->children[2]->type;

        node->type = adjustMode(node->children[1]->type, node->children[0]->type->mode, node->context, node->location);
        return node;
    }

    void ChangeMode_SetContextForChildren(Node* node)
    {
        EvalMode curMode = (EvalMode) getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode();
        node->childrenContext = new CompilationContext(node->context, newMode);
        Nest::defaultFunSetContextForChildren(node);
    }
    Node* ChangeMode_SemanticCheck(Node* node)
    {
        // Make sure we are allowed to change the mode
        EvalMode baseMode = node->context->evalMode();
        EvalMode curMode = (EvalMode) getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode();
        if ( newMode == modeUnspecified )
            REP_INTERNAL(node->location, "Cannot change the mode to Unspecified");
        if ( newMode == modeRt && baseMode != modeRt )
            REP_ERROR(node->location, "Cannot change mode to RT in a non-RT context (%1%)") % baseMode;
        if ( newMode == modeRtCt && baseMode != modeRtCt )
            REP_ERROR(node->location, "Cannot change mode to RTCT in a non-RTCT context (%1%)") % baseMode;

        if ( !node->children[0] )
            REP_INTERNAL(node->location, "No node specified as child to a ChangeMode node");

        Nest::semanticCheck(node->children[0]);
        return node->children[0];
    }
    const char* ChangeMode_toString(const Node* node)
    {
        ostringstream os;
        EvalMode curMode = (EvalMode) getCheckPropertyInt(node, propEvalMode);
        EvalMode newMode = curMode != modeUnspecified ? curMode : node->context->evalMode();
        os << "changeMode(" << node->children[0] << ", " << newMode << ")";
        return strdup(os.str().c_str());
    }

    void If_SetContextForChildren(Node* node)
    {
        node->childrenContext = node->context->createChildContext(node);

        Nest::setContext(node->children[0], node->childrenContext);
        if ( node->children[1] )
            Nest::setContext(node->children[1], node->childrenContext);
        if ( node->children[2] )
            Nest::setContext(node->children[2], node->childrenContext);
    }
    Node* If_SemanticCheck(Node* node)
    {
        Node* condition = node->children[0];
        Node* thenClause = node->children[1];
        Node* elseClause = node->children[2];
        
        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode());

        // Semantic check the condition
        semanticCheck(condition);

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(condition) )
            REP_ERROR(condition->location, "The condition of the if is not Testable");

        // Dereference the condition as much as possible
        while ( condition->type && condition->type->numReferences > 0 )
        {
            condition = mkMemLoad(condition->location, (DynNode*) condition)->node();
            setContext(condition, childrenContext(node));
            semanticCheck(condition);
        }
        node->children[0] = condition;
        // TODO (if): Remove this dereference from here

        if ( nodeEvalMode(node) == modeCt )
        {
            if ( !isCt(condition) )
                REP_ERROR(condition->location, "The condition of the ct if should be available at compile-time (%1%)") % condition->type;

            // Get the CT value from the condition, and select an active branch
            Node* c = theCompiler().ctEval(condition);
            Node* selectedBranch = getBoolCtValue(c) ? thenClause : elseClause;

            // Expand only the selected branch
            if ( selectedBranch )
                return selectedBranch;
            else
                return mkNop(node->location)->node();
        }

        // Semantic check the clauses
        if ( thenClause )
            semanticCheck(thenClause);
        if ( elseClause )
            semanticCheck(elseClause);
        return node;
    }

    void While_SetContextForChildren(Node* node)
    {
        node->childrenContext = node->context->createChildContext(node);
        CompilationContext* condContext = nodeEvalMode(node) == modeCt ? new CompilationContext(node->context, modeCt) : node->childrenContext;

        Nest::setContext(node->children[0], condContext); // condition
        if ( node->children[1] )
            Nest::setContext(node->children[1], condContext); // step
        if ( node->children[2] )
            Nest::setContext(node->children[2], node->childrenContext); // body
        // Nest::defaultFunSetContextForChildren(node);
    }
    Node* While_SemanticCheck(Node* node)
    {
        Node* condition = node->children[0];
        Node* step = node->children[1];
        Node* body = node->children[2];
        
        // Semantic check the condition
        semanticCheck(condition);
        if ( step )
            computeType(step);

        // Check that the type of the condition is 'Testable'
        if ( !isTestable(condition) )
            REP_ERROR(condition->location, "The condition of the while is not Testable");

        if ( nodeEvalMode(node) == modeCt )
        {
            if ( !isCt(condition) )
                REP_ERROR(condition->location, "The condition of the ct while should be available at compile-time (%1%)") % condition->type;
            if ( step && !isCt(step) )
                REP_ERROR(step->location, "The step of the ct while should be available at compile-time (%1%)") % step->type;

            // Create a node-list that will be our explanation
            NodeVector result;

            // Do the while
            while ( true )
            {
                // CT-evaluate the condition; if the condition evaluates to false, exit the while
                if ( !getBoolCtValue(theCompiler().ctEval(condition)) )
                    break;

                // Put (a copy of) the body in the resulting node-list
                if ( body )
                {
                    Node* curBody = cloneNode(body);
                    setContext(curBody, node->context);
                    semanticCheck(curBody);
                    result.push_back(curBody);
                }

                // If we have a step, make sure to evaluate it
                if ( step )
                {
                    theCompiler().ctEval(step);    // We don't need the actual result
                }

                // Unfortunately, we don't treat 'break' and 'continue' instructions inside the ct while instructions
            }
            result.push_back(mkNop(node->location)->node()); // Make sure our resulting type is Void

            // Set the explanation and exit
            return mkNodeList(node->location, toDyn(move(result)))->node();
        }

        // Semantic check the body and the step
        if ( body )
            semanticCheck(body);
        if ( step )
            semanticCheck(step);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    Node* Break_SemanticCheck(Node* node)
    {
        // Get the outer-most loop from the context
        Node* loop = getParentLoop(node->context);
        if ( !loop )
            REP_ERROR(node->location, "Break found outside any loop");
        setProperty(node, "loop", loop);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    Node* Continue_SemanticCheck(Node* node)
    {
        // Get the outer-most loop from the context
        Node* loop = getParentLoop(node->context);
        if ( !loop )
            REP_ERROR(node->location, "Continue found outside any loop");
        setProperty(node, "loop", loop);

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    Node* Return_SemanticCheck(Node* node)
    {
        // If we have an expression argument, semantically check it
        if ( node->children[0] )
            semanticCheck(node->children[0]);

        // Get the parent function of this return
        Function* parentFun = getParentFun(node->context);
        if ( !parentFun )
            REP_ERROR(node->location, "Return found outside any function");
        ASSERT(parentFun->resultType());
        setProperty(node, "parentFun", parentFun->node());

        // If the return has an expression, check that has the same type as the function result type
        if ( node->children[0] )
        {
            if ( !isSameTypeIgnoreMode(node->children[0]->type, parentFun->resultType()) )
                REP_ERROR(node->location, "Returned expression's type is not the same as function's return type");
        }
        else
        {
            // Make sure that the function has a void return type
            if ( parentFun->resultType()->typeKind != typeKindVoid )
                REP_ERROR(node->location, "You must return something in a function that has non-Void result type");
        }

        // The resulting type is Void
        node->type = getVoidType(node->context->evalMode());
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

int Feather::nkFeatherDeclFunction = 0;
int Feather::nkFeatherDeclClass = 0;
int Feather::nkFeatherDeclVar = 0;

int Feather::nkFeatherExpCtValue = 0;
int Feather::nkFeatherExpNull = 0;
int Feather::nkFeatherExpStackAlloc = 0;
int Feather::nkFeatherExpVarRef = 0;
int Feather::nkFeatherExpFieldRef = 0;
int Feather::nkFeatherExpFunRef = 0;
int Feather::nkFeatherExpFunCall = 0;
int Feather::nkFeatherExpMemLoad = 0;
int Feather::nkFeatherExpMemStore = 0;
int Feather::nkFeatherExpBitcast = 0;
int Feather::nkFeatherExpConditional = 0;
int Feather::nkFeatherExpChangeMode = 0;

int Feather::nkFeatherStmtIf = 0;
int Feather::nkFeatherStmtWhile = 0;
int Feather::nkFeatherStmtBreak = 0;
int Feather::nkFeatherStmtContinue = 0;
int Feather::nkFeatherStmtReturn = 0;

void Feather::initFeatherNodeKinds()
{
    nkFeatherNop = registerNodeKind("nop", &Nop_SemanticCheck);
    nkFeatherTypeNode = registerNodeKind("typeNode", &TypeNode_SemanticCheck, NULL, NULL, &TypeNode_toString);
    nkFeatherBackendCode = registerNodeKind("backendCode", &BackendCode_SemanticCheck, NULL, NULL, &BackendCode_toString);
    nkFeatherNodeList = registerNodeKind("nodeList", &NodeList_SemanticCheck, &NodeList_ComputeType, NULL, NULL);
    nkFeatherLocalSpace = registerNodeKind("localSpace", &LocalSpace_SemanticCheck, &LocalSpace_ComputeType, &LocalSpace_SetContextForChildren, NULL);
    nkFeatherGlobalConstructAction = registerNodeKind("globalConstructAction", &GlobalConstructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherGlobalDestructAction = registerNodeKind("globalDestructAction", &GlobalDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherScopeDestructAction = registerNodeKind("scopelDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherTempDestructAction = registerNodeKind("tempDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);

    nkFeatherDeclFunction = registerNodeKind("fun", &Function_SemanticCheck, &Function_ComputeType, &Function_SetContextForChildren, &Function_toString);
    nkFeatherDeclClass = registerNodeKind("class", &Class_SemanticCheck, &Class_ComputeType, &Class_SetContextForChildren, NULL);
    nkFeatherDeclVar = registerNodeKind("var", &Var_SemanticCheck, &Var_ComputeType, &Var_SetContextForChildren, NULL);

    nkFeatherExpCtValue = registerNodeKind("ctValue", &CtValue_SemanticCheck, NULL, NULL, &CtValue_toString);
    nkFeatherExpNull = registerNodeKind("null", &Null_SemanticCheck, NULL, NULL, &Null_toString);
    nkFeatherExpStackAlloc = registerNodeKind("stackAlloc", &StackAlloc_SemanticCheck, NULL, NULL, &StackAlloc_toString);
    nkFeatherExpVarRef = registerNodeKind("varRef", &VarRef_SemanticCheck, NULL, NULL, &VarRef_toString);
    nkFeatherExpFieldRef = registerNodeKind("fieldRef", &FieldRef_SemanticCheck, NULL, NULL, &FieldRef_toString);
    nkFeatherExpFunRef = registerNodeKind("funRef", &FunRef_SemanticCheck, NULL, NULL, &FunRef_toString);
    nkFeatherExpFunCall = registerNodeKind("funCall", &FunCall_SemanticCheck, NULL, NULL, &FunCall_toString);
    nkFeatherExpMemLoad = registerNodeKind("memLoad", &MemLoad_SemanticCheck, NULL, NULL, NULL);
    nkFeatherExpMemStore = registerNodeKind("memStore", &MemStore_SemanticCheck, NULL, NULL, NULL);
    nkFeatherExpBitcast = registerNodeKind("bitcast", &Bitcast_SemanticCheck, NULL, NULL, &Bitcast_toString);
    nkFeatherExpConditional = registerNodeKind("conditional", &Conditional_SemanticCheck, NULL, NULL, NULL);
    nkFeatherExpChangeMode = registerNodeKind("changeMode", &ChangeMode_SemanticCheck, NULL, &ChangeMode_SetContextForChildren, &ChangeMode_toString);

    nkFeatherStmtIf = registerNodeKind("if", &If_SemanticCheck, NULL, &If_SetContextForChildren, NULL);
    nkFeatherStmtWhile = registerNodeKind("while", &While_SemanticCheck, NULL, &While_SetContextForChildren, NULL);
    nkFeatherStmtBreak = registerNodeKind("break", &Break_SemanticCheck, NULL, NULL, NULL);
    nkFeatherStmtContinue = registerNodeKind("continue", &Continue_SemanticCheck, NULL, NULL, NULL);
    nkFeatherStmtReturn = registerNodeKind("return", &Return_SemanticCheck, NULL, NULL, NULL);


    Nop::classNodeKindRef() = nkFeatherNop;
    TypeNode::classNodeKindRef() = nkFeatherTypeNode;
    BackendCode::classNodeKindRef() = nkFeatherBackendCode;
    NodeList::classNodeKindRef() = nkFeatherNodeList;
    LocalSpace::classNodeKindRef() = nkFeatherLocalSpace;
    GlobalConstructAction::classNodeKindRef() = nkFeatherGlobalConstructAction;
    GlobalDestructAction::classNodeKindRef() = nkFeatherGlobalDestructAction;
    ScopeDestructAction::classNodeKindRef() = nkFeatherScopeDestructAction;
    TempDestructAction::classNodeKindRef() = nkFeatherTempDestructAction;

    Function::classNodeKindRef() = nkFeatherDeclFunction;
    Class::classNodeKindRef() = nkFeatherDeclClass;
    Var::classNodeKindRef() = nkFeatherDeclVar;
    
    CtValue::classNodeKindRef() = nkFeatherExpCtValue;
    Null::classNodeKindRef() = nkFeatherExpNull;
    StackAlloc::classNodeKindRef() = nkFeatherExpStackAlloc;
    VarRef::classNodeKindRef() = nkFeatherExpVarRef;
    FieldRef::classNodeKindRef() = nkFeatherExpFieldRef;
    FunRef::classNodeKindRef() = nkFeatherExpFunRef;
    FunCall::classNodeKindRef() = nkFeatherExpFunCall;
    MemLoad::classNodeKindRef() = nkFeatherExpMemLoad;
    MemStore::classNodeKindRef() = nkFeatherExpMemStore;
    Bitcast::classNodeKindRef() = nkFeatherExpBitcast;
    Conditional::classNodeKindRef() = nkFeatherExpConditional;
    ChangeMode::classNodeKindRef() = nkFeatherExpChangeMode;
    
    If::classNodeKindRef() = nkFeatherStmtIf;
    While::classNodeKindRef() = nkFeatherStmtWhile;
    Break::classNodeKindRef() = nkFeatherStmtBreak;
    Continue::classNodeKindRef() = nkFeatherStmtContinue;
    Return::classNodeKindRef() = nkFeatherStmtReturn;


    firstFeatherNodeKind = nkFeatherNop;
}


NodeList* Feather::mkNodeList(const Location& loc, DynNodeVector children, bool voidResult)
{
    Node* res = createNode(nkFeatherNodeList);
    res->location = loc;
    if ( voidResult )
        setProperty(res, propResultVoid, 1);
    res->children = move(fromDyn(children));
    return (NodeList*) res;
}
NodeList* Feather::addToNodeList(NodeList* prevList, DynNode* element)
{
    Node* res = prevList->node();
    if ( !res )
    {
        res = createNode(nkFeatherNodeList);
        if ( element )
            res->location = element->location();
        setProperty(res, propResultVoid, 1);    // voidResult == true
    }
    
    res->children.push_back(element->node());
    return (NodeList*) res;
}
NodeList* Feather::appendNodeList(NodeList* list, NodeList* newNodes)
{
    if ( !list )
        return newNodes;
    if ( !newNodes )
        return list;
    
    Node* res = list->node();
    NodeVector& otherChildren = newNodes->node()->children;
    res->children.insert(res->children.end(), otherChildren.begin(), otherChildren.end());
    return (NodeList*) res;
}


DynNode* Feather::mkNop(const Location& loc)
{
    Node* res = createNode(nkFeatherNop);
    res->location = loc;
    return (DynNode*) res;
}
DynNode* Feather::mkTypeNode(const Location& loc, TypeRef type)
{
    Node* res = createNode(nkFeatherTypeNode);
    res->location = loc;
    setProperty(res, "givenType", type);
    return (DynNode*) res;
}
DynNode* Feather::mkBackendCode(const Location& loc, string code, EvalMode evalMode)
{
    Node* res = createNode(nkFeatherBackendCode);
    res->location = loc;
    setProperty(res, propCode, move(code));
    setProperty(res, propEvalMode, (int) evalMode);
    return (DynNode*) res;
}
DynNode* Feather::mkLocalSpace(const Location& loc, DynNodeVector children)
{
    Node* res = createNode(nkFeatherLocalSpace);
    res->location = loc;
    res->children = move(fromDyn(children));
    return (DynNode*) res;
}
DynNode* Feather::mkGlobalConstructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherGlobalConstructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkGlobalDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherGlobalDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkScopeDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherScopeDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkTempDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherTempDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}


DynNode* Feather::mkFunction(const Location& loc, string name, DynNode* resType, DynNodeVector params, DynNode* body, CallConvention callConv, EvalMode evalMode)
{
    Node* res = createNode(nkFeatherDeclFunction);
    res->location = loc;
    res->children = fromDyn(move(params));
    setName(res, move(name));
    setProperty(res, "callConvention", (int) callConv);
    setEvalMode(res, evalMode);

    // Make sure all the nodes given as parameters have the right kind
    for ( Node* param: res->children )
    {
        if ( explanation(param)->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(param->location, "Node %1% must be a parameter") % param;
    }

    // The result type and body is at the beginning of the parameters
    res->children.insert(res->children.begin(), body->node());
    res->children.insert(res->children.begin(), resType->node());

    return (DynNode*) res;
}
DynNode* Feather::mkClass(const Location& loc, string name, DynNodeVector fields, EvalMode evalMode)
{
    Node* res = createNode(nkFeatherDeclClass);
    res->location = loc;
    res->children = fromDyn(move(fields));
    setName(res, move(name));
    setEvalMode(res, evalMode);

    // Make sure all the nodes given as parameters have the right kind
    for ( Node* field: res->children )
    {
        if ( field->nodeKind != nkFeatherDeclVar )
            REP_INTERNAL(field->location, "Node %1% must be a field") % field;
    }

    return (DynNode*) res;
}
DynNode* Feather::mkVar(const Location& loc, string name, DynNode* typeNode, size_t alignment, EvalMode evalMode)
{
    REQUIRE_TYPE(loc, typeNode);
    Node* res = createNode(nkFeatherDeclVar);
    res->location = loc;
    res->children = { typeNode->node() };
    setName(res, move(name));
    setProperty(res, "alignment", alignment);
    setEvalMode(res, evalMode);
    return (DynNode*) res;
}


DynNode* Feather::mkCtValue(const Location& loc, TypeRef type, string data)
{
    REQUIRE_TYPE(loc, type);
    Node* res = createNode(nkFeatherExpCtValue);
    res->location = loc;
    setProperty(res, "valueType", type);
    setProperty(res, "valueData", move(data));
    return (DynNode*) res;
}
DynNode* Feather::mkNull(const Location& loc, DynNode* typeNode)
{
    REQUIRE_NODE(loc, typeNode);
    Node* res = createNode(nkFeatherExpNull);
    res->location = loc;
    res->children = { typeNode->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkStackAlloc(const Location& loc, DynNode* typeNode, int numElements, int alignment)
{
    REQUIRE_NODE(loc, typeNode);
    Node* res = createNode(nkFeatherExpStackAlloc);
    res->location = loc;
    res->children = { typeNode->node() };
    setProperty(res, "numElements", numElements);
    setProperty(res, "alignment", alignment);
    return (DynNode*) res;
}
DynNode* Feather::mkVarRef(const Location& loc, DynNode* varDecl)
{
    REQUIRE_NODE(loc, varDecl);
    if ( varDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A VarRef node must be applied on a variable declaration (%1% given)") % varDecl->nodeKindName();
    Node* res = createNode(nkFeatherExpVarRef);
    res->location = loc;
    res->referredNodes = { varDecl->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkFieldRef(const Location& loc, DynNode* obj, DynNode* fieldDecl)
{
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    if ( fieldDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A FieldRef node must be applied on a field declaration (%1% given)") % fieldDecl->nodeKindName();
    Node* res = createNode(nkFeatherExpFieldRef);
    res->location = loc;
    res->children = { obj->node() };
    res->referredNodes = { fieldDecl->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkFunRef(const Location& loc, DynNode* funDecl, DynNode* resType)
{
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    if ( funDecl->nodeKind() != nkFeatherDeclFunction )
        REP_INTERNAL(loc, "A FunRef node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    Node* res = createNode(nkFeatherExpFunRef);
    res->location = loc;
    res->children = { resType->node() };
    res->referredNodes = { funDecl->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkFunCall(const Location& loc, DynNode* funDecl, DynNodeVector args)
{
    REQUIRE_NODE(loc, funDecl);
    Function* fun = funDecl->as<Function>();
    if ( !fun )
        REP_INTERNAL(loc, "A FunCall node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    Node* res = createNode(nkFeatherExpFunCall);
    res->location = loc;
    res->children = fromDyn(move(args));
    res->referredNodes = { funDecl->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkMemLoad(const Location& loc, DynNode* exp, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, exp);
    Node* res = createNode(nkFeatherExpMemLoad);
    res->location = loc;
    res->children = { exp->node() };
    setProperty(res, "alignment", alignment);
    setProperty(res, "volatile", isVolatile ? 1 : 0);
    setProperty(res, "atomicOrdering", (int) ordering);
    setProperty(res, "singleThreaded", singleThreaded ? 1 : 0);
    return (DynNode*) res;
}
DynNode* Feather::mkMemStore(const Location& loc, DynNode* value, DynNode* address, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    Node* res = createNode(nkFeatherExpMemStore);
    res->location = loc;
    res->children = { value->node(), address->node() };
    setProperty(res, "alignment", alignment);
    setProperty(res, "volatile", isVolatile ? 1 : 0);
    setProperty(res, "atomicOrdering", (int) ordering);
    setProperty(res, "singleThreaded", singleThreaded ? 1 : 0);
    return (DynNode*) res;
}
DynNode* Feather::mkBitcast(const Location& loc, DynNode* destType, DynNode* exp)
{
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    Node* res = createNode(nkFeatherExpBitcast);
    res->location = loc;
    res->children = { exp->node(), destType->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkConditional(const Location& loc, DynNode* condition, DynNode* alt1, DynNode* alt2)
{
    REQUIRE_NODE(loc, condition);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    Node* res = createNode(nkFeatherExpConditional);
    res->location = loc;
    res->children = { condition->node(), alt1->node(), alt2->node() };
    return (DynNode*) res;
}


DynNode* Feather::mkIf(const Location& loc, DynNode* condition, DynNode* thenClause, DynNode* elseClause, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    Node* res = createNode(nkFeatherStmtIf);
    res->location = loc;
    res->children = { condition->node(), thenClause->node(), elseClause->node() };
    if ( isCt )
        setEvalMode(res, modeCt);
    return (DynNode*) res;
}
DynNode* Feather::mkWhile(const Location& loc, DynNode* condition, DynNode* body, DynNode* step, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    Node* res = createNode(nkFeatherStmtWhile);
    res->location = loc;
    res->children = { condition->node(), step->node(), body->node() };
    if ( isCt )
        setEvalMode(res, modeCt);
    return (DynNode*) res;
}
DynNode* Feather::mkBreak(const Location& loc)
{
    Node* res = createNode(nkFeatherStmtBreak);
    res->location = loc;
    setProperty(res, "loop", (Node*) nullptr);
    return (DynNode*) res;
}
DynNode* Feather::mkContinue(const Location& loc)
{
    Node* res = createNode(nkFeatherStmtContinue);
    res->location = loc;
    setProperty(res, "loop", (Node*) nullptr);
    return (DynNode*) res;
}
DynNode* Feather::mkReturn(const Location& loc, DynNode* exp)
{
    Node* res = createNode(nkFeatherStmtReturn);
    res->location = loc;
    res->children = { exp->node() };
    setProperty(res, "parentFun", (Node*) nullptr);
    return (DynNode*) res;
}


const char* Feather::BackendCode_getCode(const Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    return getCheckPropertyString(node, propCode).c_str();
}

EvalMode Feather::BackendCode_getEvalMode(Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    EvalMode curMode = (EvalMode) getCheckPropertyInt(node, propEvalMode);
    return curMode != modeUnspecified ? curMode : node->context->evalMode();
}
