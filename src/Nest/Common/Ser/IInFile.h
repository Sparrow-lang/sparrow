#pragma once

namespace Nest { namespace Common { namespace Ser
{
    class IInFile
    {
    public:
        virtual ~IInFile() {}

        virtual void read(void* buf, size_t size) = 0;
    };
}}}
