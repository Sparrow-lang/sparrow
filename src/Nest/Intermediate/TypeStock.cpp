#include <StdInc.h>
#include "TypeStock.h"
#include "Type.h"

using namespace Nest;

TypeStock Nest::typeStock;

namespace
{
    static const int noTypesPerPage = 100;

    struct TypePage
    {
        TypePage* prevPage;
        unsigned char types[sizeof(Type)*noTypesPerPage];
    };
}

struct TypeStock::Impl
{
    unordered_map<Type*, Type*>* stocks[Type::typeIdLast];
    int usedTypesInCurPage;
    TypePage* curPage;
};

TypeStock::TypeStock()
{
    impl_ = new Impl();
    impl_->usedTypesInCurPage = noTypesPerPage;
    impl_->curPage = nullptr;
}

TypeStock::~TypeStock()
{
    // Remove all the type pages
    TypePage* p = impl_->curPage;
    while ( p )
    {
        TypePage* q = p->prevPage;
        free(p);
        p = q;
    }
    // Delete the impl object
    delete impl_;
}

void* TypeStock::allocType()
{
    if ( impl_->usedTypesInCurPage == noTypesPerPage )
    {
        TypePage* newPage = (TypePage*) malloc(sizeof(TypePage));
        newPage->prevPage = impl_->curPage;
        impl_->curPage = newPage;
        impl_->usedTypesInCurPage = 0;
    }
    Type* types = reinterpret_cast<Type*>(impl_->curPage->types);
    return &types[impl_->usedTypesInCurPage++];
}

unordered_map<Type*,Type*>*& TypeStock::getTypeMapImpl(Type::TypeId typeId)
{
    return impl_->stocks[typeId];
}



