#include <StdInc.h>
#include "NodeAllocatorImpl.h"

using namespace Nest::Common;

NodeAllocatorImpl::NodeAllocatorImpl()
    : curPage_(nullptr)
    , curPageFreeByes_(0)
{
}

NodeAllocatorImpl::~NodeAllocatorImpl()
{
}

void* NodeAllocatorImpl::alloc(size_t size)
{
    static const int noBytesPerPage = 16*65536;    // 16 x 64k pages

    ASSERT(size < noBytesPerPage);

    if ( curPageFreeByes_ < size )
    {
        curPage_ = new char[noBytesPerPage];
        curPageFreeByes_ = noBytesPerPage;
    }
    void* p = curPage_;
    curPage_ += size;
    curPageFreeByes_ -= size;
    return p;
}

void NodeAllocatorImpl::free(void* /*ptr*/)
{
    // Do nothing
}

