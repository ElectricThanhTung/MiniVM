
#include <iostream>
#include "mvm_error_report.h"
#include "mvm_class_loader.h"
#include "mvm_native_api.h"

#define CLASS_NAME(name)                     (sizeof(name) - 1), name
#define METHOD_INFO(name, desc, func)        (sizeof(name) - 1), (sizeof(desc) - 1), name, desc, (uint8_t (*)(mVM *, Stack &, uint32_t, uint64_t &))(func)

static uint8_t write(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    int32_t value = stack.ValueUInt32(stack.GetSP() - n_arg + 1);
    HeapInfo_TypeDef *mem_info = (HeapInfo_TypeDef *)((uint32_t *)((HeapInfo_TypeDef *)value)->Address)[0];
    printf((const char *)mem_info->Address);
    return 1;
}

static uint8_t writeln(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    int32_t value = stack.ValueUInt32(stack.GetSP() - n_arg + 1);
    HeapInfo_TypeDef *mem_info = (HeapInfo_TypeDef *)((uint32_t *)((HeapInfo_TypeDef *)value)->Address)[0];
    printf("%s\n", (const char *)mem_info->Address);
    return 1;
}

static uint8_t arraycopy(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    int32_t SP = stack.GetSP();
    HeapInfo_TypeDef *src = (HeapInfo_TypeDef *)stack.ValueUInt32(SP - n_arg + 1);
    int32_t srcPos = stack.ValueUInt32(SP - n_arg + 2);
    HeapInfo_TypeDef *dst = (HeapInfo_TypeDef *)stack.ValueUInt32(SP - n_arg + 3);
    int32_t dstPos = stack.ValueUInt32(SP - n_arg + 4);
    int32_t length = stack.ValueUInt32(SP - n_arg + 5);

    if(src->Type != dst->Type) {
        return_value = (uint32_t)mvm->CreateThrowableObject("Type mismatch");
        return 0;
    }
    uint8_t size_of_element;
    if(src->Type < 12) {
        static const uint8_t size_of_type[8] = {
            sizeof(int8_t),
            sizeof(int16_t),
            sizeof(float),
            sizeof(double),
            sizeof(int8_t),
            sizeof(int16_t),
            sizeof(int32_t),
            sizeof(int64_t)
        };
        size_of_element = size_of_type[src->Type - 4];
    }
    else
        size_of_element = 4;
    if((length > (src->Size / size_of_element)) || (length > (dst->Size / size_of_element))) {
        return_value = (uint32_t)mvm->CreateThrowableObject("Length parameter out of size");
        return 0;
    }
    if(size_of_element == 1) {
        uint8_t *src_array = &((uint8_t *)src->Address)[srcPos];
        uint8_t *dst_array = &((uint8_t *)dst->Address)[dstPos];
        uint8_t *src_end = src_array + length;
        for(; src_array < src_end; src_array++) {
            *dst_array = *src_array;
            dst_array++;
        }
    }
    else if(size_of_element == 2) {
        uint16_t *src_array = &((uint16_t *)src->Address)[srcPos];
        uint16_t *dst_array = &((uint16_t *)dst->Address)[dstPos];
        uint16_t *src_end = src_array + length;
        for(; src_array < src_end; src_array++) {
            *dst_array = *src_array;
            dst_array++;
        }
    }
    else if(size_of_element == 4) {
        uint32_t *src_array = &((uint32_t *)src->Address)[srcPos];
        uint32_t *dst_array = &((uint32_t *)dst->Address)[dstPos];
        uint32_t *src_end = src_array + length;
        for(; src_array < src_end; src_array++) {
            *dst_array = *src_array;
            dst_array++;
        }
    }
    else if(size_of_element == 8) {
        uint64_t *src_array = &((uint64_t *)src->Address)[srcPos];
        uint64_t *dst_array = &((uint64_t *)dst->Address)[dstPos];
        uint64_t *src_end = src_array + length;
        for(; src_array < src_end; src_array++) {
            *dst_array = *src_array;
            dst_array++;
        }
    }
    return 1;
}

static uint8_t getName(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.ValueUInt32(stack.GetSP() - n_arg + 1);

    int32_t class_file_index = mvm->GetStringClassIndex();
    uint8_t *data_ptr = mvm->GetClassFile(class_file_index);
    if(data_ptr == 0) {
        return_value = (uint32_t)mvm->CreateThrowableObject("java.lang.Object.getName(): Can't found String class");
        return 0;
    }

    const char *str;
    uint16_t str_len;
    if (objectref->Type >= 12) {
        data_ptr = mvm->GetClassFile(objectref->Type - 12);
        if (data_ptr == 0) {
            return_value = (uint32_t)mvm->CreateThrowableObject("java.lang.Object.getName(): Can't found class");
            return 0;
        }

        ConstUTF8 class_name = ClassLoader(mvm, data_ptr).GetThisClass().GetName();

        str = class_name.GetString();
        str_len = class_name.GetLength();
        if (objectref->Dimensions)
            str_len++;
    }
    else {
        static const char type_list[] = "ZCFDBSIJ";
        str = &type_list[objectref->Type - 4];
        str_len = 1;
    }

    HeapInfo_TypeDef *heap_info = mvm->CreateStringObject(str_len + objectref->Dimensions);
    HeapInfo_TypeDef *data = (HeapInfo_TypeDef *)((uint32_t *)heap_info->Address)[0];

    uint8_t *ptr = data->Address;
    for (uint16_t i = 0; i < objectref->Dimensions; i++) {
        *ptr = '[';
        ptr++;
    }
    if ((objectref->Type >= 12) && (objectref->Dimensions)) {
        *ptr = 'L';
        ptr++;
    }
    for(uint16_t i = 0; i < str_len; i++) {
        if(str[i] != '/')
            ptr[i] = str[i];
        else
            ptr[i] = '.';
    }
    ptr[str_len] = 0;

    return_value = (uint32_t)heap_info;
    return 1;
}

static uint8_t floatToRawIntBits(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    return_value = stack.ValueUInt32(stack.GetSP() - n_arg + 1);
    return 1;
}

static uint8_t intBitsToFloat(mVM *mvm, Stack &stack, uint32_t n_arg, uint64_t &return_value) {
    return_value = stack.ValueUInt32(stack.GetSP() - n_arg + 1);
    return 1;
}

SystemClassInfo java_io_printstream = {
    CLASS_NAME("java/io/PrintStream"), // Class Name
    2,                                 // Method Count
    METHOD_INFO("write", "(Ljava/lang/String;)V", write),
    METHOD_INFO("writeln", "(Ljava/lang/String;)V", writeln),
};

SystemClassInfo java_lang_system = {
    CLASS_NAME("java/lang/System"),    // Class Name
    1,                                 // Method Count
    METHOD_INFO("arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", arraycopy),
};

SystemClassInfo java_lang_object = {
    CLASS_NAME("java/lang/Object"),    // Class Name
    1,                                 // Method Count
    METHOD_INFO("getName", "()Ljava/lang/String;", getName),
};

SystemClassInfo java_lang_float = {
    CLASS_NAME("java/lang/Float"),     // Class Name
    2,                                 // Method Count
    METHOD_INFO("floatToRawIntBits", "(F)I", floatToRawIntBits),
    METHOD_INFO("intBitsToFloat", "(I)F", intBitsToFloat),
};

SystemClassInfoList SystemClassList = {
    4,                                 // Class Count
    &java_io_printstream,
    &java_lang_system,
    &java_lang_object,
    &java_lang_float,
};
