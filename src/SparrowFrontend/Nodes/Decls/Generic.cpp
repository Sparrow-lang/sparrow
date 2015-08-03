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
    : DynNode(nodeKind, origNode->location(), { (DynNode*) mkInstantiationsSet(origNode->node(), fromDyn(move(genericParams)), ifClause->node()) }, { origNode })
{
    setName(node(), getName(origNode->node()));
    setAccessType(this, accessType);
}

void Generic::doSemanticCheck()
{
    data_.explanation = mkNop(data_.location);
    Nest::setContext(data_.explanation, data_.context);
    Nest::semanticCheck(data_.explanation);
    data_.type = data_.explanation->type;
}

const DynNodeVector& Generic::genericParams() const
{
    return ((InstantiationsSet*) data_.children[0])->parameters();
}

bool SprFrontend::isGeneric(const Node* node)
{
    return node->nodeKind == nkSparrowDeclGenericClass
        || node->nodeKind == nkSparrowDeclGenericFunction;
}

size_t SprFrontend::genericParamsCount(const Node* node)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return reinterpret_cast<const GenericClass*>(node)->paramsCount();
    case nkRelSparrowDeclGenericFunction:
        return reinterpret_cast<const GenericFunction*>(node)->paramsCount();
    default:
        REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
        return 0;
    }
}
Node* SprFrontend::genericParam(const Node* node, size_t idx)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return reinterpret_cast<const GenericClass*>(node)->param(idx)->node();
    case nkRelSparrowDeclGenericFunction:
        return reinterpret_cast<const GenericFunction*>(node)->param(idx)->node();
    default:
        REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
        return nullptr;
    }
}

Instantiation* SprFrontend::genericCanInstantiate(Node* node, const NodeVector& args)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return reinterpret_cast<GenericClass*>(node)->canInstantiate(toDyn(args));
    case nkRelSparrowDeclGenericFunction:
        return reinterpret_cast<GenericFunction*>(node)->canInstantiate(toDyn(args));
    default:
        REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
        return nullptr;
    }
}
Node* SprFrontend::genericDoInstantiate(Node* node, const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation)
{
    switch ( node->nodeKind - firstSparrowNodeKind )
    {
    case nkRelSparrowDeclGenericClass:
        return reinterpret_cast<GenericClass*>(node)->instantiateGeneric(loc, context, toDyn(args), instantiation)->node();
    case nkRelSparrowDeclGenericFunction:
        return reinterpret_cast<GenericFunction*>(node)->instantiateGeneric(loc, context, toDyn(args), instantiation)->node();
    default:
        REP_INTERNAL(node->location, "Node is not a generic: %1%") % node;
        return nullptr;
    }
}
