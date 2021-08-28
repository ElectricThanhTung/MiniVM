
#include "mvm.h"
#include "mvm_sdk.h"
#include "mvm_class_loader.h"
#include "mvm_error_report.h"

static int32_t __strCmp(ConstUTF8 &str1, ConstUTF8 &str2) {
    uint16_t len1 = str1.GetLength();
    int32_t lenCmp = len1 - str2.GetLength();
    if(lenCmp)
        return lenCmp;
    const char *__str1 = str1.GetString();
    const char *__str2 = str2.GetString();
    while(len1) {
        if(*__str1 != *__str2)
            return *__str1 - *__str2;
        __str1++;
        __str2++;
        len1--;
    }
    return 0;
}

static int32_t __strCmp(ConstUTF8 &str1, const char *str2, uint16_t length) {
    int32_t lenCmp = str1.GetLength() - length;
    if(lenCmp)
        return lenCmp;
    const char *__str1 = str1.GetString();
    while(length) {
        if(*__str1 != *str2)
            return *__str1 - *str2;
        __str1++;
        str2++;
        length--;
    }
    return 0;
}

static uint32_t __strLen(const char *str) {
    uint32_t len = 0;
    while(*str++)
        len++;
    return len;
}

mVM::mVM(const uint8_t *program) {
    this->program_info = program;
    this->static_fields = 0;
    this->string_class_index = FindClassFileIndex("java/lang/String", 16);
    this->throwable_class_index = FindClassFileIndex("java/lang/Throwable", 19);
}

uint32_t mVM::GetStaticFieldsSize(uint16_t class_index) {
    ClassLoader class_loader = ClassLoader(this, GetClassFile(class_index));
    uint16_t fields_count = class_loader.GetFieldsCount();
    uint32_t size = 0;
    for(uint16_t i = 0; i < fields_count; i++) {
        FieldInfo field_info = class_loader.GetField(i);
        if((field_info.GetAccessFlag() & FIELD_STATIC) == FIELD_STATIC) {
            uint8_t field_type = field_info.GetDescriptor().GetString()[0];
            size += ((field_type == 'J') || (field_type == 'D')) ? 8 : 4;
        }
    }
    return size;
}

void mVM::SetStaticField(void *field_name, int32_t value, uint8_t is_refer) {
    uint32_t index = 0;
    uint32_t class_count = ((uint32_t *)mvm_sdk_data)[0] + ((uint32_t *)program_info)[0];
    uint8_t *class_loader[2] = {(uint8_t *)this};
    for(uint16_t i = 0; i < class_count; i++) {
        class_loader[1] = GetClassFile(i);
        uint16_t fields_count = ((ClassLoader *)class_loader)->GetFieldsCount();
        for(uint16_t j = 0; j < fields_count; j++) {
            FieldInfo field_info = ((ClassLoader *)class_loader)->GetField(j);
            if((field_info.GetAccessFlag() & FIELD_STATIC) == FIELD_STATIC) {
                ConstUTF8 name = field_info.GetName();
                if(__strCmp(*(ConstUTF8 *)field_name, name) == 0) {
                    uint32_t *ptr = &((uint32_t *)static_fields->Address)[index];
                    *ptr = value;
                    Heap_MemSetRefer(ptr, is_refer);
                    return;
                }
                uint8_t field_type = field_info.GetDescriptor().GetString()[0];
                index += ((field_type == 'J') || (field_type == 'D')) ? 2 : 1;
            }
        }
    }
}

void mVM::SetStaticField(void *field_name, int64_t value) {
    uint32_t index = 0;
    uint32_t class_count = ((uint32_t *)mvm_sdk_data)[0] + ((uint32_t *)program_info)[0];
    uint8_t *class_loader[2] = {(uint8_t *)this};
    for(uint16_t i = 0; i < class_count; i++) {
        class_loader[1] = GetClassFile(i);
        uint16_t fields_count = ((ClassLoader *)class_loader)->GetFieldsCount();
        for(uint16_t j = 0; j < fields_count; j++) {
            FieldInfo field_info = ((ClassLoader *)class_loader)->GetField(j);
            if((field_info.GetAccessFlag() & FIELD_STATIC) == FIELD_STATIC) {
                ConstUTF8 name = field_info.GetName();
                if(__strCmp(*(ConstUTF8 *)field_name, name) == 0) {
                    *(uint64_t *)&((uint32_t *)static_fields->Address)[index] = value;
                    return;
                }
                uint8_t field_type = field_info.GetDescriptor().GetString()[0];
                index += ((field_type == 'J') || (field_type == 'D')) ? 2 : 1;
            }
        }
    }
}

