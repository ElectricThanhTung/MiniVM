
#ifndef __ATTRIBUTES_H
#define __ATTRIBUTES_H

#include "mvm_type.h"
#include "mvm_const_pool.h"

class AttributeType : public Type {
public:
    uint32_t GetAttributeLength();
};

#endif // __ATTRIBUTES_H
