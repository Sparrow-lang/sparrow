#pragma once

#include "PtrRange.h"

#ifdef __cplusplus
extern "C" {
#endif

struct NestUtils_PtrArray {
    void** beginPtr;
    void** endPtr;
    void** endOfStorePtr;
};
typedef struct NestUtils_PtrArray NestUtils_PtrArray;

NestUtils_PtrArray NestUtils_emptyPtrArray();
NestUtils_PtrArray NestUtils_allocPtrArray(unsigned capacity);
void NestUtils_freePtrArray(NestUtils_PtrArray arr);

void NestUtils_reservePtrArray(NestUtils_PtrArray* arr, unsigned capacity);
void NestUtils_resizePtrArray(NestUtils_PtrArray* arr, unsigned size);

void NestUtils_appendObjectToPtrArray(NestUtils_PtrArray* arr, void* obj);
void NestUtils_appendObjectsToPtrArray(NestUtils_PtrArray* arr, Nest_PtrRange objects);

void NestUtils_insertObjectIntoPtrArray(NestUtils_PtrArray* arr, unsigned index, void* obj);
void NestUtils_insertObjectsIntoPtrArray(NestUtils_PtrArray* arr, unsigned index, Nest_PtrRange objects);

void NestUtils_eraseFromPtrArray(NestUtils_PtrArray* arr, unsigned index);

#ifdef __cplusplus
}
#endif
