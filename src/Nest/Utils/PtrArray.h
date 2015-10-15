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
typedef struct NestUtils_PtrArray PtrArray;


PtrArray NestUtils_emptyPtrArray();
PtrArray NestUtils_allocPtrArray(unsigned capacity);
void NestUtils_freePtrArray(PtrArray arr);

void NestUtils_reservePtrArray(PtrArray* arr, unsigned capacity);
void NestUtils_resizePtrArray(PtrArray* arr, unsigned size);

void NestUtils_appendObjectToPtrArray(PtrArray* arr, void* obj);
void NestUtils_appendObjectsToPtrArray(PtrArray* arr, PtrRange objects);

void NestUtils_insertObjectIntoPtrArray(PtrArray* arr, unsigned index, void* obj);
void NestUtils_insertObjectsIntoPtrArray(PtrArray* arr, unsigned index, PtrRange objects);

void NestUtils_eraseFromPtrArray(PtrArray* arr, unsigned index);

#ifdef __cplusplus
}
#endif

