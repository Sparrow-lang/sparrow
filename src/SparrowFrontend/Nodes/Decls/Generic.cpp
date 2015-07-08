#include <StdInc.h>
#include "Generic.h"
#include "GenericClass.h"
#include "GenericFunction.h"
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

Generic::Generic(int nodeKind, DynNode* origNode, DynNodeVector genericParams, DynNode* ifClause, AccessType accessType)
    : DynNode(nodeKind, origNode->location(), { mkInstantiationsSet(origNode, move(genericParams), ifClause) }, { origNode })
{
    setName(this, getName(origNode));
    setAccessType(this, accessType);
}

void Generic::doSemanticCheck()
{
    explanation_ = mkNop(location_);
    explanation_->setContext(context_);
    explanation_->semanticCheck();
    type_ = explanation_->type();
}

const DynNodeVector& Generic::genericParams() const
{
    return children_[0]->as<InstantiationsSet>()->parameters();
}

bool SprFrontend::isGeneric(const DynNode* node)
{
    return node->basicNode_->nodeKind == nkSparrowDeclGenericClass
        || node->basicNode_->nodeKind == nkSparrowDeclGenericFunction;
}

size_t SprFrontend::genericParamsCount(const DynNode* node)
{
    switch ( node->basicNode_->nodeKind )
    {
    case nkSparrowDeclGenericClass:
        return static_cast<const GenericClass*>(node)->paramsCount();
    case nkSparrowDeclGenericFunction:
        return static_cast<const GenericFunction*>(node)->paramsCount();
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return 0;
    }
}
DynNode* SprFrontend::genericParam(const DynNode* node, size_t idx)
{
    switch ( node->basicNode_->nodeKind )
    {
    case nkSparrowDeclGenericClass:
        return static_cast<const GenericClass*>(node)->param(idx);
    case nkSparrowDeclGenericFunction:
        return static_cast<const GenericFunction*>(node)->param(idx);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}

Instantiation* SprFrontend::genericCanInstantiate(DynNode* node, const DynNodeVector& args)
{
    switch ( node->basicNode_->nodeKind )
    {
    case nkSparrowDeclGenericClass:
        return static_cast<GenericClass*>(node)->canInstantiate(args);
    case nkSparrowDeclGenericFunction:
        return static_cast<GenericFunction*>(node)->canInstantiate(args);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}
DynNode* SprFrontend::genericDoInstantiate(DynNode* node, const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation)
{
    switch ( node->basicNode_->nodeKind )
    {
    case nkSparrowDeclGenericClass:
        return static_cast<GenericClass*>(node)->instantiateGeneric(loc, context, args, instantiation);
    case nkSparrowDeclGenericFunction:
        return static_cast<GenericFunction*>(node)->instantiateGeneric(loc, context, args, instantiation);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}
