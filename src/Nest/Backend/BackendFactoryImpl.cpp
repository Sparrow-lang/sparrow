#include <StdInc.h>
#include "BackendFactoryImpl.h"

using namespace Nest;

Backend* BackendFactoryImpl::createBackend(const string& name)
{
    auto it = backends_.find(name);
    if ( it != backends_.end() )
        return it->second();
    else
        return nullptr;
}

void BackendFactoryImpl::registerBackend(const string& name, const CreateBackendFn& createFn)
{
    backends_[name] = createFn;
}