
#ifndef __CONST_POOL_H
#define __CONST_POOL_H

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

class ConstNameAndType : public ConstPoolType {
public:
    ConstUTF8 GetName();
    ConstUTF8 GetDescriptor();
};

#endif // __CONST_POOL_H
