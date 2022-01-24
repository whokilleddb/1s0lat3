#include <stdlib.h>
#pragma once
#define RED(string)     "\x1b[31m" string "\x1b[0m"
#define GREEN(string)   "\x1b[32m" string "\x1b[0m"
#define YELLOW(string)  "\x1b[33m" string "\x1b[0m"
#define BLUE(string)    "\x1b[34m" string "\x1b[0m"
#define MAGENTA(string) "\x1b[35m" string "\x1b[0m"
#define CYAN(string)    "\x1b[36m" string "\x1b[0m"

#define exit_on_error(msg) ({fprintf(stderr,"[" RED("-") "] %s\n",msg); exit(EXIT_FAILURE);}) 

#define STACKSIZE 1024*1024

#define UID 1000
