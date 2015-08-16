// #include <StdInc.h>
#include "Alloc.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    char* curPage;
    unsigned int freeBytes;
} BucketInfo;

BucketInfo buckets[numAllocBuckets];

static const int noBytesPerPage = 16*65536;    // 16 x 64k pages


void* alloc(unsigned int size, AllocBucket bucket)
{
    void* p = startBuffer(size, bucket);
    endBuffer(size, bucket);
    return p;
}

void* startBuffer(unsigned int size, AllocBucket bucket)
{
    BucketInfo* bucketInfo = &buckets[bucket];

    // ASSERT(size < noBytesPerPage);
    if ( bucketInfo->freeBytes < size )
    {
        bucketInfo->curPage = (char*) calloc(noBytesPerPage, 1);
        bucketInfo->freeBytes = noBytesPerPage;
    }
    return bucketInfo->curPage;
}

void endBuffer(unsigned int actualSize, AllocBucket bucket)
{
    BucketInfo* bucketInfo = &buckets[bucket];
    bucketInfo->curPage += actualSize;
    bucketInfo->freeBytes -= actualSize;
}


char* allocStr(unsigned int len)
{
    return (char*) alloc(len, allocString);
}

char* dupString(const char* str)
{
    unsigned int len = strlen(str);
    if ( len == 0 )
        return NULL;
    char* p = (char*) alloc(len+1, allocString);
    return strcpy(p, str);
}

char* startString(unsigned int maxLen)
{
    return (char*) startBuffer(maxLen, allocString);
}
void endString(unsigned int actualLen)
{
    endBuffer(actualLen, allocString);
}
