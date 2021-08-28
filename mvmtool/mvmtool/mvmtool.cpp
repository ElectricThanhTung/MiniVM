
#include <iostream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <Windows.h>

#include "mvm_class_loader.h"

using namespace std;

typedef struct {
    uint8_t *ptr;
    uint32_t length;
} Data;

static string OutputFileName = "MiniPackage.mpk";

bool DirExists(const std::string &dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if(ftyp == INVALID_FILE_ATTRIBUTES)
        return false;

    if(ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;

    return false;
}

static wstring GetFolder(wstring file_name) {
    int32_t position = file_name.find_last_of(L"\\");
    if(position > 0)
        return file_name.substr(0, position);
    return L"";
}

static wstring GetExtension(wstring file_name) {
    int32_t position = file_name.find_last_of(L".");
    return file_name.substr(position + 1);
}

static string GetFileName(string file_name) {
    int32_t position = file_name.find_last_of(".");
    return file_name.substr(0, position);
}

static void Tolower(wstring str) {
    std::for_each(str.begin(), str.end(), [](wchar_t &c) {
        c = ::toupper(c);
    });
}

static void CreateFolder(wstring path) {
    wstring path2;
    while(1) {
        int32_t pos = path.find_first_of(L"\\");
        if(pos > 0) {
            wstring folder = path.substr(0, pos);
            path2 += folder + L"\\";
            CreateDirectory(path2.c_str(), NULL);
            path = path.substr(pos + 1, path.length());
        }
        else {
            path2 += path + L"\\";
            CreateDirectory(path2.c_str(), NULL);
            break;
        }
    }
}

static Data ReadFile(string file_path) {
    Data ret = {0, 0};
    FILE *file = fopen(file_path.c_str(), "rb");
    if(file == 0) {
        ret.ptr = 0;
        return ret;
    }
    fseek(file, 0, SEEK_END);
    uint32_t file_size = (uint64_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = new uint8_t[file_size];

    if(fread(data, 1, file_size, file) != file_size) {
        delete[] data;
        fclose(file);
        ret.ptr = 0;
        return ret;
    }
    fclose(file);
    ret.ptr = data;
    ret.length = file_size;
    return ret;
}

static void FreeData(Data &data) {
    delete[] data.ptr;
    data.ptr = 0;
    data.length = 0;
}

static int32_t CountClassFile(wstring path) {
    int32_t file_count = 0;
    WIN32_FIND_DATA data;
    HANDLE hFile = FindFirstFile((path + L"*.*").c_str(), &data);
    if(hFile == (HANDLE)-1)
        return 1;
    do {
        wstring name = wstring(data.cFileName);
        if((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0) {
            if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                wstring extension_name = GetExtension(name);
                Tolower(extension_name);
                if((extension_name.compare(L"class") == 0) && (name.compare(L"module-info.class") != 0))
                    file_count++;
            }
            else {
                if((name.compare(L".") != 0) && (name.compare(L"..") != 0)) {
                    wstring path_2;
                    if(path.compare(L"") == 0)
                        path_2 = path_2 + data.cFileName + L"\\";
                    else
                        path_2 = path + L"\\" + data.cFileName + L"\\";
                    int count = CountClassFile(path_2);
                    if(count < 0)
                        return -1;
                    file_count += count;
                }
            }
        }
    } while(FindNextFile(hFile, &data));
    return file_count;
}

static void BrowserAndFindFile(wstring path, wstring *ret_list) {
    static uint32_t file_count = 0;
    WIN32_FIND_DATA data;
    HANDLE hFile = FindFirstFile((path + L"*.*").c_str(), &data);
    if(hFile == (HANDLE)-1)
        return;
    do {
        wstring name = wstring(data.cFileName);
        if((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0) {
            if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                wstring extension_name = GetExtension(name);
                Tolower(extension_name);
                if((extension_name.compare(L"class") == 0) && (name.compare(L"module-info.class") != 0)) {
                    wstring path_2;
                    if(path.compare(L"") == 0)
                        path_2 = path_2 + data.cFileName;
                    else
                        path_2 = path + L"\\" + data.cFileName;
                    ret_list[file_count++] = path_2;
                }
            }
            else {
                if((name.compare(L".") != 0) && (name.compare(L"..") != 0)) {
                    wstring path_2;
                    if(path.compare(L"") == 0)
                        path_2 = path_2 + data.cFileName + L"\\";
                    else
                        path_2 = path + L"\\" + data.cFileName + L"\\";
                    BrowserAndFindFile(path_2, ret_list);
                }
            }
        }
    } while(FindNextFile(hFile, &data));
}

static int32_t WStringCompare(wstring &str1, wstring &str2) {
    if(str1.length() != str2.length())
        return str1.length() - str2.length();
    return str1.compare(str2);
}

static int32_t MethodNameCompare(MethodOffsetInfo &method1, MethodOffsetInfo &method2) {
    string name1 = method1.MethodName.Name;
    string name2 = method2.MethodName.Name;
    if(name1.length() != name2.length())
        return name1.length() - name2.length();
    else {
        string desc1 = method1.MethodName.Desc;
        string desc2 = method2.MethodName.Desc;
        if(desc1.length() != desc2.length())
            return desc1.length() - desc2.length();
        else {
            int32_t cmp_value = name1.compare(name2);
            if(cmp_value != 0)
                return cmp_value;
            else
                return desc1.compare(desc2);
        }
    }
}

static void SortListFiles(wstring *file_lists, uint32_t file_count) {
    for(uint32_t i = 0; i < file_count; i++) {
        uint32_t min_index = i;
        wstring min = file_lists[min_index];
        for(uint32_t j = (i + 1); j < file_count; j++) {
            if(WStringCompare(min, file_lists[j]) > 0) {
                min = file_lists[j];
                min_index = j;
            }
        }
        file_lists[min_index] = file_lists[i];
        file_lists[i] = min;
    }
}

static unsigned char AddClassFile(wstring file_name, uint32_t file_index) {
    string local_file_name = string(file_name.begin(), file_name.end());

    Data data = ReadFile(local_file_name);
    if(data.ptr == 0)
        return 0;

    ClassLoader class_loader = ClassLoader(data.ptr);
    int16_t const_pool_count = class_loader.ConstPoolCount();
    int16_t method_count = class_loader.GetMethodsCount();

    uint32_t offset_list[5] = {
        (uint32_t)((const_pool_count + method_count) * 4 + 20),
        class_loader.GetAccessFlagOffset() + (const_pool_count + method_count) * 4 + 20,
        class_loader.GetFieldsCountOffset() + (const_pool_count + method_count) * 4 + 20,
        class_loader.GetMethodsCountOffset() + (const_pool_count + method_count) * 4 + 20,
        class_loader.GetAttributesCountOffset() + (const_pool_count + method_count) * 4 + 20,
    };

    uint32_t *const_pool_offset_list = new uint32_t[const_pool_count];
    for(int16_t i = 1; i < const_pool_count; i++)
        const_pool_offset_list[i - 1] = (const_pool_count + method_count) * 4 + 20 + class_loader.GetConstPoolOffset(i);

    MethodOffsetInfo *method_offset_info_list = new MethodOffsetInfo[method_count];

    for(int16_t i = 0; i < method_count; i++)
        method_offset_info_list[i] = class_loader.GetMethodOffset(i);

    for(int16_t i = 0; i < method_count; i++) {
        uint16_t min_index = i;
        MethodOffsetInfo method_min = method_offset_info_list[min_index];
        for(int16_t j = (i + 1); j < method_count; j++) {
            if(MethodNameCompare(method_min, method_offset_info_list[j]) > 0) {
                method_min = method_offset_info_list[j];
                min_index = j;
            }
        }
        method_offset_info_list[min_index] = method_offset_info_list[i];
        method_offset_info_list[i] = method_min;
    }

    uint32_t *method_offset_list = new uint32_t[method_count];
    for(int16_t i = 0; i < method_count; i++) {
        //printf("    %s%s\r\n", method_offset_info_list[i].MethodName.Name, method_offset_info_list[i].MethodName.Desc);
        method_offset_list[i] = (const_pool_count + method_count) * 4 + 20 + method_offset_info_list[i].Offset;
        delete[] method_offset_info_list[i].MethodName.Name;
        delete[] method_offset_info_list[i].MethodName.Desc;
    }
    delete[] method_offset_info_list;

    FILE *file = fopen(OutputFileName.c_str(), "r+b");
    if(file == 0)
        return 0;

    fseek(file, 0, SEEK_END);
    uint32_t offset_of_file = (unsigned long)ftell(file);
    if((offset_of_file % 4) != 0) {
        offset_of_file += 4 - (offset_of_file % 4);
        uint8_t dummy_byte[3] = {0};
        fseek(file, offset_of_file, SEEK_SET);
    }
    fwrite(offset_list, 4, 5, file);
    fwrite(const_pool_offset_list, 4, const_pool_count, file);
    fwrite(method_offset_list, 4, method_count, file);
    fwrite(data.ptr, 1, data.length, file);
    fseek(file, (file_index + 1) * 4, SEEK_SET);
    fwrite(&offset_of_file, 4, 1, file);
    fclose(file);

    delete[] const_pool_offset_list;
    delete[] method_offset_list;
    FreeData(data);
    return 1;
}

string ReplaceString(std::wstring subject, const std::wstring &search, const std::wstring &replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return string(subject.begin(), subject.end());
}

int main(int argc, char *argv[]) {
    static char buffer[8192];
    setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));
    string sdk_path = "";

    if(argc > 2) {
        printf("\033[0;31mNumber of Command invalid!\033[0;37m\r\n");
        return 0;
    }
    if(argc == 2)
        OutputFileName = argv[1];

    wstring output_folder = wstring(OutputFileName.begin(), OutputFileName.end());
    output_folder = GetFolder(output_folder);
    if(output_folder.compare(L"") == 0)
        CreateFolder(output_folder);

    int32_t file_count = CountClassFile(L"");
    if(file_count > 0) {
        FILE *file = fopen(OutputFileName.c_str(), "wb");
        if(file == 0) {
            printf("\033[0;31mCannot write file \033[0;33m%s\033[0;37m\r\n", OutputFileName.c_str());
            return 0;
        }
        uint32_t *header = new uint32_t[file_count + 1];
        header[0] = (uint32_t)file_count;

        size_t size = fwrite(header, 4, file_count + 1, file);
        if(size != file_count + 1) {
            printf("\033[0;31mCannot write file \033[0;33m%s\033[0;37m\r\n", OutputFileName.c_str());
            return 0;
        }
        fclose(file);
        delete[] header;

        wstring *file_lists = new wstring[file_count];
        BrowserAndFindFile(L"", file_lists);
        SortListFiles(file_lists, file_count);
        for(uint32_t i = 0; i < file_count; i++) {
            string string_to_print = ReplaceString(file_lists[i], L"\\\\", L"/");
            printf("\033[0;32mPacking \033[0;33m%s\033[0;37m\r\n", string_to_print.c_str());
            if(!AddClassFile(file_lists[i], i)) {
                printf("\033[0;31mCannot write file \033[0;33m%s\r\n", string_to_print.c_str());
                break;
            }
        }
        delete[] file_lists;

        printf("\033[0;36mCreated \033[0;35m%s\033[0;37m\r\n", OutputFileName.c_str());
    }
    else
        printf("\033[0;31mNot found any java class file\033[0;37m\r\n");

    printf("\033[0;37m");
    fflush(stdout);
    return 0;
}
