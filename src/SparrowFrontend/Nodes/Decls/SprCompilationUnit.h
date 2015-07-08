#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A Sparrow compilation unit
    class SprCompilationUnit : public DynNode
    {
        DEFINE_NODE(SprCompilationUnit, nkSparrowDeclSprCompilationUnit, "Sparrow.Decl.CompilationUnit");

    public:
        SprCompilationUnit(const Location& loc, DynNode* package, NodeList* imports, NodeList* declarations);

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
