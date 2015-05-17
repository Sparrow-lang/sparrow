#pragma once

namespace Nest { namespace Common { namespace Ser
{
    class IReader
    {
    public:
        virtual ~IReader() {}

        virtual void read(const char* name, char& val) = 0;
        virtual void read(const char* name, unsigned char& val) = 0;
        virtual void read(const char* name, short& val) = 0;
        virtual void read(const char* name, unsigned short& val) = 0;
        virtual void read(const char* name, int& val) = 0;
        virtual void read(const char* name, unsigned int& val) = 0;
        virtual void read(const char* name, long long& val) = 0;
        virtual void read(const char* name, unsigned long long& val) = 0;
        virtual void read(const char* name, float& val) = 0;
        virtual void read(const char* name, double& val) = 0;
        virtual void read(const char* name, string& val) = 0;

        virtual void startObject() = 0;
        virtual void endObject() = 0;
    };
}}}
