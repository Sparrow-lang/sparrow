#pragma once

typedef struct Nest_Backend Backend;

/**
A module represents a functional block of the compiler.

One can create modules to implement "any kind" of functionality into the
compiler. Of course, one would like to interact with other modules, so typically
a module interacts with the Nest module.
*/
struct Nest_CompilerModule
{
    // Module description
    const char* name;
    const char* description;
    const char* author;
    const char* url;
    int majorVersion;
    int minorVersion;

    // Function called to initialize the module
    void (*initFun)();
    // Function called when the module is destroyed
    void (*destroyFun)();

    // Function called when the Backend was set
    void (*onBackendSetFun)(Backend*);
};
typedef struct Nest_CompilerModule Nest_CompilerModule;
typedef struct Nest_CompilerModule CompilerModule;
