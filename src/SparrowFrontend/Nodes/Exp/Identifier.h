#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An identifier (not qualified) in the Sparrow language
    class Identifier : public Node
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
    Node* getIdentifierResult(CompilationContext* ctx, const Location& loc, const NodeVector& decls, Node* baseExp);
}
