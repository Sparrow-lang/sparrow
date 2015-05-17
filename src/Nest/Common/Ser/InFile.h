#pragma once

#include "IInFile.h"

#include <stdio.h>

namespace Nest { namespace Common { namespace Ser
{
    class InFile : public IInFile
    {
        FILE* file_;

    public:
        InFile(FILE* file);
        InFile(const string& filename);

        virtual void read(void* buf, size_t size);
    };
}}}
