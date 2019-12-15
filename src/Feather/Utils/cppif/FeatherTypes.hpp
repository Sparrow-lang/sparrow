#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/TypeWithStorage.hpp"

namespace Feather {

using Nest::Type;
using Nest::TypeWithStorage;

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
struct VoidType : Type {
    DECLARE_TYPE_COMMON(VoidType)

    //! Returns an instance of this type, corresponding to the given mode
    static VoidType get(Nest::EvalMode mode);

    //! @copydoc Type::changeMode
    VoidType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const;
};

/**
 * @brief   A data type.
 *
 * A data type is the most essential type that Sparrow has. It just represents a type introduced by
 * a datatype declaration, possible with a different number of references.
 */
struct DataType : TypeWithStorage {
    DECLARE_TYPE_COMMON(DataType)

    /**
     * @brief      Factory method to create datatype types
     *
     * @param[in]  decl           The datatype declaration
     * @param[in]  numReferences  The number references for this type
     * @param[in]  mode           The mode
     *
     * @return     The corresponding datatype
     */
    static DataType get(Nest::NodeHandle decl, Nest::EvalMode mode = modeUnspecified);

    //! @copydoc Type::changeMode
    DataType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }
};

/**
 * @brief      A pointer type.
 *
 * A value of this type will hold a pointer to an object of the base type
 *
 * Constraints:
 *     - must be created on top of a type with storage
 */
struct PtrType : TypeWithStorage {
    DECLARE_TYPE_COMMON(PtrType)

    /**
     * @brief      Factory method to create a pointer type
     *
     * @param[in]  base  The type pointed at
     * @param[in]  loc   Location used when reporting errors
     *
     * @return     The corresponding pointer type
     */
    static PtrType get(TypeWithStorage base, Nest::Location loc = {});

    //! Returns the base type of this type
    TypeWithStorage base() const { return {type_->subTypes[0]}; }

    //! @copydoc Type::changeMode
    PtrType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }
};

/**
 * @brief      A Const type.
 *
 * A const type will not allow the users to change the value referred by this type.
 *
 * Constraints:
 *     - must be created on top of a type with storage
 *     - cannot be created on top of a MutableType/TempType
 */
struct ConstType : TypeWithStorage {
    DECLARE_TYPE_COMMON(ConstType)

    /**
     * @brief      Factory method to create a const type
     *
     * @param[in]  base  The base type on which we apply const-ness
     * @param[in]  loc   Location used when reporting errors
     *
     * @return     The corresponding const type
     */
    static ConstType get(TypeWithStorage base, Nest::Location loc = {});

    //! Returns the base type of this type
    TypeWithStorage base() const { return {type_->subTypes[0]}; }

    //! Transform this type into a corresponding DataType with the same number of references.
    TypeWithStorage toRef() const;

    //! @copydoc Type::changeMode
    ConstType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }
};

/**
 * @brief      A mutable type.
 *
 * A mutable type will not allow the users to change the value referred by this type.
 *
 * Constraints:
 *     - must be created on top of a type with storage
 *     - cannot be created on top of a ConstType/TempType
 */
struct MutableType : TypeWithStorage {
    DECLARE_TYPE_COMMON(MutableType)

    /**
     * @brief      Factory method to create a mutable type
     *
     * @param[in]  base  The base type on which we apply mutable-ness
     * @param[in]  loc   Location used when reporting errors
     *
     * @return     The corresponding mutable type
     */
    static MutableType get(TypeWithStorage base, Nest::Location loc = {});

    //! Returns the base type of this type
    TypeWithStorage base() const { return {type_->subTypes[0]}; }

    //! Transform this type into a corresponding DataType with the same number of references.
    TypeWithStorage toRef() const;

    //! @copydoc Type::changeMode
    MutableType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }
};

/**
 * @brief      A type representing a temporary object.
 *
 * A temp type will not allow the users to change the value referred by this type.
 *
 * Constraints:
 *     - must be created on top of a type with storage
 *     - cannot be created on top of a ConstType/MutableType
 */
struct TempType : TypeWithStorage {
    DECLARE_TYPE_COMMON(TempType)

    /**
     * @brief      Factory method to create a temp type
     *
     * @param[in]  base  The base type on which we apply temp-ness
     * @param[in]  loc   Location used when reporting errors
     *
     * @return     The corresponding temp type
     */
    static TempType get(TypeWithStorage base, Nest::Location loc = {});

    //! Returns the base type of this type
    TypeWithStorage base() const { return {type_->subTypes[0]}; }

    //! Transform this type into a corresponding DataType with the same number of references.
    TypeWithStorage toRef() const;

    //! @copydoc Type::changeMode
    TempType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
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
    DECLARE_TYPE_COMMON(ArrayType)

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
    TypeWithStorage unitType() const { return {type_->subTypes[0]}; }
    //! Returns the number of elements in the array
    int count() const { return type_->flags; }

    //! @copydoc Type::changeMode
    ArrayType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
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
    DECLARE_TYPE_COMMON(FunctionType)

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
    Type result() const { return type_->subTypes[0]; }

    //! Returns the number of parameters that we have
    int numParams() const { return type_->numSubtypes - 1; }
    //! Access operators for the parameters
    TypeWithStorage operator[](int idx) const { return {type_->subTypes[idx + 1]}; }

    //! @copydoc Type::changeMode
    FunctionType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }
};

//! Determines if the type is data-like: DataType, ConstType, MutableType, TempType
bool isDataLikeType(Type type);

//! Determines if the type is a category type: ConstType, MutableType, TempType
bool isCategoryType(Type type);

/**
 * @brief      Removes a reference from the given type.
 *
 * Constraints:
 *     - given type must be Data-like type
 *     - given type must have at least one (non-category) reference
 *     - if the given type is a category type, it needs to have an extra reference
 *
 * Postconditions:
 *     - the category of the type is kept
 *
 * @param[in]  type  The type to remove reference from
 *
 * @return     The resulting type, with one less reference
 */
TypeWithStorage removeRef(TypeWithStorage type);

//! Dereference the types; ensure that the number of references is less with 1
TypeWithStorage dereferenceType(TypeWithStorage type);

/**
 * @brief      Removes all references from this type
 *
 * Constraints:
 *     - given type must be Data-like type
 *     - returned type is always DataType
 *
 * @param[in]  type  The type to remove references from
 *
 * @return     The resulting type, with no references
 */
TypeWithStorage removeAllRefs(TypeWithStorage type);

/**
 * @brief      Removes the category from the type, if present.
 *
 * The given type can be any type (even without storage).
 * If the type is Const/Mutable/Temp, we will always create a DataType.
 *
 * @param[in]  type  The type to remove category from
 *
 * @return     Type without Const/Mutable/Temp
 */
Type removeCategoryIfPresent(Type type);
TypeWithStorage removeCategoryIfPresent(TypeWithStorage type);

/**
 * @brief      If the given type is a category, transform it to ref.
 *
 * The given type can be any type (even without storage).
 * If the type is Const/Mutable/Temp, we will always create a DataType.
 *
 * @param[in]  type  The type, that may be an Const/Mutable/Temp
 *
 * @return     Equivalent DataType
 */
Type categoryToRefIfPresent(Type type);

/**
 * @brief      Change the category of the given type
 *
 * @param[in]  type        The type to change the category for
 * @param[in]  newCatKind  The kind representing the new category
 *
 * @return     The type with the category changed.
 *
 * The @a newCatKind must match a category type.
 * The given type must have at least one reference.
 *
 * The resulting type will have the same number of references as the original type.
 */
TypeWithStorage changeCat(TypeWithStorage type, int newCatKind);

} // namespace Feather
