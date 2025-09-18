#include "common.h"


void print_exit(char *errMsg, int extVal) {
    perror(errMsg);
    exit(extVal);
}