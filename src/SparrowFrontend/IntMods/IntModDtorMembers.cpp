#include <StdInc.h>
#include "IntModDtorMembers.h"

#include <Nodes/Decls/SprFunction.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/LocalSpace.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

void IntModDtorMembers::beforeSemanticCheck(Node* n)
{
    DynNode* node = (DynNode*) n;
    
    /// Check to apply only to non-static destructors
    SprFunction* fun = node->as<SprFunction>();
    if ( !fun || getName(fun->node()) != "dtor" )
        REP_INTERNAL(node->location(), "IntModDtorMembers modifier can be applied only to destructors");
    if ( !fun->hasThisParameters() )
        REP_INTERNAL(node->location(), "IntModDtorMembers cannot be applied to static destructors");

    // If we have a body, make sure it's a local space
    if ( !fun->body() )
        return; // nothing to do
    LocalSpace* body = fun->body()->as<LocalSpace>();
    if ( !body )
        REP_INTERNAL(node->location(), "Destructor body is not a local space (needed by IntModDtorMembers)");

    // Get the class
    Class* cls = getParentClass(fun->context());
    CHECK(node->location(), cls);

    // Generate the dtor calls in reverse order of the fields; add them to the body of the destructor
    CompilationContext* context = body->childrenContext();
    const Location& loc = body->location();
    for ( DynNode* field: boost::adaptors::reverse(cls->fields()) )
    {
        // Make sure we destruct only fields of the current class
        Class* cls2 = getParentClass(field->context());
        if ( cls2 != cls )
            continue;

        if ( field->type()->numReferences == 0 )
        {
            DynNode* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field);
            fieldRef->setContext(context);
            DynNode* call = mkOperatorCall(loc, fieldRef, "dtor", nullptr);
            call->setContext(context);
            body->addChild(call);
        }

    }
}
