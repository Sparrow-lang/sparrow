#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A Sparrow compilation unit
    class SprCompilationUnit : public DynNode
    {
        DEFINE_NODE(SprCompilationUnit, nkSparrowDeclSprCompilationUnit, "Sparrow.Decl.CompilationUnit");

    public:
        SprCompilationUnit(const Location& loc, DynNode* package, Node* imports, Node* declarations);

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
