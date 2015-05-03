#pragma once

#include "NodeAllocator.h"

namespace Nest { namespace Common
{
    /// Implementation of the NodeAllocator class
    class NodeAllocatorImpl : public NodeAllocator
    {
    public:
        NodeAllocatorImpl();
        virtual ~NodeAllocatorImpl();

        virtual void* alloc(size_t size);
        virtual void free(void* ptr);

    private:
        char* curPage_;
        size_t curPageFreeByes_;
    };
}}
