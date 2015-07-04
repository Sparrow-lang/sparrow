#include <StdInc.h>
#include "TypeTraits.h"

#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/Decl.h>

#include <Feather/Type/Void.h>
#include <Feather/Type/DataType.h>
#include <Feather/Type/LValueType.h>
#include <Feather/Type/ArrayType.h>
#include <Feather/Type/FunctionType.h>


#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest;

bool Feather::isCt(Type* type)
{
    return type && type->mode() == modeCt;
}

bool Feather::isCt(Node* node)
{
    return isCt(node->type());
}
bool Feather::isCt(const vector<Nest::Type*>& types)
{
    for ( Type* t: types )
        if ( !isCt(t) )
            return false;
    return true;
}
bool Feather::isCt(const NodeVector& nodes)
{
    for ( Node* n: nodes )
    {
        if ( !n->type() )
            n->computeType();
        if ( !isCt(n->type()) )
            return false;
    }
    return true;
}

bool Feather::isTestable(Type* type)
{
    // If not Testable, check at least that is some kind of boolean
    if ( !type || !type->hasStorage() )
        return false;
    StorageType* storageType = static_cast<StorageType*>(type);
    const string* nativeName = storageType->nativeName();
    return nativeName && (*nativeName == "i1" || *nativeName == "u1");
}

bool Feather::isTestable(Node* node)
{
    return isTestable(node->type());
}

bool Feather::isInteger(Type* type)
{
    if ( !type || type->typeId() != Type::typeData )
        return false;
    DataType* dataType = static_cast<DataType*>(type);
    const string* nativeName = dataType->nativeName();
    return nativeName && (*nativeName == "i32" || *nativeName == "u32");
}

bool Feather::isInteger(Node* node)
{
    return isInteger(node->type());
}

bool Feather::isBasicNumericType(Type* type)
{
    if ( !type || !type->hasStorage() )
        return false;
    DataType* dataType = static_cast<DataType*>(type);
    const string* nativeName = dataType->nativeName();
    return nativeName && (
        *nativeName == "i1" || *nativeName == "u1" || 
        *nativeName == "i8" || *nativeName == "u8" || 
        *nativeName == "i16" || *nativeName == "u16" || 
        *nativeName == "i32" || *nativeName == "u32" || 
        *nativeName == "i64" || *nativeName == "u64" || 
        *nativeName == "float" || *nativeName == "double"
        );
}

Type* Feather::changeTypeMode(Type* type, EvalMode mode, const Location& loc)
{
    ASSERT(type);
    if ( mode == type->mode() )
        return type;

    Type* resType = nullptr;
    switch ( type->typeId() )
    {
        case typeVoid:
            resType = Void::get(mode);
            break;
        case typeData:
            resType = DataType::get(static_cast<DataType*>(type)->classDecl(), type->data_->numReferences, mode);
            break;
        case typeLValue:
        {
            Type* newElementType = changeTypeMode(static_cast<LValueType*>(type)->baseType(), mode, loc);
            resType = newElementType ? LValueType::get(newElementType) : nullptr;
            break;
        }
        case typeArray:
        {
            ArrayType* t = static_cast<ArrayType*>(type);
            Type* newUnitType = changeTypeMode(t->unitType(), mode, loc);
            resType = newUnitType ? ArrayType::get((StorageType*) newUnitType, t->count()) : nullptr;
            break;
        }
        case typeFunction:
        {
            FunctionType* t = static_cast<FunctionType*>(type);
            resType = FunctionType::get(t->resultType(), t->paramTypes(), mode);
            break;
        }
//        case typeConcept:
        default:
        {
            // Just switch the flags in the type
            TypeData t = *type->data_;
            t.mode = mode;
            resType = Type::fromBasicType(getStockType(t));
            // TODO (type): This is ugly; need to do it dynamically
            // We also need to to update description
        }
    }
    if ( !resType )
        REP_INTERNAL(loc, "Don't know how to change eval mode of type %1%") % type->toString();

    if ( mode == modeCt && resType->mode() != modeCt )
        REP_ERROR(loc, "Type '%1%' cannot be used at compile-time") % type;
    if ( mode == modeRt && resType->mode() != modeRt )
        REP_ERROR(loc, "Type '%1%' cannot be used at run-time") % type;

    return resType;
}

Type* Feather::addRef(Type* type)
{
    ASSERT(type);
    if ( !type->hasStorage() )
        REP_INTERNAL(Location(), "Invalid type given when adding reference (%1%)") % type->toString();
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences()+1, t->mode());
}

Type* Feather::removeRef(Type* type)
{
    ASSERT(type);
    if ( !type->hasStorage() || type->noReferences() < 1 )
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type->toString();
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences()-1, t->mode());
}

Type* Feather::removeAllRef(Type* type)
{
    ASSERT(type);
    if ( !type->hasStorage() )
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type->toString();
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), 0, t->mode());
}

