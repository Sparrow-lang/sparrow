#include <StdInc.h>
#include "SprConditional.h"
#include <NodeCommonsCpp.h>
#include <Helpers/Convert.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>

using namespace SprFrontend;

SprConditional::SprConditional(const Location& location, DynNode* condition, DynNode* alt1, DynNode* alt2)
    : DynNode(classNodeKind(), location, {condition, alt1, alt2})
{
}

void SprConditional::doSemanticCheck()
{
    ASSERT(children_.size() == 3);
    DynNode* cond = children_[0];
    DynNode* alt1 = children_[1];
    DynNode* alt2 = children_[2];

    alt1->semanticCheck();
    alt2->semanticCheck();

    TypeRef t1 = alt1->type();
    TypeRef t2 = alt2->type();

    // Get the common type
    TypeRef resType = commonType(context_, t1, t2);
    if ( resType == StdDef::typeVoid )
        REP_ERROR(location_, "Cannot deduce the result type for a conditional expression (%1%, %2%)") % t1 % t2;
    
    // Convert both types to the result type
    ConversionResult c1  = canConvertType(context_, t1, resType);
    ConversionResult c2  = canConvertType(context_, t2, resType);

    alt1 = c1.apply(context_, alt1);
    alt2 = c2.apply(context_, alt2);

    setExplanation(Feather::mkConditional(location_, cond, alt1, alt2));
}

