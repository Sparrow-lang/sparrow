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

        /// Get/create a type given a type id and the key corresponding to the type
        /// The given 'createFun' functor must have the following signature 'TypePtr (void*, MapKey)' and must create the
        /// type in the given location for the given key
        template<typename TypePtr, typename MapKey, typename CreateFun>
        TypePtr getCreateType(Type::TypeId typeId, const MapKey& key, CreateFun createFun)
        {
            unordered_map<MapKey, TypePtr>*& tMap = reinterpret_cast<unordered_map<MapKey, TypePtr>*&>(getTypeMapImpl(typeId));
            if ( !tMap )
                tMap = new unordered_map<MapKey, TypePtr>();
            auto it = tMap->find(key);
            if ( it != tMap->end() )
                return it->second;

            void* p = typeStock.allocType();
            TypePtr res = createFun(p, key);
            (*tMap)[key] = res;
            return res;
        }

    private:
        /// Allocates one type
        void* allocType();

        /// Gets the map corresponding to a type id
        unordered_map<Type*,Type*>*& getTypeMapImpl(Type::TypeId typeId);

    private:
        struct Impl;
        Impl* impl_;
    };
}
