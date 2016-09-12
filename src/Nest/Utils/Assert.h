#pragma once

// Define a custom assert of MSVC compiler, use the standard assert for the rest
#ifndef NDEBUG
    #if _MSC_VER > 1000
        #include <crtdbg.h>
        #include <stdio.h>
        #define ASSERT(expr) (void)( (!!(expr)) || ( (fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", (#expr), __FILE__, __LINE__), _CrtDbgBreak(), 0) )
        #define VERIFY ASSERT
    #else
        #include <assert.h>
        #define ASSERT(expr) assert(expr)
        #define VERIFY ASSERT
    #endif
#else
    #define ASSERT(expr) if (1) ((void)0); else ((void)0)
    #define VERIFY(expr) (expr)
#endif

