#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a reference to a field of a given object
    /// Given the field, this node will have the same type as the field
    class FieldRef : public Node
    {
        DEFINE_NODE(FieldRef, nkFeatherExpFieldRef, "Feather.Exp.FieldRef");

    public:
        FieldRef(const Location& loc, Node* obj, Node* field);

        /// Getter to the node that represents the object from which we access the field
        Node* object() const;

        /// Getter for the field this node refers to
        Node* field() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
