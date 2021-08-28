
#include "mvm_error_report.h"
#include "mvm_heap.h"

#define BLOCK_SIZE            16
#define NUM_BLOCK             (1 * 1024 * 1024 / BLOCK_SIZE)

static int32_t heap_current_index = 0;
static int32_t last_heap_info = -1;
static uint32_t heap_mem[(NUM_BLOCK * BLOCK_SIZE) / sizeof(uint32_t)];
static uint32_t heap_manager[NUM_BLOCK / 32] = {0};
static uint8_t heap_manager_refer[sizeof(heap_mem) / 32] = {0};

void Heap_MemSetRefer(uint32_t *ptr, uint8_t value) {
    uint32_t index = ptr - ((uint32_t *)heap_mem);
    if(value)
        heap_manager_refer[index / 8] |= 1 << (index % 8);
    else
        heap_manager_refer[index / 8] &= ~(1 << (index % 8));
}

uint8_t Heap_MemGetRefer(uint32_t *ptr) {
    uint32_t index = ptr - ((uint32_t *)heap_mem);
    return heap_manager_refer[index / 8] & (1 << (index % 8));
}

static void Heap_MemReferCpy(uint32_t *dst, uint32_t *src, uint32_t length) {
    uint32_t dst_index = dst - ((uint32_t *)heap_mem);
    uint32_t src_index = src - ((uint32_t *)heap_mem);

    for(uint32_t i = 0; i < length; i++) {
        if(heap_manager_refer[src_index / 8] & (1 << (src_index % 8)))
            heap_manager_refer[dst_index / 8] |= 1 << (dst_index % 8);
        else
            heap_manager_refer[dst_index / 8] &= ~(1 << (dst_index % 8));
        dst_index++;
        src_index++;
    }
}

static void Heap_MemReferClr(uint32_t *ptr, uint32_t length) {
    uint32_t index = ptr - ((uint32_t *)heap_mem);

    uint32_t N = (8 - (index % 8)) % 8;
    uint8_t mask = 0xFF >> N;
    if(N > length) {
        mask |= ~(0xFF >> (N - length));
        N = length;
    }
    heap_manager_refer[index / 8] &= mask;
    index += N;
    length -= N;

    if(length) {
        N = length / 8;
        for(uint32_t i = 0; i < N; i++)
            heap_manager_refer[i + index / 8] = 0;
        index += N * 8;

        N = length % 8;
        heap_manager_refer[index / 8] &= 0xFF << N;
    }
}

static void Heap_BlockStatusClr(uint32_t index_start, uint32_t length) {
    uint32_t N = (32 - (index_start % 32)) % 32;
    uint32_t mask = 0xFFFFFFFF >> N;
    if(N > length) {
        mask |= ~(0xFFFFFFFF >> (N - length));
        N = length;
    }
    heap_manager[index_start / 32] &= mask;
    index_start += N;
    length -= N;

    if(length) {
        N = length / 32;
        for(uint32_t i = 0; i < N; i++)
            heap_manager[i + index_start / 32] = 0;
        index_start += N * 32;

        N = length % 32;
        heap_manager[index_start / 32] &= 0xFFFFFFFF << N;
    }
}

static void Heap_BlockStatusSet(uint32_t index_start, uint32_t length) {
    uint32_t N = (32 - (index_start % 32)) % 32;
    uint32_t mask = 0xFFFFFFFF >> N;
    if(N > length) {
        mask |= ~(0xFFFFFFFF >> (N - length));
        N = length;
    }
    heap_manager[index_start / 32] |= ~mask;
    index_start += N;
    length -= N;

    if(length) {
        N = length / 32;
        for(uint32_t i = 0; i < N; i++)
            heap_manager[i + index_start / 32] = 0xFFFFFFFF;
        index_start += N * 32;

        N = length % 32;
        heap_manager[index_start / 32] |= ~(0xFFFFFFFF << N);
    }
}

static uint32_t Heap_GetBlockStaus(uint32_t block_index) {
    return heap_manager[block_index / 32] & (1 << (block_index % 32));
}

static HeapInfo_TypeDef *GetMemNearest(uint8_t *address) {
    HeapInfo_TypeDef *heaps_info = (HeapInfo_TypeDef *)&heap_mem[((sizeof(heap_mem) - sizeof(HeapInfo_TypeDef)) / sizeof(heap_mem[0]))];
    HeapInfo_TypeDef *heap_info_min = 0;
    uint8_t *addess_min = (uint8_t *)0xFFFFFFFF;
    for(int32_t i = 0; i <= last_heap_info; i++) {
        if(heaps_info[-i].Address &&
           ((uint32_t)heaps_info[-i].Address > (uint32_t)address) &&
           ((uint32_t)heaps_info[-i].Address < (uint32_t)addess_min)) {
            addess_min = heaps_info[-i].Address;
            heap_info_min = &heaps_info[-i];
        }
    }
    return heap_info_min;
}

