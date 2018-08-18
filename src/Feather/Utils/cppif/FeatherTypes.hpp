#pragma once

#include "Nest/Api/Type.h"
#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

namespace Feather {

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
struct TypeBase {
    //! The actual type we are wrapping
    Nest::TypeRef type_{};

    //! Constructor that initializes this with a null type
    TypeBase() = default;
    //! Constructor from the raw Nest::TypeRef
    TypeBase(Nest::TypeRef type)
        : type_(type) {}

    //! Implicit conversion to the raw type
    operator Nest::TypeRef() const { return type_; }

    //! Checks if the type is valid (not-null)
    explicit operator bool() const { return type_ != nullptr; }

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
    TypeBase changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{});

    //!@}
};

/**
 * @brief   Wrapper for a type indicating the absence of a data type.
 *
 * Used for functions that don't return anything.
 *
 * Always has a mode associated with it. We can have therefore two types of Void types, depending on
 * the mode.
 *
 * @see     TypeWithStorage
 */
struct VoidType : TypeBase {
    //! Constructor that initializes this with a null type
    VoidType() = default;
    //! Constructor from the raw Nest::TypeRef
    explicit VoidType(Nest::TypeRef type);

    //! Returns an instance of this type, corresponding to the given mode
    static VoidType get(Nest::EvalMode mode);

    //! @copydoc TypeBase::changeMode
    VoidType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{});
};

/**
 * @brief   Base class for all wrapper structs of types with storage.
 *
 * You cannot directly create objects of this type. Used derived classes
 *
 * @see     DataTypes
 */
struct TypeWithStorage : TypeBase {
    TypeWithStorage() = default;
    TypeWithStorage(Nest::TypeRef type);

    //!@{ Type properties

    //! Returns the number of references used for this type
    int numReferences() const { return type_->numReferences; }

    //! Returns the referred node; the node that introduced this type
    Nest::NodeHandle referredNode() const { return type_->referredNode; }

    //!@}
    //!@{ Operations

    //! @copydoc TypeBase::changeMode
    TypeWithStorage changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return TypeWithStorage(TypeBase::changeMode(mode, loc));
    }

    //!@}
};

/**
 * @brief   A data type.
 *
 * A data type is the most essential type that Sparrow has. It just represents a type introduced by
 * a datatype declaration, possible with a different number of references.
 */
struct DataType : TypeWithStorage {
    DataType() = default;
    DataType(Nest::TypeRef type);

    /**
     * @brief      Factory method to create datatype types
     *
     * @param[in]  decl           The datatype declaration
     * @param[in]  numReferences  The number references for this type
     * @param[in]  mode           The mode
     *
     * @return     The corresponding datatype
     */
    static DataType get(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode);

    //! @copydoc TypeBase::changeMode
    DataType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return DataType(TypeBase::changeMode(mode, loc));
    }
};

/**
 * @brief      A LValue type.
 *
 * This represents the type of a variable. Something that we can take the address of.
 *
 * Constraints:
 *     - must be created on top of a type with storage
 */
struct LValueType : TypeWithStorage {
    LValueType() = default;
    LValueType(Nest::TypeRef type);

    /**
     * @brief      Factory method to create a LValue type
     *
     * @param[in]  base  The base type on which we apply LValue-ness
     *
     * @return     The corresponding LValue type
     */
    static LValueType get(TypeWithStorage base);

    //! Returns the base type of this type
    TypeWithStorage base() const { return TypeWithStorage(type_->subTypes[0]); }

    //! Transform this type into a corresponding DataType with the same number of references.
    DataType toRef() const;

    //! @copydoc TypeBase::changeMode
    LValueType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return LValueType(TypeBase::changeMode(mode, loc));
    }
};

/**
 * @brief      The type corresponding to an array.
 *
 * Constraints:
 *     - must be created on top of a type with storage
 *     - the count must be greater than 0
 */
