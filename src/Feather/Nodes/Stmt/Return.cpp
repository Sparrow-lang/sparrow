#include <StdInc.h>
#include "Return.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Nodes/Decls/Function.h>
#include <Util/Context.h>
#include <Util/TypeTraits.h>


Return::Return(const Location& location, DynNode* exp)
    : DynNode(classNodeKind(), location, {exp})
{
    setProperty("parentFun", (DynNode*) nullptr);
}

DynNode* Return::expression() const
{
    return (DynNode*) data_.children[0];
}

Function* Return::parentFun() const
{
    return (Function*) getCheckPropertyNode("parentFun");
}

void Return::dump(ostream& os) const
{
    os << "return(" << data_.children[0] << ")";
}

void Return::doSemanticCheck()
{
    // If we have an expression argument, semantically check it
    if ( data_.children[0] )
        Nest::semanticCheck(data_.children[0]);

    // Get the parent function of this return
    Function* parentFun = getParentFun(data_.context);
    if ( !parentFun )
        REP_ERROR(data_.location, "Return found outside any function");
    ASSERT(parentFun->resultType());
    setProperty("parentFun", parentFun);

    // If the return has an expression, check that has the same type as the function result type
    if ( data_.children[0] )
    {
        if ( !isSameTypeIgnoreMode(data_.children[0]->type, parentFun->resultType()) )
            REP_ERROR(data_.location, "Returned expression's type is not the same as function's return type");
    }
    else
    {
        // Make sure that the function has a void return type
        if ( parentFun->resultType()->typeKind != typeKindVoid )
            REP_ERROR(data_.location, "You must return something in a function that has non-Void result type");
    }

    // The resulting type is Void
    data_.type = getVoidType(data_.context->evalMode());
}