static void __MemCpy(uint32_t *dst, uint32_t *src, uint32_t length) {
    uint32_t *addr_start = dst;
    uint32_t *addr_end = addr_start + length;
    for(; addr_start < addr_end; addr_start++) {
        *addr_start = *src;
        src++;
    }
}

static void Heap_CheckUsedAndMark(HeapInfo_TypeDef *heap_info) {
    HeapInfo_TypeDef *heaps_info = (HeapInfo_TypeDef *)&heap_mem[((sizeof(heap_mem) - sizeof(HeapInfo_TypeDef)) / sizeof(heap_mem[0]))];
    int32_t length;
    int32_t i;
    uint32_t *ptr;
    if(heap_info->Attribute == 3) {
        length = ((int32_t *)heap_info->Address)[0] + 2;
        i = 1;
        ptr = (uint32_t *)(heap_info->Address + 4);
    }
    else {
        length = heap_info->Size / 4;
        i = 0;
        ptr = (uint32_t *)heap_info->Address;
    }
    for(; i < length; i++) {
        if(*ptr && Heap_MemGetRefer(ptr)) {
            if((*ptr >= (uint32_t)&heaps_info[-last_heap_info]) && (*ptr <= (uint32_t)&heaps_info[0])) {
                HeapInfo_TypeDef *heap = (HeapInfo_TypeDef *)*ptr;
                if(heap->Address && !heap->Refer) {
                    heap->Refer = 1;
                    if(heap->Attribute == 2)
                        Heap_CheckUsedAndMark(heap);
                }
            }
        }
        ptr++;
    }
}

static void Heap_FreeNotUsed() {
    HeapInfo_TypeDef *heaps_info = (HeapInfo_TypeDef *)&heap_mem[((sizeof(heap_mem) - sizeof(HeapInfo_TypeDef)) / sizeof(heap_mem[0]))];
    uint32_t used_count = 0;

    for(int32_t i = 0; i <= last_heap_info; i++) {
        if(heaps_info[-i].Address)
            if(((heaps_info[-i].Attribute & 0x01) == 0x01)) {
                heaps_info[-i].Refer = 1;
                used_count++;
            }
            else
                heaps_info[-i].Refer = 0;
    }
    for(int32_t i = 0; i <= last_heap_info; i++) {
        if(heaps_info[-i].Address && ((heaps_info[-i].Attribute & 0x01) == 0x01)) {
            Heap_CheckUsedAndMark(&heaps_info[-i]);
            if(--used_count == 0)
                break;
        }
    }
    int new_last_heap_info = 0;
    for(int32_t i = 0; i <= last_heap_info; i++) {
        if(heaps_info[-i].Address) {
            if(!heaps_info[-i].Refer)
                Heap_Free(&heaps_info[-i]);
            else
                new_last_heap_info = i;
        }
    }
    Heap_BlockStatusClr(NUM_BLOCK - 1 - (last_heap_info * sizeof(HeapInfo_TypeDef)) / NUM_BLOCK, (last_heap_info - new_last_heap_info) * sizeof(HeapInfo_TypeDef) / NUM_BLOCK);
    last_heap_info = new_last_heap_info;
}

static void Heap_GroupMemory() {
    for(uint32_t i = 0; i < NUM_BLOCK; i++) {
        if(!Heap_GetBlockStaus(i)) {
            HeapInfo_TypeDef *mem_nearest = GetMemNearest((uint8_t *)(((uint32_t)heap_mem) + i * BLOCK_SIZE));
            if(!mem_nearest)
                return;
            uint32_t *dst = (uint32_t *)((uint32_t)heap_mem + i * BLOCK_SIZE);
            uint32_t *src = (uint32_t *)mem_nearest->Address;
            uint32_t length = mem_nearest->Size / sizeof(uint32_t);
            if(mem_nearest->Size % sizeof(uint32_t))
                length++;
            __MemCpy(dst, src, length);
            uint32_t src_block_offset = (((uint32_t)src) - ((uint32_t)heap_mem)) / BLOCK_SIZE;
            uint32_t NumBlock = mem_nearest->Size / BLOCK_SIZE;
            if(mem_nearest->Size % BLOCK_SIZE)
                NumBlock++;

            Heap_BlockStatusClr(src_block_offset, NumBlock);
            Heap_BlockStatusSet(i, NumBlock);

            Heap_MemReferCpy(dst, src, length);
            mem_nearest->Address = (uint8_t *)dst;
            i += NumBlock - 1;
        }
    }
}

