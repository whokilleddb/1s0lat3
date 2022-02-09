#pragma once
#ifndef __MOUNT_NS
#define __MOUNT_NS

#include <stdlib.h>
#include <stdio.h>

#define ROOTFS "rootfs" // Name of rootfs directory 
#define FSTYPE "ext4"   // FS Type
    
int prepare_mountns(void);

#endif