
#ifndef __MVM_SYSTEM_API_H
#define __MVM_SYSTEM_API_H

#include "mvm_std_type.h"
#include "mvm_stack.h"
#include "mvm.h"

typedef struct {
    const uint32_t NameLength;
    const uint32_t DescLength;
    const char *Name;
    const char *Descriptor;
    uint8_t (*Method)(mVM *mvm, Stack &arg, uint32_t n_arg, uint64_t &return_value);
} SystemMethodInfo;

typedef struct {
    const uint32_t ClassNameLength;
    const char *ClassName;
    const uint32_t MethodCount;
    SystemMethodInfo MethodList[];
} SystemClassInfo;

typedef struct {
    const uint32_t ClassCount;
    const SystemClassInfo *ClassList[];
} SystemClassInfoList;

extern SystemClassInfoList SystemClassList;

#endif // __MVM_SYSTEM_API_H
