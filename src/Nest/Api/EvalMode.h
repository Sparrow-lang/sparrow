#pragma once

/// Enumeration that determines the evaluation mode of nodes: rt, ct, or rtct
enum Nest_EvalMode {
    modeUnspecified = 0,
    modeRt,   ///< Only available at run-time
    modeCt,   ///< Only available at compile-time
    modeRtCt, ///< Available both at run-time and compile-time, depending on the context in which
              ///< invoked
};

typedef enum Nest_EvalMode Nest_EvalMode;
typedef enum Nest_EvalMode EvalMode;
