#pragma once

#include "SparrowFrontend/Nodes/Decl.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

using Feather::NodeList;
using Feather::TypeWithStorage;
using Nest::CompilationContext;
using Nest::Location;
using Nest::NodeHandle;
using Nest::StringRef;
using SprFrontend::ParameterDecl;

struct SampleTypes;

class GenGenericParams {
public:
    //! The maximum number of parameters that we might have
    static constexpr int maxNumParams = 10;

    //! The options used for generating generic params
    struct Options {
        bool useConcept{true};
        bool useCt{true};
        bool useRt{true};
        bool useDependent{true};
    };

    //! Inner data
    struct Data {
        //! The number of parameters generated so far
        int numParams_{0};
        //! The parameter declarations generated
        ParameterDecl params_[maxNumParams];
        //! The types of the parameters generated; null for dependent params
        TypeWithStorage types_[maxNumParams];
        //! For dependent params this will indicate the param dependent upon; -1 if not a dependent
        //! param
        int dependentIndices_[maxNumParams];
    };

    GenGenericParams(Options options);

    //! Generate a list of parameters
    NodeList genParameters(const Location& loc);

    //! Generate a set of bound values corresponding to the generated parameters
    //! Assumes we are using only numeric types
    vector<NodeHandle> genBoundValues(
            const Location& loc, CompilationContext* ctx, const SampleTypes& types);

    //! Returns the inner data; useful for testing
    const Data& innerData() const { return data_; }

    //! Returns true if the generated params contain a concept param
    bool usesConcepts() const;

    //! Returns true if at least one parameter is CT
    bool hasCtParams() const;

    //! Returns true if there is at least one dependent parameter generated
    bool hasDepedentParams() const;

private:
    //! Generate a new parameter
    ParameterDecl genParam(const Location& loc, StringRef name);

    //! Generates a type to be used for a parameters
    TypeWithStorage genType() const;

    //! Generate a value for the given type
    static NodeHandle genValueForType(Location loc, TypeWithStorage t, const SampleTypes& types);

    //! Data containing the generated params
    Data data_;

    //! The options used in generation
    Options options_;
};
