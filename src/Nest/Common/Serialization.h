#pragma once

#include "Ser/OutFile.h"
#include "Ser/BinWriter.h"
#include "Ser/JsonWriter.h"
#include "Ser/OutArchive.h"

namespace Nest { namespace Common
{
    template<typename T>
    inline void saveToBinFile(const T& obj, const string& filename)
    {
        using namespace Ser;
        OutFile f(filename.c_str());
        BinWriter w = f;
        OutArchive ar = w;
        ar.write("root", obj);
    }

    template<typename T>
    inline void saveToJsonFile(const T& obj, const string& filename)
    {
        using namespace Ser;
        OutFile f(filename.c_str());
        JsonWriter w = f;
        OutArchive ar = w;
        ar.write("root", obj);
    }
}}
