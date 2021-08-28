
#ifndef __MVM_CLASS_LOADER_H
#define __MVM_CLASS_LOADER_H

#include "mvm_const_pool.h"
#include "mvm_stack.h"
#include "mvm_info.h"

class ClassLoader {
private:
    uint8_t *mvm_ptr;
    uint8_t *data_ptr;
public:
    ClassLoader();
    ClassLoader(void *mvm, uint8_t *data);
    uint32_t Magic();
    uint16_t MinorVersion();
    uint16_t MajorVersion();
    uint16_t ConstPoolCount();
    ConstPoolType GetConstPool(uint16_t index);
    ClassAccessFlag GetAccessFlag();
    ConstClass GetThisClass();
    ConstClass GetSupperClass();
    uint16_t GetInterfaceCount();
    ConstClass GetInterface(uint16_t index);
    uint16_t GetFieldsCount();
    FieldInfo GetField(uint16_t index);
    uint32_t GetFieldsSize();
    int32_t GetFieldOffset(ConstUTF8 &field_name);
    uint16_t GetMethodsCount();
    MethodInfo GetMethod(uint16_t index);
    uint16_t GetAttributeCount();
    AttributeType GetAttribute(uint16_t index);
    MethodInfo FindMethod(ConstNameAndType &name);
    MethodInfo FindMethod(const char *name, uint16_t name_len, const char *desc, uint16_t desc_len);
    uint8_t RunMethod(Stack &stack, MethodInfo &method, uint32_t n_arg, uint64_t &ret_val_ptr);
};

#endif // __MVM_CLASS_LOADER_H
