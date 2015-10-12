#pragma once

#include <NodeCommonsH.h>

#include <Helpers/Convert.h>

#include <Nest/Intermediate/NodeVector.hpp>

namespace SprFrontend
{
    /// Base class for all the callable items in Sparrow.
    /// A callable is an entity (declaration) that can be called by a fun application.
    class Callable
    {
    public:
        virtual ~Callable() {}

        /// Getter for the location of this callable (i.e., location of the function)
        virtual const Location& location() const = 0;
        /// Method used to get a string representation of the callable (i.e., function name)
        virtual string toString() const = 0;

        /// The number of parameters of the callable
        virtual size_t paramsCount() const = 0;
        /// Returns the parameter at the given index
        virtual Node* param(size_t idx) const = 0;
        /// The type of the parameter at the given index
        virtual TypeRef paramType(size_t idx) const;
        /// The evaluation mode of the callable (declaration)
        virtual EvalMode evalMode() const = 0;
        /// An autoCt callable is a 'rtct' callable for which passing all CT arguments will make a CT call
        virtual bool isAutoCt() const = 0;


        /// Checks if we can call this with the given arguments
        /// This method can cache some information needed by the 'generateCall'
        virtual ConversionType canCall(CompilationContext* context, const Location& loc, NodeRange args, EvalMode evalMode, bool noCustomCvt = false);

        /// Same as above, but makes the check only on type, and not on the actual argument; doesn't cache any args_
        virtual ConversionType canCall(CompilationContext* context, const Location& loc, const vector<TypeRef>& argTypes, EvalMode evalMode, bool noCustomCvt = false);

        /// Generates the node that actually calls this callable
        /// This must be called only if 'canCall' method returned a success conversion type
        virtual Node* generateCall(const Location& loc) = 0;

    protected:
        Callable();

        /// If the parameter with the given index has a default value, this will return that expression
        Node* paramDefaultVal(size_t idx) const;

        /// Returns the arguments with the conversions applied
        NodeVector argsWithConversion();

    protected:
        /// The context in which we may generate the call; set by 'canCall'
        CompilationContext* context_;

        /// The original arguments passed to 'canCall'
        NodeVector args_;

        /// The conversions needed for each argument; computed by 'canCall'
        vector<ConversionResult> conversions_;
    };

    typedef vector<Callable*> Callables;

    /// Destroys the given list of callables
    void destroyCallables(Callables& callables);
}
