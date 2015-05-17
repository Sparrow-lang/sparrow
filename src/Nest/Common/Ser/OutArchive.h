#pragma once

#include "IWriter.h"

namespace Nest { namespace Common { namespace Ser
{
    class OutArchive
    {
        IWriter& writer_;

    public:
        OutArchive(IWriter& writer);

        // Numeric types and string
        void write(const char* name, char val);
        void write(const char* name, unsigned char val);
        void write(const char* name, short val);
        void write(const char* name, unsigned short val);
        void write(const char* name, int val);
        void write(const char* name, unsigned int val);
        void write(const char* name, long long val);
        void write(const char* name, unsigned long long val);
        void write(const char* name, float val);
        void write(const char* name, double val);
        void write(const char* name, const char* val);
        void write(const char* name, const string& val);

        // Objects
        template<typename T>
        void write(const char* name, const T& obj)
        {
            writer_.startObject(name);
            obj.save(*this);
            writer_.endObject();
        }
        template<typename T>
        void write(const char* name, const T* obj)
        {
            writePointer(name, obj);
        }
        template<typename T>
        void write(const char* name, T* obj)
        {
            writePointer(name, obj);
        }

        template<typename T, typename WriteFun>
        void write(const char* name, const T& obj, WriteFun writeFun)
        {
            writer_.startObject(name);
            writeFun(*this, obj);
            writer_.endObject();
        }

        template<typename Cont, typename WriteFun>
        void writeArray(const char* name, const Cont& cont, WriteFun writeFun)
        {
            writer_.startArray(name, cont.size());
            for ( const auto& el: cont )
            {
                writer_.startArrayEl();
                writeFun(*this, el);

            }
            writer_.endArray();
        }

        void flush();

    private:
        template<typename T>
        void writePointer(const char* name, const T* obj)
        {
            writer_.startObject(name);
            if ( !obj )
            {
                // Null pointer
                writer_.write("p", 0);
            }
            else
            {
                unsigned long long ref = reinterpret_cast<unsigned long long>((void*) obj);
                auto it = writtenPointers_.find((void*) obj);
                if ( it == writtenPointers_.end() )
                {
                    // First time we see this pointer
                    writtenPointers_.insert((void*) obj);
                    writer_.write("p", 1);
                    writer_.write("ref", ref);
                    writer_.startObject("obj");
                    obj->save(*this);
                    writer_.endObject();
                }
                else
                {
                    // The object was already serialized
                    writer_.write("p", 2);
                    writer_.write("ref", ref);
                }
            }
            writer_.endObject();
        }

        std::unordered_set<void*> writtenPointers_;
    };
}}}
