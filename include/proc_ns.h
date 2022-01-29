#pragma once
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.h"

static void prepare_procfs(){
    // mkdir /proc
    if ((mkdir("/proc", 0555) != 0) && (errno != EEXIST)){
        exit_on_error("Failed to mkdir /proc");
    }

    // mount proc
    if (mount("proc", "/proc", "proc", 0, "")){
        exit_on_error("Failed to mount proc");
    }
}
