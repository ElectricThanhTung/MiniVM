
#ifndef __MVM_ATTRIBUTES_H
#define __MVM_ATTRIBUTES_H

#include "mvm_type.h"
#include "mvm_const_pool.h"

class AttributeType : public Type {
public:
    ConstUTF8 GetName();
    uint32_t GetAttributeLength();
};

class ConstValueAttribute : public AttributeType {
public:
    ConstPoolType GetConstValue();
};

class ExceptionTable : public Type {
public:
    uint16_t GetStartPC();
    uint16_t GetEndPC();
    uint16_t GetHandlerPC();
    uint16_t GetCatchType();
};

class CodeAttribute : public AttributeType {
public:
    uint16_t GetMaxStack();
    uint16_t GetMaxLocals();
    uint32_t GetCodeLength();
    uint8_t *GetCode();
    uint16_t GetExceptionTableLength();
    ExceptionTable GetExceptionTable(uint16_t index);
    uint16_t GetAttributesCount();
    AttributeType GetAttribute(uint16_t index);
};

typedef union {
    uint8_t SameFrame : 6;
    // TODO
} StackMapFrame;

class StackMapTable : public AttributeType {
public:
    uint16_t GetNumberOfEntries();
    StackMapFrame GetEntries(uint16_t index);
};

class ExceptionAttribute : public AttributeType {
public:
    uint16_t GetNumberOfException();
    uint16_t *GetExceptionTable();
};

class Classes : public Type {
public:
    ConstClass GetInnerClass();
    ConstClass GetOuterClass();
    ConstUTF8 GetInnerName();
    InnerClassAccessFlag GetInnerClassAccessFlag();
};

class InnerClassAttribute : public AttributeType {
public:
    uint16_t GetNumberOfClasses();
    Classes GetClasses(uint16_t index);
};

class EnclosingMethodAttribute : public AttributeType {
public:
    ConstClass GetClass();
    ConstNameAndType GetMethod();
};

typedef AttributeType SyntheticAttribute;

class SignatureAttribute : public AttributeType {
public:
    ConstUTF8 GetSignature();
};

class SourceFileAttribute : public AttributeType {
public:
    ConstUTF8 GetSourceFile();
};

class SourceDebugExtensionAttribute : public AttributeType {
public:
    uint8_t *GetDebugExtension();
};

class LineNumberTable : public Type {
public:
    uint16_t GetStartPC();
    uint16_t GetLineNumber();
};

class LineNumberTableAttribute : public AttributeType {
public:
    uint16_t GetLineNumberTableLength();
    LineNumberTable GetLineNumberTable(uint16_t index);
};

class LocalVariableTable : public Type {
public:
    uint16_t GetStartPC();
    uint16_t GetLength();
    ConstUTF8 GetName();
    ConstUTF8 GetDescriptor();
    uint16_t GetIndex();
};

class LocalVariableTableAttribute : public AttributeType {
public:
    uint16_t GetLocalVariableTableLength();
    LocalVariableTable GetLocalVariableTable(uint16_t index);
};

typedef AttributeType DeprecatedAttribute;

class RuntimeVisibleAnnotationsAttribute : public AttributeType {
public:
    // TODO
};

class RuntimeInvisibleAnnotationsAttribute : public AttributeType {
public:
    // TODO
};

class RuntimeVisibleParameterAnnotationsAttribute : public AttributeType {
public:
    // TODO
};

class RuntimeInvisibleParameterAnnotationsAttribute : public AttributeType {
public:
    // TODO
};

class AnnotationDefaultAttribute : public AttributeType {
public:
    // TODO
};

class BootstrapMethods : public Type {
public:
    ConstMethodHandle GetBootstrapMethodRef();
    uint16_t GetNumBootstrapArguments();
    ConstPoolType GetBootstrapArguments(uint16_t index);
};

class BootstrapMethodsAttribute : public AttributeType {
public:
    uint16_t GetNumBootstrapMethods();
    BootstrapMethods GetBootstrapMethods(uint16_t index);
};

#endif // __MVM_ATTRIBUTES_H
