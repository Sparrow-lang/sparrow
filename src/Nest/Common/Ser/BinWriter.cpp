#include <StdInc.h>
#include "BinWriter.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

BinWriter::BinWriter(IOutFile& outFile)
    : outFile_(outFile)
{
}

void BinWriter::write(const char* name, char val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, unsigned char val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, short val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, unsigned short val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, int val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, unsigned int val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, long long val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, unsigned long long val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, float val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, double val)
{
    outFile_.write(&val, sizeof(val));
}
void BinWriter::write(const char* name, const char* val)
{
    unsigned int size = strlen(val);
    outFile_.write(&size, sizeof(size));
    outFile_.write(val, size);
}
void BinWriter::write(const char* name, const string& val)
{
    unsigned int size = (unsigned int) val.size();
    outFile_.write(&size, sizeof(size));
    outFile_.write(val.c_str(), size);
}

void BinWriter::startObject(const char* name)
{
    // No object separator
}
void BinWriter::endObject()
{
    // No object separator
}

void BinWriter::startArray(const char* name, size_t size)
{
    unsigned int s = size;
    outFile_.write(&s, sizeof(s));
}
void BinWriter::endArray()
{
    // No array separator
}
void BinWriter::startArrayEl()
{
    // No array element separator
}

void BinWriter::flush()
{
    outFile_.flush();
}
