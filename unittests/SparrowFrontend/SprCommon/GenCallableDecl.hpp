#pragma once

#include "GenGenericParams.hpp"

using SprFrontend::ConceptDecl;
using SprFrontend::DataTypeDecl;
using SprFrontend::PackageDecl;
using SprFrontend::SprFunctionDecl;

//! Generator class for callable decls.
//! This knows to generate the following decls
//!     - functions (generic or not)
//!     - generic packages
//!     - generic datatypes
//!     - datatypes (with ctors)
//!     - concepts
//!
//! It uses a GenGenericParams object to generate the parameters needed for the callable.
//! Each time it generates a decls, it will reset the GenGenericParams object.
class GenCallableDecl {
public:
    GenCallableDecl(const Location& loc, CompilationContext* ctx, const SampleTypes& types);

    SprFunctionDecl genFunction(bool ifClauseVal = true);
    PackageDecl genGenPackage();
    DataTypeDecl genGenDatatype();
    DataTypeDecl genConcreteDatatype();
    ConceptDecl genConcept();

    //! Generates any of the above
    NodeHandle genCallableDecl();

    //! Returns the object used to generate the parameters for our decls
    const GenGenericParams& paramsGenerator() const { return paramsGenerator_; }

private:
    //! Object used to generate the params for our decls
    GenGenericParams paramsGenerator_;
    //! The location at which we generate our decls
    Location location_;
    //! The compilation context to be set for the generated decls
    CompilationContext* context_;
    //! The sample types to be used when generating parameters
    const SampleTypes& types_;
};
