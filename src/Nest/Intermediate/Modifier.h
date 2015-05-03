#pragma once

FWD_CLASS1(Nest, Node);

namespace Nest
{
    /// Class that represents a modifier; objects of this type will be called to modify the compilation of nodes
    class Modifier
    {
    public:
        virtual ~Modifier() {}

        virtual void beforeSetContext(Node* /*node*/) {};
        virtual void afterSetContext(Node* /*node*/) {};
        virtual void beforeComputeType(Node* /*node*/) {};
        virtual void afterComputeType(Node* /*node*/) {};
        virtual void beforeSemanticCheck(Node* /*node*/) {};
        virtual void afterSemanticCheck(Node* /*node*/) {};
    };
}
