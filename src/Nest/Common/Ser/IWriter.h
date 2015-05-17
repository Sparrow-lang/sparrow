#pragma once

namespace Nest { namespace Common { namespace Ser
{
    class IWriter
    {
    public:
        virtual ~IWriter() {}

        virtual void write(const char* name, char val) = 0;
        virtual void write(const char* name, unsigned char val) = 0;
        virtual void write(const char* name, short val) = 0;
        virtual void write(const char* name, unsigned short val) = 0;
        virtual void write(const char* name, int val) = 0;
        virtual void write(const char* name, unsigned int val) = 0;
        virtual void write(const char* name, long long val) = 0;
        virtual void write(const char* name, unsigned long long val) = 0;
        virtual void write(const char* name, float val) = 0;
        virtual void write(const char* name, double val) = 0;
        virtual void write(const char* name, const char* val) = 0;
        virtual void write(const char* name, const string& val) = 0;

        virtual void startObject(const char* name) = 0;
        virtual void endObject() = 0;

        virtual void startArray(const char* name, size_t size) = 0;
        virtual void endArray() = 0;
        virtual void startArrayEl() = 0;

        virtual void flush() = 0;
    };
}}}
