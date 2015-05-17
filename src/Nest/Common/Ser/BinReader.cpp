#include <StdInc.h>
#include "BinReader.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

BinReader::BinReader(IInFile& inFile)
    : inFile_(inFile)
{
}

void BinReader::read(const char* name, char& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, unsigned char& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, short& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, unsigned short& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, int& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, unsigned int& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, long long& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, unsigned long long& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, float& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, double& val)
{
    inFile_.read(&val, sizeof(val));
}
void BinReader::read(const char* name, string& val)
{
    unsigned int size;
    inFile_.read(&size, sizeof(size));
    val.resize(size);
    inFile_.read(&val[0], size);
}

void BinReader::startObject()
{
    // No object separator
}
void BinReader::endObject()
{
    // No object separator
}
