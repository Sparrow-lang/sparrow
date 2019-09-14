#pragma once

#include "Nest/Api/Type.h"
#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Api/Location.h"
#include "Nest/Utils/cppif/Fwd.hpp"

namespace Nest {

struct NodeHandle;

/**
 * @brief   Base class for all type wrappers.
 *
 * All the types have a mode; we allow it to query the mode of the type.
 *
 * Properties:
 *     - if mode is RT, canBeUsedAtRt must be true
 *
 * @see     VoidType, TypeWithStorage, DataType
 */
struct Type {
    //! The actual type we are wrapping
    Nest::TypeRef type_{nullptr};

    //! Constructor that initializes this with a null type
    Type() = default;
    //! Constructor from the raw Nest::TypeRef
    Type(Nest::TypeRef type)
        : type_(type) {}

    //! Implicit conversion to the raw type
    operator Nest::TypeRef() const { return type_; }

    //! Checks if the type is valid (not-null)
    explicit operator bool() const { return type_ != nullptr; }

    //! Makes a cast based of the kind.
    //! If the current type has the kind of the given type, then make the cast. Otherwise,
    //! construct an empty result type.
    //! This is somehow similar to dynamic_cast, but based on statically available info.
    template <typename T> T kindCast() const {
        return type_ && kind() == T::staticKind() ? T(type_) : T();
    }

    //!@{ Type properties

    //! Returns the kind of the type
    int kind() const { return type_->typeKind; }

    //! Returns the mode of this type.
    //! Should only be called on non-null types
    Nest::EvalMode mode() const { return type_->mode; }

    //! Returns true if this type can be used at RT
    bool canBeUsedAtRt() const { return type_->canBeUsedAtRt; }

    //! Checks if the type has storage
    bool hasStorage() const { return type_->hasStorage; }

    //! Returns the number of references used for this type
    int numReferences() const { return type_->numReferences; }

    //! Returns the referred node; the node that introduced this type
    Nest::NodeHandle referredNode() const;

    //! Returns a string description of the type. Works even for null types
    const char* description() const { return type_ ? type_->description : "<null>"; }

    //!@}
    //!@{ Operations

    /**
     * @brief      Changes the mode of this type, and checks the operation.
     *
     * Will raise error if the mode cannot be changed.
     *
     * @param[in]  mode  The desired eval mode
     * @param[in]  loc   The location, used in case of reporting error
     *
     * @return     The type with the new eval mode; null if error
     */
    Type changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const;

    //!@}
};

/**
 * @brief      Returns true if the types are the same, ignoring the mode of the type.
 *
 * This work for all kinds of types.
 *
 * @param[in]  t1    The first type to compare
 * @param[in]  t2    The second type to compare
 *
 * @return     True if the two types are the same (ignoring the mode)
 */
bool sameTypeIgnoreMode(Type t1, Type t2);

//! Stream dump operator for types
ostream& operator<<(ostream& os, Type type);

} // namespace Nest

// custom specialization of std::hash can be injected in namespace std
namespace std {
template <> struct hash<Nest::Type> {
    size_t operator()(Nest::Type obj) const noexcept {
        return std::hash<Nest::TypeRef>()(obj.type_);
    }
};
} // namespace std

//! To be used inside type classes to declare type boilerplate code
#define DECLARE_TYPE_COMMON(T)                                                                     \
private:                                                                                           \
    static int staticKindValue;                                                                    \
                                                                                                   \
    static T changeTypeModeImpl(T type, Nest::EvalMode newMode);                                   \
                                                                                                   \
public:                                                                                            \
    using ThisType = T;                                                                            \
    static int registerTypeKind();                                                                 \
    static int staticKind();                                                                       \
    T() = default;                                                                                 \
    T(Nest::TypeRef t);

#define DEFINE_TYPE_COMMON_IMPL(T, BaseType)                                                       \
    int T::registerTypeKind() {                                                                    \
        auto chModeF = reinterpret_cast<FChangeTypeMode>(&T::changeTypeModeImpl); /*NOLINT*/       \
        staticKindValue = Nest_registerTypeKind(chModeF);                                          \
        return staticKindValue;                                                                    \
    }                                                                                              \
    int T::staticKindValue{-1};                                                                    \
    T::T(TypeRef type)                                                                             \
        : BaseType(type) {                                                                         \
        if (type && type->typeKind != T::staticKindValue)                                          \
            REP_INTERNAL(NOLOC, #T " constructed with other type kind (%1%)") % type;              \
    }                                                                                              \
    int T::staticKind() { return staticKindValue; }
