
#include "mvm_attributes.h"
#include "mvm_class_loader.h"
#include "mvm_data_reader.h"

ConstUTF8 AttributeType::GetName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(data_offset));
    return *(ConstUTF8 *)&ret;
}

uint32_t AttributeType::GetAttributeLength() {
    return ReadUInt32(&data_offset[2]);
}

ConstPoolType ConstValueAttribute::GetConstValue() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(data_offset));
    return ret;
}

uint16_t ExceptionTable::GetStartPC() {
    return ReadUInt16(data_offset);
}

uint16_t ExceptionTable::GetEndPC() {
    return ReadUInt16(&data_offset[2]);
}

uint16_t ExceptionTable::GetHandlerPC() {
    return ReadUInt16(&data_offset[4]);
}

uint16_t ExceptionTable::GetCatchType() {
    return ReadUInt16(&data_offset[6]);
}

uint16_t CodeAttribute::GetMaxStack() {
    return ReadUInt16(&data_offset[6]);
}

uint16_t CodeAttribute::GetMaxLocals() {
    return ReadUInt16(&data_offset[8]);
}

uint32_t CodeAttribute::GetCodeLength() {
    return ReadUInt32(&data_offset[10]);
}

uint8_t *CodeAttribute::GetCode() {
    return &data_offset[14];
}

uint16_t CodeAttribute::GetExceptionTableLength() {
    uint8_t *ptr = &data_offset[14 + GetCodeLength()];
    return ReadUInt16(ptr);
}

ExceptionTable CodeAttribute::GetExceptionTable(uint16_t index) {
    uint8_t *ptr = &data_offset[16 + GetCodeLength()];
    uint8_t *ret[2] = {data_ptr, &ptr[index * 8]};
    return *(ExceptionTable *)ret;
}

uint16_t CodeAttribute::GetAttributesCount() {
    uint8_t *ptr = &data_offset[16 + GetExceptionTableLength() * 8];
    return ReadUInt16(ptr);
}

AttributeType CodeAttribute::GetAttribute(uint16_t index) {
    uint8_t *temp[2] = {data_ptr, &data_offset[18 + GetExceptionTableLength() * 8]};
    while(index--) {
        uint32_t length = ((AttributeType *)temp)->GetAttributeLength();
        temp[1] += 6 + length;
    }
    return *(AttributeType *)temp;
}

uint16_t StackMapTable::GetNumberOfEntries() {
    return ReadUInt16(&data_offset[6]);
}

StackMapFrame StackMapTable::GetEntries(uint16_t index) {
    // TODO
    static const uint8_t *null[2] = {0, 0};
    return *(StackMapFrame *)null;
}

uint16_t ExceptionAttribute::GetNumberOfException() {
    return ReadUInt16(&data_offset[6]);
}

uint16_t *ExceptionAttribute::GetExceptionTable() {
    return (uint16_t *)&data_offset[8];
}

ConstClass Classes::GetInnerClass() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(data_offset));
    return *(ConstClass *)&ret;
}

ConstClass Classes::GetOuterClass() {
    static const uint8_t *null[2] = {0, 0};
    uint16_t index = ReadUInt16(&data_offset[2]);
    if(index == 0)
        return *(ConstClass *)&null;
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[2]));
    return *(ConstClass *)&ret;
}

ConstUTF8 Classes::GetInnerName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[4]));
    return *(ConstUTF8 *)&ret;
}

InnerClassAccessFlag Classes::GetInnerClassAccessFlag() {
    return (InnerClassAccessFlag)ReadUInt16(&data_offset[8]);
}

uint16_t InnerClassAttribute::GetNumberOfClasses() {
    return ReadUInt16(&data_offset[6]);
}

Classes InnerClassAttribute::GetClasses(uint16_t index) {
    uint8_t *ret[2] = {data_ptr, &data_offset[6 + index * 8]};
    return *(Classes *)ret;
}

ConstClass EnclosingMethodAttribute::GetClass() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[6]));
    return *(ConstClass *)&ret;
}

ConstNameAndType EnclosingMethodAttribute::GetMethod() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[8]));
    return *(ConstNameAndType *)&ret;
}

ConstUTF8 SignatureAttribute::GetSignature() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[6]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 SourceFileAttribute::GetSourceFile() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[6]));
    return *(ConstUTF8 *)&ret;
}

uint8_t *SourceDebugExtensionAttribute::GetDebugExtension() {
    return &data_offset[6];
}

uint16_t LineNumberTable::GetStartPC() {
    return ReadUInt16(data_offset);
}

uint16_t LineNumberTable::GetLineNumber() {
    return ReadUInt16(&data_offset[2]);
}

uint16_t LineNumberTableAttribute::GetLineNumberTableLength() {
    return ReadUInt16(&data_offset[6]);
}

LineNumberTable LineNumberTableAttribute::GetLineNumberTable(uint16_t index) {
    uint8_t *ret[2] = {data_ptr, &data_offset[8 + index * 4]};
    return *(LineNumberTable *)ret;
}

uint16_t LocalVariableTable::GetStartPC() {
    return ReadUInt16(data_offset);
}

uint16_t LocalVariableTable::GetLength() {
    return ReadUInt16(&data_offset[2]);
}

ConstUTF8 LocalVariableTable::GetName() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[4]));
    return *(ConstUTF8 *)&ret;
}

ConstUTF8 LocalVariableTable::GetDescriptor() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(&data_offset[6]));
    return *(ConstUTF8 *)&ret;
}

uint16_t LocalVariableTable::GetIndex() {
    return data_offset[8];
}

uint16_t LocalVariableTableAttribute::GetLocalVariableTableLength() {
    return ReadUInt16(&data_offset[6]);
}

LocalVariableTable LocalVariableTableAttribute::GetLocalVariableTable(uint16_t index) {
    uint8_t *ret[2] = {data_ptr, &data_offset[8 + index * 10]};
    return *(LocalVariableTable *)ret;
}

// TODO RuntimeVisibleAnnotationsAttribute
// TODO RuntimeInvisibleAnnotationsAttribute
// TODO RuntimeVisibleParameterAnnotationsAttribute
// TODO RuntimeInvisibleParameterAnnotationsAttribute
// TODO AnnotationDefaultAttribute

ConstMethodHandle BootstrapMethods::GetBootstrapMethodRef() {
    ConstPoolType ret = ClassLoader(0, data_ptr).GetConstPool(ReadUInt16(data_offset));
    return *(ConstMethodHandle *)&ret;
}

uint16_t BootstrapMethods::GetNumBootstrapArguments() {
    return ReadUInt16(&data_offset[2]);
}

ConstPoolType BootstrapMethods::GetBootstrapArguments(uint16_t index) {
    uint8_t *ret[2] = {data_ptr, &data_offset[4 + index * 2]};
    return *(ConstPoolType *)ret;
}

uint16_t BootstrapMethodsAttribute::GetNumBootstrapMethods() {
    return ReadUInt16(&data_offset[6]);
}

BootstrapMethods BootstrapMethodsAttribute::GetBootstrapMethods(uint16_t index) {
    uint8_t *temp[2] = {data_ptr, &data_offset[8]};
    while(index--)
        temp[1] += ((BootstrapMethods *)temp)->GetNumBootstrapArguments() * 2 + 4;
    return *(BootstrapMethods *)temp;
}
