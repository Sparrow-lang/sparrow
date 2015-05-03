#pragma once

// Define a custom assert of MSVC compiler, use the standard assert for the rest
#ifndef NDEBUG
    #if _MSC_VER > 1000
        #include <crtdbg.h>
        #include <iostream>
        #define ASSERT(expr) (void)( (!!(expr)) || ( (cerr << "Assertion failed: " << (#expr) << ", file "##__FILE__##", line " << __LINE__ << endl), _CrtDbgBreak(), 0) )
        #define VERIFY ASSERT
    #else
        #include <assert.h>
        #define ASSERT(expr) assert(expr)
        #define VERIFY ASSERT
    #endif
#else
    #define ASSERT(expr) if (true) ((void)0); else (!!(expr))
    #define VERIFY(expr) (expr)
#endif