void Heap_GarbageCollect() {
    Heap_FreeNotUsed();
    Heap_GroupMemory();
}

static unsigned int GetN(unsigned int value) {
    unsigned int N = 0;
    if(value & 0xFFFF0000)
        N = 16;
    if(value & 0xFF00FF00)
        N += 8;
    if(value & 0xF0F0F0F0)
        N += 4;
    if(value & 0xCCCCCCCC)
        N += 2;
    if(value & 0xAAAAAAAA)
        N += 1;
    return N;
}

static int32_t GetIndex(uint32_t num) {
    static uint32_t start_index = 0;
    int32_t index_temp = -1;
    uint32_t index_end = ((sizeof(heap_mem) - ((last_heap_info + 1) * sizeof(HeapInfo_TypeDef))) / BLOCK_SIZE);
    if((start_index * 32) >= index_end)
        start_index = 0;
    for(uint32_t i = start_index; i < (NUM_BLOCK / 32); i++) {
        uint32_t value = heap_manager[i];
        while(value != 0xFFFFFFFF) {
            uint32_t temp = (~value) & (value + 1);
            uint32_t N = GetN(temp);
            uint32_t index = N + i * 32;
            uint8_t found = 1;
            if((index + num) > index_end)
                return -1;
            for(uint32_t j = 1; j < num; j++) {
                if(Heap_GetBlockStaus(index + j)) {
                    i = (index + j) / 32;
                    value = heap_manager[i];
                    value |= ~(0xFFFFFFFF << ((index + j) % 32));
                    found = 0;
                    break;
                }
            }
            if(found) {
                if(index_temp > 0)
                    start_index = index_temp;
                else
                    start_index = i;
                return index;
            }
            else {
                if(index_temp < 0)
                    index_temp = i;
                continue;
            }
        }
    }
    if(index_temp > 0)
        start_index = index_temp;
    else
        start_index = 0;
    return -1;
}

static HeapInfo_TypeDef *Heap_TryMalloc(uint32_t size, uint8_t attribute, uint32_t type) {
    HeapInfo_TypeDef *heaps_info = (HeapInfo_TypeDef *)&heap_mem[((sizeof(heap_mem) - sizeof(HeapInfo_TypeDef)) / sizeof(heap_mem[0]))];
    HeapInfo_TypeDef *heap_info = 0;
    int32_t last_heap_info_old = last_heap_info;
    if(heap_current_index >= last_heap_info)
        heap_current_index = 0;
    for(; heap_current_index < last_heap_info; heap_current_index++) {
        if(heaps_info[-heap_current_index].Address == 0) {
            heap_info = &heaps_info[-heap_current_index];
            break;
        }
    }
    if(heap_info == 0) {
        if(last_heap_info < 0x7FFFFFFF) {
            last_heap_info++;
            heap_info = &heaps_info[-last_heap_info];
        }
        else
            return 0;
    }
    uint32_t block_of_heap_info = (((uint32_t)heap_info) - ((uint32_t)heap_mem)) / BLOCK_SIZE;
    Heap_BlockStatusSet(block_of_heap_info, 1);

    if(size == 0) {
        heap_info->Address = (uint8_t *)0xFFFFFFFF;
        heap_info->Attribute = attribute;
        heap_info->Size = size;
        heap_info->Type = (uint16_t)type;
        heap_info->Dimensions = (type >> 16) & ((1 << 12) - 1);
        heap_info->MoniterCount = 0;
        return heap_info;
    }

    uint32_t N = size / BLOCK_SIZE;
    if(size % BLOCK_SIZE)
        N++;
    int32_t index = GetIndex(N);
    if(index < 0) {
        last_heap_info = last_heap_info_old;
        return 0;
    }
    Heap_BlockStatusSet(index, N);

    heap_info->Address = (uint8_t *)&heap_mem[(index * BLOCK_SIZE) / sizeof(heap_mem[0])];
    heap_info->Attribute = attribute;
    heap_info->Size = size;
    heap_info->Type = (uint16_t)type;
    heap_info->Dimensions = (type >> 16) & ((1 << 12) - 1);
    heap_info->MoniterCount = 0;
    return heap_info;
}

HeapInfo_TypeDef *Heap_Malloc(uint32_t size, uint8_t attribute, uint32_t type) {
    HeapInfo_TypeDef *heap_info = Heap_TryMalloc(size, attribute, type);
    if(heap_info == 0) {
        Heap_FreeNotUsed();
        heap_info = Heap_TryMalloc(size, attribute, type);
        if(heap_info == 0) {
            Heap_GroupMemory();
            heap_info = Heap_TryMalloc(size, attribute, type);
        }
    }
    if(heap_info) {
        uint32_t length = heap_info->Size / sizeof(uint32_t);
        if(heap_info->Size % sizeof(uint32_t))
            length++;
        if(heap_info->Address != (uint8_t *)0xFFFFFFFF)
            Heap_MemReferClr((uint32_t *)heap_info->Address, length);
    }
    return heap_info;
}

