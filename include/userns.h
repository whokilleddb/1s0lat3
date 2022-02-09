#pragma once
#ifndef __PREPARE_USER_NS
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "utils.h"

int prepare_user_ns(int pid);

#endif