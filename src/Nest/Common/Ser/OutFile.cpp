#include <StdInc.h>
#include "OutFile.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

OutFile::OutFile(FILE* file)
    : file_(file)
{
}

OutFile::OutFile(const string& filename)
    : file_(fopen(filename.c_str(), "w"))
{
    if ( !file_ )
        throw runtime_error("Cannot open file for writing: " + filename);
}

void OutFile::write(const void* buf, size_t size)
{
    size_t written = fwrite(buf, 1, size, file_);
    if ( written != size )
        throw runtime_error("Cannot write to file");
}

void OutFile::flush()
{
    fflush(file_);
}
