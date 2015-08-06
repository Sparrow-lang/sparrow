#include <StdInc.h>
#include "TypeTraits.h"

#include <Feather/Util/Decl.h>
#include <Feather/FeatherTypes.h>

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/CompilationContext.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest;

bool Feather::isCt(TypeRef type)
{
    return type && type->mode == modeCt;
}

bool Feather::isCt(Node* node)
{
    return isCt(node->type);
}
bool Feather::isCt(const vector<Nest::TypeRef>& types)
{
    for ( TypeRef t: types )
        if ( !isCt(t) )
            return false;
    return true;
}
bool Feather::isCt(const NodeVector& nodes)
{
    for ( Node* n: nodes )
    {
        if ( !n->type )
            Nest::computeType(n);
        if ( !isCt(n->type) )
            return false;
    }
    return true;
}

bool Feather::isTestable(TypeRef type)
{
    // If not Testable, check at least that is some kind of boolean
    if ( !type || !type->hasStorage )
        return false;
    const string* nativeName = Feather::nativeName(type);
    return nativeName && (*nativeName == "i1" || *nativeName == "u1");
}

bool Feather::isTestable(Node* node)
{
    return isTestable(node->type);
}

bool Feather::isInteger(TypeRef type)
{
    if ( !type || type->typeKind != typeKindData )
        return false;
    const string* nativeName = Feather::nativeName(type);
    return nativeName && (*nativeName == "i32" || *nativeName == "u32");
}

bool Feather::isInteger(Node* node)
{
    return isInteger(node->type);
}

bool Feather::isBasicNumericType(TypeRef type)
{
    if ( !type || !type->hasStorage )
        return false;
    const string* nativeName = Feather::nativeName(type);
    return nativeName && (
        *nativeName == "i1" || *nativeName == "u1" || 
        *nativeName == "i8" || *nativeName == "u8" || 
        *nativeName == "i16" || *nativeName == "u16" || 
        *nativeName == "i32" || *nativeName == "u32" || 
        *nativeName == "i64" || *nativeName == "u64" || 
        *nativeName == "float" || *nativeName == "double"
        );
}

TypeRef Feather::changeTypeMode(TypeRef type, EvalMode mode, const Location& loc)
{
    ASSERT(type);
    if ( mode == type->mode )
        return type;

    TypeRef resType = Nest::changeTypeMode(type, mode);
    if ( !resType )
        REP_INTERNAL(loc, "Don't know how to change eval mode of type %1%") % type;

    if ( mode == modeCt && resType->mode != modeCt )
        REP_ERROR(loc, "Type '%1%' cannot be used at compile-time") % type;
    if ( mode == modeRt && resType->mode != modeRt )
        REP_ERROR(loc, "Type '%1%' cannot be used at run-time") % type;

    return resType;
}

TypeRef Feather::addRef(TypeRef type)
{
    ASSERT(type);
    if ( !type->hasStorage )
        REP_INTERNAL(Location(), "Invalid type given when adding reference (%1%)") % type;
    return getDataType(type->referredNode, type->numReferences+1, type->mode);
}

TypeRef Feather::removeRef(TypeRef type)
{
    ASSERT(type);
    if ( !type->hasStorage || type->numReferences < 1 )
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type;
    return getDataType(type->referredNode, type->numReferences-1, type->mode);
}

TypeRef Feather::removeAllRef(TypeRef type)
{
    ASSERT(type);
    if ( !type->hasStorage )
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type;
    return getDataType(type->referredNode, 0, type->mode);
}

TypeRef Feather::removeLValue(TypeRef type)
{
    ASSERT(type);
    if ( type->typeKind != typeKindLValue )
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type;
    return getDataType(type->referredNode, type->numReferences-1, type->mode);
}

TypeRef Feather::removeLValueIfPresent(TypeRef type)
{
    ASSERT(type);
    if ( type->typeKind != typeKindLValue )
        return type;
    return getDataType(type->referredNode, type->numReferences-1, type->mode);
}

