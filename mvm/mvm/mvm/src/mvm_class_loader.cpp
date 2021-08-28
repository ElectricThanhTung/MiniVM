
#include "mvm_class_loader.h"
#include "mvm.h"
#include "mvm_heap.h"
#include "mvm_sync.h"
#include "mvm_opcodes.h"
#include "mvm_native_api.h"
#include "mvm_data_reader.h"
#include "mvm_error_report.h"

#define F_NaN             0x7FC00000
#define D_NaN             0x7FF8000000000000

static int32_t __strCmp(ConstUTF8 &str1, ConstUTF8 &str2) {
    uint16_t len1 = str1.GetLength();
    uint16_t len2 = str2.GetLength();
    if(len1 != len2)
        return len1 - len2;
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
    uint16_t len1 = str1.GetLength();
    if(len1 != length)
        return len1 - length;
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

static int32_t _strCmp2(const char *str1, const char *str2, uint16_t length) {
    while(length) {
        if(*str1 != *str2)
            return *str1 - *str2;
        str1++;
        str2++;
        length--;
    }
    return 0;
}

static int32_t __strIndexOf(const char *str, const int8_t ch, uint16_t str_length) {
    int32_t index = 0;
    while(str[index] && str_length) {
        if(str[index] == ch)
            return index;
        index++;
        str_length--;
    }
    return -1;
}

ClassLoader::ClassLoader() {
    mvm_ptr = 0;
    data_ptr = 0;
}

ClassLoader::ClassLoader(void *mvm, uint8_t *data) {
    mvm_ptr = (uint8_t *)mvm;
    data_ptr = data;
}

uint32_t ClassLoader::Magic() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[0];
    return ReadUInt32(ptr);
}

uint16_t ClassLoader::MinorVersion() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[0];
    return ReadUInt16(&ptr[4]);
}

uint16_t ClassLoader::MajorVersion() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[0];
    return ReadUInt16(&ptr[6]);
}

uint16_t ClassLoader::ConstPoolCount() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[0];
    return ReadUInt16(&ptr[8]);
}

ConstPoolType ClassLoader::GetConstPool(uint16_t index) {
    uint16_t pool_count = ConstPoolCount();
    index--;
    if(index >= pool_count)
        Error("Pool index out of range");
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[5 + index];
    uint8_t *ret[2] = {data_ptr, ptr};
    return *(ConstPoolType *)ret;
}

ClassAccessFlag ClassLoader::GetAccessFlag() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[1];
    return (ClassAccessFlag)ReadUInt16(ptr);
}

ConstClass ClassLoader::GetThisClass() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[1];
    ConstPoolType ret = this->GetConstPool(ReadUInt16(&ptr[2]));
    return *(ConstClass *)&ret;
}

ConstClass ClassLoader::GetSupperClass() {
    static const int8_t *null[2] = {0, 0};
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[1];
    uint16_t index = ReadUInt16(&ptr[4]);
    if(index == 0)
        return *(ConstClass *)null;
    ConstPoolType ret = this->GetConstPool(index);
    return *(ConstClass *)&ret;
}

uint16_t ClassLoader::GetInterfaceCount() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[1];
    return ReadUInt16(&ptr[6]);
}

ConstClass ClassLoader::GetInterface(uint16_t index) {
    if(index >= this->GetInterfaceCount())
        Error("Interface index out of range");
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[1];
    ConstPoolType ret = this->GetConstPool(ReadUInt16(&ptr[8 + index * 2]));
    return *(ConstClass *)&ret;
}

uint16_t ClassLoader::GetFieldsCount() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[2];
    return ReadUInt16(ptr);
}

FieldInfo ClassLoader::GetField(uint16_t index) {
    if(index >= GetFieldsCount())
        Error("Field index out of range");
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[2];
    uint8_t *field_struct[2] = {data_ptr, &ptr[2]};

    // This case used when all attribute count are always equal to 0
    field_struct[1] += 8 * index;

    // This case used when any attribute count aren't orther than 0
    /*
    uint8_t *attribute_struct[2] = {data_ptr, &field_struct[1][8]};
    while(index--) {
        uint16_t attribute_count = ((FieldInfo *)field_struct)->GetAttributesCount();
        uint32_t size = 0;
        for(uint16_t i = 0; i < attribute_count; i++) {
            uint32_t size_of_attribute = 6 + ((AttributeType *)attribute_struct)->GetAttributeLength();
            size += size_of_attribute;
            attribute_struct[1] += size_of_attribute;
        }
        field_struct[1] += 8 + size;
        attribute_struct[1] = &field_struct[1][8];
    }
    */

    return *(FieldInfo *)field_struct;
}

uint32_t ClassLoader::GetFieldsSize() {
    ConstClass super_class = this->GetSupperClass();
    uint32_t size = 0;
    if(super_class.GetClassData() != 0) {
        ConstUTF8 super_name = super_class.GetName();
        uint8_t *super_class_data = ((mVM *)mvm_ptr)->FindClassFile(&super_name);
        if(!super_class_data)
            Error("Can't found class");
        ClassLoader class_loader = ClassLoader(mvm_ptr, super_class_data);
        size = class_loader.GetFieldsSize();
    }
    uint16_t fields_count = GetFieldsCount();
    for(uint16_t i = 0; i < fields_count; i++) {
        FieldInfo field_info = GetField(i);
        if((field_info.GetAccessFlag() & FIELD_STATIC) == 0) {
            uint8_t field_type = field_info.GetDescriptor().GetString()[0];
            size += ((field_type == 'J') || (field_type == 'D')) ? 8 : 4;
        }
    }
    return size;
}

int32_t ClassLoader::GetFieldOffset(ConstUTF8 &field_name) {
    ConstClass super_class = this->GetSupperClass();
    int32_t index = 0;
    if(super_class.GetClassData() != 0) {
        ConstUTF8 super_name = super_class.GetName();
        uint8_t *super_class_data = ((mVM *)mvm_ptr)->FindClassFile(&super_name);
        if(!super_class_data)
            Error("Can't found class");
        ClassLoader class_loader = ClassLoader(mvm_ptr, super_class_data);
        index = class_loader.GetFieldOffset(field_name);
        if(index < 0)
            index = class_loader.GetFieldsSize();
        else
            return index;
    }
    uint16_t fields_count = GetFieldsCount();
    for(uint16_t i = 0; i < fields_count; i++) {
        FieldInfo field_info = GetField(i);
        if((field_info.GetAccessFlag() & FIELD_STATIC) == 0) {
            ConstUTF8 field_name_2 = field_info.GetName();
            if(__strCmp(field_name, field_name_2) == 0)
                return index;
            uint8_t field_type = field_info.GetDescriptor().GetString()[0];
            index += ((field_type == 'J') || (field_type == 'D')) ? 8 : 4;
        }
    }
    return -1;
}

uint16_t ClassLoader::GetMethodsCount() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[3];
    return ReadUInt16(ptr);
}

MethodInfo ClassLoader::GetMethod(uint16_t index) {
    if(index >= GetMethodsCount())
        Error("Method Index Out of Range");
    uint16_t const_pool_count = ConstPoolCount();
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[5 + const_pool_count + index];
    uint8_t *method_struct[2] = {data_ptr, ptr};
    return *(MethodInfo *)method_struct;
}

