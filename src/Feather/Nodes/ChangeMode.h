#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// DynNode that can change the evaluation mode of the children node
    class ChangeMode : public DynNode
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
        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doSemanticCheck();
    };
}
