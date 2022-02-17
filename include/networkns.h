#ifndef __NETWORK_NS
#define __NETWORK_NS

#include "utils.h"

// Netlink Message Payload
#define MAX_PAYLOAD 1024
#define VETH0 "veth0"   // The interface facing the root namespace
#define VETH1 "veth1"   // The interface facing the network namespace
#define IP0 "10.1.1.1" // IP address associated with VETH0
#define IP1 "10.1.1.2" // IP address associated with VETH1
#define NETMASK "255.255.255.0" // Netmask of our virtual network
#define METRICS 201 // Metrics for routing

int prepare_networkns(int child_pid);
int interface_up(char *ifname, char *ip, char *netmask, short if_flags);
int ns_fd(int pid);
int add_route();
int create_veth(int child_pid);
#endif
