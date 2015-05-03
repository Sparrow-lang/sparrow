#pragma once

#include "BackendFactory.h"

namespace Nest
{
    /// Simple implementation for the backend factory
    class BackendFactoryImpl : public BackendFactory
    {
    public:
        virtual Backend* createBackend(const string& name);

        virtual void registerBackend(const string& name, const CreateBackendFn& createFn);

    private:
        /// The map of registered backends; maps from backend name to the functor that creates the backend
        unordered_map<string, CreateBackendFn> backends_;
    };
}
