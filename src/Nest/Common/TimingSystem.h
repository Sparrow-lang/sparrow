#pragma once

#include <vector>
#include <string>

#include <boost/timer/timer.hpp>

namespace Nest { namespace Common
{
    /// Structure that holds the information for a timing item
    struct TimingItem
    {
        TimingItem();

        void addTime(double elapsed, bool addAsCurrentTime);

        string description_;   ///< Description for the current item
        double totalElapsed_;       ///< The time elapsed in the current timing item, and other items on the stack
        double selfElapsed_;        ///< The time elapsed only in this timing item
        unsigned long long count_;  ///< The number of times this timing item is called

        int idx_;   ///< Used internally, to avoid counting a timing item twice in recursive calls
    };

    /**
     * \brief   Class that encapsulates a timing system
     *
     * This class can be used to measure the execution performance of the program. It basically allows a set of timing
     * items to be created; these timing items can be nested. When counging the time for a particular timing item, we
     * measure the actual time spent in the timing item alone, but also we count the timer for all the timing items on
     * the stack. This way, we can have a good view on the way the program spends the execution time.
     *
     * The 'enter()' and 'leave()' functions can be used to enter/leave a timing item. Still it's better to
     * use the 'ScopedTimer' class or ENTER_TIMER or ENTER_TIMER_DESC macro to perform scoped timing.
     *
     * The timing items are identified by string keys, but you can also set textual descriptions to it.
     *
     * At the end, call the dump() method to dump the results to the cerr.
     *
     * \see     ScopedTimer, ENTER_TIMER, ENTER_TIMER_DESC
     */
    class TimingSystem
    {
    public:
        TimingSystem();
        ~TimingSystem();

        /// Sets a description to the timing item identified by the given key; creates the timing item if it does not exist
        void setTimingItemDescription(const string& key, const string& description);

        /// Enter the timer specified by the given key; creates the timing item if it does not exist
        void enter(const string& key);

        /// Leaves the last entered timing item; throws if no timing item is present
        void leave();

        /// Get the timing items
        const unordered_map<string, TimingItem>& timingItems() const { return timingItems_; }

        /// Dump the timing information
        void dump();

    private:
        /// Get the time elapsed from the last call of this method
        double getLastElapsed();

        /// Add the time elapsed from the last call to this method to all the timing items on the stack
        void addCurrentTime();

    private:
        /// The CPU timer object used to measure execution times
        boost::timer::cpu_timer timer_;

        /// The times last read from the timer
        boost::timer::cpu_times lastTimes_;

        /// The map of timing items
        unordered_map<string, TimingItem> timingItems_;

        /// The stack of timers currently active
        vector<TimingItem*> timersStack_;
    };

    /// Helper class to enter/leave a timer for a scope
    class ScopedTimer
    {
    public:
        ScopedTimer(TimingSystem* timingSystem, const string& key) : timingSystem_(timingSystem) { if ( timingSystem ) timingSystem->enter(key); }
        ~ScopedTimer() { if ( timingSystem_ ) timingSystem_->leave(); }

    private:
        TimingSystem* timingSystem_;
    };
}}

#define DISABLE_TIMING

#ifndef DISABLE_TIMING
    #define CAT1(x, y)  x##y
    #define CAT2(x, y)  CAT1(x,y)


    #define ENTER_TIMER(timingSystemPtr, key) \
        Nest::Common::ScopedTimer CAT2(scopedTimer_, __LINE__) (timingSystemPtr, key)
    #define ENTER_TIMER_DESC(timingSystemPtr, key, description) \
        if ( timingSystemPtr ) timingSystemPtr->setTimingItemDescription(key, description); \
        Nest::Common::ScopedTimer CAT2(scopedTimer_, __LINE__) (timingSystemPtr, key)
#else
    #define ENTER_TIMER(timingSystemPtr, key)                       ((void)0)
    #define ENTER_TIMER_DESC(timingSystemPtr, key, description)     ((void)0)
#endif
