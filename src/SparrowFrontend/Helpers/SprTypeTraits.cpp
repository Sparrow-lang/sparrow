#include <StdInc.h>
#include "SprTypeTraits.h"
#include "Overload.h"
#include "StdDef.h"
#include "Impl/Callable.h"
#include <SparrowFrontendTypes.h>
#include <NodeCommonsCpp.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Exp/CtValue.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/Decl.h>


using namespace SprFrontend;
using namespace Feather;

namespace
{
    bool getNumericProperties(Type* t, int& numBits, bool& isUnsigned, bool& isFloating)
    {
        if ( !t->hasStorage() )
            return false;
        Class* cls = classForType(t);
        ASSERT(cls);
        
        const string* nativeName = cls->getPropertyString(propNativeName);
        if ( !nativeName || nativeName->size() <= 1 || !islower((*nativeName)[0]) )
            return false;
        
        if ( *nativeName == "double" )
        {
            numBits = 64;
            isUnsigned = false;
            isFloating = true;
            return true;
        }
        else if ( *nativeName == "float" )
        {
            numBits = 32;
            isUnsigned = false;
            isFloating = true;
            return true;
        }
        else if ( nativeName->size() > 1 && ((*nativeName)[0] == 'i' || (*nativeName)[0] == 'u') )
        {
            try
            {
                numBits = boost::lexical_cast<int>(nativeName->substr(1));
                isUnsigned = (*nativeName)[0] == 'u';
                isFloating = false;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        return false;
        
    }
}

Nest::Type* SprFrontend::commonType(CompilationContext* context, Nest::Type* t1, Nest::Type* t2)
{
    // Check if the types are the same
    if ( t1 == t2 )
        return t1;
    
    // Check for numerics
    int numBits1, numBits2;
    bool isUnsigned1, isUnsigned2;
    bool isFloating1, isFloating2;
    if ( getNumericProperties(t1, numBits1, isUnsigned1, isFloating1)
        && getNumericProperties(t2, numBits2, isUnsigned2, isFloating2) )
    {
        // Floating & non-floating => return the floating point type
        if ( isFloating1 && !isFloating2 )
            return t1;
        if ( !isFloating1 && isFloating2 )
            return t2;
        
        // If the values are too small, return Int
        if ( numBits1 < 32 && numBits2 < 32 )
            return StdDef::typeInt;
        
        // Prefer the bigger type to the small type
        if ( numBits1 > numBits2 )
            return t1;
        if ( numBits1 < numBits2 )
            return t2;
        
        // Prefer unsigned, if the types have the same type
        if ( numBits1 == numBits2 && isUnsigned1 && !isUnsigned2 )
            return t1;
        if ( numBits1 == numBits2 && !isUnsigned1 && isUnsigned2 )
            return t2;
    }
    
    // If there is one conversion from one type to another (and not vice-versa) take the less specialized type
    ConversionFlags flags = flagDontAddReference;
    ConversionResult c1 = canConvertType(context, t1, t2, flags);
    ConversionResult c2 = canConvertType(context, t2, t1, flags);
    if ( c1 && !c2 )
    {
        return t2;
    }
    if ( !c1 && c2 )
    {
        return t1;
    }
    
    return StdDef::typeVoid;
}

Nest::Type* SprFrontend::doDereference1(Nest::Node* arg, Nest::Node*& cvt)
{
    cvt = arg;

    // If the base is an expression with a data type, treat this as a data access
    Type* t = arg->type();
    if ( !t->hasStorage() )
        return t;

    // If we have N references apply N-1 dereferencing operations
    for ( size_t i=1; i<t->noReferences(); ++i )
    {
        cvt = mkMemLoad(arg->location(), cvt);
    }
    return Type::fromBasicType(getDataType(classForType(t), 0, t->data_->mode));  // Zero references
}

namespace
{
    Node* checkDataTypeConversion(Node* node)
    {
        const Location& loc = node->location();
        Type* t = node->type();
        Class* cls = classForType(t);
        if ( effectiveEvalMode(cls) != modeRtCt )
            REP_INTERNAL(loc, "Cannot convert ct to rt for non-rtct classes (%1%)") % cls;

        // Check if we have a ct-to-rt ctor
        Callable* call = selectCtToRtCtor(node->context(), t);
        if ( !call )
            REP_ERROR(loc, "Cannot convert %1% from CT to RT (make sure 'ctorFromRt' method exists)") % t;

        // Generate the call to the ctor
        node->computeType();
        NodeVector args(1, node);
        auto cr = call->canCall(node->context(), loc, args, modeRt, true);
        ASSERT(cr);
        Node* res = call->generateCall(loc);
        res = mkMemLoad(loc, res);

        // Sanity check
        res->setContext(node->context());
        res->computeType();
        if ( res->type() != changeTypeMode(node->type(), modeRt) )
            REP_INTERNAL(loc, "Cannot convert %1% from CT to RT (invalid returned type)") % t;

        return res;
    }
}

Node* SprFrontend::convertCtToRt(Node* node)
{
    const Location& loc = node->location();

    Type* t = node->type();

    if ( t->typeId() == Type::typeVoid )
    {
        theCompiler().ctEval(node);
        return Feather::mkNop(loc);
    }

    if ( !t->hasStorage() )
        REP_ERROR(loc, "Cannot convert a non-storage type from CT to RT (%1%)") % t;

    if ( t->typeId() != Type::typeData )
        REP_ERROR(loc, "Cannot convert from CT to RT a node of non-data type (%1%)") % t->toString();

    if ( t->noReferences() > 0 )
        REP_ERROR(loc, "Cannot convert references from CT to RT (%1%)") % t;

    if ( isBasicNumericType(t) || changeTypeMode(t, modeRtCt) == StdDef::typeStringRef )
        return theCompiler().ctEval(node);
    else
        return checkDataTypeConversion(node);
}

Type* SprFrontend::getType(Node* typeNode)
{
    typeNode->semanticCheck();
    if ( !typeNode->type() )
        REP_ERROR(typeNode->location(), "Invalid type name");
    
    Type* t = tryGetTypeValue(typeNode);
    if ( t )
        return t;
    
    REP_ERROR(typeNode->location(), "Invalid type name (%1%)") % typeNode->type()->toString();
    return nullptr;
}

Type* SprFrontend::tryGetTypeValue(Node* typeNode)
{
    typeNode->semanticCheck();
    
    Type* t = Feather::lvalueToRefIfPresent(typeNode->type());
    
    if ( t == StdDef::typeRefType )
    {
        Node* n = theCompiler().ctEval(typeNode);
        CtValue* ctVal = n->as<CtValue>();
        if ( ctVal )
        {
            Type*** t = ctVal->value<Type**>();
            if ( !t || !*t || !**t )
                REP_ERROR(typeNode->location(), "No type was set for node");
            return **t;
        }
    }
    else if ( t == StdDef::typeType )
    {
        Node* n = theCompiler().ctEval(typeNode);
        CtValue* ctVal = n->as<CtValue>();
        if ( ctVal )
        {
            Type** t = ctVal->value<Type*>();
            if ( !t || !*t )
                REP_ERROR(typeNode->location(), "No type was set for node");
            return *t;
        }
    }
    
    return nullptr;
}

Type* SprFrontend::evalTypeIfPossible(Node* typeNode)
{
    Type* t = tryGetTypeValue(typeNode);
    return t ? t : typeNode->type();
}

Node* SprFrontend::createTypeNode(CompilationContext* context, const Location& loc, Type* t)
{
    Node* res = mkCtValue(loc, StdDef::typeType, &t);
    if ( context )
        res->setContext(context);
    return res;
}

Nest::Type* SprFrontend::getAutoType(Nest::Node* typeNode, bool addRef)
{
    Type* t = typeNode->type();
    
    // Nothing to do for function types
    if ( t->typeId() == Type::typeFunction )
        return t;
    
    // Remove l-value if we have one
    if ( t->typeId() == Type::typeLValue )
        t = Type::fromBasicType(baseType(t->data_));
    
    // Dereference
    t = Feather::removeAllRef(t);
    
    if ( addRef )
        t = Feather::addRef(t);
    t = Feather::changeTypeMode(t, modeRtCt, typeNode->location());
    return t;
}

bool SprFrontend::isConceptType(Type* t)
{
    return t->typeId() == Type::typeConcept;
}

bool SprFrontend::isConceptType(Type* t, bool& isRefAuto)
{
    if ( t->typeId() == Type::typeConcept )
    {
        isRefAuto = t->noReferences() > 0;
        return true;
    }
    return false;
}

Type* SprFrontend::changeRefCount(Type* type, int numRef, const Location& loc)
{
    ASSERT(type);
    
    // If we have a LValue type, remove it
    while ( type->typeId() == Type::typeLValue )
        type = Type::fromBasicType(baseType(type->data_));

    switch ( type->typeId() )
    {
        case Type::typeData:
        {
            type = Type::fromBasicType(getDataType(type->data_->referredNode->as<Class>(), numRef, type->data_->mode));
            break;
        }
        case Type::typeConcept:
        {
            type = Type::fromBasicType(getConceptType(conceptOfType(type->data_), numRef, type->data_->mode));
            break;
        }
        default:
            REP_INTERNAL(loc, "Cannot change reference count for type %1%") % type;
    }
    return type;
}

