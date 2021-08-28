
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

int32_t ConstInteger::GetValue() {
    return ReadUInt32(&data_offset[1]);
}

float ConstFloat::GetValue() {
    uint32_t ret = ReadUInt32(&data_offset[1]);
    return *(float *)&ret;
}

int64_t ConstLong::GetValue() {
    return ReadUInt64(&data_offset[1]);
}

double ConstDouble::GetValue() {
    uint64_t ret = ReadUInt64(&data_offset[1]);
    return *(double *)&ret;
}

ConstUTF8 ConstClass::GetName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 ConstString::GetUTF8() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstUTF8 *)&ret;
}

ConstClass ConstField::GetConstClass() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstClass *)&ret;
}

ConstNameAndType ConstField::GetNameAndType() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[3]));
    return *(ConstNameAndType *)&ret;
}

ConstUTF8 ConstNameAndType::GetName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 ConstNameAndType::GetDescriptor() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[3]));
    return *(ConstUTF8 *)&ret;
}

ReferenceKind ConstMethodHandle::GetReferenceKind() {
    return (ReferenceKind)(data_offset[1]);
}

ConstPoolType ConstMethodHandle::GetReference() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[2]));
    return ret;
}

ConstUTF8 ConstMethodType::GetDescriptor() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[1]));
    return *(ConstUTF8 *)&ret;
}

void ConstInvokeDynamic::BootstrapMethodAttr() {
    // TODO
}

ConstNameAndType ConstInvokeDynamic::GetNameAndType() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[3]));
    return *(ConstNameAndType *)&ret;
}
