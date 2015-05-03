#pragma once

namespace Nest { namespace Common
{
    /// Class used to allocate node objects. No destruction is given; nodes should be freed when exiting the compiler
    class NodeAllocator
    {
    public:
        virtual ~NodeAllocator() {}

        /// Does the memory allocation
        virtual void* alloc(size_t size) = 0;

        /// Does memory deallocation
        virtual void free(void* ptr) = 0;

        template <typename T>
        T* alloc()
        {
            return reinterpret_cast<T*>(alloc(sizeof(T)));
        }
    };
}}
