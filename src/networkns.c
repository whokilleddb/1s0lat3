/* The Holy Bible which helped me through this:
https://maz-programmersdiary.blogspot.com/2011/09/netlink-sockets.html

God protect the precious soul who wrote this
*/
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/veth.h>
#include <libnetlink.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "utils.h"
#include "networkns.h"


/*
Struct representing the Netlink message taken from:
Line 111:  https://github.com/shemminger/iproute2/blob/main/ip/ip_common.h (Commit: 7ca868a)

Link Message Format: https://www.infradead.org/~tgr/libnl/doc/route.html#_netlink_protocol
Netlink Message Header Format: https://datatracker.ietf.org/doc/html/rfc3549#section-2.3.2
Struct ifinfomsg definition: https://elixir.bootlin.com/linux/v4.9/source/include/uapi/linux/rtnetlink.h#L481
*/
struct iplink_req{
    struct nlmsghdr n; // Netlink message header
    struct ifinfomsg i; // Request family specific Link header 
    char buf[MAX_PAYLOAD];
};


// Check netlink message response from kernel
int check_response(int sock_fd){
    struct iovec iov;
    struct msghdr msg = {
            .msg_name = NULL,
            .msg_namelen = 0,
            .msg_iov = &iov,
            .msg_iovlen = 1
    };
    
    char *resp;
    resp = (char *)malloc(MAX_PAYLOAD);
    if (resp == NULL){
        fprintf(stderr, "[" RED("!") "] Out of memory\n");
        return -1;
    }

    // Check response
    struct iovec *resp_iov = msg->msg_iov;
    resp_iov->iov_base = &resp;
    resp_iov->iov_len = MAX_PAYLOAD;

    recv_reply: 
        ssize_t resp_len = recvmsg(fd, msg, 0);
        if (resp_len == 0 || resp_len < 0 ){
            if(errno==EINTR){
                goto recv_reply;
            }
            else{
                fprintf(stderr, "[" RED("!") "] Error occured while receiving "RED("Netlink")" message\n");
                return -1;
            }
        }


    // Cast pointer to nlmsghdr type
    struct nlmsghdr *hdr = (struct nlmsghdr *)resp;
    int nlmsglen = hdr->nlmsg_len;
    int datalen = nlmsglen - sizeof(struct nlmsghdr); // Get data length

    if(datalen < 0 || nlmsglen > resp_len){
        if(msg.msg_flags == MSG_TRUNC ){
            fprintf(stderr, "[" RED("!") "] Received truncated "RED("Netlink")" message\n");
            return -1;
        }
        fprintf(stderr, "[" RED("!") "] Received malformed "RED("Netlink")" message\n");
    }


    // Check for error message
    if (hdr->nlmsg_flags == NLMSG_ERROR){
        // See netlink (7)
        struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(hdr);
        if(datalen < sizeof(struct nlmsgerr)){
            fprintf(stderr, "[" RED("!") "] Received malformed "RED("Netlink")" message\n");
            return -1;
        }
        if (err->error){
            fprintf(stderr, "[" RED("!") "] Received error code: %d\n", err->error);
            return -1;
        }
    }
    free(resp);
    return 0;
}


// Send Netlink message
int send_nlmsg(int sock_fd, struct nlmsghdr *n){
    // See EXAMPLES from man netlink(7)
    // See: https://bit.ly/3oBuYAM
    struct iovec iov = {
        .iov_base = n,  /* Starting address */
        .iov_len = n->nlmsg_len     /* Starting address */
    };

    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &iov,
        .msg_iovlen = 1
    };

    n->nlmsg_pid = 0;
    n->nlmsg_seq++;

    if (sendmsg(sock_fd, &msg, 0) < 0){
        fprintf(stderr,"[" RED("!") "] Failed to send "RED("Netlink") "message\n");
        return -1;
    }

    return 0;
}


