#include <StdInc.h>
#include "OutArchive.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

OutArchive::OutArchive(IWriter& writer)
    : writer_(writer)
{
}

void OutArchive::write(const char* name, char val)               { writer_.write(name, val); }
void OutArchive::write(const char* name, unsigned char val)      { writer_.write(name, val); }
void OutArchive::write(const char* name, short val)              { writer_.write(name, val); }
void OutArchive::write(const char* name, unsigned short val)     { writer_.write(name, val); }
void OutArchive::write(const char* name, int val)                { writer_.write(name, val); }
void OutArchive::write(const char* name, unsigned int val)       { writer_.write(name, val); }
void OutArchive::write(const char* name, long long val)          { writer_.write(name, val); }
void OutArchive::write(const char* name, unsigned long long val) { writer_.write(name, val); }
void OutArchive::write(const char* name, float val)              { writer_.write(name, val); }
void OutArchive::write(const char* name, double val)             { writer_.write(name, val); }
void OutArchive::write(const char* name, const char* val)        { writer_.write(name, val); }
void OutArchive::write(const char* name, const string& val)      { writer_.write(name, val); }

void OutArchive::flush()
{
    writer_.flush();
}
