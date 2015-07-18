#include <StdInc.h>
#include "SprConcept.h"
#include "InstantiationsSet.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <SparrowFrontendTypes.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/TypeTraits.h>

using namespace SprFrontend;
using namespace Feather;

SprConcept::SprConcept(const Location& loc, string name, string paramName, DynNode* baseConcept, DynNode* ifClause, AccessType accessType)
    : DynNode(classNodeKind(), loc, {baseConcept, ifClause, nullptr})
{
    setName(node(), move(name));
    setAccessType(this, accessType);
    setProperty("spr.paramName", move(paramName));
}

bool SprConcept::isFulfilled(TypeRef type)
{
    InstantiationsSet* instantiationsSet = (InstantiationsSet*) data_.children[2];

    if ( !isSemanticallyChecked() || !instantiationsSet )
        REP_INTERNAL(data_.location, "Invalid concept");

    DynNode* typeValue = createTypeNode(data_.context, data_.location, type);
    typeValue->semanticCheck();

    return nullptr != instantiationsSet->canInstantiate({typeValue}, data_.context->evalMode());
}

TypeRef SprConcept::baseConceptType() const
{
    DynNode* baseConcept = (DynNode*) data_.children[0];

    TypeRef res = baseConcept ? getType(baseConcept) : getConceptType();
    res = adjustMode(res, data_.context, data_.location);
    return res;
}

void SprConcept::doSetContextForChildren()
{
    addToSymTab(node());

    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(node()));

    DynNode::doSetContextForChildren();
}

void SprConcept::doSemanticCheck()
{
    ASSERT(data_.children.size() == 3);
    DynNode* baseConcept = (DynNode*) data_.children[0];
    DynNode* ifClause = (DynNode*) data_.children[1];
    Node*& instantiationsSet = data_.children[2];
    const string& paramName = getCheckPropertyString("spr.paramName");

    // Compile the base concept node; make sure it's ct
    if ( baseConcept )
    {
        baseConcept->semanticCheck();
        if ( !isCt(baseConcept->node()) )
            REP_ERROR(baseConcept->location(), "Base concept type needs to be compile-time (type=%1%)") % baseConcept->type();
    }

    DynNode* param = baseConcept
        ? mkSprParameter(data_.location, paramName, baseConcept)
        : mkSprParameter(data_.location, paramName, getConceptType());
    param->setContext(data_.childrenContext);
    param->computeType();       // But not semanticCheck, as it will complain of instantiating a var of type auto

    delete instantiationsSet;
    instantiationsSet = (Node*) new InstantiationsSet(this, { param }, ifClause);
    setExplanation(Feather::mkNop(data_.location));
}