HeapInfo_TypeDef *Heap_Calloc(uint32_t size, uint8_t attribute, uint32_t type) {
    HeapInfo_TypeDef *ptr = (HeapInfo_TypeDef *)Heap_Malloc(size, attribute, type);
    if(ptr) {
        uint32_t *start_ptr = (uint32_t *)ptr->Address;
        uint32_t *end_ptr = start_ptr + (size / 4) + ((size % 4) ? 1 : 0);
        for(; start_ptr < end_ptr; start_ptr++)
            *start_ptr = 0;
    }
    return ptr;
}

void Heap_Free(HeapInfo_TypeDef *heap_info) {
    if(heap_info == 0)
        return;
    if(heap_info->Address != (uint8_t *)0xFFFFFFFF) {
        uint32_t offset = (((uint32_t)heap_info->Address) - ((uint32_t)heap_mem)) / BLOCK_SIZE;
        uint32_t NumBlock = heap_info->Size / BLOCK_SIZE;
        if(heap_info->Size % BLOCK_SIZE)
            NumBlock++;
        Heap_BlockStatusClr(offset, NumBlock);
    }
    int32_t heap_info_index = (((uint32_t)&heap_mem[(sizeof(heap_mem) / sizeof(heap_mem[0])) - 3]) - ((uint32_t)heap_info)) / (sizeof(heap_mem[0]) * 3);
    if(heap_current_index > heap_info_index)
        heap_info_index = heap_info_index;
    heap_info->Address = 0;
}

uint8_t Heap_Realloc(HeapInfo_TypeDef *heap_info, uint32_t size) {
    uint32_t size_old = heap_info->Size;
    uint32_t N = size / BLOCK_SIZE;
    if(size % BLOCK_SIZE)
        N++;
    uint32_t NumBlock = heap_info->Size / BLOCK_SIZE;
    if(heap_info->Size % BLOCK_SIZE)
        NumBlock++;
    if(N <= NumBlock) {
        uint32_t offset = ((uint32_t)heap_info->Address - (uint32_t)heap_mem) / BLOCK_SIZE;
        Heap_BlockStatusClr(offset, NumBlock - N);
        heap_info->Size = size;
        for(; size_old < size; size_old++)
            ((uint8_t *)(heap_info->Address))[size_old] = 0;
        return 1;
    }
    uint32_t index = (((uint32_t)heap_info->Address + NumBlock * BLOCK_SIZE) - (uint32_t)heap_mem) / BLOCK_SIZE;
    uint32_t count = 0;
    N -= NumBlock;
    for(uint32_t i = index; i < NUM_BLOCK; i++) {
        if(!Heap_GetBlockStaus(i)) {
            if(++count == N) {
                Heap_BlockStatusSet(index, N);
                heap_info->Size = size;
                for(; size_old < size; size_old++)
                    ((uint8_t *)(heap_info->Address))[size_old] = 0;
                return 1;
            }
        }
        else
            break;
    }
    uint32_t *addr_old = (uint32_t *)heap_info->Address;
    size_old = heap_info->Size / 4;
    if(heap_info->Size % 4)
        size_old++;
    HeapInfo_TypeDef *heap_new = Heap_Malloc(size, 0, 0);
    if(heap_new) {
        Heap_Free(heap_info);
        heap_info->Address = heap_new->Address;
        heap_info->Size = heap_new->Size;
        for(uint32_t i = 0; i < size_old; i++)
            ((uint32_t *)heap_info->Address)[i] = addr_old[i];
        Heap_MemReferCpy((uint32_t *)heap_new->Address, addr_old, size_old);
        uint32_t size_new = heap_info->Size / 4;
        if(heap_info->Size % 4)
            size_new++;
        for(uint32_t i = size_old; i < size_new; i++)
            ((uint32_t *)heap_info->Address)[i] = 0;
        heap_new->Address = 0;
        return 1;
    }
    return 0;
}

uint32_t Heap_GetSize(HeapInfo_TypeDef *heap_info) {
    if(heap_info)
        return heap_info->Size;
    return 0;
}

uint32_t Heap_GetSizeReality(HeapInfo_TypeDef *heap_info) {
    if(heap_info) {
        uint32_t size = (heap_info->Size / BLOCK_SIZE) *BLOCK_SIZE;
        if(heap_info->Size % BLOCK_SIZE)
            size += BLOCK_SIZE;
        return size;
    }
    return 0;
}
