#pragma once

namespace Nest
{
    struct Node;
    typedef vector<Node*> NodeVector;
}

namespace Feather
{
    class DynNode;
    typedef vector<DynNode*> DynNodeVector;

    inline DynNodeVector toDyn(Nest::NodeVector v)
    {
        return move(reinterpret_cast<DynNodeVector&>(v));
    }

    inline Nest::NodeVector fromDyn(DynNodeVector v)
    {
        return move(reinterpret_cast<Nest::NodeVector&>(v));
    }
}
