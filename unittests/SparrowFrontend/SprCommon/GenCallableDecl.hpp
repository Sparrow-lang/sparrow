#pragma once

#include "GenGenericParams.hpp"

using Nest::CompilationContext;
using SprFrontend::ConceptDecl;
using SprFrontend::DataTypeDecl;
using SprFrontend::PackageDecl;
using SprFrontend::SprFunctionDecl;

//! Generator for an arbitrary (generic) function.
//! Note: in some cases this may be a regular function (i.e., no params at all)
//! If paramsData is given, it will fill it with the params data used to generate this function
rc::Gen<SprFunctionDecl> arbFunction(bool ifClauseVal = true);

//! Same as above, but with the given paramsData
rc::Gen<SprFunctionDecl> arbFunction(const ParamsData& paramsData, bool ifClauseVal = true);

//! Creates a function with the given params (and the given if clause value)
//! Does not need to be called from a rc::exec context
SprFunctionDecl genFunction(Feather::NodeList params, bool ifClauseVal = true);

//! Generator for arbitrary Feather function declarations
rc::Gen<Feather::FunctionDecl> arbFeatherFunction(const ParamsData& paramsData);
rc::Gen<Feather::FunctionDecl> arbFeatherFunction();

//! Generator for arbitrary generic packages
rc::Gen<PackageDecl> arbGenPackage(const ParamsData& paramsData, bool ifClauseVal = true);
rc::Gen<PackageDecl> arbGenPackage(bool ifClauseVal = true);

//! Generator for arbitrary generic datatypes
rc::Gen<DataTypeDecl> arbGenDatatype(const ParamsData& paramsData, bool ifClauseVal = true);
rc::Gen<DataTypeDecl> arbGenDatatype(bool ifClauseVal = true);

//! Generator for arbitrary concrete datatypes
rc::Gen<DataTypeDecl> arbConcreteDatatype();
//! Generator for arbitrary concepts
rc::Gen<ConceptDecl> arbConcept();

//! Generator or any callable decl (any of above)
rc::Gen<NodeHandle> arbCallableDecl();
