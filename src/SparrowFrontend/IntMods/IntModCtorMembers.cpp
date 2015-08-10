#include <StdInc.h>
#include "IntModCtorMembers.h"

#include <Nodes/Decls/SprFunction.h>
#include <Nodes/Builder.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    /// Search the given body for a constructor with the given properties.
    ///
    /// This can search for constructors of given classes, constructors called on this, or called for the given field.
    ///
    /// It will search only the instructions directly inside the given local space, or in a child local space
    /// It will not search inside conditionals, or other instructions
    bool hasCtorCall(Node* inSpace, Node* ofClass, bool checkThis, Node* forField)
    {
        // Check all the items in the local space
        for ( Node* n: inSpace->children )
        {
            Nest::computeType(n);
            n = Nest::explanation(n);
            if ( !n )
                continue;

            // Check inner local spaces
            if ( n->nodeKind == nkFeatherLocalSpace )
            {
                if ( hasCtorCall(n, ofClass, checkThis, forField) )
                    return true;
                continue;
            }

            // We consider function calls for our checks
            if ( n->nodeKind != nkFeatherExpFunCall )
                continue;
            if ( getName(n->referredNodes[0]) != "ctor" )
                continue;
            if ( n->children.empty() )
                continue;
            Node* thisArg = n->children[0];

            // If a class is given, check that the call is made to a function of that class
            if ( ofClass )
            {
                Node* parentCls = getParentClass(n->referredNodes[0]->context);
                if ( parentCls != ofClass )
                    continue;
            }

            // Check for this to be passed as argument
            if ( checkThis )
            {
                // If we have a MemLoad, just ignore it
                if ( thisArg->nodeKind == nkFeatherExpMemLoad )
                    thisArg = explanation(thisArg->children[0]);

                if ( !thisArg || thisArg->nodeKind != nkFeatherExpVarRef
                    || getName(thisArg->referredNodes[0]) != "$this" )
                    continue;
            }

            // If a field is given, check that the this argument is a reference to our field
            if ( forField )
            {
                // If we have a Bitcast, just ignore it
                if ( thisArg->nodeKind == nkFeatherExpBitcast )
                    thisArg = explanation(thisArg->children[0]);

                if ( !thisArg || thisArg->nodeKind != nkFeatherExpFieldRef
                    || thisArg->referredNodes[0] != forField )
                    continue;
            }

            return true;
        }
        return false;
    }
}

void IntModCtorMembers::beforeSemanticCheck(Node* node)
{
    /// Check to apply only to non-static constructors
    if ( node->nodeKind != nkSparrowDeclSprFunction || getName(node) != "ctor" )
        REP_INTERNAL(node->location, "IntModCtorMembers modifier can be applied only to constructors");
    SprFunction* fun = (SprFunction*) node;
    if ( !fun->hasThisParameters() )
        REP_INTERNAL(node->location, "IntModCtorMembers cannot be applied to static constructors");

    // If we have a body, make sure it's a local space
    if ( !fun->body() )
        return; // nothing to do
    Node* body = fun->body();
    if ( body->nodeKind != nkFeatherLocalSpace )
        REP_INTERNAL(node->location, "Constructor body is not a local space (needed by IntModCtorMembers)");

    // Get the class
    Node* cls = getParentClass(fun->context());
    CHECK(node->location, cls);

    // If we are calling other constructor of this class, don't add any initialization
    if ( hasCtorCall(body, cls, true, nullptr) )
        return;

    // Generate the ctor calls in the order of the fields; add them to the body of the constructor
    const Location& loc = body->location;
    for ( Node* field: boost::adaptors::reverse(cls->children) )
    {
        // Make sure we initialize only fields of the current class
        Node* cls2 = getParentClass(field->context);
        if ( cls2 != cls )
            continue;

        if ( !hasCtorCall(body, nullptr, false, field) )
        {
            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field);
            Node* call = nullptr;
            if ( field->type->numReferences == 0 )
            {
                call = mkOperatorCall(loc, fieldRef, "ctor", nullptr);
            }
            else
            {
                call = mkOperatorCall(loc, fieldRef, ":=", buildNullLiteral(loc));
            }
            Nest::setContext(call, Nest::childrenContext(body));
            body->children.insert(body->children.begin(), call);
        }
    }
}
