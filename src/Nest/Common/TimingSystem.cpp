#include <StdInc.h>
#include "TimingSystem.h"

#include <boost/format.hpp>
#include <limits>
#include <sstream>

using namespace Nest::Common;

TimingItem::TimingItem()
    : totalElapsed_(0.0)
    , selfElapsed_(0.0)
    , count_(0)
    , idx_(-1)
{
}

void TimingItem::addTime(double elapsed, bool addAsCurrentTime)
{
    totalElapsed_ += elapsed;
    if ( addAsCurrentTime )
        selfElapsed_ += elapsed;
}


#ifndef DISABLE_TIMING
namespace
{
    int getPercent(double curVal, double maxVal)
    {
        static const double epsilon = numeric_limits<double>::epsilon();
        return maxVal <= epsilon ? 100 : (int) (100*curVal / maxVal + 0.5);
    }
}
#endif

TimingSystem::TimingSystem()
{
    timer_.start();
    getLastElapsed();
}

TimingSystem::~TimingSystem()
{
}

void TimingSystem::setTimingItemDescription(const string& key, const string& description)
{
    timingItems_[key].description_ = description;
}

void TimingSystem::enter(const string& key)
{
    addCurrentTime();
    TimingItem* item = &timingItems_[key];
    item->idx_ = timersStack_.size();   // Don't count this item twice on the stack
    ++item->count_;
    timersStack_.push_back(item);
}

void TimingSystem::leave()
{
    addCurrentTime();
    timersStack_.pop_back();

    // Update indices
    for ( size_t i=0; i<timersStack_.size(); ++i )
        timersStack_[i]->idx_ = i;
}

double TimingSystem::getLastElapsed()
{
    boost::timer::cpu_times curTimes = timer_.elapsed();
    double seconds = double(curTimes.wall - lastTimes_.wall) * 1.0e-9;
    lastTimes_ = curTimes;
    return seconds;
}

void TimingSystem::addCurrentTime()
{
    double elapsed = getLastElapsed();
    size_t sz = timersStack_.size();
    for ( size_t i=0; i<sz; ++i )
        if ( (int) i == timersStack_[i]->idx_ )
            timersStack_[i]->addTime(elapsed, i+1 == sz);
}


void TimingSystem::dump()
{
#ifndef DISABLE_TIMING
    addCurrentTime();

    // Get the total time
    double totalTime = 0.0;
    for ( const auto& p: timingItems_ )
    {
        totalTime += p.second.selfElapsed_;
    }

    cerr << "    ----------------------------------------------------------------" << endl;
    cerr << "    |   Absolute    |     Self      |     Count     | Description   " << endl;
    cerr << "    ----------------------------------------------------------------" << endl;
    for ( const auto& p: timingItems_ )
    {
        double selfElapsed = p.second.selfElapsed_;
        double totalElapsed = p.second.totalElapsed_;
        boost::format fmt("    | %7.3fs %3d%% | %7.3fs %3d%% | %9d | %s");
        fmt % totalElapsed % getPercent(totalElapsed, totalTime)
            % selfElapsed % getPercent(selfElapsed, totalElapsed)
            % p.second.count_
            % (p.second.description_.empty() ? p.first : p.second.description_);
        cerr << fmt.str() << endl;
    }
    cerr << "    ------------------------------------------------" << endl;
#endif
}
