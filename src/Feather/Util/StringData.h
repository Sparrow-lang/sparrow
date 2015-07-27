#pragma once

namespace Feather
{
    /// Structure that matches a string as it is translated in the generated code
    struct StringData
    {
        const char* begin;
        const char* end;

        StringData()
            : begin(nullptr)
            , end(nullptr)
        {}

        StringData(const string& str)
            : begin(str.c_str())
            , end(begin + str.size())
        {}

        static StringData copyStdString(const string& src)
        {
            StringData res;
            if ( src.empty() )
                return res;

            size_t size = src.size();
            char* ptr = (char*) malloc(size+1);
            memcpy(ptr, &src[0], size);
            ptr[size] = 0;
            res.begin = ptr;
            res.end = ptr + size;
            return res;
        }

        string toStdString() const
        {
            return string(begin, end);
        }
    };
}
