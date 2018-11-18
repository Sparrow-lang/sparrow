#pragma once

#include "Nest/Utils/cppif/Type.hpp"

//! Mock type that we can use in our unit-tests
struct MockType : Nest::Type {
    //! Registers this type
    static void registerType();

    //! Gets the type corresponding to the given parameters
    static MockType get(bool hasStorage, int numRefs, Nest::EvalMode mode);

    MockType() = default;
    MockType(Nest::TypeRef type);

    //! Returns a new type with the given mode, and the rest of parameters the same
    MockType changeMode(Nest::EvalMode newMode, Nest::Location loc = Nest::Location{}) const;

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
