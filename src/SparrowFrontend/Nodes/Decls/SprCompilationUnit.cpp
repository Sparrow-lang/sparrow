#include <StdInc.h>
#include "SprCompilationUnit.h"
#include <NodeCommonsCpp.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/QualifiedId.h>
#include <Nodes/Exp/Literal.h>

using namespace SprFrontend;

SprCompilationUnit::SprCompilationUnit(const Location& loc, Node* package, NodeList* imports, NodeList* declarations)
    : Node(classNodeKind(), loc, {package, imports, declarations})
{
}

void SprCompilationUnit::doSetContextForChildren()
{
    ASSERT(children_.size() == 3);
    Node* packageName = children_[0];
    Node* imports = children_[1];
    NodeList* declarations = (NodeList*)children_[2];
    if ( packageName )
        packageName->setContext(context_);

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
            NodeVector decls = context_->currentSymTab()->lookupCurrent(names[i]);
            if ( decls.size() == 1 )
            {
                context_ = decls.front()->childrenContext();
                continue;
            }

            // We didn't find the package part. From now on create new namespaces
            for ( int j=(int)names.size()-1; j>=i; --j )
            {
                Node* pk = mkSprPackage(packageName->location(), move(names[j]), declarations);
                declarations = Feather::mkNodeList(packageName->location(), {pk}, true);
                children_[2] = declarations;
            }
            break;
        }
    }

    // If we don't have a children context, create one
    childrenContext_ = context_;
    //if ( !childrenContext_ )
    //    childrenContext_ = context_->createChildContext(this, evalMode());

    // Set the context for all the children
    if ( declarations )
        declarations->setContext(childrenContext_);
    if ( imports )
        imports->setContext(childrenContext_);

    // Handle imports
    if ( imports )
    {
        const Nest::SourceCode* sourceCode = location_.sourceCode();
        for ( Node* i: imports->children() )
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
    ASSERT(children_.size() == 3);
    NodeList* declarations = (NodeList*)children_[2];

    // Compute the type for the children
    if ( declarations )
    {
        declarations->computeType();
        checkForAllowedNamespaceChildren(declarations);
    }

    type_ = Feather::getVoidType(Feather::modeCt);
}

void SprCompilationUnit::doSemanticCheck()
{
    computeType();

    ASSERT(children_.size() == 3);
    NodeList* declarations = (NodeList*)children_[2];

    setExplanation(declarations ? declarations : Feather::mkNop(location_));
}