Type* Feather::removeLValue(Type* type)
{
    ASSERT(type);
    if ( type->typeId() != Type::typeLValue )
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type->toString();
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences()-1, t->mode());
}

Type* Feather::removeLValueIfPresent(Type* type)
{
    ASSERT(type);
    if ( type->typeId() != Type::typeLValue )
        return type;
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences()-1, t->mode());
}

Type* Feather::lvalueToRef(Type* type)
{
    ASSERT(type);
    if ( type->typeId() != Type::typeLValue )
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type->toString();
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences(), t->mode());
}

Type* Feather::lvalueToRefIfPresent(Type* type)
{
    ASSERT(type);
    if ( type->typeId() != Type::typeLValue )
        return type;
    StorageType* t = static_cast<StorageType*>(type);
    return DataType::get(t->classDecl(), t->noReferences(), t->mode());
}

Class* Feather::classForType(Nest::Type* t)
{
    return t->hasStorage() ? static_cast<StorageType*>(t)->classDecl() : nullptr;
}

Node* Feather::classForTypeRaw(Nest::Type* t)
{
    return t->hasStorage() ? static_cast<StorageType*>(t)->classDecl() : nullptr;
}

bool Feather::isSameTypeIgnoreMode(Nest::Type* t1, Nest::Type* t2)
{
    ASSERT(t1);
    ASSERT(t2);
    if ( t1 == t2 )
        return true;
    if ( t1->typeId() != t2->typeId() || t1->mode() == t2->mode() )
        return false;
    Type* t = Feather::changeTypeMode(t1, t2->mode());
    return t == t2;
}

EvalMode Feather::combineMode(EvalMode mode, EvalMode baseMode, const Location& loc, bool forceBase)
{
    switch ( baseMode )
    {
    case modeRt:
        if ( forceBase )
            return modeRt;
        else
            return mode == modeRtCt ? modeRt : mode;
    case modeCt:
        if ( mode == modeRt )
            REP_ERROR(loc, "Cannot use the RT mode inside of a CT context");
        return modeCt;
    case modeRtCt:
    default:
        return mode;
    }
}

Nest::Type* Feather::adjustMode(Nest::Type* srcType, CompilationContext* context, const Location& loc)
{
    ASSERT(srcType);
    ASSERT(context);
    EvalMode resMode = combineMode(srcType->mode(), context->evalMode(), loc);
    return changeTypeMode(srcType, resMode, loc);
}

Nest::Type* Feather::adjustMode(Nest::Type* srcType, Nest::EvalMode baseMode, CompilationContext* context, const Location& loc)
{
    ASSERT(srcType);
    ASSERT(context);
    baseMode = combineMode(baseMode, context->evalMode(), loc);
    EvalMode resMode = combineMode(srcType->mode(), baseMode, loc, true);
    return changeTypeMode(srcType, resMode, loc);
}

void Feather::checkEvalMode(Node* src, Nest::EvalMode referencedEvalMode)
{
    ASSERT(src && src->type());
    EvalMode nodeEvalMode = src->type()->mode();
    EvalMode contextEvalMode = src->context()->evalMode();

    // Check if the context eval mode requirements are fulfilled
    if ( contextEvalMode == modeRtCt && nodeEvalMode == modeRt )
        REP_INTERNAL(src->location(), "Cannot have RT nodes in a RT-CT context");
    if ( contextEvalMode == modeCt && nodeEvalMode != modeCt )
        REP_INTERNAL(src->location(), "Cannot have non-CT nodes in a CT context");

    // Check if the referenced eval mode requirements are fulfilled
    if ( referencedEvalMode == modeCt && nodeEvalMode != modeCt )
        REP_INTERNAL(src->location(), "CT node required; found: %1%") % nodeEvalMode;
    if ( referencedEvalMode == modeRt && nodeEvalMode != modeRt )
        REP_INTERNAL(src->location(), "RT node required; found: %1%") % nodeEvalMode;

    // If the node has direct children, check them
    const NodeVector& children = src->children();
    if ( !children.empty() )
    {
        // If we have a CT eval mode, then all the children must be CT
        if ( nodeEvalMode == modeCt )
        {
            for ( Node* child: children )
            {
                if ( !child || !child->type() )
                    continue;

                // Ignore declarations
                if ( isDecl(child) )
                    continue;

                if ( child->type()->mode() != modeCt )
                    REP_INTERNAL(child->location(), "Children of a CT node must be CT; current mode: %1% (%2%)") % child->type()->mode() % child;
            }
        }
        // If we have a RT-CT eval mode, then no children must be RT
        if ( nodeEvalMode == modeRtCt )
        {
            for ( Node* child: children )
            {
                if ( !child || !child->type() )
                    continue;

                // Ignore declarations
                if ( isDecl(child) )
                    continue;

                if ( child->type() && child->type()->mode() == modeRt )
                    REP_INTERNAL(child->location(), "Children of a RT-CT node must not be RT; current mode: %1%") % child->type()->mode();
            }
        }
    }
}
