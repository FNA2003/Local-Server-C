#include "common.h"


print_exit(char *errMsg, int extVal) {
    perror(errMsg);
    exit(extVal);
}