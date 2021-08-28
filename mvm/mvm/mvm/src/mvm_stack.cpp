
#include "mvm_stack.h"

Stack::Stack() {
    this->stack_mem = 0;
    this->MinSize = 0;
}

Stack::Stack(uint32_t size) {
    stack_mem = Heap_Calloc(size + 4, 3, 10);
    ((int32_t *)stack_mem->Address)[0] = -1;
    this->MinSize = 0;
}

void Stack::Realloc(uint32_t size) {
    if(stack_mem == 0) {
        stack_mem = Heap_Calloc(size + 4, 3, 10);
        ((int32_t *)stack_mem->Address)[0] = -1;
    }
    else
        Heap_Realloc(stack_mem, size + 4);
}

uint32_t Stack::GetSize() {
    if(stack_mem == 0)
        return 0;
    return Heap_GetSize(stack_mem) - 4;
}

uint32_t Stack::GetFreeSpace() {
    if(stack_mem == 0)
        return 0;
    int32_t size = Heap_GetSize(stack_mem) - 4;
    if(size <= 0)
        return 0;
    return size - ((((int32_t *)stack_mem->Address)[0]) + 1) * 4;
}

int32_t Stack::GetSP() {
    if(stack_mem == 0)
        return -1;
    return ((int32_t *)stack_mem->Address)[0];
}

void Stack::SetSP(int32_t value) {
    ((int32_t *)stack_mem->Address)[0] = value;
}

uint32_t &Stack::ValueUInt32(uint16_t index) {
    return ((uint32_t *)stack_mem->Address)[index + 1];
}

uint64_t &Stack::ValueUInt64(uint16_t index) {
    return *(uint64_t *)&((uint32_t *)stack_mem->Address)[index + 1];
}

void Stack::PushZero(uint32_t n) {
    int32_t *ptr = &((int32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 2];
    int32_t *end_ptr = ptr + n;
    for(; ptr < end_ptr; ptr++)
        *ptr = 0;
    ((int32_t *)stack_mem->Address)[0] += n;
}

uint32_t Stack::PopUInt32() {
    return ((uint32_t *)stack_mem->Address)[((((int32_t *)stack_mem->Address)[0])--) + 1];
}

float Stack::PopFloat() {
    return *(float *)&((uint32_t *)stack_mem->Address)[((((int32_t *)stack_mem->Address)[0])--) + 1];
}

uint64_t Stack::PopUInt64() {
    int32_t index = ((int32_t *)stack_mem->Address)[0];
    uint32_t byte_h = ((uint32_t *)stack_mem->Address)[index + 1];
    uint32_t byte_l = ((uint32_t *)stack_mem->Address)[index];
    ((int32_t *)stack_mem->Address)[0] -= 2;
    return ((uint64_t)byte_h << 32) | byte_l;
}

double Stack::PopDouble() {
    int32_t index = ((int32_t *)stack_mem->Address)[0];
    uint32_t byte_h = ((uint32_t *)stack_mem->Address)[index + 1];
    uint32_t byte_l = ((uint32_t *)stack_mem->Address)[index];
    ((int32_t *)stack_mem->Address)[0] -= 2;
    uint64_t ret = ((uint64_t)byte_h << 32) | byte_l;
    return *(double *)&ret;
}

void Stack::Push(uint32_t value) {
    uint32_t *ptr = &((uint32_t *)stack_mem->Address)[(++((int32_t *)stack_mem->Address)[0]) + 1];
    *ptr = value;
    Heap_MemSetRefer(ptr, 0);
}

void Stack::Push(void *value) {
    uint32_t *ptr = &((uint32_t *)stack_mem->Address)[(++((int32_t *)stack_mem->Address)[0]) + 1];
    *ptr = (uint32_t)value;
    Heap_MemSetRefer(ptr, 1);
}

void Stack::Push(float value) {
    uint32_t *ptr = &((uint32_t *)stack_mem->Address)[(++((int32_t *)stack_mem->Address)[0]) + 1];
    *ptr = *(uint32_t *)&value;
    Heap_MemSetRefer(ptr, 0);
}

void Stack::Push(uint64_t value) {
    uint32_t *ptr = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 2];
    ((int32_t *)stack_mem->Address)[0] += 2;
    ptr[0] = (uint32_t)value;
    ptr[1] = (uint32_t)(value >> 32);
    Heap_MemSetRefer(&ptr[0], 0);
    Heap_MemSetRefer(&ptr[1], 0);
}