// Create veth pair
int create_veth(int sock_fd, char *ifname, char *peername){
    // For reference, see: https://github.com/shemminger/iproute2/blob/main/ip/iplink.c
    struct iplink_req req = {
        /* From netlink(3)
        NLMSG_LENGTH()
        Given the payload length, len, this macro returns the aligned length to store
        in the nlmsg_len field of the nlmsghdr.
        */

        .n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
        /* From netlink(7)
        NLM_F_REQUEST:  Must be set on all request messages
        NLM_F_ACK:      Request for an acknowledgement on success.
        NLM_F_CREATE:   Create new device if it doesn't already exist
        NLM_F_EXCL:     Don't replace if the object already exists
        */
        .n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL,
        /* From rtnetlink(7)
        RTM_NEWLINK: Create a specific network interface.
        */
        .n.nlmsg_type = RTM_NEWLINK,

        // Set socket type to Netlink
        .i.ifi_family = PF_NETLINK,
    };


    // Adding Attributes to our Netlink message 
    struct nlmsghdr *n = &req.n;
    int maxlen = sizeof(req);

    /* From libnetlink(3):
    int addattr_l(struct nlmsghdr *n, int maxlen, int type, void *data, int alen)
        Add a variable length attribute of type type and with value data and alen length to netlink message n,
        which is part of a buffer of length maxlen.  data is copied.

    From https://www.infradead.org/~tgr/libnl/doc/route.html:
    IFLA_IFNAME: Used to add link name attribute
    */
    if (addattr_l(n, maxlen, IFLA_IFNAME, ifname, strlen(ifname)+1) <0){
        fprintf(stderr,"[" RED("!") "] Failed to set interface name attribut\n");
        return -1;
    }

    /* Create a network bridge
    Add Attribute
    See: https://unix.stackexchange.com/questions/441877/identify-if-a-network-interface-is-a-veth-using-sys-class-net
    Also, I highly recommend refering to the Bible mentioned at the start of this code for the upcoming sections
    */
    struct rtattr *linkinfo;
    struct rtattr *linkdata;
    struct rtattr *peerinfo;

    if(memset(&linkinfo,0,sizeof(rtattr)) == NULL || memset(&linkdata,0,sizeof(rtattr)) == NULL || memset(&peerinfo,0,sizeof(rtattr)) == NULL ){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }

    linkinfo =  addattr_nest(n, maxlen, IFLA_LINKINFO); // Nested attribute to contain veth info

    // Specify a 'veth' type interface
    if (addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, DEVICETYPE, strlen(DEVICETYPE)) < 0 ){
        addattr_nest_end(n, linkinfo);
        fprintf(stderr,"[" RED("-") "] Could not create "RED("'veth'")" device\n");
        return -1;
    }

    // Add another nested attribute data
    linkdata = addattr_nest(n, maxlen, IFLA_INFO_DATA);

    // Add nested attribute containing peer name
    peerinfo = addattr_nest(n, maxlen, VETH_INFO_PEER);
    n->nlmsg_len += sizeof(struct ifinfomsg);

    if (addattr_l(n, maxlen, IFLA_IFNAME, peername, strlen(peername) + 1)<0){
        addattr_nest_end(n, linkinfo);
        addattr_nest_end(n, linkdata);
        fprintf(stderr,"[" RED("-") "] Could not add peername attribute\n");
        return -1;
    }

    // End attributes
    addattr_nest_end(n, peerinfo);
    addattr_nest_end(n, linkdata);
    addattr_nest_end(n, linkinfo);

    // Send message
    if (send_nlmsg(sock_fd, n) != 0){
        fprintf(stderr,"[" RED("-") "] Failed to send "RED("Netlink Message")"\n");
        return -1;
    }

    // Check Response
    if (check_response(sock_fd) != 0){
        fprintf(stderr, "[" RED("!") "] Error occured while fetching netlink response");
        return -1;
    }
    
    return 0;
}