int32_t mVM::GetStaticFieldUInt32(void *field_name) {
    uint32_t index = 0;
    uint32_t class_count = ((uint32_t *)mvm_sdk_data)[0] + ((uint32_t *)program_info)[0];
    uint8_t *class_loader[2] = {(uint8_t *)this};
    for(uint16_t i = 0; i < class_count; i++) {
        class_loader[1] = GetClassFile(i);
        uint16_t fields_count = ((ClassLoader *)class_loader)->GetFieldsCount();
        for(uint16_t j = 0; j < fields_count; j++) {
            FieldInfo field_info = ((ClassLoader *)class_loader)->GetField(j);
            if((field_info.GetAccessFlag() & FIELD_STATIC) == FIELD_STATIC) {
                ConstUTF8 name = field_info.GetName();
                if(__strCmp(*(ConstUTF8 *)field_name, name) == 0)
                    return ((uint32_t *)static_fields->Address)[index];
                uint8_t field_type = field_info.GetDescriptor().GetString()[0];
                index += ((field_type == 'J') || (field_type == 'D')) ? 2 : 1;
            }
        }
    }
    Error("Can't found field!");
    return 0;
}

int64_t mVM::GetStaticFieldUInt64(void *field_name) {
    uint32_t index = 0;
    uint32_t class_count = ((uint32_t *)mvm_sdk_data)[0] + ((uint32_t *)program_info)[0];
    uint8_t *class_loader[2] = {(uint8_t *)this};
    for(uint16_t i = 0; i < class_count; i++) {
        class_loader[1] = GetClassFile(i);
        uint16_t fields_count = ((ClassLoader *)class_loader)->GetFieldsCount();
        for(uint16_t j = 0; j < fields_count; j++) {
            FieldInfo field_info = ((ClassLoader *)class_loader)->GetField(j);
            if((field_info.GetAccessFlag() & FIELD_STATIC) == FIELD_STATIC) {
                ConstUTF8 name = field_info.GetName();
                if(__strCmp(*(ConstUTF8 *)field_name, name) == 0)
                    return *(uint64_t *)&((uint32_t *)static_fields->Address)[index];
                uint8_t field_type = field_info.GetDescriptor().GetString()[0];
                index += ((field_type == 'J') || (field_type == 'D')) ? 2 : 1;
            }
        }
    }
    Error("Can't found field!");
    return 0;
}

uint8_t *mVM::FindClassFile(void *name) {
    int32_t index = FindClassFileIndex(((ConstUTF8 *)name)->GetString(), ((ConstUTF8 *)name)->GetLength());
    if(index >= 0)
        return GetClassFile(index);
    return 0;
}

int32_t mVM::FindClassFileIndex(void *name) {
    return FindClassFileIndex(((ConstUTF8 *)name)->GetString(), ((ConstUTF8 *)name)->GetLength());
}

int32_t mVM::FindClassFileIndex(const char *name, uint16_t name_length) {
    uint8_t *class_loader[2] = {(uint8_t *)this};

    uint32_t sdk_class_count = ((uint32_t *)mvm_sdk_data)[0];
    int32_t r = sdk_class_count - 1;
    int32_t l = 0;
    while(r >= l) {
        uint32_t mid = l + (r - l) / 2;
        class_loader[1] = (uint8_t *)(((uint32_t *)mvm_sdk_data)[1 + mid] + mvm_sdk_data);
        ConstUTF8 class_name = ((ClassLoader *)class_loader)->GetThisClass().GetName();
        int32_t cmp_value = __strCmp(class_name, name, name_length);
        if(cmp_value == 0)
            return mid;
        else if(cmp_value > 0)
            r = mid - 1;
        else
            l = mid + 1;
    }

    r = ((uint32_t *)program_info)[0] - 1;
    l = 0;
    while(r >= l) {
        uint32_t mid = l + (r - l) / 2;
        class_loader[1] = (uint8_t *)(((uint32_t *)program_info)[1 + mid] + program_info);
        ConstUTF8 class_name = ((ClassLoader *)class_loader)->GetThisClass().GetName();
        int32_t cmp_value = __strCmp(class_name, name, name_length);
        if(cmp_value == 0)
            return mid + sdk_class_count;
        else if(cmp_value > 0)
            r = mid - 1;
        else
            l = mid + 1;
    }

    return -1;
}

int32_t mVM::GetStringClassIndex() {
    return string_class_index;
}

uint8_t *mVM::GetClassFile(uint16_t index) {
    if(index < ((uint32_t *)mvm_sdk_data)[0])
        return (uint8_t *)(((uint32_t *)mvm_sdk_data)[1 + index] + mvm_sdk_data);
    else {
        index -= ((uint32_t *)mvm_sdk_data)[0];
        if(index < ((uint32_t *)program_info)[0])
            return (uint8_t *)(((uint32_t *)program_info)[1 + index] + program_info);
    }
    return 0;
}

