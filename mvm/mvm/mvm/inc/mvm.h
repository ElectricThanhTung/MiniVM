
#ifndef __MVM_H
#define __MVM_H

#include "mvm_std_type.h"
#include "mvm_heap.h"
#include "mvm_stack.h"

class mVM {
private:
    const uint8_t *program_info;
    HeapInfo_TypeDef *static_fields;
    Stack stack;
    int32_t string_class_index;
    int32_t throwable_class_index;
    uint32_t GetStaticFieldsSize(uint16_t class_index);
public:
    mVM(const uint8_t *program);
    uint8_t *FindClassFile(void *name);
    int32_t FindClassFileIndex(void *name);
    int32_t FindClassFileIndex(const char *name, uint16_t name_length);
    int32_t GetStringClassIndex();
    uint8_t *GetClassFile(uint16_t index);
    uint64_t Run(const char *argv[], uint32_t argc);
    void SetStaticField(void *field_name, int32_t value, uint8_t is_refer);
    void SetStaticField(void *field_name, int64_t value);
    int32_t GetStaticFieldUInt32(void *field_name);
    int64_t GetStaticFieldUInt64(void *field_name);
    HeapInfo_TypeDef *CreateStringObject(const char *str);
    HeapInfo_TypeDef *CreateStringObject(uint16_t length);
    HeapInfo_TypeDef *CreateStringObject(const char *str, uint16_t length);
    HeapInfo_TypeDef *CreateThrowableObject(const char *msg);
    Stack *GetStack();
    HeapInfo_TypeDef *GetStaticFields();
};

#endif // __MVM_H
