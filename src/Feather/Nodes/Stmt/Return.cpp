#include <StdInc.h>
#include "Return.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Function.h>
#include <Util/Context.h>
#include <Util/TypeTraits.h>


Return::Return(const Location& location, Node* exp)
    : Node(location, {exp})
{
    setProperty("parentFun", (Node*) nullptr);
}

Node* Return::expression() const
{
    return children_[0];
}

Function* Return::parentFun() const
{
    return (Function*) getCheckPropertyNode("parentFun");
}

void Return::dump(ostream& os) const
{
    os << "return(" << children_[0] << ")";
}

void Return::doSemanticCheck()
{
    // If we have an expression argument, semantically check it
    if ( children_[0] )
        children_[0]->semanticCheck();

    // Get the parent function of this return
    Function* parentFun = getParentFun(context_);
    if ( !parentFun )
        REP_ERROR(location_, "Return found outside any function");
    ASSERT(parentFun->resultType());
    setProperty("parentFun", parentFun);

    // If the return has an expression, check that has the same type as the function result type
    if ( children_[0] )
    {
        if ( !isSameTypeIgnoreMode(children_[0]->type(), parentFun->resultType()) )
            REP_ERROR(location_, "Returned expression's type is not the same as function's return type");
    }
    else
    {
        // Make sure that the function has a void return type
        if ( parentFun->resultType()->typeId() != Type::typeVoid )
            REP_ERROR(location_, "You must return something in a function that has non-Void result type");
    }

    // The resulting type is Void
    type_ = Type::fromBasicType(getVoidType(context_->evalMode()));
}
