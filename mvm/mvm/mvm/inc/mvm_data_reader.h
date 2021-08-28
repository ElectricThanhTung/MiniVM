
#ifndef __MVM_DATA_READER_H
#define __MVM_DATA_READER_H

#include "mvm_std_type.h"

#define ReadUInt16(buffer)        (((uint16_t)(buffer)[0] << 8) | (buffer)[1])

#define ReadUInt32(buffer)        (((uint32_t)(buffer)[0] << 24) | ((uint16_t)(buffer)[1] << 16) |            \
                                  ((buffer)[2] << 8) | (buffer)[3])

#define ReadUInt64(buffer)        (((uint64_t)(buffer)[0] << 56) | ((uint64_t)(buffer)[1] << 48) |            \
                                  ((uint64_t)(buffer)[2] << 40) | ((uint64_t)(buffer)[3] << 32) |             \
                                  ((uint32_t)(buffer)[4] << 24) | ((uint32_t)(buffer)[5] << 16) |             \
                                  ((uint16_t)(buffer)[6] << 8) | (buffer)[7])

//uint16_t ReadUInt16(uint8_t *buffer);
//uint32_t ReadUInt32(uint8_t *buffer);
//uint64_t ReadUInt64(uint8_t *buffer);

#endif // __MVM_DATA_READER_H
