#pragma once

#include <boost/function.hpp>

namespace Nest { namespace Common
{
    /**
     * \brief   Helper class that can call specified actions on destructor
     *
     * This class can be used in RAII techniques to call actions on destructor.
     * It should be initialized with a functor. The functor will be called on destructor, when exiting the scope.
     *
     * This class implements move semantics, so that the action is called only once, no matter how many copies we make
     * of this object.
     *
     * The destruct actions is guarded by a try catch, so that the destructor doesn't throw.
     *
     * \see     saveValue()
     */
    class ScopeGuard
    {
    public:
        ScopeGuard();
        ScopeGuard(const boost::function<void()>& destructAction);
        ScopeGuard(const ScopeGuard& src);
        ~ScopeGuard();

        /// Dismisses this guard. The action won't be called at destruction
        void dismiss();

    private:
        boost::function<void()> destructAction_;
        mutable bool active_;
    };


    /// Helper function that creates a scope guard given the given destruct action
    ScopeGuard makeGuard(const boost::function<void()>& destructAction);
}}

#define __IMPL_SCOPEGUARD_CONCAT1(x,y) x##y
#define __IMPL_SCOPEGUARD_CONCAT(x,y) __IMPL_SCOPEGUARD_CONCAT1(x,y)

#define ON_BLOCK_EXIT(actionFunctor) \
    Nest::Common::ScopeGuard __IMPL_SCOPEGUARD_CONCAT(scopeGuard, __LINE__)(actionFunctor)
