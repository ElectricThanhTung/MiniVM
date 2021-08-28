
#ifndef __MVM_HEAP_H
#define __MVM_HEAP_H

#include "mvm_std_type.h"

typedef struct {
    uint8_t *Address;
    uint32_t Size;
    uint16_t Type;
    uint16_t Dimensions : 12;
    uint16_t Refer : 1;
    uint16_t Attribute : 3;
    uint32_t MoniterCount;
} HeapInfo_TypeDef;

void Heap_GarbageCollect();
HeapInfo_TypeDef *Heap_Malloc(uint32_t size, uint8_t attribute, uint32_t type);
HeapInfo_TypeDef *Heap_Calloc(uint32_t size, uint8_t attribute, uint32_t type);
void Heap_Free(HeapInfo_TypeDef *heap_info);
uint8_t Heap_Realloc(HeapInfo_TypeDef *heap_info, uint32_t size);
uint32_t Heap_GetSize(HeapInfo_TypeDef *heap_info);
uint32_t Heap_GetSizeReality(HeapInfo_TypeDef *heap_info);

void Heap_MemSetRefer(uint32_t *ptr, uint8_t value);
uint8_t Heap_MemGetRefer(uint32_t *ptr);

#endif // __MVM_HEAP_H
