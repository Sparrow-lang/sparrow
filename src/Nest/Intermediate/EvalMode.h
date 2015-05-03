#pragma once

#include <iostream>

namespace Nest
{
    /// Enumeration that determines the evaluation mode of nodes: rt, ct, or rtct
    enum EvalMode
    {
        modeUnspecified = 0,
        modeRt,             ///< Only available at run-time
        modeCt,             ///< Only available at compile-time
        modeRtCt,           ///< Available both at run-time and compile-time, depending on the context in which invoked
    };

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, EvalMode mode)
    {
        switch ( mode )
        {
        case modeRt:        os << "rt"; break;
        case modeCt:        os << "ct"; break;
        case modeRtCt:      os << "rtct"; break;
        default:            os << "unspecified"; break;
        }
        return os;
    }
}
