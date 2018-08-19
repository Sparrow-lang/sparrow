#pragma once

//! Mock type that we can use in our unit-tests
struct MockType {
    //! Registers this type
    static void registerType();

    //! Gets the type corresponding to the given parameters
    static MockType get(bool hasStorage, int numRefs, Nest::EvalMode mode);

    //! The actual type that we are wrapping
    Nest::TypeRef type{nullptr};

    MockType();
    MockType(Nest::TypeRef t);

    //! Implicit conversion to Nest::TypeRef
    operator Nest::TypeRef() { return type; }

    //! Returns the kind of this type -- should be the same for all types of this kind
    int kind() const;

    //! Indicates if this type has storage
    bool hasStorage() const;
    //! Indicates the number of references for this type
    int numReferences() const;
    //! Indicates the evaluation mode for this type
    Nest::EvalMode mode();

    //! Returns a new type with the given mode, and the rest of parameters the same
    MockType changeMode(Nest::EvalMode newMode) const;

private:
    //! The kind of this type; initialized after a call to registerType()
    static int typeKind;
};

ostream& operator<<(ostream& os, MockType t);

//! Returns a generator for our mock type
rc::Gen<MockType> arbMockType();

namespace rc {

//! Helper to make RapidCheck generate arbitrary types
template <> struct Arbitrary<MockType> {
    static Gen<MockType> arbitrary() { return arbMockType(); }
};

} // namespace rc
