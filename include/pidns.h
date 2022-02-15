#pragma once
#ifndef __PROC_NS
#define __PROC_NS
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.h"

int prepare_pidns(void);
#endif