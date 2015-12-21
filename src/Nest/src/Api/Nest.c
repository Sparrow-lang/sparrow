#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"

void Nest_initModule()
{
    // Any Nest initialization goes here
}

static CompilerModule theNestModule = {
        "Nest",
        "The core of the compiler; defines the main services of the compiler, and the interactions between different components",
        "LucTeo",
        "www.lucteo.ro",
        1,
        0,
        &Nest_initModule,
        0,
        0
    };

CompilerModule* getNestModule()
{
    return &theNestModule;
}