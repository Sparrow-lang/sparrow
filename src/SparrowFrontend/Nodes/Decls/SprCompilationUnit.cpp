#include <StdInc.h>
#include "SprCompilationUnit.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/QualifiedId.h>
#include <Nodes/Exp/Literal.h>

using namespace SprFrontend;

SprCompilationUnit::SprCompilationUnit(const Location& loc, Node* package, Node* imports, Node* declarations)
    : DynNode(classNodeKind(), loc, {package, imports, declarations})
{
    ASSERT( !imports || imports->nodeKind == Feather::nkFeatherNodeList );
    ASSERT( !declarations || declarations->nodeKind == Feather::nkFeatherNodeList );
}

void SprCompilationUnit::doSetContextForChildren()
{
    ASSERT(data_.children.size() == 3);
    Node* packageName = data_.children[0];
    Node* imports = data_.children[1];
    Node* declarations = data_.children[2];
    if ( packageName )
        Nest::setContext(packageName, data_.context);

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
            NodeVector decls = data_.context->currentSymTab()->lookupCurrent(names[i]);
            if ( decls.size() == 1 )
            {
                data_.context = Nest::childrenContext(decls.front());
                continue;
            }

            // We didn't find the package part. From now on create new namespaces
            for ( int j=(int)names.size()-1; j>=i; --j )
            {
                Node* pk = mkSprPackage(packageName->location, move(names[j]), declarations);
                declarations = Feather::mkNodeList(packageName->location, {pk}, true);
                data_.children[2] = declarations;
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
        Nest::setContext(declarations, data_.childrenContext);
    if ( imports )
        Nest::setContext(imports, data_.childrenContext);

    // Handle imports
    if ( imports )
    {
        const Nest::SourceCode* sourceCode = data_.location.sourceCode();
        for ( Node* i: imports->children )
        {
            Literal* lit = (Literal*) ofKind(i, nkSparrowExpLiteral);
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
    Node* declarations = data_.children[2];

    // Compute the type for the children
    if ( declarations )
    {
        Nest::computeType(declarations);
        checkForAllowedNamespaceChildren(declarations);
    }

    data_.type = Feather::getVoidType(Feather::modeCt);
}

void SprCompilationUnit::doSemanticCheck()
{
    Nest::computeType(node());

    ASSERT(data_.children.size() == 3);
    Node* declarations = data_.children[2];

    setExplanation(declarations ? declarations : Feather::mkNop(data_.location));
}

