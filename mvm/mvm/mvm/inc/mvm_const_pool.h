
#ifndef __MVM_CONST_POOL_H
#define __MVM_CONST_POOL_H

#include "mvm_type.h"

class ConstPoolType : public Type {
public:
    ConstPoolTag GetPoolTag();
};

class ConstUTF8 : public ConstPoolType {
public:
    uint16_t GetLength();
    const char *GetString();
};

class ConstInteger : public ConstPoolType {
public:
    int32_t GetValue();
};

class ConstFloat : public ConstPoolType {
public:
    float GetValue();
};

class ConstLong : public ConstPoolType {
public:
    int64_t GetValue();
};

class ConstDouble : public ConstPoolType {
public:
    double GetValue();
};

class ConstClass : public ConstPoolType {
public:
    ConstUTF8 GetName();
};

class ConstString : public ConstPoolType {
public:
    ConstUTF8 GetUTF8();
};

class ConstNameAndType : public ConstPoolType {
public:
    ConstUTF8 GetName();
    ConstUTF8 GetDescriptor();
};

class ConstField : public ConstPoolType {
public:
    ConstClass GetConstClass();
    ConstNameAndType GetNameAndType();
};

typedef ConstField ConstMethod;
typedef ConstField ConstInterfaceMethod;

class ConstMethodHandle : public ConstPoolType {
public:
    ReferenceKind GetReferenceKind();
    ConstPoolType GetReference();
};

class ConstMethodType : public ConstPoolType {
public:
    ConstUTF8 GetDescriptor();
};

class ConstInvokeDynamic : public ConstPoolType {
public:
    void BootstrapMethodAttr();
    ConstNameAndType GetNameAndType();
};

#endif // __MVM_CONST_POOL_H
