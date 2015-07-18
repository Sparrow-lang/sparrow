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
    setName(node(), getName(origNode->node()));
    setAccessType(this, accessType);
}

void Generic::doSemanticCheck()
{
    data_.explanation = mkNop(data_.location)->node();
    Nest::setContext(data_.explanation, data_.context);
    Nest::semanticCheck(data_.explanation);
    data_.type = data_.explanation->type;
}

const DynNodeVector& Generic::genericParams() const
{
    return ((InstantiationsSet*) data_.children[0])->parameters();
}

bool SprFrontend::isGeneric(const DynNode* node)
{
    return node->data_.nodeKind == nkSparrowDeclGenericClass
        || node->data_.nodeKind == nkSparrowDeclGenericFunction;
}

size_t SprFrontend::genericParamsCount(const DynNode* node)
{
    switch ( node->data_.nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return static_cast<const GenericClass*>(node)->paramsCount();
    case nkRelSparrowDeclGenericFunction:
        return static_cast<const GenericFunction*>(node)->paramsCount();
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return 0;
    }
}
DynNode* SprFrontend::genericParam(const DynNode* node, size_t idx)
{
    switch ( node->data_.nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return static_cast<const GenericClass*>(node)->param(idx);
    case nkRelSparrowDeclGenericFunction:
        return static_cast<const GenericFunction*>(node)->param(idx);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}

Instantiation* SprFrontend::genericCanInstantiate(DynNode* node, const DynNodeVector& args)
{
    switch ( node->data_.nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return static_cast<GenericClass*>(node)->canInstantiate(args);
    case nkRelSparrowDeclGenericFunction:
        return static_cast<GenericFunction*>(node)->canInstantiate(args);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}
DynNode* SprFrontend::genericDoInstantiate(DynNode* node, const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation)
{
    switch ( node->data_.nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return static_cast<GenericClass*>(node)->instantiateGeneric(loc, context, args, instantiation);
    case nkRelSparrowDeclGenericFunction:
        return static_cast<GenericFunction*>(node)->instantiateGeneric(loc, context, args, instantiation);
    default:
        REP_INTERNAL(node->location(), "Node is not a generic: %1%") % node->nodeKindName();
        return nullptr;
    }
}
