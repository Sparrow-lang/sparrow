#include <StdInc.h>
#include "Alloc.h"


struct BucketInfo
{
    char* curPage;
    size_t freeBytes;
};

BucketInfo buckets[numAllocBuckets];

static const int noBytesPerPage = 16*65536;    // 16 x 64k pages


void* alloc(size_t size, AllocBucket bucket)
{
    void* p = startBuffer(size, bucket);
    endBuffer(size, bucket);
    return p;
}

void* startBuffer(size_t size, AllocBucket bucket)
{
    BucketInfo& bucketInfo = buckets[bucket];

    ASSERT(size < noBytesPerPage);
    if ( bucketInfo.freeBytes < size )
    {
        bucketInfo.curPage = (char*) calloc(noBytesPerPage, 1);
        bucketInfo.freeBytes = noBytesPerPage;
    }
    return bucketInfo.curPage;
}

void endBuffer(size_t actualSize, AllocBucket bucket)
{
    BucketInfo& bucketInfo = buckets[bucket];
    bucketInfo.curPage += actualSize;
    bucketInfo.freeBytes -= actualSize;
}


char* allocStr(size_t len)
{
    return (char*) alloc(len, allocString);
}

char* dupString(const char* str)
{
    size_t len = strlen(str);
    if ( len == 0 )
        return NULL;
    char* p = (char*) alloc(len+1, allocString);
    return strcpy(p, str);
}

char* startString(size_t maxLen)
{
    return (char*) startBuffer(maxLen, allocString);
}
void endString(size_t actualLen)
{
    endBuffer(actualLen, allocString);
}