uint64_t mVM::Run(const char *argv[], uint32_t argc) {
    uint32_t static_fields_size = 0;
    uint32_t class_count = ((uint32_t *)mvm_sdk_data)[0] + ((uint32_t *)program_info)[0];
    for(uint16_t i = 0; i < class_count; i++)
        static_fields_size += GetStaticFieldsSize(i);
    if(static_fields_size)
        static_fields = Heap_Calloc(static_fields_size, 1, 10);

    uint64_t return_value = 0;
    MethodInfo main_method;
    for(uint16_t i = 0; i < class_count; i++) {
        ClassLoader class_loader = ClassLoader(this, GetClassFile(i));
        MethodInfo method_info = class_loader.FindMethod("<clinit>", 8, "()V", 3);
        if(method_info.GetClassData() != 0) {
            if(!class_loader.RunMethod(this->stack, method_info, 0, return_value)) {
                HeapInfo_TypeDef *throwable_object = ((HeapInfo_TypeDef *)(uint32_t)return_value);
                HeapInfo_TypeDef *string_object = (HeapInfo_TypeDef *)((uint32_t *)throwable_object->Address)[0];
                HeapInfo_TypeDef *byte_object = (HeapInfo_TypeDef *)((uint32_t *)string_object->Address)[0];
                Error((const char *)byte_object->Address);
                return_value = 0;
                goto exit;
            }
        }

        method_info = class_loader.FindMethod("main", 4, "([Ljava/lang/String;)V", 22);
        if(method_info.GetClassData() != 0)
            main_method = method_info;
    }

    if(main_method.GetClassData() != 0) {
        if(argc) {
            if(stack.GetFreeSpace() < 4) {
                stack.MinSize = 16;
                stack.Realloc(stack.MinSize);
            }
            if(string_class_index < 0) {
                Error("public static void Main(String[]): Can't found class java.lang.String");
                goto exit;
            }
            HeapInfo_TypeDef *string_arg = Heap_Malloc(argc * 4, 2, (1 << 16) | (string_class_index + 12));
            stack.Push((void *)string_arg);
            for(uint32_t i = 0; i < argc; i++) {
                ((uint32_t *)(string_arg->Address))[i] = (uint32_t)this->CreateStringObject(argv[i]);
                Heap_MemSetRefer(&((uint32_t *)(string_arg->Address))[i], 1);
            }
        }
        if(!ClassLoader(this, main_method.GetClassData()).RunMethod(stack, main_method, 1, return_value)) {
            HeapInfo_TypeDef *throwable_object = ((HeapInfo_TypeDef *)(uint32_t)return_value);
            HeapInfo_TypeDef *string_object = (HeapInfo_TypeDef *)((uint32_t *)throwable_object->Address)[0];
            HeapInfo_TypeDef *byte_object = (HeapInfo_TypeDef *)((uint32_t *)string_object->Address)[0];
            Error((const char *)byte_object->Address);
            return_value = 0;
        }
        goto exit;
    }

    Error("Can't found main method!");
exit:
    Heap_Free(static_fields);
    this->stack.Free();
    return return_value;
}

HeapInfo_TypeDef *mVM::CreateStringObject(const char *str) {
    HeapInfo_TypeDef *string_object = this->CreateStringObject(__strLen(str));
    if(string_object) {
        HeapInfo_TypeDef *data = (HeapInfo_TypeDef *)((uint32_t *)string_object->Address)[0];
        uint8_t *ptr = data->Address;
        while(*str) {
            *ptr = *str;
            str++;
            ptr++;
        }
        *ptr = 0;
    }
    return string_object;
}

HeapInfo_TypeDef *mVM::CreateStringObject(uint16_t length) {
    if(string_class_index < 0)
        return 0;
    HeapInfo_TypeDef *string_object = Heap_Malloc(4, 1, string_class_index + 12);
    if(string_object) {
        HeapInfo_TypeDef *data = Heap_Malloc(length + 1, 0, (1 << 16) | 8);
        ((uint32_t *)string_object->Address)[0] = (uint32_t)data;
        Heap_MemSetRefer((uint32_t *)string_object->Address, 1);
        string_object->Attribute = 2;
    }
    return string_object;
}

HeapInfo_TypeDef *mVM::CreateStringObject(const char *str, uint16_t length) {
    HeapInfo_TypeDef *string_object = this->CreateStringObject(length);
    if(string_object) {
        HeapInfo_TypeDef *data = (HeapInfo_TypeDef *)((uint32_t *)string_object->Address)[0];
        uint8_t *ptr = data->Address;
        while(length--) {
            *ptr = *str;
            str++;
            ptr++;
        }
        *ptr = 0;
    }
    return string_object;
}

HeapInfo_TypeDef *mVM::CreateThrowableObject(const char *msg) {
    HeapInfo_TypeDef *throwable_object = Heap_Malloc(4, 1, throwable_class_index + 12);
    if(throwable_object) {
        HeapInfo_TypeDef *string_object = this->CreateStringObject(msg);
        if(string_object) {
            ((uint32_t *)throwable_object->Address)[0] = (uint32_t)string_object;
            Heap_MemSetRefer((uint32_t *)throwable_object->Address, 1);
            throwable_object->Attribute = 2;
        }
        else {
            Heap_Free(throwable_object);
            return 0;
        }
    }
    return throwable_object;
}

Stack *mVM::GetStack() {
    return &this->stack;
}

HeapInfo_TypeDef *mVM::GetStaticFields() {
    return static_fields;
}
