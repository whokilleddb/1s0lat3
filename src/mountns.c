#include <sys/syscall.h>  
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "pidns.h"
#include "mountns.h"
#include "utils.h"

// Prepare Mount Namespace
int prepare_mountns(void){
    // mount --bind rootfs rootfs
    if(mount(ROOTFS, ROOTFS, FSTYPE, MS_BIND,"") < 0){
        fprintf(stderr,"[" RED("!") "] Failed to mount" RED("%s") "\n", ROOTFS);
        return -1;
    }

    // cd rootfs
    if(chdir(ROOTFS) < 0){
        fprintf(stderr,"[" RED("!") "] Failed to change directory\n");
        return -1;
    }

    // mkdir put_old
    const char *put_old = ".put_old";
    if((mkdir(put_old,0777) != 0) && (errno != EEXIST)){
        fprintf(stderr,"[" RED("!") "] Could not create "RED(".put_old") "\n");
        return -1;
    }

    // pivot_root . .put_old
    if(syscall(SYS_pivot_root,".",put_old)<0){
        fprintf(stderr,"[" RED("!") "] Could not "RED("PIVOT ROOT") "\n");
        return -1;
    }

    // cd /
    if (chdir("/") < 0){
        fprintf(stderr,"[" RED("!") "] Change Directory operation failed\n");
        return -1;
    }

    //prepare proc fs
    if (prepare_pidns() != 0){
        return -1;
    }
    fprintf(stdout,"[" GREEN("i") "] Successfully created " GREEN("PID") " namespace\n");

    //umount .put_old
    if(umount2(put_old, MNT_DETACH)){
        fprintf(stderr,"[" RED("!") "] Failed to unmount "RED(".put_old")"\n");
        return -1;
    }

    // remove .put_old
    if( rmdir(put_old) < 0){
        fprintf(stderr,"[" RED("!") "] Failed to remove "RED(".put_old")"\n");
        return -1;
    }
    return 0;
}