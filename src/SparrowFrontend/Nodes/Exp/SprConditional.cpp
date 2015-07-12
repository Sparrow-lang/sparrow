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
    ASSERT(data_.children.size() == 3);
    DynNode* cond = (DynNode*) data_.children[0];
    DynNode* alt1 = (DynNode*) data_.children[1];
    DynNode* alt2 = (DynNode*) data_.children[2];

    alt1->semanticCheck();
    alt2->semanticCheck();

    TypeRef t1 = alt1->type();
    TypeRef t2 = alt2->type();

    // Get the common type
    TypeRef resType = commonType(data_.context, t1, t2);
    if ( resType == StdDef::typeVoid )
        REP_ERROR(data_.location, "Cannot deduce the result type for a conditional expression (%1%, %2%)") % t1 % t2;
    
    // Convert both types to the result type
    ConversionResult c1  = canConvertType(data_.context, t1, resType);
    ConversionResult c2  = canConvertType(data_.context, t2, resType);

    alt1 = c1.apply(data_.context, alt1);
    alt2 = c2.apply(data_.context, alt2);

    setExplanation(Feather::mkConditional(data_.location, cond, alt1, alt2));
}