// Bring up interface
int interface_up(char *ifname, char *ip, char *netmask){
    // See: https://stackoverflow.com/questions/5858655/linux-programmatically-up-down-an-interface-kernel/5859449
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP );
    if(s < 0){
        fprintf(stderr, "["RED("!")"] Failed to create SOCK_DGRAM socket\n");
        return -1;
    }

    // man netdevice(7)
    struct ifreq ifr;
    if( memset(&ifr, 0, sizeof(struct ifreq)) == NULL){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }

    // Copy interface name
    strncpy(ifr.ifr_name, ifname, strlen(ifname));

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
    ifr.ifr_flags |= IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST;
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


// Move interface to namespace
int move_interface_to_ns(int sock_fd, char *ifname, int netns){
    // send interface to namespace
     struct iplink_req req = {
        .n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
        .n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        .n.nlmsg_type = RTM_NEWLINK,
        .i.ifi_family = PF_NETLINK,
    };

    // See: https://pyroute2.org/pyroute2-0.3.9alpha1/netns.html
    addattr_l(&req.n, sizeof(req), IFLA_NET_NS_FD, &netns, 4);
    addattr_l(&req.n, sizeof(req), IFLA_IFNAME, ifname, strlen(ifname));

    // Send message
    if (send_nlmsg(sock_fd, n) != 0){
        fprintf(stderr,"[" RED("-") "] Failed to send "RED("Netlink Message")"\n");
        return -1;
    }

    // Check Response
    if (check_response(sock_fd) != 0){
        fprintf(stderr, "[" RED("!") "] Error occured while fetching netlink response");
        return -1;
    }

    return 0;
}

// Prepare Network Namespace
int prepare_networkns(int child_pid){
    /*
    PF_NETLINK: Netlink socket
    SOCK_RAW: Raw Socket, requires sudo privileges
    SOCK_CLOEXEC: Avoids a race condition between getting a new socket from accept and setting the FD_CLOEXEC flag afterwards.
    NETLINK_ROUTE: Communication channel between user-space routing dæmon and kernel packet forwarding module.

    PS: PF_NETLINK and AF_NETLINK point to the same value as can be seen in socket.h:
    #define PF_NETLINK AF_NETLINK
    */
    int s = socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if(s < 0){
        fprintf(stderr, "["RED("!")"] Failed to create" RED("Netlink") " socket\n");
        return -1;
    }

    // Create veth pair using netlink messages to the kernel
    if (create_veth(s, VETH0, VETH1)!= 0){
        fprintf(stderr, "[" RED("!")"] Could not create "RED("veth")" pair");
        return -1;
    }

    // Set veth0 up with given ip and netmask
    if(interface_up(VETH0, IP0, NETMASK) < 0){
        fprintf(stderr, "["RED("!")"] Could not setup %s interface\n", VETH0);
        return -1;
    }

    int host_fd = ns_fd(getpid());
    int child_fd = ns_fd(child_pid);

    // Move VETH1 to child namespace
    if(move_interface_to_ns(sock_fd, VETH1, child_fd) < 0){
        fprintf(stderr,"["RED("!")"] Failed to move %s to child namespace\n", VETH1);
        return -1;
    }

    // See man setns(2)
    if (setns(child_fd, CLONE_NEWNET) < 0){
        fprintf(stderr,"["RED("!")"] Failed to move thread to child namespace\n", VETH1);
        return -1;
    }

    // Set veth1 up with given ip and netmask
    if(interface_up(VETH1, IP1, NETMASK) < 0){
        fprintf(stderr, "["RED("!")"] Could not setup %s interface\n", VETH1);
        return -1;
    }

    // See man setns(2)
    if (setns(host_fd, CLONE_NEWNET) < 0){
        fprintf(stderr,"["RED("!")"] Failed to move thread back to host namespace\n", VETH1);
        return -1;
    }

    close(s);
    close(child_fd);
    close(host_fd);
    return 0;
}
