#pragma once

#include "IReader.h"

namespace Nest { namespace Common { namespace Ser
{
    class InArchive
    {
        IReader& reader_;

    public:
        InArchive(IReader& reader);

        // Numeric types and string
        void read(const char* name, char& val);
        void read(const char* name, unsigned char& val);
        void read(const char* name, short& val);
        void read(const char* name, unsigned short& val);
        void read(const char* name, int& val);
        void read(const char* name, unsigned int& val);
        void read(const char* name, long long& val);
        void read(const char* name, unsigned long long& val);
        void read(const char* name, float& val);
        void read(const char* name, double& val);
        void read(const char* name, string& val);

        template <typename T>
        void read(const char* name, T& obj)
        {
            reader_.startObject();
            obj.load(*this);
            reader_.endObject();
        }
    };
}}}
