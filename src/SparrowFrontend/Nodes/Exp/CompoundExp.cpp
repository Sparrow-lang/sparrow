#include <StdInc.h>
#include "CompoundExp.h"
#include "Identifier.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>

using namespace SprFrontend;
using namespace Nest;
using namespace Feather;


CompoundExp::CompoundExp(const Location& loc, DynNode* base, string id)
    : DynNode(classNodeKind(), loc, {base})
{
    setProperty("name", move(id));
    setProperty(propAllowDeclExp, 0);
}

const string& CompoundExp::id() const
{
    return getCheckPropertyString("name");
}

DynNode* CompoundExp::baseDataExp() const
{
    DynNode*const* res = getPropertyDynNode("baseDataExp");
    return res ? *res : nullptr;
}

string CompoundExp::toString() const
{
    return getCheckPropertyString("name");
}

void CompoundExp::dump(ostream& os) const
{
    os << data_.children[0] << "." << getCheckPropertyString("name");
}

void CompoundExp::doSemanticCheck()
{
    DynNode* base = (DynNode*) data_.children[0];
    const string& id = getCheckPropertyString("name");

    // For the base expression allow it to return DeclExp
    base->setProperty(propAllowDeclExp, 1, true);

    // Compile the base expression
    // We can expect at the base node both traditional expressions and nodes yielding decl-type types
    base->semanticCheck();

    // Try to get the declarations pointed by the base node
    DynNode* baseDataExp = nullptr;
    DynNodeVector baseDecls = getDeclsFromNode(base, baseDataExp);
    
    // If the base has storage, retain it as the base data expression
    if ( baseDecls.empty() && base->type()->hasStorage )
        baseDataExp = base;
    if ( baseDataExp )
        baseDataExp->computeType();
    setProperty("baseDataExp", baseDataExp);

    // Get the declarations that this node refers to
    DynNodeVector decls;
    if ( !baseDecls.empty() )
    {
        // Get the referred declarations; search for our id inside the symbol table of the declarations of the base
        for ( DynNode* baseDecl: baseDecls )
        {
            DynNodeVector declsCur = toDyn(baseDecl->childrenContext()->currentSymTab()->lookupCurrent(id));
            decls.insert(decls.end(), declsCur.begin(), declsCur.end());
        }
    }
    else if ( base->type()->hasStorage )
    {
        // If the base is an expression with a data type, treat this as a data access
        Node* classDecl = classForTypeRaw(base->type());
        Nest::computeType(classDecl);

        // Search for a declaration in the class 
        decls = toDyn(classDecl->childrenContext->currentSymTab()->lookupCurrent(id));
    }

    if ( decls.empty() )
        REP_ERROR(data_.location, "No declarations found with the name '%1%' inside %2%: %3%") % id % base % base->type();

    
    bool allowDeclExp = 0 != getCheckPropertyInt(propAllowDeclExp);
    DynNode* res = getIdentifierResult(data_.context, data_.location, move(decls), baseDataExp, allowDeclExp);
    ASSERT(res);
    setExplanation(res);
}
