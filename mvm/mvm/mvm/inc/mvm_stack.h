
#ifndef __MVM_STACK_H
#define __MVM_STACK_H

#include "mvm_std_type.h"
#include "mvm_heap.h"

class Stack {
private:
    HeapInfo_TypeDef *stack_mem;
public:
    uint32_t MinSize;
    Stack();
    Stack(uint32_t size);
    void Realloc(uint32_t size);
    uint32_t GetSize();
    uint32_t GetFreeSpace();

    int32_t GetSP();
    void SetSP(int32_t value);

    uint32_t &ValueUInt32(uint16_t index);
    uint64_t &ValueUInt64(uint16_t index);

    void PushZero(uint32_t n);
    uint32_t PopUInt32();
    float PopFloat();
    uint64_t PopUInt64();
    double PopDouble();

    void Push(uint32_t value);
    void Push(float value);
    void Push(uint64_t value);
    void Push(double value);
    void Push(void *value);

    void Dup();
    void DupX1();
    void DupX2();
    void Dup2();
    void Dup2X1();
    void Dup2X2();

    HeapInfo_TypeDef *GetHeapInfo();

    void Free();
};

#endif // __MVM_STACK_H
