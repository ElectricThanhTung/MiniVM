
#include <stdio.h>
#include "mvm_error_report.h"

void Error(const char *message) {
    printf("%s\r\n", message);
    fflush(stdout);
    unsigned char temp = getchar();
}
