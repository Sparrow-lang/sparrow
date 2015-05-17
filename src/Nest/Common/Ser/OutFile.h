#pragma once

#include "IOutFile.h"

#include <stdio.h>

namespace Nest { namespace Common { namespace Ser
{
    class OutFile : public IOutFile
    {
        FILE* file_;

    public:
        OutFile(FILE* file);
        OutFile(const string& filename);

        virtual void write(const void* buf, size_t size);
        virtual void flush();
    };
}}}
