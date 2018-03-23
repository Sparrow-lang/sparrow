#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    allocGeneral,
    allocNode,
    allocType,
    allocString,

    numAllocBuckets
} AllocBucket;

/// Call this function to allocate some memory in the compiler.
/// It tries to allocate in pages, therefore maximizing locality.
void* alloc(unsigned int size, AllocBucket bucket);

/// Returns a pointer that can hold up to the given amount of bytes, but does not
/// mark the full region as occupied. Instead, the user should call endBuffer()
/// to indicate the actual length of the buffer
void* startBuffer(unsigned int size, AllocBucket bucket);
/// Ends a buffer started with startBuffer()
void endBuffer(unsigned int actualSize, AllocBucket bucket);

/// Allocate memory for a string of the given length
char* allocStr(unsigned int len);

/// Duplicate the given string
char* dupString(const char* str);

/// Similar to startBuffer, but for strings
char* startString(unsigned int maxLen);
/// Similar to endBuffer, but for strings
void endString(unsigned int actualLen);

#ifdef __cplusplus
}
#endif
