#include <StdInc.h>
#include "ScopeGuard.h"

using namespace Nest::Common;

ScopeGuard::ScopeGuard()
    : destructAction_()
    , active_(true)
{
}

ScopeGuard::ScopeGuard(const boost::function<void()>& destructAction)
    : destructAction_(destructAction)
    , active_(true)
{
}

ScopeGuard::ScopeGuard(const ScopeGuard& src)
    : destructAction_(src.destructAction_)
    , active_(true)
{
    src.active_ = false;
}

ScopeGuard::~ScopeGuard()
{
    if ( active_ )
    {
        try
        {
    	    destructAction_();
        }
        catch (...)
        {
        }
    }
}

void ScopeGuard::dismiss()
{
    active_ = false;
}


ScopeGuard Nest::Common::makeGuard(const boost::function<void()>& destructAction)
{
    return ScopeGuard(destructAction);
}
