#pragma once

#include "IWriter.h"
#include "IOutFile.h"

namespace Nest { namespace Common { namespace Ser
{
    class JsonWriter : public IWriter
    {
        IOutFile& outFile_;
        int indent_;
        bool insideArray_;
        bool firstAttribute_;

    public:
        JsonWriter(IOutFile& outFile);

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

    private:
        void writeS(const char* str);
        void writeSpaces();
        void writePrefix(const char* name);
        void writeNumber(const char* name, long long val);
        void writeFloatingNumber(const char* name, double val);
        void writeString(const char* name, const char* val, size_t size);
    };
}}}
