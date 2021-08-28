
#ifndef __INFO_H
#define __INFO_H

#include "mvm_type.h"
#include "mvm_const_pool.h"
#include "mvm_attributes.h"

class FieldInfo : public Type {
public:
    FieldAccessFlag GetAccessFlag();
    ConstUTF8 GetName();
    ConstUTF8 GetDescriptor();
    uint16_t GetAttributesCount();
    AttributeType GetAttributes(uint16_t index);
};

class MethodInfo : public FieldInfo {
public:
    MethodAccessFlag GetAccessFlag();
};

#endif // __INFO_H
