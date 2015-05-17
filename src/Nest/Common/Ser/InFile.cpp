#include <StdInc.h>
#include "InFile.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

InFile::InFile(FILE* file)
    : file_(file)
{
}

InFile::InFile(const string& filename)
    : file_(fopen(filename.c_str(), "r"))
{
    if ( !file_ )
        throw runtime_error("Cannot open file for reading: " + filename);
}

void InFile::read(void* buf, size_t size)
{
    size_t read = fread(buf, 1, size, file_);
    if ( read != size )
        throw runtime_error("Cannot read from file");
}
