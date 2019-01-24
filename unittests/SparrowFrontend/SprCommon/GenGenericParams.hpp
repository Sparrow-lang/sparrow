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

    GenGenericParams(const Location& loc, CompilationContext* ctx, Options options,
            const SampleTypes* types = nullptr);

    //! Generate a list of parameters
    NodeList genParameters();

    //! Generate a set of bound values corresponding to the generated parameters
    //! Assumes we are using only numeric types
    vector<NodeHandle> genBoundValues();

    //! Returns the inner data; useful for testing
    const Data& innerData() const { return data_; }

    //! Returns true if the generated params contain a concept param
    bool usesConcepts() const;

    //! Returns true if at least one parameter is CT
    bool hasCtParams() const;

    //! Returns true if there is at least one dependent parameter generated
    bool hasDepedentParams() const;

    //! true iff usesConcepts || hasCtParams || hasDepedentParams
    bool isGeneric() const;

private:
    //! Generate a new parameter
    ParameterDecl genParam(const Location& loc, StringRef name);

    //! Generates a type to be used for a parameters
    TypeWithStorage genType() const;

    //! Generate a value for the given type
    NodeHandle genValueForType(const Location& loc, TypeWithStorage t);

    //! Data containing the generated params
    Data data_;

    //! The options used in generation
    Options options_;

    //! The location at which we are generating the paramters
    Location location_;
    //! The compilation context to be used for the values generated
    CompilationContext* context_;

    //! The types we are using; needed to be able to generate values
    //! If null, getBoundValues() cannot be called.
    const SampleTypes* types_{nullptr};
};
