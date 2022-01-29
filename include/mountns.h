#pragma once
#include <sys/syscall.h>  
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "proc_ns.h"
#include "utils.h"

static void prepare_mount_ns(char* rootfs){
    const char *mnt = rootfs;

    // mount --bind rootfs rootfs
    if(mount(rootfs, mnt, FSTYPE, MS_BIND,"") < 0){
        exit_on_error("Failed to mount rootfs");
    }

    // cd rootfs
    if(chdir(mnt) < 0){
        exit_on_error("Failed to change directory");
    }

    // mkdir put_old
    const char *put_old = ".put_old";
    if(mkdir(put_old,0777) && errno != EEXIST ){
        exit_on_error("Could not create .put_old");
    }

    // pivot_root . .put_old
    if(syscall(SYS_pivot_root,".",put_old)<0){
        exit_on_error("Could Not Pivot Root");
    }

    // cd /
    if (chdir("/") < 0){
        exit_on_error("Could not change directory to /");
    }

    //prepare proc fs
    prepare_procfs();

    //umount .put_old
    if(umount2(put_old, MNT_DETACH)){
        exit_on_error("Failed to umount .put_old");
    }
}