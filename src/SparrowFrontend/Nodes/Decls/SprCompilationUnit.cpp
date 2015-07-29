#include <StdInc.h>
#include "SprCompilationUnit.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/QualifiedId.h>
#include <Nodes/Exp/Literal.h>

using namespace SprFrontend;

SprCompilationUnit::SprCompilationUnit(const Location& loc, DynNode* package, NodeList* imports, NodeList* declarations)
    : DynNode(classNodeKind(), loc, {package, imports, declarations})
{
}

void SprCompilationUnit::doSetContextForChildren()
{
    ASSERT(data_.children.size() == 3);
    DynNode* packageName = (DynNode*) data_.children[0];
    DynNode* imports = (DynNode*) data_.children[1];
    NodeList* declarations = (NodeList*)data_.children[2];
    if ( packageName )
        packageName->setContext(data_.context);

    // Handle package name - create namespaces for it
    if ( packageName )
    {
        // Get the parts of the package name
        vector<string> names;
        interpretQualifiedId(packageName, names);
        ASSERT(!names.empty() && !names.back().empty());

        for ( int i=0; i<(int)names.size(); ++i )
        {
            // Try to find an existing package in the current symbol table
            DynNodeVector decls = Feather::toDyn(data_.context->currentSymTab()->lookupCurrent(names[i]));
            if ( decls.size() == 1 )
            {
                data_.context = decls.front()->childrenContext();
                continue;
            }

            // We didn't find the package part. From now on create new namespaces
            for ( int j=(int)names.size()-1; j>=i; --j )
            {
                DynNode* pk = mkSprPackage(packageName->location(), move(names[j]), declarations);
                declarations = (NodeList*) Feather::mkNodeList(packageName->location(), {pk->node()}, true);
                data_.children[2] = declarations->node();
            }
            break;
        }
    }

    // If we don't have a children context, create one
    data_.childrenContext = data_.context;
    //if ( !data_.childrenContext )
    //    data_.childrenContext = data_.context->createChildContext(this, evalMode());

    // Set the context for all the children
    if ( declarations )
        declarations->setContext(data_.childrenContext);
    if ( imports )
        imports->setContext(data_.childrenContext);

    // Handle imports
    if ( imports )
    {
        const Nest::SourceCode* sourceCode = data_.location.sourceCode();
        for ( DynNode* i: imports->children() )
        {
            Literal* lit = i->as<Literal>();
            if ( lit && lit->isString() )
            {
                Nest::theCompiler().addSourceCodeByFilename(sourceCode, lit->asString());
            }
            else
            {
                vector<string> qid;
                interpretQualifiedId(i, qid);
                Nest::theCompiler().addSourceCodeByQid(sourceCode, qid);
            }
        }
    }
}

void SprCompilationUnit::doComputeType()
{
    ASSERT(data_.children.size() == 3);
    NodeList* declarations = (NodeList*)data_.children[2];

    // Compute the type for the children
    if ( declarations )
    {
        declarations->computeType();
        checkForAllowedNamespaceChildren(declarations);
    }

    data_.type = Feather::getVoidType(Feather::modeCt);
}

void SprCompilationUnit::doSemanticCheck()
{
    computeType();

    ASSERT(data_.children.size() == 3);
    NodeList* declarations = (NodeList*)data_.children[2];

    setExplanation(declarations ? declarations : (DynNode*) Feather::mkNop(data_.location));
}

