#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An identifier (not qualified) in the Sparrow language
    class Identifier : public DynNode
    {
        DEFINE_NODE(Identifier, nkSparrowExpIdentifier, "Sparrow.Exp.Identifier");

    public:
        Identifier(const Location& loc, string id);

        const string& id() const;

        virtual string toString() const;

    protected:
        virtual void doSemanticCheck();
    };
    
    /// Get the result of the identifier node, given the set of the declarations this identifier points to
    /// This method can be used both with identifiers and compound expressions
    DynNode* getIdentifierResult(CompilationContext* ctx, const Location& loc, const DynNodeVector& decls, DynNode* baseExp, bool allowDeclExp);
}
