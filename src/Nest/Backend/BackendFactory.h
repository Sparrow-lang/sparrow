#pragma once

#include <functional>

FWD_CLASS1(Nest, Backend);

namespace Nest
{
    /// Interface for a factory class that creates backend instances given the name of the backend
    class BackendFactory
    {
    public:
        /// Functor types that creates backend objects
        typedef function<Backend* ()> CreateBackendFn;

        virtual ~BackendFactory() {}

        /// Creates the backend that is represented by the given name
        /// Returns nullptr if no backend is found
        virtual Backend* createBackend(const string& name) = 0;

        /// Registers a backend by its name
        virtual void registerBackend(const string& name, const CreateBackendFn& createFn) = 0;
    };
}