uint16_t ClassLoader::GetAttributeCount() {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[4];
    return ReadUInt16(ptr);
}

AttributeType ClassLoader::GetAttribute(uint16_t index) {
    uint8_t *ptr = data_ptr + ((uint32_t *)data_ptr)[4];
    uint8_t *temp[2] = {data_ptr, &ptr[2]};
    while(index--)
        temp[1] += ((AttributeType *)temp)->GetAttributeLength() + 6;
    return *(AttributeType *)temp;
}

MethodInfo ClassLoader::FindMethod(ConstNameAndType &name) {
    const char *method_name = name.GetName().GetString();
    const char *method_desc = name.GetDescriptor().GetString();
    uint16_t method_name_len = name.GetName().GetLength();
    uint16_t method_desc_len = name.GetDescriptor().GetLength();

    int32_t r = this->GetMethodsCount() - 1;
    int32_t l = 0;
    while(r >= l) {
        uint32_t mid = l + (r - l) / 2;
        MethodInfo method = this->GetMethod(mid);
        ConstUTF8 method_name_2 = method.GetName();
        int32_t cmp_value = method_name_len - method_name_2.GetLength();
        if(cmp_value == 0) {
            ConstUTF8 method_desc_2 = method.GetDescriptor();
            cmp_value = method_desc_len - method_desc_2.GetLength();
            if(cmp_value == 0) {
                cmp_value = _strCmp2(method_name, method_name_2.GetString(), method_name_len);
                if(cmp_value == 0) {
                    cmp_value = _strCmp2(method_desc, method_desc_2.GetString(), method_desc_len);
                    if(cmp_value == 0)
                        return method;
                }
            }
        }
        if(cmp_value < 0)
            r = mid - 1;
        else
            l = mid + 1;
    }
    static const int8_t *null[2] = {0, 0};
    return *(MethodInfo *)null;
}

MethodInfo ClassLoader::FindMethod(const char *name, uint16_t name_len, const char *desc, uint16_t desc_len) {
    int32_t r = this->GetMethodsCount() - 1;
    int32_t l = 0;
    while(r >= l) {
        uint32_t mid = l + (r - l) / 2;
        MethodInfo method = this->GetMethod(mid);
        ConstUTF8 method_name_2 = method.GetName();
        int32_t cmp_value = name_len - method_name_2.GetLength();
        if(cmp_value == 0) {
            ConstUTF8 method_desc_2 = method.GetDescriptor();
            cmp_value = desc_len - method_desc_2.GetLength();
            if(cmp_value == 0) {
                cmp_value = _strCmp2(name, method_name_2.GetString(), name_len);
                if(cmp_value == 0) {
                    cmp_value = _strCmp2(desc, method_desc_2.GetString(), desc_len);
                    if(cmp_value == 0)
                        return method;
                }
            }
        }
        if(cmp_value < 0)
            r = mid - 1;
        else
            l = mid + 1;
    }
    static const int8_t *null[2] = {0, 0};
    return *(MethodInfo *)null;
}

static int32_t GetExceptionPC(uint8_t *ex_table_info, uint16_t pc_offset) {
    uint16_t ex_count = ReadUInt16(ex_table_info);
    ex_table_info += 2;
    for(uint16_t i = 0; i < ex_count; i++) {
        uint16_t start_pc = ReadUInt16(ex_table_info);
        uint16_t end_pc = ReadUInt16(&ex_table_info[2]);
        if((pc_offset >= start_pc) && (pc_offset < end_pc))
            return ReadUInt16(&ex_table_info[4]);
        ex_table_info += 8;
    }
    return -1;
}

static uint8_t RunNativeMethod(mVM *mvm, Stack &stack, MethodInfo &method, uint32_t n_arg, uint64_t &return_value) {
    ConstUTF8 class_name = ClassLoader(0, method.GetClassData()).GetThisClass().GetName();
    ConstUTF8 name = method.GetName();
    ConstUTF8 desc = method.GetDescriptor();
    for(uint32_t i = 0; i < SystemClassList.ClassCount; i++) {
        if(__strCmp(class_name, SystemClassList.ClassList[i]->ClassName, SystemClassList.ClassList[i]->ClassNameLength) == 0) {
            for(uint32_t j = 0; j < SystemClassList.ClassList[i]->MethodCount; j++) {
                const SystemMethodInfo *sys_method = &SystemClassList.ClassList[i]->MethodList[j];
                if((__strCmp(name, sys_method->Name, sys_method->NameLength) == 0) &&
                   (__strCmp(desc, sys_method->Descriptor, sys_method->DescLength) == 0))
                    return SystemClassList.ClassList[i]->MethodList[j].Method(mvm, stack, n_arg, return_value);
            }
            break;
        }
    }
    Error("Can't found native method");
    return 0;
}

static uint32_t GetArgInfo(const char *desc_str, uint8_t *ret_type) {
    uint32_t n_arg = 0;
    uint32_t index = 1;
    uint8_t is_object = 0;
    uint8_t chr = desc_str[index];
    while(chr != ')') {
        if(!is_object) {
            if(chr == 'L') {
                is_object = 1;
                n_arg++;
            }
            else if(chr == '[') {
                n_arg++;
                index++;
                chr = desc_str[index];
                while(chr == '[') {
                    index++;
                    chr = desc_str[index];
                }
                if(chr == 'L')
                    is_object = 1;
                else {
                    index++;
                    chr = desc_str[index];
                }
                continue;
            }
            else
                n_arg += (chr == 'J' || (chr == 'D')) ? 2 : 1;
        }
        else if(chr == ';')
            is_object = 0;
        index++;
        chr = desc_str[index];
    }
    *ret_type = desc_str[index + 1];
    return n_arg;
}

static uint8_t CreateMultiArray(HeapInfo_TypeDef *heap_root, int32_t *counts, uint8_t dimensions, uint32_t type, uint32_t type_size) {
    for(int32_t i = 0; i < counts[0]; i++) {
        HeapInfo_TypeDef *ptr = (HeapInfo_TypeDef *)((uint32_t *)heap_root->Address)[i];
        if(ptr && dimensions) {
            uint32_t dimensions_count = (type >> 16) - 1;
            if(!CreateMultiArray(ptr, counts + 1, dimensions - 1, (dimensions_count << 16) | (type & 0xFFFF), type_size))
                return 0;
        }
        else {
            HeapInfo_TypeDef *heap_info;
            if(dimensions == 0) {
                if((type >> 16))
                    heap_info = Heap_Calloc(counts[1] * type_size, 2, type);
                else if(type > 11)
                    heap_info = Heap_Calloc(type_size, 2, type);
                else
                    heap_info = Heap_Calloc(counts[1] * type_size, 0, type);
            }
            else
                heap_info = Heap_Calloc(counts[1] * 4, 2, type);

            if(!heap_info)
                return 0;
            uint32_t *ptr = &((uint32_t *)heap_root->Address)[i];
            *ptr = (uint32_t)heap_info;
            Heap_MemSetRefer(ptr, 1);
        }
    }
    return 1;
}

