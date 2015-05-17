#pragma once

#include "IReader.h"
#include "IInFile.h"

namespace Nest { namespace Common { namespace Ser
{
    class BinReader : public IReader
    {
        IInFile& inFile_;

    public:
        BinReader(IInFile& inFile);

        virtual void read(const char* name, char& val);
        virtual void read(const char* name, unsigned char& val);
        virtual void read(const char* name, short& val);
        virtual void read(const char* name, unsigned short& val);
        virtual void read(const char* name, int& val);
        virtual void read(const char* name, unsigned int& val);
        virtual void read(const char* name, long long& val);
        virtual void read(const char* name, unsigned long long& val);
        virtual void read(const char* name, float& val);
        virtual void read(const char* name, double& val);
        virtual void read(const char* name, string& val);

        virtual void startObject();
        virtual void endObject();
    };
}}}
