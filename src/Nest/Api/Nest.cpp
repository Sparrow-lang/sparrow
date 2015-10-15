#include "Nest/Internal/StdInc.hpp"
#include "Nest.h"
#include "CompilerModule.h"

void Nest_initModule()
{
    // Any Nest initialization goes here
}

CompilerModule* getNestModule()
{
    CompilerModule* nestModule = new CompilerModule {
        "Nest",
        "The core of the compiler; defines the main services of the compiler, and the interactions between different components",
        "LucTeo",
        "www.lucteo.ro",
        1,
        0,
        &Nest_initModule,
        NULL,
        NULL
    };
    return nestModule;
}