void Stack::Push(double value) {
    uint64_t temp = *(uint64_t *)&value;
    uint32_t *ptr = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 2];
    ((int32_t *)stack_mem->Address)[0] += 2;
    ptr[0] = (uint32_t)temp;
    ptr[1] = (uint32_t)(temp >> 32);
    Heap_MemSetRefer(&ptr[0], 0);
    Heap_MemSetRefer(&ptr[1], 0);
}

void Stack::Dup() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    stack[1] = stack[0];
    (((int32_t *)stack_mem->Address)[0])++;
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(stack));
}

void Stack::DupX1() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    int32_t value2 = stack[-1];
    int32_t value1 = *stack;
    stack[-1] = value1;
    stack[0] = value2;
    stack[1] = value1;
    (((int32_t *)stack_mem->Address)[0])++;
    Heap_MemSetRefer(&stack[-1], Heap_MemGetRefer(stack));
    Heap_MemSetRefer(&stack[0], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(stack));
}

void Stack::DupX2() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    int32_t value3 = stack[-2];
    int32_t value2 = stack[-1];
    int32_t value1 = *stack;
    stack[-2] = value1;
    stack[-1] = value3;
    stack[0] = value2;
    stack[1] = value1;
    (((int32_t *)stack_mem->Address)[0])++;
    Heap_MemSetRefer(&stack[-2], Heap_MemGetRefer(&stack[0]));
    Heap_MemSetRefer(&stack[-1], Heap_MemGetRefer(&stack[-2]));
    Heap_MemSetRefer(&stack[0], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(&stack[0]));
}

void Stack::Dup2() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    stack[1] = stack[-1];
    stack[2] = stack[0];
    ((int32_t *)stack_mem->Address)[0] += 2;
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[2], Heap_MemGetRefer(&stack[0]));
}

void Stack::Dup2X1() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    int32_t value3 = stack[-2];
    int32_t value2 = stack[-1];
    int32_t value1 = *stack;
    stack[-2] = value2;
    stack[-1] = value1;
    stack[0] = value3;
    stack[1] = value2;
    stack[2] = value1;
    ((int32_t *)stack_mem->Address)[0] += 2;
    Heap_MemSetRefer(&stack[-2], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[-1], Heap_MemGetRefer(&stack[0]));
    Heap_MemSetRefer(&stack[0], Heap_MemGetRefer(&stack[-2]));
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[2], Heap_MemGetRefer(&stack[0]));
}

void Stack::Dup2X2() {
    uint32_t *stack = &((uint32_t *)stack_mem->Address)[((int32_t *)stack_mem->Address)[0] + 1];
    int32_t value4 = stack[-3];
    int32_t value3 = stack[-2];
    int32_t value2 = stack[-1];
    int32_t value1 = *stack;
    stack[-3] = value2;
    stack[-2] = value1;
    stack[-1] = value4;
    stack[0] = value3;
    stack[1] = value2;
    stack[2] = value1;
    ((int32_t *)stack_mem->Address)[0] += 2;
    Heap_MemSetRefer(&stack[-3], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[-2], Heap_MemGetRefer(&stack[0]));
    Heap_MemSetRefer(&stack[-1], Heap_MemGetRefer(&stack[-3]));
    Heap_MemSetRefer(&stack[0], Heap_MemGetRefer(&stack[-2]));
    Heap_MemSetRefer(&stack[1], Heap_MemGetRefer(&stack[-1]));
    Heap_MemSetRefer(&stack[2], Heap_MemGetRefer(&stack[0]));
}

HeapInfo_TypeDef *Stack::GetHeapInfo() {
    return stack_mem;
}

void Stack::Free() {
    Heap_Free(stack_mem);
}
