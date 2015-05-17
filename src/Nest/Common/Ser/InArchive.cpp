#include <StdInc.h>
#include "InArchive.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

InArchive::InArchive(IReader& reader)
    : reader_(reader)
{
}

void InArchive::read(const char* name, char& val)               { reader_.read(name, val); }
void InArchive::read(const char* name, unsigned char& val)      { reader_.read(name, val); }
void InArchive::read(const char* name, short& val)              { reader_.read(name, val); }
void InArchive::read(const char* name, unsigned short& val)     { reader_.read(name, val); }
void InArchive::read(const char* name, int& val)                { reader_.read(name, val); }
void InArchive::read(const char* name, unsigned int& val)       { reader_.read(name, val); }
void InArchive::read(const char* name, long long& val)          { reader_.read(name, val); }
void InArchive::read(const char* name, unsigned long long& val) { reader_.read(name, val); }
void InArchive::read(const char* name, float& val)              { reader_.read(name, val); }
void InArchive::read(const char* name, double& val)             { reader_.read(name, val); }
void InArchive::read(const char* name, string& val)             { reader_.read(name, val); }
