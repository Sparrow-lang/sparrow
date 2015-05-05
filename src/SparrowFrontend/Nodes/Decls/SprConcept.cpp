#include <StdInc.h>
#include "SprConcept.h"
#include "InstantiationsSet.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Type/ConceptType.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/TypeTraits.h>

using namespace SprFrontend;
using namespace Feather;

SprConcept::SprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType)
    : Node(loc, {baseConcept, ifClause, nullptr})
{
    setName(this, move(name));
    setAccessType(this, accessType);
    setProperty("spr.paramName", move(paramName));
}

bool SprConcept::isFulfilled(Type* type)
{
    InstantiationsSet* instantiationsSet = children_[2]->as<InstantiationsSet>();

    if ( !isSemanticallyChecked() || !instantiationsSet )
        REP_INTERNAL(location_, "Invalid concept");

    Node* typeValue = createTypeNode(context_, location_, type);
    typeValue->semanticCheck();

    return nullptr != instantiationsSet->canInstantiate({typeValue}, context_->evalMode());
}

Type* SprConcept::baseConceptType() const
{
    Node* baseConcept = children_[0];

    Type* res = baseConcept ? getType(baseConcept) : ConceptType::get();
    res = adjustMode(res, context_, location_);
    return res;
}

void SprConcept::doSetContextForChildren()
{
    addToSymTab(this);

    if ( !childrenContext_ )
        childrenContext_ = context_->createChildContext(this, effectiveEvalMode(this));

    Node::doSetContextForChildren();
}

void SprConcept::doSemanticCheck()
{
    ASSERT(children_.size() == 3);
    Node* baseConcept = children_[0];
    Node* ifClause = children_[1];
    Node*& instantiationsSet = children_[2];
    const string& paramName = getCheckPropertyString("spr.paramName");

    // Compile the base concept node; make sure it's ct
    if ( baseConcept )
    {
        baseConcept->semanticCheck();
        if ( !isCt(baseConcept) )
            REP_ERROR(baseConcept->location(), "Base concept type needs to be compile-time (type=%1%)") % baseConcept->type();
    }

    Node* param = baseConcept
        ? mkSprParameter(location_, paramName, baseConcept)
        : mkSprParameter(location_, paramName, ConceptType::get());
    param->setContext(childrenContext_);
    param->computeType();       // But not semanticCheck, as it will complain of instantiating a var of type auto

    delete instantiationsSet;
    instantiationsSet = new InstantiationsSet(this, { param }, ifClause);
    setExplanation(Feather::mkNop(location_));
}
