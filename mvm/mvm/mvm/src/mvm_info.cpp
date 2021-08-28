
#include "mvm_info.h"
#include "mvm_class_loader.h"
#include "mvm_data_reader.h"

FieldAccessFlag FieldInfo::GetAccessFlag() {
    return (FieldAccessFlag)ReadUInt16(data_offset);
}

ConstUTF8 FieldInfo::GetName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[2]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 FieldInfo::GetDescriptor() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[4]));
    return *(ConstUTF8 *)&ret;
}

uint16_t FieldInfo::GetAttributesCount() {
    return ReadUInt16(&data_offset[6]);
}

AttributeType FieldInfo::GetAttributes(uint16_t index) {
    uint8_t *ptr = &data_offset[8];
    while(index) {
        ptr += ReadUInt32(&ptr[2]) + 6;
        index--;
    }
    uint8_t *ret[2] = {data_ptr, ptr};
    return *(AttributeType *)ret;
}

MethodAccessFlag MethodInfo::GetAccessFlag() {
    return (MethodAccessFlag)ReadUInt16(data_offset);
}
