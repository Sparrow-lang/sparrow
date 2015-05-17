#pragma once

#include "EvalMode.h"

#include <vector>

//#ifdef _DEBUG
#define USE_TYPE_DESCRIPTIONS
//#endif

#ifdef USE_TYPE_DESCRIPTIONS
    #define SET_TYPE_DESCRIPTION(t)     (t).description_ = strdup((t).toString().c_str());
#else
    #define SET_TYPE_DESCRIPTION(t)     /*nothing*/
#endif

FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
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
        int typeId() const { return typeId_; }

        /// Returns a textual description of this type.
        virtual string toString() const = 0;
        
        /// Returns true if the construct having this type needs to have some associated storage
        virtual bool hasStorage() const { return false; }
        
        /// For storage types, this will return the number of references
        virtual uint8_t noReferences() const { return 0; }
        
        /// The CT mode of this type
        virtual EvalMode mode() const { return mode_; }
        
        /// Returns true if this type can (somehow, by a conversion) be used ar runtime
        virtual bool canBeUsedAtRt() const { return true; }

        /// Returns true if this type can (somehow, by a conversion) be used ar compile-time
        virtual bool canBeUsedAtCt() const { return true; }

        /// Creates a new tyoe with the given eval mode, if possible; if not, returns null
        virtual Type* changeMode(EvalMode newMode) { return nullptr; }

        // Serialization
    public:
        void save(Common::Ser::OutArchive& ar) const;
        void load(Common::Ser::InArchive& ar);
        
    protected:
        Type(unsigned typeId, EvalMode mode)
            : typeId_(typeId)
            , mode_(mode)
            , flags_(0)
            , additionalData_(nullptr)
#ifdef USE_TYPE_DESCRIPTIONS
            , description_("")
#endif
        {}
        
    protected:
        /// The type ID
        unsigned typeId_ : 8;
        
        // The evaluation mode of this type
        EvalMode mode_ : 8;
        
        /// The flags of this type
        unsigned flags_ : 32;
        
        /// The subtypes of this type
        vector<Type*> subTypes_;
        
        /// Additional data for the type (i.e., for a DataType we store here the class pointer)
        void* additionalData_;

#ifdef USE_TYPE_DESCRIPTIONS
    public:
        const char* description_;
#endif
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
