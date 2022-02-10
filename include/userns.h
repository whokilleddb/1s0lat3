#pragma once
#ifndef __USER_NS
#define __USER_NS
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "utils.h"

int prepare_userns(int pid);

#endif