struct ArrayType : TypeWithStorage {
    ArrayType() = default;
    ArrayType(Nest::TypeRef type);

    /**
     * @brief      Factory method to create an array type
     *
     * @param[in]  unitType  The unit type of the array
     * @param[in]  count     The number of elements in the array
     *
     * @return     The corresponding array type
     */
    static ArrayType get(TypeWithStorage unitType, int count);

    //! Returns the unit type of this array type
    TypeWithStorage unitType() const { return TypeWithStorage(type_->subTypes[0]); }
    //! Returns the number of elements in the array
    int count() const { return type_->flags; }

    //! @copydoc TypeBase::changeMode
    ArrayType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return ArrayType(TypeBase::changeMode(mode, loc));
    }
};

/**
 * @brief      The type of a function.
 *
 * Contains a series of types, the first one being the result type.
 * The result type can be non-storage.
 *
 * Constraints:
 *     - must be created with at least one type (the result type)
 *     - all parameter types, must be with storage
 */
struct FunctionType : TypeWithStorage {
    FunctionType() = default;
    FunctionType(Nest::TypeRef type);

    /**
     * @brief      Factory method to create a function type
     *
     * @param      resultTypeAndParams  Array of types for the result and params; at least one
     * @param[in]  numTypes             The number types; must be >= 1
     * @param[in]  mode                 The mode for the function
     *
     * @return     The corresponding function type
     */
    static FunctionType get(Nest::TypeRef* resultTypeAndParams, int numTypes, Nest::EvalMode mode);

    //! Returns the result type
    TypeBase result() const { return type_->subTypes[0]; }

    //! Returns the number of parameters that we have
    int numParams() const { return type_->numSubtypes - 1; }
    //! Access operators for the parameters
    TypeWithStorage operator[](int idx) const { return TypeWithStorage(type_->subTypes[idx + 1]); }

    //! @copydoc TypeBase::changeMode
    FunctionType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return FunctionType(TypeBase::changeMode(mode, loc));
    }
};

/**
 * @brief      Adds a reference to the given type.
 *
 * The total number of references for the resulting type will be 1 + the total number of references
 * of the given type.
 *
 * Constraints:
 *     - given type must be DataType or LValueType
 *     - returned type is always DataType
 *
 * @param[in]  type  The type to add reference to
 *
 * @return     The resulting type, with one more reference
 */
DataType addRef(TypeWithStorage type);

/**
 * @brief      Removes a reference from the given type.
 *
 * Constraints:
 *     - given type must have at least one reference
 *     - given type must be DataType or LValueType
 *     - returned type is always DataType
 *
 * @param[in]  type  The type to remove reference from
 *
 * @return     The resulting type, with one less reference
 */
DataType removeRef(TypeWithStorage type);

/**
 * @brief      Removes all references from this type
 *
 * Constraints:
 *     - given type must be DataType or LValueType
 *     - returned type is always DataType
 *
 * @param[in]  type  The type to remove references from
 *
 * @return     The resulting type, with no references
 */
DataType removeAllRefs(TypeWithStorage type);

/**
 * @brief      Removes the LValue from the type, if present.
 *
 * The given type can be any type (even without storage).
 * If the type is LValue, we will always create a DataType.
 *
 * @param[in]  type  The type to remove LValue from
 *
 * @return     Type without LValue, or the original type if no LValue was present
 */
TypeBase removeLValueIfPresent(TypeBase type);

/**
 * @brief      If the given type is an LValue, transform it to ref.
 *
 * The given type can be any type (even without storage).
 * If the type is LValue, we will always create a DataType.
 *
 * @param[in]  type  The type, that may be an LValue
 *
 * @return     Equivalent DataType
 */
TypeBase lvalueToRefIfPresent(TypeBase type);

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
bool sameTypeIgnoreMode(TypeBase t1, TypeBase t2);

//! Stream dump operator for types
ostream& operator<<(ostream& os, TypeBase type);

} // namespace Feather
