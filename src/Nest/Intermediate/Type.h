#pragma once

#include "EvalMode.h"

#include <vector>

#define SET_TYPE_DESCRIPTION(t)     (t).data_.description = strdup((t).toString().c_str());

FWD_CLASS1(Nest, Node)
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    class Type;
    
    // TODO: Remove this
    enum TypeIdNew
    {
        typeVoid = 0,
        typeData,
        typeLValue,
        typeArray,
        typeFunction,

        typeConcept,

        typeIdLast,
    };

    /// Represents a type
    struct TypeData
    {
        unsigned typeId : 8;            ///< The type ID
        EvalMode mode : 8;              ///< The evaluation mode of this type
        unsigned numSubtypes : 16;      ///< The number of subtypes of this type
        unsigned numReferences : 16;    ///< The number of references of this type
        unsigned hasStorage : 1;        ///< True if this is type with storage
        unsigned canBeUsedAtCt : 1;     ///< Can we use this type at CT?
        unsigned canBeUsedAtRt : 1;     ///< Can we use this type at RT?
        unsigned flags : 32;            ///< Additional flags

        /// The subtypes of this type
        Type** subTypes;

        /// Optional, the node that introduces this type
        Node* referredNode;

        /// The description of the type -- mainly used for debugging purposes
        const char* description;
    };

    /// Equality comparison, by content
    bool operator ==(const TypeData& lhs, const TypeData& rhs);

    /// Get a stock type that matches the settings from the reference
    /// We guarantee that the same types will have the same pointers
    TypeData* getStockType(const TypeData& reference);


    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const TypeData* t)
    {
        if ( t )
            os << t->description;
        else
            os << "<null-type>";
        return os;
    }
    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, TypeData* t)
    {
        if ( t )
            os << t->description;
        else
            os << "<null-type>";
        return os;
    }



    /// Base class for types
    class Type
    {
    public:
        enum TypeId
        {
            typeVoid = 0,
            typeData,
            typeLValue,
            typeArray,
            typeFunction,

            typeConcept,
            
            typeIdLast,
        };
        
    public:
        virtual ~Type() {}

        /// Gets the type ID of this type
        int typeId() const { return data_.typeId; }

        /// Returns a textual description of this type.
        virtual string toString() const { return data_.description; }
        
        /// Returns true if the construct having this type needs to have some associated storage
        virtual bool hasStorage() const { return data_.hasStorage; }
        
        /// For storage types, this will return the number of references
        virtual uint8_t noReferences() const { return data_.numReferences; }
        
        /// The CT mode of this type
        virtual EvalMode mode() const { return data_.mode; }
        
        /// Returns true if this type can (somehow, by a conversion) be used ar runtime
        virtual bool canBeUsedAtRt() const { return data_.canBeUsedAtRt; }

        /// Returns true if this type can (somehow, by a conversion) be used ar compile-time
        virtual bool canBeUsedAtCt() const { return data_.canBeUsedAtCt; }

        /// Creates a new tyoe with the given eval mode, if possible; if not, returns null
        virtual Type* changeMode(EvalMode newMode) { return nullptr; }

        // Serialization
    public:
        void save(Common::Ser::OutArchive& ar) const;
        void load(Common::Ser::InArchive& ar);
        
    protected:
        Type(unsigned typeId, EvalMode mode);

    public:
        TypeData data_;
    };

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const Type& t)
    {
        os << t.toString();
        return os;
    }

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const Type* t)
    {
        if ( t )
            os << t->toString();
        return os;
    }
}