TypeRef Feather::lvalueToRef(TypeRef type)
{
    ASSERT(type);
    if ( type->typeKind != typeKindLValue )
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type;
    return getDataType(type->referredNode, type->numReferences, type->mode);
}

TypeRef Feather::lvalueToRefIfPresent(TypeRef type)
{
    ASSERT(type);
    if ( type->typeKind != typeKindLValue )
        return type;
    return getDataType(type->referredNode, type->numReferences, type->mode);
}

Node* Feather::classForType(Nest::TypeRef t)
{
    return t->hasStorage ? t->referredNode : nullptr;
}

bool Feather::isSameTypeIgnoreMode(Nest::TypeRef t1, Nest::TypeRef t2)
{
    ASSERT(t1);
    ASSERT(t2);
    if ( t1 == t2 )
        return true;
    if ( t1->typeKind != t2->typeKind || t1->mode == t2->mode )
        return false;
    TypeRef t = Feather::changeTypeMode(t1, t2->mode);
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

Nest::TypeRef Feather::adjustMode(Nest::TypeRef srcType, CompilationContext* context, const Location& loc)
{
    ASSERT(srcType);
    ASSERT(context);
    EvalMode resMode = combineMode(srcType->mode, context->evalMode(), loc);
    return changeTypeMode(srcType, resMode, loc);
}

Nest::TypeRef Feather::adjustMode(Nest::TypeRef srcType, Nest::EvalMode baseMode, CompilationContext* context, const Location& loc)
{
    ASSERT(srcType);
    ASSERT(context);
    baseMode = combineMode(baseMode, context->evalMode(), loc);
    EvalMode resMode = combineMode(srcType->mode, baseMode, loc, true);
    return changeTypeMode(srcType, resMode, loc);
}

void Feather::checkEvalMode(Node* src, Nest::EvalMode referencedEvalMode)
{
    ASSERT(src && src->type);
    EvalMode nodeEvalMode = src->type->mode;
    EvalMode contextEvalMode = src->context->evalMode();

    // Check if the context eval mode requirements are fulfilled
    if ( contextEvalMode == modeRtCt && nodeEvalMode == modeRt )
        REP_INTERNAL(src->location, "Cannot have RT nodes in a RT-CT context");
    if ( contextEvalMode == modeCt && nodeEvalMode != modeCt )
        REP_INTERNAL(src->location, "Cannot have non-CT nodes in a CT context");

    // Check if the referenced eval mode requirements are fulfilled
    if ( referencedEvalMode == modeCt && nodeEvalMode != modeCt )
        REP_INTERNAL(src->location, "CT node required; found: %1%") % nodeEvalMode;
    if ( referencedEvalMode == modeRt && nodeEvalMode != modeRt )
        REP_INTERNAL(src->location, "RT node required; found: %1%") % nodeEvalMode;

    // If the node has direct children, check them
    const NodeVector& children = src->children;
    if ( !children.empty() )
    {
        // If we have a CT eval mode, then all the children must be CT
        if ( nodeEvalMode == modeCt )
        {
            for ( Node* child: children )
            {
                if ( !child || !child->type )
                    continue;

                // Ignore declarations
                if ( isDecl(child) )
                    continue;

                if ( child->type->mode != modeCt )
                    REP_INTERNAL(child->location, "Children of a CT node must be CT; current mode: %1% (%2%)") % child->type->mode % child;
            }
        }
        // If we have a RT-CT eval mode, then no children must be RT
        if ( nodeEvalMode == modeRtCt )
        {
            for ( Node* child: children )
            {
                if ( !child || !child->type )
                    continue;

                // Ignore declarations
                if ( isDecl(child) )
                    continue;

                if ( child->type && child->type->mode == modeRt )
                    REP_INTERNAL(child->location, "Children of a RT-CT node must not be RT; current mode: %1%") % child->type->mode;
            }
        }
    }
}
