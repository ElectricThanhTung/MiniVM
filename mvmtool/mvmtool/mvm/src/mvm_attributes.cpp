
#include "mvm_attributes.h"
#include "mvm_class_loader.h"
#include "mvm_data_reader.h"

uint32_t AttributeType::GetAttributeLength() {
    return ReadUInt32(&data_offset[2]);
}
