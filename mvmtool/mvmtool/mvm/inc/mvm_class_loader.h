
#ifndef __CLASS_LOADER_H
#define __CLASS_LOADER_H

#include "mvm_const_pool.h"

typedef struct {
    const char *Name;
    const char *Desc;
} MethodNameInfo;

typedef struct {
    MethodNameInfo MethodName;
    uint32_t Offset;
} MethodOffsetInfo;

class ClassLoader {
private:
    uint8_t *data_ptr;
    uint8_t *access_flag_ptr;
    uint8_t *fields_count_ptr;
    uint8_t *methods_count_ptr;
    uint8_t *attributes_count_ptr;
    uint32_t GetConstPoolTableSize();
    uint32_t GetInterfaceTableSize();
    uint32_t GetFieldsTableSize();
    uint32_t GetMethodsTableSize();
    void CalcFieldsCountOffset();
    void CalcMethodsCountOffset();
    void CalcAttributesCountOffset();
public:
    ClassLoader();
    ClassLoader(uint8_t *data);
    uint16_t ConstPoolCount();
    ConstPoolType GetConstPool(int16_t index);
    int32_t GetConstPoolOffset(int16_t index);
    uint16_t GetInterfaceCount();
    uint16_t GetFieldsCount();
    uint16_t GetMethodsCount();
    uint16_t GetAttributeCount();

    MethodOffsetInfo GetMethodOffset(uint16_t index);

    uint32_t GetAccessFlagOffset();
    uint32_t GetFieldsCountOffset();
    uint32_t GetMethodsCountOffset();
    uint32_t GetAttributesCountOffset();
};

#endif // __CLASS_LOADER_H
