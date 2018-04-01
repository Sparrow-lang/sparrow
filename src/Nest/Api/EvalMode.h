#pragma once

/// Enumeration that determines the evaluation mode of nodes: rt, ct
enum Nest_EvalMode {
    modeUnspecified = 0,
    modeRt, //!< Available both at run-time and compile-time, depending on the invocation context
    modeCt, //!< Only available at compile-time
};

typedef enum Nest_EvalMode Nest_EvalMode;
typedef enum Nest_EvalMode EvalMode;
