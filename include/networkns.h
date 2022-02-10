#ifndef __NETWORK_NS
#define __NETWORK_NS

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "utils.h"

// Netlink Message Payload
#define MAX_PAYLOAD 1024
#define VETH0 "veth0"   // The interface facing the root namespace
#define VETH1 "veth1"   // The interface facing the network namespace
#define IP0 "10.1.1.1" // IP address associated with VETH0
#define IP1 "10.1.1.2" // IP address associated with VETH1
#define NETMASK "255.255.255.0" // Netmask of our virtual network
#define DEVICETYPE "veth"

int check_response(int sock_fd);
int send_nlmsg(int sock_fd, struct nlmsghdr *n);
int create_veth(int sock_fd, char *ifname, char *peername);
int interface_up(char *ifname, char *ip, char *netmask);
int ns_fd(int pid);
int move_interface_to_ns(int sock_fd, char *ifname, int netns);
int prepare_networkns(int child_pid);
#endif