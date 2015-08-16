#pragma once

typedef enum
{
    allocGeneral,
    allocNode,
    allocType,
    allocString,

    numAllocBuckets
} AllocBucket;


/// Call this function to allocate some memory in the compiler.
/// It tries to allocate in pages, therefore maximizing locality.
void* alloc(size_t size, AllocBucket bucket = allocGeneral);

/// Returns a pointer that can hold up to the given amount of bytes, but does not
/// mark the full region as occupied. Instead, the user should call endBuffer()
/// to indicate the actual length of the buffer
void* startBuffer(size_t size, AllocBucket bucket = allocGeneral);
/// Ends a buffer started with startBuffer()
void endBuffer(size_t actualSize, AllocBucket bucket = allocGeneral);

/// Allocate memory for a string of the given length
char* allocStr(size_t len);

/// Duplicate the given string
char* dupString(const char* str);

/// Similar to startBuffer, but for strings
char* startString(size_t maxLen = 256);
/// Similar to endBuffer, but for strings
void endString(size_t actualLen);
