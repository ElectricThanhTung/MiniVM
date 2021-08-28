
#include <iostream>
#include "mvm_class_loader.h"
#include "mvm_data_reader.h"
#include "mvm_info.h"
#include "mvm_attributes.h"

#define F_NaN             0x7FC00000
#define D_NaN             0x7FF8000000000000

ClassLoader::ClassLoader() {
    data_ptr = 0;
    access_flag_ptr = 0;
    fields_count_ptr = 0;
    methods_count_ptr = 0;
    attributes_count_ptr = 0;
}

ClassLoader::ClassLoader(uint8_t *data) {
    data_ptr = data;
    access_flag_ptr = 0;
    fields_count_ptr = 0;
    methods_count_ptr = 0;
    attributes_count_ptr = 0;
}

void ClassLoader::CalcFieldsCountOffset() {
    if(!access_flag_ptr)
        access_flag_ptr = &data_ptr[10] + GetConstPoolTableSize();
    fields_count_ptr = &access_flag_ptr[8] + GetInterfaceTableSize();
}

void ClassLoader::CalcMethodsCountOffset() {
    if(!fields_count_ptr)
        CalcFieldsCountOffset();
    methods_count_ptr = &fields_count_ptr[2] + GetFieldsTableSize();
}

void ClassLoader::CalcAttributesCountOffset() {
    if(!methods_count_ptr)
        CalcMethodsCountOffset();
    attributes_count_ptr = &methods_count_ptr[2] + GetMethodsTableSize();
}

uint16_t ClassLoader::ConstPoolCount() {
    return ReadUInt16(&data_ptr[8]);
}