uint8_t ClassLoader::RunMethod(Stack &stack, MethodInfo &method, uint32_t n_arg, uint64_t &return_value) {
    int32_t sp_start = stack.GetSP() - n_arg;
    uint32_t stack_min_size_old = stack.MinSize;
    HeapInfo_TypeDef *exception_object = 0;
    uint8_t *pc = 0;
    uint8_t *pc_start = 0;
    uint8_t *pc_end = 0;
    int32_t locals_index = sp_start + 1;

    uint16_t attribute_count = method.GetAttributesCount();
    for(uint16_t i = 0; i < attribute_count; i++) {
        AttributeType attribute = method.GetAttributes(i);
        ConstUTF8 attribute_name = attribute.GetName();
        if(attribute_name.GetLength() != 4)
            continue;
        if(__strCmp(attribute_name, "Code", 4) == 0) {
            pc = ((CodeAttribute *)&attribute)->GetCode();
            pc_start = pc;
            pc_end = pc + ((CodeAttribute *)&attribute)->GetCodeLength();
            uint32_t locals_count = ((CodeAttribute *)&attribute)->GetMaxLocals();
            uint32_t stack_count= ((CodeAttribute *)&attribute)->GetMaxStack();
            uint32_t stack_size = (locals_count + stack_count) * 4;
            if((stack_size) > stack.GetFreeSpace()) {
                uint32_t new_size = (stack_size - stack.GetFreeSpace()) + stack.GetSize();
                stack.MinSize = (new_size / 16) * 16;
                if(new_size % 16)
                    stack.MinSize += 16;
                stack.Realloc(stack.MinSize);
            }
            stack.PushZero(locals_count - n_arg);
            break;
        }
    }

    if(!pc) {
        Error("Can't fonnd code");
        return 0;
    }

    while(pc < pc_end) {
        OpCode_TypeDef opcode = (OpCode_TypeDef)*(pc++);
        switch(opcode) {
            case OP_NOP:
                break;
            case OP_ICONST_M1:
                stack.Push((uint32_t)-1);
                break;
            case OP_ACONST_NULL:
            case OP_ICONST_0:
                stack.Push((uint32_t)0);
                break;
            case OP_ICONST_1:
                stack.Push((uint32_t)1);
                break;
            case OP_ICONST_2:
                stack.Push((uint32_t)2);
                break;
            case OP_ICONST_3:
                stack.Push((uint32_t)3);
                break;
            case OP_ICONST_4:
                stack.Push((uint32_t)4);
                break;
            case OP_ICONST_5:
                stack.Push((uint32_t)5);
                break;
            case OP_LCONST_0:
                stack.Push((uint64_t)0);
                break;
            case OP_LCONST_1:
                stack.Push((uint64_t)1);
                break;
            case OP_FCONST_0:
                stack.Push((float)0);
                break;
            case OP_FCONST_1:
                stack.Push((float)1);
                break;
            case OP_FCONST_2:
                stack.Push((float)2);
                break;
            case OP_DCONST_0:
                stack.Push((double)0);
                break;
            case OP_DCONST_1:
                stack.Push((double)1);
                break;

            case OP_BIPUSH:
                stack.Push((uint32_t)(int8_t)*(pc++));
                break;
            case OP_SIPUSH:
                stack.Push((uint32_t)(int16_t)ReadUInt16(pc));
                pc += 2;
                break;

            case OP_LDC:
            case OP_LDC_W: {
                uint16_t index;
                if(opcode == OP_LDC)
                    index = *(pc++);
                else {
                    index = ReadUInt16(pc);
                    pc += 2;
                }
                ConstPoolType pool = GetConstPool(index);
                ConstPoolTag tag = pool.GetPoolTag();
                if(tag == CONST_STRING) {
                    ConstUTF8 const_utf8 = ((ConstString *)&pool)->GetUTF8();
                    HeapInfo_TypeDef *heap_info = ((mVM *)mvm_ptr)->CreateStringObject(const_utf8.GetString(), const_utf8.GetLength());
                    if(!heap_info) {
                        Error("Can't Create java.lang.String object");
                        goto exit;
                    }
                    stack.Push((void *)heap_info);
                    break;
                }
                else if(tag == CONST_INTEGER)
                    stack.Push((uint32_t)((ConstInteger *)&pool)->GetValue());
                else if(tag == CONST_FLOAT)
                    stack.Push(((ConstFloat *)&pool)->GetValue());
                else if(tag == CONST_CLASS) {
                    // TODO
                }
                else if(tag == CONST_METHOD_TYPE) {
                    // TODO
                }
                else if(tag == CONST_METHOD_HANDLE) {
                    // TODO
                }
                break;
            }

            case OP_LDC2_W: {
                uint16_t index = ReadUInt16(pc);
                pc += 2;
                ConstPoolType pool = GetConstPool(index);
                ConstPoolTag tag = pool.GetPoolTag();
                if(tag == CONST_DOUBLE)
                    stack.Push(((ConstDouble *)&pool)->GetValue());
                else if(tag == CONST_LONG)
                    stack.Push((uint64_t)((ConstLong *)&pool)->GetValue());
                break;
            }

            case OP_ALOAD:
                stack.Push((void *)stack.ValueUInt32(locals_index + *(pc++)));
                break;
            case OP_ALOAD_0:
                stack.Push((void *)stack.ValueUInt32(locals_index));
                break;
            case OP_ALOAD_1:
                stack.Push((void *)stack.ValueUInt32(locals_index + 1));
                break;
            case OP_ALOAD_2:
                stack.Push((void *)stack.ValueUInt32(locals_index + 2));
                break;
            case OP_ALOAD_3:
                stack.Push((void *)stack.ValueUInt32(locals_index + 3));
                break;
            case OP_ILOAD:
            case OP_FLOAD:
                stack.Push((uint32_t)stack.ValueUInt32(locals_index + *(pc++)));
                break;
            case OP_ILOAD_0:
            case OP_FLOAD_0:
                stack.Push((uint32_t)stack.ValueUInt32(locals_index));
                break;
            case OP_ILOAD_1:
            case OP_FLOAD_1:
                stack.Push((uint32_t)stack.ValueUInt32(locals_index + 1));
                break;
            case OP_ILOAD_2:
            case OP_FLOAD_2:
                stack.Push((uint32_t)stack.ValueUInt32(locals_index + 2));
                break;
            case OP_ILOAD_3:
            case OP_FLOAD_3:
                stack.Push((uint32_t)stack.ValueUInt32(locals_index + 3));
                break;

            case OP_LLOAD:
            case OP_DLOAD:
                stack.Push(stack.ValueUInt64(locals_index + *(pc++)));
                break;
            case OP_LLOAD_0:
            case OP_DLOAD_0:
                stack.Push(stack.ValueUInt64(locals_index));
                break;
            case OP_LLOAD_1:
            case OP_DLOAD_1:
                stack.Push(stack.ValueUInt64(locals_index + 1));
                break;
            case OP_LLOAD_2:
            case OP_DLOAD_2:
                stack.Push(stack.ValueUInt64(locals_index + 2));
                break;
            case OP_LLOAD_3:
            case OP_DLOAD_3:
                stack.Push(stack.ValueUInt64(locals_index + 3));
                break;

            case OP_CALOAD:
            case OP_SALOAD: {
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't load from null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 2))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                stack.Push((uint32_t)((int16_t *)arrayref->Address)[index]);
                break;
            }
            case OP_AALOAD: {
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't load from null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 4))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                stack.Push((void *)(((int32_t *)arrayref->Address)[index]));
                break;
            }
            case OP_IALOAD:
            case OP_FALOAD: {
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't load from null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 4))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                stack.Push((uint32_t)(((int32_t *)arrayref->Address)[index]));
                break;
            }
            case OP_LALOAD:
            case OP_DALOAD: {
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't load from null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 8))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                stack.Push((uint64_t)((int32_t *)arrayref->Address)[index]);
                break;
            }
            case OP_BALOAD: {
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't load from null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)arrayref->Size)) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                stack.Push((uint32_t)((int8_t *)arrayref->Address)[index]);
                break;
            }

            case OP_ASTORE: {
                uint32_t *ptr = &stack.ValueUInt32(locals_index + *(pc++));
                *ptr = stack.PopUInt32();
                Heap_MemSetRefer(ptr, 1);
                break;
            }
            case OP_ASTORE_0: {
                uint32_t *ptr = &stack.ValueUInt32(locals_index);
                *ptr = stack.PopUInt32();
                Heap_MemSetRefer(ptr, 1);
                break;
            }
            case OP_ASTORE_1: {
                uint32_t *ptr = &stack.ValueUInt32(locals_index + 1);
                *ptr = stack.PopUInt32();
                Heap_MemSetRefer(ptr, 1);
                break;
            }
            case OP_ASTORE_2: {
                uint32_t *ptr = &stack.ValueUInt32(locals_index + 2);
                *ptr = stack.PopUInt32();
                Heap_MemSetRefer(ptr, 1);
                break;
            }
            case OP_ASTORE_3: {
                uint32_t *ptr = &stack.ValueUInt32(locals_index + 3);
                *ptr = stack.PopUInt32();
                Heap_MemSetRefer(ptr, 1);
                break;
            }
            case OP_ISTORE:
            case OP_FSTORE:
                stack.ValueUInt32(locals_index + *(pc++)) = stack.PopUInt32();
                break;
            case OP_ISTORE_0:
            case OP_FSTORE_0:
                stack.ValueUInt32(locals_index) = stack.PopUInt32();
                break;
            case OP_ISTORE_1:
            case OP_FSTORE_1:
                stack.ValueUInt32(locals_index + 1) = stack.PopUInt32();
                break;
            case OP_ISTORE_2:
            case OP_FSTORE_2:
                stack.ValueUInt32(locals_index + 2) = stack.PopUInt32();
                break;
            case OP_ISTORE_3:
            case OP_FSTORE_3:
                stack.ValueUInt32(locals_index + 3) = stack.PopUInt32();
                break;

            case OP_LSTORE:
            case OP_DSTORE:
                stack.ValueUInt64(locals_index + *(pc++)) = stack.PopUInt64();
                break;
            case OP_LSTORE_0:
            case OP_DSTORE_0:
                stack.ValueUInt64(locals_index) = stack.PopUInt64();
                break;
            case OP_LSTORE_1:
            case OP_DSTORE_1:
                stack.ValueUInt64(locals_index + 1) = stack.PopUInt64();
                break;
            case OP_LSTORE_2:
            case OP_DSTORE_2:
                stack.ValueUInt64(locals_index + 2) = stack.PopUInt64();
                break;
            case OP_LSTORE_3:
            case OP_DSTORE_3:
                stack.ValueUInt64(locals_index + 3) = stack.PopUInt64();
                break;

            case OP_AASTORE: {
                int32_t value = stack.PopUInt32();
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't store to null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 4))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                ((uint32_t *)arrayref->Address)[index] = value;
                Heap_MemSetRefer((uint32_t *)&arrayref[index], 1);
                break;
            }
            case OP_IASTORE:
            case OP_FASTORE: {
                int32_t value = stack.PopUInt32();
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't store to null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 4))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                ((int32_t *)arrayref->Address)[index] = value;
                break;
            }
            case OP_DASTORE:
            case OP_LASTORE: {
                int64_t value = stack.PopUInt64();
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't store to null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 8))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                ((int64_t *)arrayref->Address)[index] = value;
                break;
            }
            case OP_BASTORE: {
                int32_t value = stack.PopUInt32();
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't store to null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)arrayref->Size)) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                ((int8_t *)arrayref->Address)[index] = (int8_t)value;
                break;
            }
            case OP_CASTORE:
            case OP_SASTORE: {
                int32_t value = stack.PopUInt32();
                int32_t index = stack.PopUInt32();
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't store to null array");
                    goto exception_handler;
                }
                if((index < 0) || (index >= (int32_t)(arrayref->Size / 2))) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Index out of bounds for length");
                    goto exception_handler;
                }
                ((int16_t *)arrayref->Address)[index] = (int16_t)value;
                break;
            }

            case OP_POP:
                stack.PopUInt32();
                break;
            case OP_POP2:
                stack.PopUInt64();
                break;
            case OP_DUP:
                stack.Dup();
                break;
            case OP_DUP_X1: {
                stack.DupX1();
                break;
            }
            case OP_DUP_X2: {
                stack.DupX2();
                break;
            }
            case OP_DUP2:
                stack.Dup2();
                break;
            case OP_DUP2_X1: {
                stack.Dup2X1();
                break;
            }
            case OP_DUP2_X2: {
                stack.Dup2X2();
                break;
            }

            case OP_IADD: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 + value2));
                break;
            }
            case OP_LADD: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 + value2));
                break;
            }
            case OP_FADD: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                stack.Push((float)(value1 + value2));
                break;
            }
            case OP_DADD: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                stack.Push((double)(value1 + value2));
                break;
            }
            case OP_ISUB: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 - value2));
                break;
            }
            case OP_LSUB: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 - value2));
                break;
            }
            case OP_FSUB: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                stack.Push((float)(value1 - value2));
                break;
            }
            case OP_DSUB: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                stack.Push((double)(value1 - value2));
                break;
            }
            case OP_IMUL: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 * value2));
                break;
            }
            case OP_LMUL: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 * value2));
                break;
            }
            case OP_FMUL: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                stack.Push((float)(value1 * value2));
                break;
            }
            case OP_DMUL: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                stack.Push((double)(value1 * value2));
                break;
            }
            case OP_IDIV: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                if(value2 == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("divide by zero");
                    goto exception_handler;
                }
                stack.Push((uint32_t)(value1 / value2));
                break;
            }
            case OP_LDIV: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                if(value2 == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("divide by zero");
                    goto exception_handler;
                }
                stack.Push((uint64_t)(value1 / value2));
                break;
            }
            case OP_FDIV: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                stack.Push((float)(value1 / value2));
                break;
            }
            case OP_DDIV: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                stack.Push((double)(value1 / value2));
                break;
            }
            case OP_IREM: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                if(value2 == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("divide by zero");
                    goto exception_handler;
                }
                stack.Push((uint32_t)(value1 % value2));
                break;
            }
            case OP_LREM: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                if(value2 == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("divide by zero");
                    goto exception_handler;
                }
                stack.Push((uint64_t)(value1 % value2));
                break;
            }
            case OP_FREM: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                int32_t temp = (int32_t)(value1 / value2);
                stack.Push((float)(value1 - (temp * value2)));
                break;
            }
            case OP_DREM: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                int64_t temp = (int64_t)(value1 / value2);
                stack.Push((double)(value1 - (temp * value2)));
                break;
            }
            case OP_INEG: {
                int32_t value = stack.PopUInt32();
                stack.Push((uint32_t)-value);
                break;
            }
            case OP_LNEG: {
                int64_t value = stack.PopUInt64();
                stack.Push((uint64_t)-value);
                break;
            }
            case OP_FNEG:
                stack.Push((float)-stack.PopFloat());
                break;
            case OP_DNEG: {
                stack.Push((double)-stack.PopDouble());
                break;
            }
            case OP_ISHL: {
                int32_t position = stack.PopUInt32();
                int32_t value = stack.PopUInt32();
                stack.Push((uint32_t)(value << position));
                break;
            }
            case OP_LSHL: {
                int32_t position = stack.PopUInt32();
                int64_t value = stack.PopUInt64();
                stack.Push((uint64_t)(value << position));
                break;
            }
            case OP_ISHR: {
                int32_t position = stack.PopUInt32();
                int32_t value = stack.PopUInt32();
                stack.Push((uint32_t)(value >> position));
                break;
            }
            case OP_LSHR: {
                int32_t position = stack.PopUInt32();
                int64_t value = stack.PopUInt64();
                stack.Push((uint64_t)(value >> position));
                break;
            }
            case OP_IUSHR: {
                int32_t position = stack.PopUInt32();
                uint32_t value = stack.PopUInt32();
                stack.Push((uint32_t)(value >> position));
                break;
            }
            case OP_LUSHR: {
                int32_t position = stack.PopUInt32();
                uint64_t value = stack.PopUInt64();
                stack.Push((uint64_t)(value >> position));
                break;
            }
            case OP_IAND: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 & value2));
                break;
            }
            case OP_LAND: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 & value2));
                break;
            }
            case OP_IOR: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 | value2));
                break;
            }
            case OP_LOR: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 | value2));
                break;
            }
            case OP_IXOR: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                stack.Push((uint32_t)(value1 ^ value2));
                break;
            }
            case OP_LXOR: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint64_t)(value1 ^ value2));
                break;
            }
            case OP_IINC: {
                uint8_t index = *(pc++);
                int8_t const_value = (int8_t)*(pc++);
                stack.ValueUInt32(locals_index + index) += const_value;
                break;
            }

            case OP_I2L:
                stack.Push((uint64_t)(int32_t)stack.PopUInt32());
                break;
            case OP_I2F:
                stack.Push((float)(int32_t)stack.PopUInt32());
                break;
            case OP_I2D:
                stack.Push((double)(int32_t)stack.PopUInt32());
                break;
            case OP_L2I:
                stack.Push((uint32_t)(int64_t)stack.PopUInt64());
                break;
            case OP_L2F:
                stack.Push((float)(int64_t)stack.PopUInt64());
                break;
            case OP_L2D: {
                stack.Push((double)(int64_t)stack.PopUInt64());
                break;
            }
            case OP_F2I:
                stack.Push((uint32_t)stack.PopFloat());
                break;
            case OP_F2L:
                stack.Push((uint64_t)stack.PopFloat());
                break;
            case OP_F2D:
                stack.Push((double)stack.PopFloat());
                break;
            case OP_D2I:
                stack.Push((uint32_t)stack.PopDouble());
                break;
            case OP_D2L:
                stack.Push((uint64_t)stack.PopDouble());
                break;
            case OP_D2F:
                stack.Push((float)stack.PopDouble());
                break;
            case OP_I2B:
                stack.Push((uint32_t)(int8_t)stack.PopUInt32());
                break;
            case OP_I2S:
            case OP_I2C:
                stack.Push((uint32_t)(int16_t)stack.PopUInt32());
                break;

            case OP_LCMP: {
                int64_t value2 = stack.PopUInt64();
                int64_t value1 = stack.PopUInt64();
                stack.Push((uint32_t)((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 0)));
                break;
            }
            case OP_FCMPL:
            case OP_FCMPG: {
                float value2 = stack.PopFloat();
                float value1 = stack.PopFloat();
                if((*(uint32_t *)&value1 == F_NaN) || (*(uint32_t *)&value2 == F_NaN))
                    stack.Push((uint32_t)((opcode == OP_FCMPL) ? -1 : 1));
                else if(value1 > value2)
                    stack.Push((uint32_t)1);
                else if(value1 == value2)
                    stack.Push((uint32_t)0);
                else
                    stack.Push((uint32_t)-1);
                break;
            }
            case OP_DCMPL:
            case OP_DCMPG: {
                double value2 = stack.PopDouble();
                double value1 = stack.PopDouble();
                if((*(uint64_t *)&value1 == D_NaN) || (*(uint64_t *)&value2 == D_NaN))
                    stack.Push((uint32_t)((opcode == OP_DCMPL) ? -1 : 1));
                else if(value1 > value2)
                    stack.Push((uint32_t)1);
                else if(value1 == value2)
                    stack.Push((uint32_t)0);
                else
                    stack.Push((uint32_t)-1);
                break;
            }

            case OP_IFNULL:
            case OP_IFEQ:
                pc += (!stack.PopUInt32()) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IFNONNULL:
            case OP_IFNE:
                pc += stack.PopUInt32() ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IFLT:
                pc += ((int32_t)stack.PopUInt32() < 0) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IFLE:
                pc += ((int32_t)stack.PopUInt32() <= 0) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IFGT:
                pc += ((int32_t)stack.PopUInt32() > 0) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IFGE:
                pc += ((int32_t)stack.PopUInt32() >= 0) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            case OP_IF_ACMPEQ:
            case OP_IF_ICMPEQ: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 == value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }
            case OP_IF_ACMPNE:
            case OP_IF_ICMPNE: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 != value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }
            case OP_IF_ICMPLT: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 < value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }
            case OP_IF_ICMPGE: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 >= value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }
            case OP_IF_ICMPGT: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 > value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }
            case OP_IF_ICMPLE: {
                int32_t value2 = stack.PopUInt32();
                int32_t value1 = stack.PopUInt32();
                pc += (value1 <= value2) ? ((int16_t)ReadUInt16(pc) - 1) : 2;
                break;
            }

            case OP_GOTO:
                pc += (int16_t)ReadUInt16(pc) - 1;
                break;
            case OP_GOTO_W:
                pc += (int32_t)ReadUInt32(pc) - 1;
                break;

            case OP_JSR:
                stack.Push((uint32_t)(pc + 2));
                pc += (int16_t)ReadUInt16(pc);
                break;
            case OP_JSRW:
                stack.Push((uint32_t)(pc + 2));
                pc += (int32_t)ReadUInt32(pc);
                break;
            case OP_RET:
                pc = &pc_start[stack.ValueUInt32(locals_index + *pc)];
                break;
            case OP_TABLESWITCH: {
                int32_t index = stack.PopUInt32();
                uint8_t padding = (4 - ((pc - pc_start) % 4)) % 4;
                int32_t low = (int32_t)ReadUInt32(pc + padding + 4);
                if(index < low) {
                    pc += (int32_t)ReadUInt32(pc + padding) - 1;
                    break;
                }
                if(index > (int32_t)ReadUInt32(pc + padding + 8)) {
                    pc += (int32_t)ReadUInt32(pc + padding) - 1;
                    break;
                }
                pc += (int32_t)ReadUInt32(pc + padding + 12 + 4 * (index - low)) - 1;
                break;
            }
            case OP_LOOKUPSWITCH: {
                int32_t key = stack.PopUInt32();
                uint8_t padding = (4 - ((pc - pc_start) % 4)) % 4;
                uint8_t *start_pc = pc;
                pc += padding + 4;
                int32_t npairs = (int32_t)ReadUInt32(pc);
                pc += 4;
                while(npairs--) {
                    int32_t pairs = (int32_t)ReadUInt32(pc);
                    pc += 4;
                    if(key == pairs) {
                        pc = start_pc + (int32_t)ReadUInt32(pc) - 1;
                        goto found;
                    }
                    pc += 4;
                }
                pc = start_pc + (int32_t)ReadUInt32(start_pc + padding) - 1;
            found:
                break;
            }
            case OP_ARETURN:
            case OP_IRETURN:
            case OP_FRETURN:
                return_value = stack.PopUInt32();
                goto exit;
            case OP_LRETURN:
            case OP_DRETURN:
                return_value = stack.PopUInt64();
                goto exit;
            case OP_RETURN:
                goto exit;

            case OP_GETSTATIC: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                char field_type = ((ConstField *)&const_pool)->GetNameAndType().GetDescriptor().GetString()[0];
                ConstUTF8 name = ((ConstField *)&const_pool)->GetNameAndType().GetName();
                if((field_type == 'J') || (field_type == 'D'))
                    stack.Push((uint64_t)((mVM *)mvm_ptr)->GetStaticFieldUInt64(&name));
                else if((field_type == 'L') || (field_type == '['))
                    stack.Push((void *)((mVM *)mvm_ptr)->GetStaticFieldUInt32(&name));
                else
                    stack.Push((uint32_t)((mVM *)mvm_ptr)->GetStaticFieldUInt32(&name));
                break;
            }
            case OP_PUTSTATIC: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                char field_type = ((ConstField *)&const_pool)->GetNameAndType().GetDescriptor().GetString()[0];
                ConstUTF8 name = ((ConstField *)&const_pool)->GetNameAndType().GetName();
                if((field_type == 'J') || (field_type == 'D'))
                    ((mVM *)mvm_ptr)->SetStaticField(&name, (int64_t)stack.PopUInt64());
                else if((field_type == 'L') || (field_type == '['))
                    ((mVM *)mvm_ptr)->SetStaticField(&name, (int32_t)stack.PopUInt32(), 1);
                else
                    ((mVM *)mvm_ptr)->SetStaticField(&name, (int32_t)stack.PopUInt32(), 0);
                break;
            }
            case OP_GETFIELD: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(!objectref) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't access to null object");
                    goto exception_handler;
                }
                uint8_t *class_data = ((mVM *)mvm_ptr)->GetClassFile(objectref->Type - 12);
                if(class_data) {
                    ConstUTF8 field_name = ((ConstField *)&const_pool)->GetNameAndType().GetName();
                    int32_t offset = ClassLoader(mvm_ptr, class_data).GetFieldOffset(field_name);
                    if(offset < 0) {
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found field");
                        goto exception_handler;
                    }
                    char field_type = ((ConstField *)&const_pool)->GetNameAndType().GetDescriptor().GetString()[0];
                    if((field_type == 'J') || (field_type == 'D'))
                        stack.Push((uint64_t)*(uint64_t *)&((uint8_t *)objectref->Address)[offset]);
                    else if((field_type == 'L') || (field_type == '['))
                        stack.Push((void *)*(uint32_t *)&((uint8_t *)objectref->Address)[offset]);
                    else
                        stack.Push((uint32_t)*(uint32_t *)&((uint8_t *)objectref->Address)[offset]);
                    break;
                }
                exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                goto exception_handler;
            }
            case OP_PUTFIELD: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                char field_type = ((ConstField *)&const_pool)->GetNameAndType().GetDescriptor().GetString()[0];
                int64_t value;
                if((field_type == 'J') || (field_type == 'D'))
                    value = stack.PopUInt64();
                else
                    value = stack.PopUInt32();
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(!objectref) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't access to null object");
                    goto exception_handler;
                }
                uint8_t *class_data = ((mVM *)mvm_ptr)->GetClassFile(objectref->Type - 12);
                if(class_data) {
                    ConstUTF8 field_name = ((ConstField *)&const_pool)->GetNameAndType().GetName();
                    int32_t offset = ClassLoader(mvm_ptr, class_data).GetFieldOffset(field_name);
                    if(offset < 0) {
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found fields");
                        goto exception_handler;
                    }
                    if((field_type == 'J') || (field_type == 'D'))
                        *(uint64_t *)&((uint8_t *)objectref->Address)[offset] = (int64_t)value;
                    else if((field_type == 'L') || (field_type == '[')) {
                        uint32_t *ptr = (uint32_t *)&((uint8_t *)objectref->Address)[offset];
                        *ptr = (int32_t)value;
                        Heap_MemSetRefer(ptr, 1);
                    }
                    else
                        *(uint32_t *)&((uint8_t *)objectref->Address)[offset] = (int32_t)value;
                    break;
                }
                exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                goto exception_handler;
            }

            case OP_INVOKEINTERFACE:
            case OP_INVOKEVIRTUAL:
            case OP_INVOKESPECIAL:
            case OP_INVOKESTATIC: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += (opcode == OP_INVOKEINTERFACE) ? 4 : 2;
                ConstNameAndType name_and_type = ((ConstMethod *)&const_pool)->GetNameAndType();
                uint8_t ret_type;
                uint32_t n_arg_2 = GetArgInfo(name_and_type.GetDescriptor().GetString(), &ret_type);
                uint8_t *class_data = 0;
                if(opcode != OP_INVOKESTATIC) {
                    n_arg_2++;
                    int32_t objectref_index = stack.GetSP() - n_arg_2 + 1;
                    HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.ValueUInt32(objectref_index);
                    if(objectref == 0) {
                        stack.SetSP(objectref_index - 1);
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't access to null object");
                        goto exception_handler;
                    }
                    if((name_and_type.GetName().GetString()[0] == '<') || (objectref->Type < 12) || (objectref->Dimensions)) {
                        ConstUTF8 class_name = ((ConstMethod *)&const_pool)->GetConstClass().GetName();
                        class_data = ((mVM *)mvm_ptr)->FindClassFile(&class_name);
                    }
                    else
                        class_data = ((mVM*)mvm_ptr)->GetClassFile(objectref->Type - 12);
                }
                else {
                    ConstUTF8 class_name = ((ConstMethod *)&const_pool)->GetConstClass().GetName();
                    class_data = ((mVM *)mvm_ptr)->FindClassFile(&class_name);
                }

                MethodInfo method_info;
                while(1) {
                    if(class_data == 0) {
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                        goto exception_handler;
                    }
                    ClassLoader class_loader = ClassLoader(mvm_ptr, class_data);
                    method_info = class_loader.FindMethod(name_and_type);
                    if(method_info.GetClassData() == 0) {
                        ConstClass super_class = class_loader.GetSupperClass();
                        if(super_class.GetClassData() == 0) {
                            exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found method");
                            goto exception_handler;
                        }
                        ConstUTF8 class_name = super_class.GetName();
                        class_data = ((mVM *)mvm_ptr)->FindClassFile(&class_name);
                    }
                    else
                        break;
                }

                uint8_t report;
                uint64_t ret_value = 0;
                MethodAccessFlag method_access_flag = method_info.GetAccessFlag();
                if((method_access_flag & METHOD_SYNCHRONIZED) == METHOD_SYNCHRONIZED)
                    Sync_Start();
                if((method_access_flag & METHOD_NATIVE) == METHOD_NATIVE) {
                    report = RunNativeMethod((mVM *)mvm_ptr, stack, method_info, n_arg_2, ret_value);
                    stack.SetSP(stack.GetSP() - n_arg_2);
                }
                else
                    report = ClassLoader(mvm_ptr, method_info.GetClassData()).RunMethod(stack, method_info, n_arg_2, ret_value);
                if((method_access_flag & METHOD_SYNCHRONIZED) == METHOD_SYNCHRONIZED)
                    Sync_Stop();
                if(report) {
                    if((ret_type == 'J') || (ret_type == 'D'))
                        stack.Push((uint64_t)ret_value);
                    else if((ret_type == 'L') || (ret_type == '['))
                        stack.Push((void *)(uint32_t)ret_value);
                    else if(ret_type != 'V')
                        stack.Push((uint32_t)ret_value);
                }
                else {
                    exception_object = (HeapInfo_TypeDef *)(uint32_t)ret_value;
                    goto exception_handler;
                }
                break;
            }

            case OP_NEW: {
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                ConstUTF8 class_name = ((ConstClass *)&const_pool)->GetName();
                int32_t class_file_index = ((mVM *)mvm_ptr)->FindClassFileIndex(&class_name);
                if(class_file_index >= 0) {
                    uint8_t *class_data = ((mVM *)mvm_ptr)->GetClassFile(class_file_index);
                    ClassLoader class_loader = ClassLoader(mvm_ptr, class_data);
                    HeapInfo_TypeDef *heap_info = Heap_Calloc(class_loader.GetFieldsSize(), 2, class_file_index + 12);
                    if(!heap_info) {
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't create object");
                        goto exception_handler;
                    }
                    stack.Push((void *)heap_info);
                    break;
                }
                exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                goto exception_handler;
            }
            case OP_NEWARRAY: {
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
                int32_t count = stack.PopUInt32();
                if(count < 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("The number of elements in the array can't be less than 0");
                    goto exception_handler;
                }
                uint8_t atype = (*(pc++));
                uint8_t type_size = size_of_type[atype - 4];
                HeapInfo_TypeDef *heap_info = Heap_Calloc(type_size * count, 0, (1 << 16) | atype);
                if(!heap_info) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't create array");
                    goto exception_handler;
                }
                stack.Push((void *)heap_info);
                break;
            }
            case OP_ARRAYLENGTH: {
                static const uint8_t size_of_type[9] = {
                    sizeof(int8_t),
                    sizeof(int16_t),
                    sizeof(float),
                    sizeof(double),
                    sizeof(int8_t),
                    sizeof(int16_t),
                    sizeof(int32_t),
                    sizeof(int64_t)
                };
                HeapInfo_TypeDef *arrayref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(arrayref == 0) {
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't access to null array");
                    goto exception_handler;
                }
                int8_t type_size;
                if((arrayref->Dimensions == 1) && (arrayref->Type < 12))
                    type_size = size_of_type[arrayref->Type - 4];
                else
                    type_size = 4;
                int32_t length = arrayref->Size / type_size;
                stack.Push((uint32_t)length);
                break;
            }
            case OP_ANEWARRAY:
            case OP_MULTIANEWARRAY: {
                static const char type_char_list[] = "ZCFDBSIJ";
                ConstPoolType const_pool = GetConstPool(ReadUInt16(pc));
                pc += 2;
                uint8_t dimensions = (opcode == OP_ANEWARRAY) ? 1 : *(pc++);
                int32_t offset = stack.GetSP() - dimensions + 1;

                int32_t *count = (int32_t *)&stack.ValueUInt32(offset);
                if(count[0] < 0) {
                    stack.SetSP(offset - 1);
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("The number of elements in the array can't be less than 0");
                    goto exception_handler;
                }

                ConstUTF8 class_name = ((ConstClass *)&const_pool)->GetName();
                const char *class_name_str = class_name.GetString();
                uint16_t class_name_len = class_name.GetLength();
                uint16_t dimensions_count = 0;
                while(*class_name_str == '[') {
                    class_name_str++;
                    class_name_len--;
                    dimensions_count++;
                }

                uint16_t type = 0;
                uint32_t type_size = 0;

                if(opcode == OP_ANEWARRAY)
                    dimensions_count++;

                if((*class_name_str == 'L') || (opcode == OP_ANEWARRAY && dimensions_count == 1)) {
                    int32_t class_file_index;
                    if(*class_name_str == 'L')
                        class_file_index = ((mVM *)mvm_ptr)->FindClassFileIndex(class_name_str + 1, class_name_len - 2);
                    else
                        class_file_index = ((mVM *)mvm_ptr)->FindClassFileIndex(class_name_str, class_name_len);
                    if(class_file_index < 0) {
                        exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                        goto exception_handler;
                    }
                    uint8_t *class_data = ((mVM *)mvm_ptr)->GetClassFile(class_file_index);
                    type_size = ClassLoader(mvm_ptr, class_data).GetFieldsSize();
                    type = class_file_index + 12;
                }
                else {
                    for(uint8_t i = 0; i < 8; i++) {
                        if(*class_name_str == type_char_list[i]) {
                            type = i + 4;
                            type_size = 1 << (i % 4);
                            break;
                        }
                    }
                }

                HeapInfo_TypeDef *heap_root = Heap_Calloc(count[0] * 4, 1, (dimensions_count << 16) | type);
                dimensions_count--;
                if(!heap_root) {
                    stack.SetSP(offset - 1);
                    exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't create array");
                    goto exception_handler;
                }

                int16_t d = dimensions - 1;
                if(dimensions_count || (((type & 0xFFFF) > 3) && ((type & 0xFFFF) < 12)))
                    d--;
                for(uint8_t i = 0; i < dimensions; i++) {
                    if(d >= 0) {
                        if(!CreateMultiArray(heap_root, count, (uint8_t)d, (dimensions_count << 16) | type, type_size)) {
                            stack.SetSP(offset - 1);
                            exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't create array");
                            goto exception_handler;
                        }
                    }
                }
                heap_root->Attribute = 2;
                stack.SetSP(offset - 1);
                stack.Push((void *)heap_root);
                break;
            }

            case OP_ATHROW:
                exception_object = (HeapInfo_TypeDef *)stack.PopUInt32();
                goto exception_handler;
            case OP_CHECKCAST: {
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(!objectref) {
                    stack.Push((uint32_t)0);
                    pc += 2;
                    break;
                }
                uint16_t index = ReadUInt16(pc);
                pc += 2;
                ConstPoolType const_pool = GetConstPool(index);
                ConstUTF8 class_name = ((ConstClass *)&const_pool)->GetName();
                uint32_t type = objectref->Type;
                if((type > 3) && (type < 12)) {
                    static const char type_list[] = "ZCFDBSIJ";
                    const char *class_name_str = class_name.GetString();
                    uint16_t class_name_len = class_name.GetLength();
                    while(*class_name_str == '[') {
                        class_name_str++;
                        class_name_len--;
                    }
                    if(class_name_len == 1) {
                        stack.Push((uint32_t)((*class_name_str == type_list[type - 4]) ? 1 : 0));
                        break;
                    }
                    stack.Push((uint32_t)0);
                }
                else {
                    uint32_t class_index = type - 12;
                    const char *class_str = class_name.GetString();
                    uint16_t class_len = class_name.GetLength();
                    if(objectref->Dimensions) {
                        while(*class_str == '[') {
                            class_str++;
                            class_len--;
                        }
                        class_str++;
                        class_len -= 2;
                    }
                    stack.Push((uint32_t)((class_index == ((mVM *)mvm_ptr)->FindClassFileIndex(class_str, class_len)) ? 1 : 0));
                }
                break;
            }
            case OP_INSTANCEOF: {
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                if(!objectref) {
                    stack.Push((uint32_t)0);
                    pc += 2;
                    break;
                }
                uint16_t index = ReadUInt16(pc);
                pc += 2;
                ConstPoolType const_pool = GetConstPool(index);
                ConstClass cosnt_class = *(ConstClass *)&const_pool;
                ConstUTF8 class_name = cosnt_class.GetName();

                uint32_t type = objectref->Type;
                if((type > 3) && (type < 12)) {
                    static const char type_list[] = "ZCFDBSIJ";
                    const char *class_name_str = class_name.GetString();
                    uint16_t class_name_len = class_name.GetLength();
                    while(*class_name_str == '[') {
                        class_name_str++;
                        class_name_len--;
                    }
                    if(class_name_len == 1) {
                        stack.Push((uint32_t)((*class_name_str == type_list[type - 4]) ? 1 : 0));
                        break;
                    }
                    stack.Push((uint32_t)0);
                }
                else {
                    const char *class_name_str = class_name.GetString();
                    uint16_t class_name_len = class_name.GetLength();
                    if(objectref->Dimensions) {
                        while(*class_name_str == '[') {
                            class_name_str++;
                            class_name_len--;
                        }
                        class_name_str++;
                        class_name_len -= 2;
                    }
                    uint32_t class_index = type - 12;
                    while(1) {
                        int32_t class_index_2 = ((mVM *)mvm_ptr)->FindClassFileIndex(class_name_str, class_name_len);
                        if(class_index_2 < 0) {
                            exception_object = ((mVM *)mvm_ptr)->CreateThrowableObject("Can't found class");
                            goto exception_handler;
                        }
                        if(class_index == class_index_2) {
                            stack.Push((uint32_t)1);
                            break;
                        }
                        else if(ClassLoader(mvm_ptr, class_name.GetClassData()).GetSupperClass().GetClassData() == 0) {
                            stack.Push((uint32_t)0);
                            break;
                        }
                        else {
                            uint8_t *class_data = ((mVM *)mvm_ptr)->GetClassFile(class_index_2);
                            ClassLoader class_loader = ClassLoader(mvm_ptr, class_data);
                            ConstClass super_class = class_loader.GetSupperClass();
                            if(super_class.GetClassData() == 0) {
                                stack.Push((uint32_t)0);
                                break;
                            }
                            class_name = super_class.GetName();
                            class_name_str = class_name.GetString();
                            class_name_len = class_name.GetLength();
                        }
                    }
                }
                break;
            }
            case OP_MONITORENTER: {
                // TODO
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                while(objectref->MoniterCount) {
                    // DO SOMETHING
                }
                if(objectref->MoniterCount < 0xFFFFFFFF)
                    objectref->MoniterCount++;
                break;
            }
            case OP_MONITOREXIT: {
                // TODO
                HeapInfo_TypeDef *objectref = (HeapInfo_TypeDef *)stack.PopUInt32();
                objectref->MoniterCount--;
                break;
            }
            case OP_WIDE: {
                OpCode_TypeDef wide_opcode = (OpCode_TypeDef)*(pc++);
                switch(wide_opcode) {
                    case OP_IINC: {
                        uint16_t index = ReadUInt16(pc);
                        pc += 2;
                        int16_t const_value = ReadUInt16(pc);
                        pc += 2;
                        stack.ValueUInt32(locals_index + index) += const_value;
                        break;
                    }
                    case OP_ALOAD:
                        stack.Push((void *)stack.ValueUInt32(locals_index + ReadUInt16(pc)));
                        pc += 2;
                        break;
                    case OP_FLOAD:
                    case OP_ILOAD: {
                        stack.Push((uint32_t)stack.ValueUInt32(locals_index + ReadUInt16(pc)));
                        pc += 2;
                        break;
                    }
                    case OP_LLOAD:
                    case OP_DLOAD: {
                        stack.Push(stack.ValueUInt64(locals_index + ReadUInt16(pc)));
                        pc += 2;
                        break;
                    }
                    case OP_ASTORE: {
                        uint32_t *ptr = &stack.ValueUInt32(locals_index + ReadUInt16(pc));
                        pc += 2;
                        *ptr = stack.PopUInt32();
                        Heap_MemSetRefer(ptr, 1);
                    }
                    case OP_FSTORE:
                    case OP_ISTORE: {
                        stack.ValueUInt32(locals_index + ReadUInt16(pc)) = stack.PopUInt32();
                        pc += 2;
                        break;
                    }
                    case OP_LSTORE:
                    case OP_DSTORE: {
                        stack.ValueUInt64(locals_index + ReadUInt16(pc)) = stack.PopUInt64();
                        pc += 2;
                        break;
                    }
                    case OP_RET:
                        pc = &pc_start[stack.ValueUInt32(locals_index + ReadUInt16(pc))];
                        break;
                }
            }
            case OP_BREAKPOINT:
                break;
            default:
                Error("Uknow the opcode");
                goto exit;
        }
        continue;
    exception_handler:
        int32_t pc_index = GetExceptionPC(pc_end, pc - 1 - pc_start);
        if(pc_index < 0) {
            stack.MinSize = stack_min_size_old;
            stack.SetSP(sp_start);
            return_value = (uint32_t)exception_object;
            return 0;
        }
        else
            stack.Push((void *)exception_object);
        pc = &pc_start[pc_index];
    }
exit:
    stack.MinSize = stack_min_size_old;
    stack.SetSP(sp_start);
    return 1;
}
