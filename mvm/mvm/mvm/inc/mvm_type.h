
#ifndef __MVM_TYPE_H
#define __MVM_TYPE_H

#include "mvm_std_type.h"

typedef enum : uint8_t {
    CONST_UTF8 = 1,
    CONST_INTEGER = 3,
    CONST_FLOAT = 4,
    CONST_LONG = 5,
    CONST_DOUBLE = 6,
    CONST_CLASS = 7,
    CONST_STRING = 8,
    CONST_FIELD = 9,
    CONST_METHOD = 10,
    CONST_INTERFACE_METHOD = 11,
    CONST_NAME_AND_TYPE = 12,
    CONST_METHOD_HANDLE = 15,
    CONST_METHOD_TYPE = 16,
    CONST_INVOKE_DYNAMIC = 18,
} ConstPoolTag;

typedef enum : uint8_t {
    REF_GET_FIELD = 1,
    REF_GET_STATIC = 2,
    REF_PUT_FIELD = 3,
    REF_PUT_STATIC = 4,
    REF_INVOKE_VIRTUAL = 5,
    REF_INVOKE_STATIC = 6,
    REF_INVOKE_SPECIAL = 7,
    REF_NEW_INVOKE_SPECIAL = 8,
    REF_INVOKE_INTERFACE = 9,
} ReferenceKind;

typedef enum : uint16_t {
    CLASS_PUBLIC = 0x0001,
    CLASS_FINAL = 0x0010,
    CLASS_SUPER = 0x0020,
    CLASS_INTERFACE = 0x0200,
    CLASS_ABSTRACT = 0x0400,
    CLASS_SYNTHETIC = 0x1000,
    CLASS_ANNOTATION = 0x2000,
    CLASS_ENUM = 0x4000,
} ClassAccessFlag;

typedef enum : uint16_t {
    FIELD_PUBLIC = 0x0001,
    FIELD_PRIVATE = 0x0002,
    FIELD_PROTECTED = 0x0004,
    FIELD_STATIC = 0x0008,
    FIELD_FINAL = 0x0010,
    FIELD_VOLATILE = 0x0040,
    FIELD_TRANSIENT = 0x0080,
    FIELD_SYNTHETIC = 0x1000,
    FIELD_ENUM = 0x4000,
} FieldAccessFlag;

typedef enum : uint16_t {
    METHOD_PUBLIC = 0x0001,
    METHOD_PRIVATE = 0x0002,
    METHOD_PROTECTED = 0x0004,
    METHOD_STATIC = 0x0008,
    METHOD_FINAL = 0x0010,
    METHOD_SYNCHRONIZED = 0x0020,
    METHOD_BRIDGE = 0x0040,
    METHOD_VARARGS = 0x0080,
    METHOD_NATIVE = 0x0100,
    METHOD_ABSTRACT = 0x0400,
    METHOD_STRICT = 0x0800,
    METHOD_SYNTHETIC = 0x1000,
} MethodAccessFlag;

typedef enum : uint16_t {
    INNER_CLASS_PUBLIC = 0x0001,
    INNER_CLASS_PRIVATE = 0x0002,
    INNER_CLASS_PROTECTED = 0x0004,
    INNER_CLASS_STATIC = 0x0008,
    INNER_CLASS_FINAL = 0x0010,
    INNER_CLASS_INTERFACE = 0x0200,
    INNER_CLASS_ABSTRACT = 0x0400,
    INNER_CLASS_SYNTHETIC = 0x1000,
    INNER_CLASS_ANNOTATION = 0x2000,
    INNER_CLASS_ENUM = 0x4000,
} InnerClassAccessFlag;

class Type {
protected:
    uint8_t *data_ptr;
    uint8_t *data_offset;
public:
    Type();
    uint8_t *GetClassData();
};

#endif // __MVM_TYPE_H
