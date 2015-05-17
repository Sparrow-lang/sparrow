#pragma once

#include "IWriter.h"
#include "IOutFile.h"

namespace Nest { namespace Common { namespace Ser
{
    class BinWriter : public IWriter
    {
        IOutFile& outFile_;

    public:
        BinWriter(IOutFile& outFile);

        virtual void write(const char* name, char val);
        virtual void write(const char* name, unsigned char val);
        virtual void write(const char* name, short val);
        virtual void write(const char* name, unsigned short val);
        virtual void write(const char* name, int val);
        virtual void write(const char* name, unsigned int val);
        virtual void write(const char* name, long long val);
        virtual void write(const char* name, unsigned long long val);
        virtual void write(const char* name, float val);
        virtual void write(const char* name, double val);
        virtual void write(const char* name, const char* val);
        virtual void write(const char* name, const string& val);

        virtual void startObject(const char* name);
        virtual void endObject();

        virtual void startArray(const char* name, size_t size);
        virtual void endArray();
        virtual void startArrayEl();

        virtual void flush();
    };
}}}
