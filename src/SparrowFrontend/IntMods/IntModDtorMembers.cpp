#include <StdInc.h>
#include "IntModDtorMembers.h"

#include <Nodes/Decls/SprFunction.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

void IntModDtorMembers::beforeSemanticCheck(Node* node)
{
    /// Check to apply only to non-static destructors
    if ( node->nodeKind != nkSparrowDeclSprFunction || getName(node) != "dtor" )
        REP_INTERNAL(node->location, "IntModDtorMembers modifier can be applied only to destructors");
    SprFunction* fun = (SprFunction*) node;
    if ( !fun->hasThisParameters() )
        REP_INTERNAL(node->location, "IntModDtorMembers cannot be applied to static destructors");

    // If we have a body, make sure it's a local space
    if ( !fun->body() )
        return; // nothing to do
    Node* body = fun->body();
    if ( body->nodeKind != nkFeatherLocalSpace )
        REP_INTERNAL(node->location, "Destructor body is not a local space (needed by IntModDtorMembers)");

    // Get the class
    Node* cls = getParentClass(fun->context());
    CHECK(node->location, cls);

    // Generate the dtor calls in reverse order of the fields; add them to the body of the destructor
    CompilationContext* context = Nest::childrenContext(body);
    const Location& loc = body->location;
    for ( Node* field: boost::adaptors::reverse(cls->children) )
    {
        // Make sure we destruct only fields of the current class
        Node* cls2 = getParentClass(field->context);
        if ( cls2 != cls )
            continue;

        if ( field->type->numReferences == 0 )
        {
            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field);
            Nest::setContext(fieldRef, context);
            Node* call = mkOperatorCall(loc, fieldRef, "dtor", nullptr);
            Nest::setContext(call, context);
            body->children.push_back(call);
        }

    }
}
