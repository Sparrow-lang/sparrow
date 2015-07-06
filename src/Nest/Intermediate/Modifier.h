#pragma once

FWD_CLASS1(Nest, DynNode);

namespace Nest
{
    /// Class that represents a modifier; objects of this type will be called to modify the compilation of nodes
    class Modifier
    {
    public:
        virtual ~Modifier() {}

        virtual void beforeSetContext(DynNode* /*node*/) {};
        virtual void afterSetContext(DynNode* /*node*/) {};
        virtual void beforeComputeType(DynNode* /*node*/) {};
        virtual void afterComputeType(DynNode* /*node*/) {};
        virtual void beforeSemanticCheck(DynNode* /*node*/) {};
        virtual void afterSemanticCheck(DynNode* /*node*/) {};
    };
}
