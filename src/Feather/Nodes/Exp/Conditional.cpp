#include <StdInc.h>
#include "Conditional.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/FeatherNodes.h>
#include <Util/TypeTraits.h>


Conditional::Conditional(const Location& location, DynNode* condition, DynNode* alt1, DynNode* alt2)
    : DynNode(classNodeKind(), location, {condition, alt1, alt2})
{
}

DynNode* Conditional::condition() const
{
    return (DynNode*) data_.children[0];
}

DynNode* Conditional::alternative1() const
{
    return (DynNode*) data_.children[1];
}

DynNode* Conditional::alternative2() const
{
    return (DynNode*) data_.children[2];
}


void Conditional::dump(ostream& os) const
{
    os << "conditional(" << data_.children[0] << ", " << data_.children[1] << ", " << data_.children[2] << ")";
}

void Conditional::doSemanticCheck()
{
    // Semantic check the condition
    Nest::semanticCheck(data_.children[0]);

    // Check that the type of the condition is 'Testable'
    if ( !isTestable(data_.children[0]) )
        REP_ERROR(data_.children[0]->location, "The condition of the conditional expression is not Testable");

    // Dereference the condition as much as possible
    while ( data_.children[0]->type && data_.children[0]->type->numReferences > 0 )
    {
        data_.children[0] = mkMemLoad(data_.children[0]->location, data_.children[0]);
        Nest::setContext(data_.children[0], childrenContext());
        Nest::semanticCheck(data_.children[0]);
    }
    // TODO (conditional): This shouldn't be performed here

    // Semantic check the alternatives
    Nest::semanticCheck(data_.children[1]);
    Nest::semanticCheck(data_.children[2]);

    // Make sure the types of the alternatives are equal
    if ( !isSameTypeIgnoreMode(data_.children[1]->type, data_.children[2]->type) )
        REP_ERROR(data_.location, "The types of the alternatives of a conditional must be equal (%1% != %2%)") % data_.children[1]->type % data_.children[2]->type;

    data_.type = adjustMode(data_.children[1]->type, data_.children[0]->type->mode, data_.context, data_.location);
}
