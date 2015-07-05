#include <StdInc.h>
#include "IntModCtorMembers.h"

#include <Nodes/Decls/SprFunction.h>
#include <Helpers/DeclsHelpers.h>
#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Nodes/Exp/FunCall.h>
#include <Feather/Nodes/Exp/VarRef.h>
#include <Feather/Nodes/Exp/FieldRef.h>
#include <Feather/Nodes/Exp/Bitcast.h>
#include <Feather/Nodes/Exp/MemLoad.h>
#include <Feather/Nodes/LocalSpace.h>
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
    bool hasCtorCall(LocalSpace* inSpace, Class* ofClass, bool checkThis, Node* forField)
    {
        // Check all the items in the local space
        for ( Node* n: inSpace->children() )
        {
            n->computeType();
            n = n->explanation();
            if ( !n )
                continue;

            // Check inner local spaces
            LocalSpace* ls = n->as<LocalSpace>();
            if ( ls )
            {
                if ( hasCtorCall(ls, ofClass, checkThis, forField) )
                    return true;
                continue;
            }

            // We consider function calls for our checks
            FunCall* funCall = n->as<FunCall>();
            if ( !funCall || getName(funCall->funDecl()) != "ctor" )
                continue;
            if ( funCall->arguments().empty() )
                continue;
            Node* thisArg = funCall->arguments()[0];

            // If a class is given, check that the call is made to a function of that class
            if ( ofClass )
            {
                Class* parentCls = getParentClass(funCall->funDecl()->context());
                if ( parentCls != ofClass )
                    continue;
            }

            // Check for this to be passed as argument
            if ( checkThis )
            {
                // If we have a MemLoad, just ignore it
                MemLoad* ml = thisArg->as<MemLoad>();
                if ( ml )
                    thisArg = ml->argument()->explanation();

                VarRef* varRef = thisArg->as<VarRef>();
                if ( !varRef || getName(varRef->variable()) != "$this" )
                    continue;
            }

            // If a field is given, check that the this argument is a reference to our field
            if ( forField )
            {
                // If we have a Bitcast, just ignore it
                Bitcast* bc = thisArg->as<Bitcast>();
                if ( bc )
                    thisArg = bc->exp()->explanation();

                FieldRef* fieldRef = thisArg->as<FieldRef>();
                if ( !fieldRef || fieldRef->field() != forField )
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
    SprFunction* fun = node->as<SprFunction>();
    if ( !fun || getName(fun) != "ctor" )
        REP_INTERNAL(node->location(), "IntModCtorMembers modifier can be applied only to constructors");
    if ( !fun->hasThisParameters() )
        REP_INTERNAL(node->location(), "IntModCtorMembers cannot be applied to static constructors");

    // If we have a body, make sure it's a local space
    if ( !fun->body() )
        return; // nothing to do
    LocalSpace* body = fun->body()->as<LocalSpace>();
    if ( !body )
        REP_INTERNAL(node->location(), "Constructor body is not a local space (needed by IntModCtorMembers)");

    // Get the class
    Class* cls = getParentClass(fun->context());
    CHECK(node->location(), cls);

    // If we are calling other constructor of this class, don't add any initialization
    if ( hasCtorCall(body, cls, true, nullptr) )
        return;

    // Generate the ctor calls in the order of the fields; add them to the body of the constructor
    const Location& loc = body->location();
    for ( Node* field: boost::adaptors::reverse(cls->fields()) )
    {
        // Make sure we initialize only fields of the current class
        Class* cls2 = getParentClass(field->context());
        if ( cls2 != cls )
            continue;

        if ( !hasCtorCall(body, nullptr, false, field) )
        {
            Node* fieldRef = mkFieldRef(loc, mkMemLoad(loc, mkThisExp(loc)), field);
            Node* call = nullptr;
            if ( field->type()->numReferences == 0 )
            {
                call = mkOperatorCall(loc, fieldRef, "ctor", nullptr);
            }
            else
            {
                call = mkOperatorCall(loc, fieldRef, ":=", mkNullLiteral(loc));
            }
            call->setContext(body->childrenContext());
            body->insertChildInFront(call);
        }
    }
}