ConstPoolType ClassLoader::GetConstPool(int16_t index) {
    static const int8_t *null[2] = {0, 0};
    int16_t pool_count = ConstPoolCount();
    if(index)
        index--;
    else {
        printf("Pool index can't be less than 1\r\n");
        return *(ConstPoolType *)null;
    }
    if(index >= pool_count) {
        printf("Pool index out of range\r\n");
        return *(ConstPoolType *)null;
    }
    uint8_t *ptr = &data_ptr[10];
    while((index--) > 0) {
        ConstPoolTag pool_tag = (ConstPoolTag)*ptr;
        switch(pool_tag) {
            case CONST_UTF8:
            {
                uint8_t *temp[2] = {(uint8_t *)data_ptr, ptr};
                ptr += ((ConstUTF8 *)temp)->GetLength() + 3;
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                ptr += 5;
                break;
            case CONST_LONG:
            case CONST_DOUBLE:
                ptr += 9;
                index--;
                break;
            case CONST_CLASS:
            case CONST_STRING:
            case CONST_METHOD_TYPE:
                ptr += 3;
                break;
            case CONST_METHOD_HANDLE:
                ptr += 4;
                break;
            default:
                printf("Uknow pool type\r\n");
        }
    }
    uint8_t *ret[2] = {data_ptr, ptr};
    return *(ConstPoolType *)ret;
}

int32_t ClassLoader::GetConstPoolOffset(int16_t index) {
    int16_t pool_count = ConstPoolCount();
    if(index)
        index--;
    else {
        printf("Pool index can't be less than 1\r\n");
        return -1;
    }
    if(index >= pool_count) {
        printf("Pool index out of range\r\n");
        return -1;
    }
    uint8_t *ptr = &data_ptr[10];
    while((index--) > 0) {
        ConstPoolTag pool_tag = (ConstPoolTag)*ptr;
        switch(pool_tag) {
            case CONST_UTF8: {
                uint8_t *temp[2] = {(uint8_t *)data_ptr, ptr};
                ptr += ((ConstUTF8 *)temp)->GetLength() + 3;
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                ptr += 5;
                break;
            case CONST_LONG:
            case CONST_DOUBLE:
                ptr += 9;
                index--;
                break;
            case CONST_CLASS:
            case CONST_STRING:
            case CONST_METHOD_TYPE:
                ptr += 3;
                break;
            case CONST_METHOD_HANDLE:
                ptr += 4;
                break;
            default:
                printf("Uknow pool type\r\n");
        }
    }
    return (uint32_t)(ptr - data_ptr);
}

uint32_t ClassLoader::GetConstPoolTableSize() {
    uint16_t pool_count = ConstPoolCount() - 1;
    uint8_t *ptr = &data_ptr[10];
    while(pool_count--) {
        ConstPoolTag pool_tag = (ConstPoolTag)*ptr;
        switch(pool_tag) {
            case CONST_UTF8:
            {
                uint8_t *temp[2] = {(uint8_t *)data_ptr, ptr};
                ptr += ((ConstUTF8 *)temp)->GetLength() + 3;
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                ptr += 5;
                break;
            case CONST_LONG:
            case CONST_DOUBLE:
                ptr += 9;
                pool_count--;
                break;
            case CONST_CLASS:
            case CONST_STRING:
            case CONST_METHOD_TYPE:
                ptr += 3;
                break;
            case CONST_METHOD_HANDLE:
                ptr += 4;
                break;
            default:
                printf("Uknow this Pool Type\r\n");
        }
    }
    return (uint32_t)(ptr - &data_ptr[10]);
}

uint16_t ClassLoader::GetInterfaceCount() {
    if(!access_flag_ptr)
        access_flag_ptr = &data_ptr[10] + GetConstPoolTableSize();
    return ReadUInt16(&access_flag_ptr[6]);
}

uint32_t ClassLoader::GetInterfaceTableSize() {
    return this->GetInterfaceCount() * 2;
}

uint16_t ClassLoader::GetFieldsCount() {
    if(!fields_count_ptr)
        CalcFieldsCountOffset();
    return ReadUInt16(fields_count_ptr);
}

uint32_t ClassLoader::GetFieldsTableSize() {
    uint16_t field_count = GetFieldsCount();
    uint8_t *field_struct[2] = {data_ptr, &fields_count_ptr[2]};
    uint8_t *attribute_struct[2] = {data_ptr, &field_struct[1][8]};
    while(field_count--) {
        uint16_t attribute_count = ((FieldInfo *)field_struct)->GetAttributesCount();
        uint32_t size = 0;
        for(uint16_t i = 0; i < attribute_count; i++) {
            uint32_t size_of_attribute = 6 + ((AttributeType *)attribute_struct)->GetAttributeLength();
            size += size_of_attribute;
            attribute_struct[1] += size_of_attribute;
        }
        field_struct[1] += 8 + size;
        attribute_struct[1] = &field_struct[1][8];
    }
    return (uint32_t)(field_struct[1] - &fields_count_ptr[2]);
}

uint16_t ClassLoader::GetMethodsCount() {
    if(!methods_count_ptr)
        CalcMethodsCountOffset();
    return ReadUInt16(methods_count_ptr);
}

MethodOffsetInfo ClassLoader::GetMethodOffset(uint16_t index) {
    if(index >= GetMethodsCount())
        printf("Method Index Out of Range\r\n");
    uint8_t *method_struct[2] = {data_ptr, &methods_count_ptr[2]};
    uint8_t *attribute_struct[2] = {data_ptr, &method_struct[1][8]};
    while(index--) {
        uint16_t attribute_count = ((MethodInfo *)method_struct)->GetAttributesCount();
        uint32_t size = 0;
        for(uint16_t i = 0; i < attribute_count; i++) {
            uint32_t size_of_attribute = 6 + ((AttributeType *)attribute_struct)->GetAttributeLength();
            size += size_of_attribute;
            attribute_struct[1] += size_of_attribute;
        }
        method_struct[1] += 8 + size;
        attribute_struct[1] = &method_struct[1][8];
    }

    ConstUTF8 name = ((MethodInfo *)method_struct)->GetName();
    ConstUTF8 desc = ((MethodInfo *)method_struct)->GetDescriptor();

    int8_t *name_buffer = new int8_t[name.GetLength() + 1];
    int8_t *desc_buffer = new int8_t[desc.GetLength() + 1];

    index = 0;
    const char *temp = name.GetString();
    for(uint16_t i = 0; i < name.GetLength(); i++)
        name_buffer[index++] = temp[i];
    name_buffer[index] = 0;

    index = 0;
    temp = desc.GetString();
    for(uint16_t i = 0; i < desc.GetLength(); i++)
        desc_buffer[index++] = temp[i];
    desc_buffer[index] = 0;

    MethodOffsetInfo ret;
    ret.MethodName.Name = (const char *)name_buffer;
    ret.MethodName.Desc = (const char *)desc_buffer;
    ret.Offset = (uint32_t)(method_struct[1] - data_ptr);

    return ret;
}

uint32_t ClassLoader::GetMethodsTableSize() {
    if(!methods_count_ptr)
        CalcMethodsCountOffset();
    uint16_t method_count = GetMethodsCount();
    uint8_t *method_struct[2] = {data_ptr, &methods_count_ptr[2]};
    uint8_t *attribute_struct[2] = {data_ptr, &method_struct[1][8]};
    while(method_count--) {
        uint16_t attribute_count = ((MethodInfo *)method_struct)->GetAttributesCount();
        uint32_t size = 0;
        for(uint16_t i = 0; i < attribute_count; i++) {
            uint32_t size_of_attribute = 6 + ((AttributeType *)attribute_struct)->GetAttributeLength();
            size += size_of_attribute;
            attribute_struct[1] += size_of_attribute;
        }
        method_struct[1] += 8 + size;
        attribute_struct[1] = &method_struct[1][8];
    }
    return (uint32_t)(method_struct[1] - &methods_count_ptr[2]);
}

uint16_t ClassLoader::GetAttributeCount() {
    if(!attributes_count_ptr)
        CalcAttributesCountOffset();
    return ReadUInt16(attributes_count_ptr);
}

uint32_t ClassLoader::GetAccessFlagOffset() {
    if(!access_flag_ptr)
        access_flag_ptr = &data_ptr[10] + GetConstPoolTableSize();
    return access_flag_ptr - data_ptr;
}

uint32_t ClassLoader::GetFieldsCountOffset() {
    if(!access_flag_ptr)
        access_flag_ptr = &data_ptr[10] + GetConstPoolTableSize();
    fields_count_ptr = &access_flag_ptr[8] + GetInterfaceTableSize();
    return fields_count_ptr - data_ptr;
}

uint32_t ClassLoader::GetMethodsCountOffset() {
    if(!fields_count_ptr)
        CalcFieldsCountOffset();
    methods_count_ptr = &fields_count_ptr[2] + GetFieldsTableSize();
    return methods_count_ptr - data_ptr;
}

uint32_t ClassLoader::GetAttributesCountOffset() {
    if(!methods_count_ptr)
        CalcMethodsCountOffset();
    attributes_count_ptr = &methods_count_ptr[2] + GetMethodsTableSize();
    return attributes_count_ptr - data_ptr;
}
