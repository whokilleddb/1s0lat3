#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

void exit_on_error(const char *format, ...){
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}