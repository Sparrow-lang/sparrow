#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Node that can change the evaluation mode of the children node
    class ChangeMode : public Node
    {
        DEFINE_NODE(ChangeMode, nkFeatherExpChangeMode, "Feather.ChangeMode");

    public:
        ChangeMode(const Location& loc, EvalMode mode, Node* child = nullptr);

        /// Getter for child of this node
        Node* child() const;

        /// Setter for the child of this node, the one that will have a different eval mode
        void setChild(Node* child);
        
        /// Getter for the evaluation mode of this node
        EvalMode evalMode() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doSemanticCheck();
    };
}
