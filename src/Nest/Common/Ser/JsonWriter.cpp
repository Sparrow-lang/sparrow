#include <StdInc.h>
#include "JsonWriter.h"

using namespace Nest;
using namespace Nest::Common;
using namespace Nest::Common::Ser;

JsonWriter::JsonWriter(IOutFile& outFile)
    : outFile_(outFile)
    , indent_(0)
    , insideArray_(true)
    , firstAttribute_(true)
{
}

void JsonWriter::write(const char* name, char val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, unsigned char val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, short val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, unsigned short val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, int val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, unsigned int val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, long long val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, unsigned long long val)
{
    writeNumber(name, val);
}
void JsonWriter::write(const char* name, float val)
{
    writeFloatingNumber(name, val);
}
void JsonWriter::write(const char* name, double val)
{
    writeFloatingNumber(name, val);
}
void JsonWriter::write(const char* name, const char* val)
{
    writeString(name, val, strlen(val));
}
void JsonWriter::write(const char* name, const string& val)
{
    writeString(name, val.c_str(), val.size());
}

void JsonWriter::startObject(const char* name)
{
    writePrefix(name);
    writeS("{");
    ++indent_;
    firstAttribute_ = true;
}
void JsonWriter::endObject()
{
    writeS("\n");
    --indent_;
    writeSpaces();
    writeS("}");
    outFile_.flush();
    firstAttribute_ = false;
}

void JsonWriter::startArray(const char* name, size_t size)
{
    writePrefix(name);
    writeS("[");
    ++indent_;
    firstAttribute_ = true;
    insideArray_ = true;
}
void JsonWriter::endArray()
{
    writeS("\n");
    --indent_;
    writeSpaces();
    writeS("]");
    firstAttribute_ = false;
    insideArray_ = false;    
}
void JsonWriter::startArrayEl()
{
    insideArray_ = true;
}

void JsonWriter::flush()
{
    outFile_.flush();
}

void JsonWriter::writeS(const char* str)
{
    outFile_.write(str, strlen(str));
}

void JsonWriter::writeSpaces()
{
    static const int indentSize = 2;
    static const int spacesLen = 20;
    char spaces[spacesLen+1] = "                    ";
    int toWrite = indentSize*indent_;
    // Write in big chunks
    while ( toWrite > spacesLen )
    {
        outFile_.write(spaces, spacesLen);
        toWrite -= spacesLen;
    }
    // Write the remaining of spaces
    if ( toWrite > 0 )
        outFile_.write(spaces, toWrite);
}

void JsonWriter::writePrefix(const char* name)
{
    if ( firstAttribute_ )
        writeS("\n");
    else
        writeS(",\n");
    firstAttribute_ = false;
    writeSpaces();
    if ( !insideArray_ )
    {
        writeS("\"");
        writeS(name);
        writeS("\": ");
    }    
    insideArray_ = false;
}

void JsonWriter::writeNumber(const char* name, long long val)
{
    writePrefix(name);
    char buf[24];
    sprintf(buf, "%lld", val);
    writeS(buf);
}
void JsonWriter::writeFloatingNumber(const char* name, double val)
{
    writePrefix(name);
    char buf[32];
    sprintf(buf, "%f", val);
    writeS(buf);
}
void JsonWriter::writeString(const char* name, const char* val, size_t size)
{
    // Check if the string contains some 'strange' characters
    bool hasStrangeCharacters = false;
    for ( int i=0; i<size; ++i )
    {
        if ( !isprint(val[i]) )
        {
            hasStrangeCharacters = true;
            break;
        }
    }

    writePrefix(name);
    writeS("\"");
    if ( !hasStrangeCharacters )
        writeS(val);
    else
    {
        for ( int i=0; i<size; ++i )
        {
            unsigned char ch = val[i];
            if ( ch == '\\' )
                writeS("\\\\");                
            else if ( ch == '"' )
                writeS("\\\"");
            else if ( isprint(ch) )
                outFile_.write(&ch, 1);
            else
            {
                unsigned char hi = (ch & 0xF0) >> 4;
                unsigned char lo = ch & 0x0F;
                static const char* hexChars = "0123456789ABCDEF";
                char hex[6] = { '\\', 'u', '0', '0', hexChars[hi], hexChars[lo] };
                outFile_.write(hex, 6);
            }
        }
    }
    writeS("\"");
}
