#include <errno.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "utils.h"

int prepare_pidns(void){
    // mkdir /proc
    if ((mkdir("/proc", 0555) != 0) && (errno != EEXIST)){
        fprintf(stderr,"[" RED("!") "] Failed to create " RED("/proc") "directory\n");
        return -1;
    }

    // mount proc
    if (mount("proc", "/proc", "proc", 0, "")){
        fprintf(stderr,"[" RED("!") "] Failed to mount the " RED("/proc") "directory\n");
        return -1;
    }
    return 0;
}