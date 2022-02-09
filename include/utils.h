#pragma once

#ifndef __UTILS__
#define __UTILS__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define RED(string)     "\x1b[31m" string "\x1b[0m"
#define GREEN(string)   "\x1b[32m" string "\x1b[0m"
#define YELLOW(string)  "\x1b[33m" string "\x1b[0m"
#define BLUE(string)    "\x1b[34m" string "\x1b[0m"
#define MAGENTA(string) "\x1b[35m" string "\x1b[0m"
#define CYAN(string)    "\x1b[36m" string "\x1b[0m"

#define UID 1000
#define VETH0 "veth0"   // The interface facing the root namespace
#define VETH1 "veth1"   // The interface facing the network namespace
#define IP_0 "10.1.1.1" // IP address associated with VETH0
#define IP_1 "10.1.1.2" // IP address associated with VETH1
#define NETMASK "255.255.255.0" // Netmask of our virtual network
#define DEVICETYPE "veth"

void exit_on_error(const char *format, ...);

#endif