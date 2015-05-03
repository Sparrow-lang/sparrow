#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A Sparrow compilation unit
    class SprCompilationUnit : public Node
    {
        DEFINE_NODE(SprCompilationUnit, nkSparrowDeclSprCompilationUnit, "Sparrow.Decl.CompilationUnit");

    public:
        SprCompilationUnit(const Location& loc, Node* package, NodeList* imports, NodeList* declarations);

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
