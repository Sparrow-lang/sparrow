#pragma once

namespace Nest { namespace Common { namespace Ser
{
    class IOutFile
    {
    public:
        virtual ~IOutFile() {}

        virtual void write(const void* buf, size_t size) = 0;
        virtual void flush() = 0;
    };
}}}
