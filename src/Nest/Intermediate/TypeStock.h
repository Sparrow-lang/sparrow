#pragma once

#include "Type.h"

namespace Nest
{
    class TypeStock;
    
    extern TypeStock typeStock;
    
    class TypeStock
    {
    public:
        TypeStock();
        ~TypeStock();

        /// Get/create a type given its underlying basic type data
        template<typename TypeClass>
        TypeClass* getCreateType(TypeData* basicType)
        {
            auto& tMap = getTypeMapImpl();
            auto it = tMap.find(basicType);
            if ( it != tMap.end() )
                return reinterpret_cast<TypeClass*>(it->second);

            void* p = typeStock.allocType();
            TypeClass* res = new (p) TypeClass();
            res->data_ = basicType;
            tMap[basicType] = res;
            return res;
        }

    private:
        /// Allocates one type
        void* allocType();

        /// Gets the map corresponding to a type id
        unordered_map<const TypeData*,Type*>& getTypeMapImpl();

    private:
        struct Impl;
        Impl* impl_;
    };
}
