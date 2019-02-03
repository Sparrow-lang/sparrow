#pragma once

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using Feather::NodeList;
using Feather::TypeWithStorage;
using Nest::NodeHandle;
using SprFrontend::ParameterDecl;

struct SampleTypes;

//! The maximum number of parameters that we might have
constexpr int maxNumParams = 10;

//! The options used for generating generic params
struct ParamsGenOptions {
    int minNumParams{0};
    bool useConcept{true};
    bool useCt{true};
    bool useRt{true};
    bool useDependent{true};
};

//! The data generated for parameters
struct ParamsData {
    //! The params list node generated
    NodeList paramsNode_;

    //! The number of parameters generated so far
    int numParams_{0};
    //! The parameter declarations generated
    ParameterDecl params_[maxNumParams];
    //! The types of the parameters generated; null for dependent params
    TypeWithStorage types_[maxNumParams];
    //! For dependent params this will indicate the param dependent upon; -1 for non-dependent param
    int dependentIndices_[maxNumParams];

    //! Returns true if the generated params contain a concept param
    bool usesConcepts() const;

    //! Returns true if at least one parameter is CT
    bool hasCtParams() const;

    //! Returns true if there is at least one dependent parameter generated
    bool hasDepedentParams() const;

    //! true iff usesConcepts || hasCtParams || hasDepedentParams
    bool isGeneric() const;
};

//! Generator for parameters
rc::Gen<ParamsData> arbParamsData(ParamsGenOptions options = {});

//! Generator for bound values that match the given generated parameters
rc::Gen<vector<NodeHandle>> arbBoundValues(const ParamsData& params, const SampleTypes& types);

//! Generator for arguments corresponding to the given params
//! The arguments will be convertible to the params if exactMatch==false
rc::Gen<vector<NodeHandle>> arbArguments(const ParamsData& params,
        const SampleTypes* sampleTypes = nullptr, bool exactMatch = false);

//! Semantic check a set of generated values
void semanticCheck(const vector<NodeHandle>& values, Nest::CompilationContext* ctx);
