
#include <iostream>
#include <time.h>
#include <cstdio>
#include <string>
#include <algorithm>

#include "mvm.h"
#include "mvm_heap.h"
#include "mvm_error_report.h"

using namespace std;

static uint8_t *ReadFile(string file_name) {
    FILE *file = fopen(file_name.c_str(), "rb");
    if(file == 0) {
        printf("Cannot open file %s\r\n", file_name.c_str());
        return 0;
    }
    fseek(file, 0, SEEK_END);
    unsigned long file_size = (unsigned long)ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *data = new uint8_t[file_size];

    if(fread(data, 1, file_size, file) != file_size) {
        delete[] data;
        fclose(file);
        printf("Cannot read file %s\r\n", file_name.c_str());
        return 0;
    }
    fclose(file);
    return data;
}

int main(int argc, char *argv[]) {
    static char buffer[8192];
    setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));

    if(argc >= 2) {
        uint8_t *program = ReadFile(argv[1]);

        clock_t start = clock();

        const char **arg = (argc > 2) ? (const char **)&argv[2] : 0;
        mVM mvm = mVM(program);
        mvm.Run(arg, argc - 2);

        delete[] program;

        clock_t end = clock();
        printf("\r\n\r\nComplete in %.3fs\r\n\r\n", (float)(end - start) / 1000);
    }
    else
        printf("Nothing to run\r\n\r\n");

    fflush(stdout);
    return getchar();
}
