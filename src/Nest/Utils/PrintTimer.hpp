#pragma once

#include <boost/timer/timer.hpp>


namespace Nest { namespace Common
{
    /// Helper timer that prints the elapsed time at the console
    ///
    /// In order for this to do something, the "enable" constructor parameter must be true; otherwise this has no effect.
    class PrintTimer
    {
        typedef boost::timer::cpu_timer TimerType;
        typedef boost::optional<TimerType> OptType;

    public:
        PrintTimer(bool enable, const char* beginStr, const char* endStrFormat, int places = 3)
            : places_(places)
            , endStrFormat_(endStrFormat)
        {
            if ( enable )
            {
                cout << beginStr;
                timer_ = TimerType();
            }
        }

        ~PrintTimer()
        {
            if ( timer_ )
            {
                timer_->stop();
                cout << timer_->format(places_, endStrFormat_);
            }
        }

    private:
        int places_;
        string endStrFormat_;
        OptType timer_;
    };
}}
