#include <StdInc.h>
#include "Conditional.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/FeatherNodes.h>
#include <Util/TypeTraits.h>


Conditional::Conditional(const Location& location, Node* condition, Node* alt1, Node* alt2)
    : Node(location, {condition, alt1, alt2})
{
}

Node* Conditional::condition() const
{
    return children_[0];
}

Node* Conditional::alternative1() const
{
    return children_[1];
}

Node* Conditional::alternative2() const
{
    return children_[2];
}


void Conditional::dump(ostream& os) const
{
    os << "conditional(" << children_[0] << ", " << children_[1] << ", " << children_[2] << ")";
}

void Conditional::doSemanticCheck()
{
    // Semantic check the condition
    children_[0]->semanticCheck();

    // Check that the type of the condition is 'Testable'
    if ( !isTestable(children_[0]) )
        REP_ERROR(children_[0]->location(), "The condition of the conditional expression is not Testable");

    // Dereference the condition as much as possible
    while ( children_[0]->type() && children_[0]->type()->noReferences() > 0 )
    {
        children_[0] = mkMemLoad(children_[0]->location(), children_[0]);
        children_[0]->setContext(childrenContext());
        children_[0]->semanticCheck();
    }
    // TODO (conditional): This shouldn't be performed here

    // Semantic check the alternatives
    children_[1]->semanticCheck();
    children_[2]->semanticCheck();

    // Make sure the types of the alternatives are equal
    if ( !isSameTypeIgnoreMode(children_[1]->type(), children_[2]->type()) )
        REP_ERROR(location_, "The types of the alternatives of a conditional must be equal (%1% != %2%)") % children_[1]->type() % children_[2]->type();

    type_ = adjustMode(children_[1]->type(), children_[0]->type()->mode(), context_, location_);
}
