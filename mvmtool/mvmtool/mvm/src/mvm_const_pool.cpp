
#include "mvm_const_pool.h"
#include "mvm_class_loader.h"
#include "mvm_data_reader.h"

ConstPoolTag ConstPoolType::GetPoolTag() {
    return (ConstPoolTag)data_offset[0];
}

uint16_t ConstUTF8::GetLength() {
    return ReadUInt16(&data_offset[1]);
}

const char *ConstUTF8::GetString() {
    return (const char *)&data_offset[3];
}

ConstUTF8 ConstNameAndType::GetName() {
    ConstPoolType ret = ClassLoader(class_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 ConstNameAndType::GetDescriptor() {
    ConstPoolType ret = ClassLoader(class_ptr).GetConstPool(ReadUInt16(&data_offset[3]));
    return *(ConstUTF8 *)&ret;
}
