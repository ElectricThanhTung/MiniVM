
#include "mvm_type.h"

Type::Type() {
    data_ptr = 0;
    data_offset = 0;
}

uint8_t *Type::GetClassData() {
    return data_ptr;
}
