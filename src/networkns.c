#include <fcntl.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/veth.h>
#include <net/route.h>
#include <sys/types.h>

#include "networkns.h"
#include "utils.h"


// Create veth pair
int create_veth(int child_pid){
    // See: https://github.com/thom311/libnl/blob/master/tests/test-create-veth.c
	struct nl_sock *sk;

	sk = nl_socket_alloc();
	if (nl_connect(sk, NETLINK_ROUTE) < 0) {
		fprintf(stderr, "[" RED("!")"] Unable to connect "RED("Netlink")" socket\n");
		return -1;
	}
    
    if(rtnl_link_veth_add(sk, VETH0, VETH1, child_pid) != 0){
		fprintf(stderr, "[" RED("!")"] Unable to create VETH pair\n");
        nl_close(sk);
		return -1;
    }

    nl_close(sk);
    return 0;

}


// Bring up interface
int interface_up(char *ifname, char *ip, char *netmask, short if_flags){
    // See: https://stackoverflow.com/questions/5858655/linux-programmatically-up-down-an-interface-kernel/5859449
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP );
    if(s < 0){
        fprintf(stderr, "["RED("!")"] Failed to create SOCK_DGRAM socket\n");
        return -1;
    }

    // man netdevice(7)
    struct ifreq ifr;
    if(memset(&ifr, 0, sizeof(struct ifreq)) == NULL){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }

    // Copy interface name
    strcpy(ifr.ifr_name, ifname);

    struct sockaddr_in saddr;
    if(memset(&saddr, 0, sizeof(struct sockaddr_in)) == NULL){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = 0;
    char *p = (char *)&saddr;

    // From man netdevice(7)
    // Set IP address of the interface
    saddr.sin_addr.s_addr = inet_addr(ip);
    memcpy((char *)&(ifr.ifr_addr), p, sizeof(struct sockaddr));
    if(ioctl(s, SIOCSIFADDR, &ifr) < 0){
        fprintf(stderr, "["RED("!")"] Failed to set up Interface: %s with IP: %s "RED("ioctl()")" call\n", ifname, ip);
        return -1;
    }

    // Set Netmask of the interface
    saddr.sin_addr.s_addr = inet_addr(netmask);
    memcpy((char *)&(ifr.ifr_addr), p, sizeof(struct sockaddr));
    if(ioctl(s, SIOCSIFNETMASK, &ifr) < 0){
        fprintf(stderr, "["RED("!")"] Failed to set up Interface: %s with Netmask: %s "RED("ioctl()")" call\n", ifname, netmask);
        return -1;
    }

    // Set interface flags
    ifr.ifr_flags |= if_flags;
    if(ioctl(s, SIOCSIFFLAGS, &ifr) <0 ){
        fprintf(stderr, "["RED("!")"] Failed to set interface flags for %s with "RED("ioctl()")" call\n", ifname);
        return -1;
    }

    if(close(s) < 0){
        fprintf(stderr, "["RED("!")" Failed to close socket\n");
        return -1;
    }

    return 0;
}


// Add routing table
int add_route(){
    //See: https://stackoverflow.com/questions/22733967/linux-how-to-set-default-route-from-c
    int sockfd;
    struct rtentry route;
    struct sockaddr_in *addr;
    int err = 0;

    // create the socket
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
        fprintf(stderr,"["RED("!")"] Could not create socket\n");
        return -1;
    }

    memset(&route, 0, sizeof(route));
    addr = (struct sockaddr_in*) &route.rt_gateway;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(IP0);
    addr = (struct sockaddr_in*) &route.rt_dst;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr = (struct sockaddr_in*) &route.rt_genmask;
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = METRICS;
    route.rt_dev = VETH1;
    if ((err = ioctl(sockfd, SIOCADDRT, &route)) != 0) {
        fprintf(stderr,"["RED("!")"] " RED("SIOCADDRT")" failed\n");
        return -1;
    }
    return 0;
}

// Get a file descriptor inside a namespace
int ns_fd(int pid){
    char *path;
    if(asprintf(&path, "/proc/%d/ns/net", pid) < 0){
        fprintf(stderr, "["RED("!") "] Out of memory\n");
        return -1;
    }

    int fd = open(path, O_RDONLY);
    if(fd < 0){
        fprintf(stderr, "["RED("!") "] Could not open "RED("/proc/%d/ns/net")" for reading\n", pid);
        return -1;
    }

    free(path);
    return fd;
}


// Prepare Network Namespace
int  prepare_networkns(int child_pid){
    // Create veth pair
    if(create_veth(child_pid) != 0){
        fprintf(stderr,"["RED("!")"] Failed to create VETH pair\n");
        return -1;
    }

    // Set veth0 up with given ip and netmask
    short if_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST;
    if(interface_up(VETH0, IP0, NETMASK, if_flags) < 0){
        fprintf(stderr, "["RED("!")"] Could not setup %s interface\n", VETH0);
        return -1;
    }
    
    int host_fd = ns_fd(getpid());
    int child_fd = ns_fd(child_pid);

    // See man setns(2)
    if (setns(child_fd, CLONE_NEWNET) < 0){
        fprintf(stderr,"["RED("!")"] Failed to move thread to child namespace\n");
        return -1;
    }

    // Set veth1 up with given ip and netmask
    if(interface_up(VETH1, IP1, NETMASK, if_flags) < 0){
        fprintf(stderr, "["RED("!")"] Could not setup %s interface\n", VETH1);
        return -1;
    }

    // Set localhost up inside namespace
    short lo_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if(interface_up("lo", "127.0.0.1", "255.0.0.0", lo_flags) < 0){
        fprintf(stderr, "["RED("!")"] Could not setup %s interface\n", VETH1);
        return -1;
    }

    // Add route via default gateway inside namespace
    if (add_route() != 0){
        fprintf(stderr, "["RED("!")"] Failed to add route\n");
        return -1;
    }

    // See man setns(2)
    if (setns(host_fd, CLONE_NEWNET) < 0){
        fprintf(stderr,"["RED("!")"] Failed to move thread back to host namespace\n");
        return -1;
    }

    close(host_fd);
    close(child_fd);
    return 0;
}
