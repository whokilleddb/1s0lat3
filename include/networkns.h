/* The Holy Bible which helped me through this:
https://maz-programmersdiary.blogspot.com/2011/09/netlink-sockets.html

God protect the precious soul who wrote this
*/
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <libnetlink.h>
#include <linux/veth.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <sys/uio.h>
#include <string.h>

#include "utils.h"

// Netlink Message Payload
#define MAX_PAYLOAD 1024

/*
Struct referencing the Netlink message
See https://github.com/shemminger/iproute2/blob/main/ip/ip_common.h
*/
struct iplink_req{
    // See:https://datatracker.ietf.org/doc/html/rfc3549#section-2.3.2
    struct nlmsghdr n; // Netlink message header
    // See https://elixir.bootlin.com/linux/v4.9/source/include/uapi/linux/rtnetlink.h#L481
    struct ifinfomsg i;
    char buf[MAX_PAYLOAD];
};

// Check response from Kernel
void check_response(int sock_fd){
    struct iovec iov;
    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &iov, 
        .msg_iovlen = 1
    };

}


// send Netlink message
static void send_nlmsg(int sock_fd, struct nlmsghdr *n){
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

    n->nlmsg_seq++;

    if (sendmsg(sock_fd, &msg, 0) < 0){
        exit_on_error("[-] Netlink Request Failed");
    }

    check_response(sock_fd);
}

// Create veth device
static int create_veth(int sock_fd, char *ifname, char *peername){
    /* Taken from source code of ip command
    See Link 1060: https://github.com/shemminger/iproute2/blob/main/ip/iplink.c
    */
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
        exit_on_error("Could Not Setup veth pair");
    }

   /* Create a network bridge
   Add Attribute
   See: https://unix.stackexchange.com/questions/441877/identify-if-a-network-interface-is-a-veth-using-sys-class-net
   Also, I highly recommend refering to the Bible mentioned at the start of this code for the upcoming sections
   */
   struct rtattr *linkinfo;
   struct rtattr *linkdata;
   struct rtattr *peerinfo;
   linkinfo =  addattr_nest(n, maxlen, IFLA_LINKINFO);

   // Specify a veth type device
   if (addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, DEVICETYPE, strlen(DEVICETYPE)+1) < 0 ){
       addattr_nest_end(n, linkinfo);
       exit_on_error("[-] Could not create veth device :(");
   }

   // Add another nested attribute data
   linkdata = addattr_nest(n, maxlen, IFLA_INFO_DATA);

   // Add nested attribute containing peer name
   peerinfo = addattr_nest(n, maxlen, VETH_INFO_PEER);
   n->nlmsg_len += sizeof(struct ifinfomsg);

   if (addattr_l(n, maxlen, IFLA_IFNAME, peername, strlen(peername) + 1)<0){
       addattr_nest_end(n, linkdata);
       exit_on_error("[-] Could not add peer");
   }

    // End attributes
    addattr_nest_end(n, peerinfo);
    addattr_nest_end(n, linkdata);
    addattr_nest_end(n, linkinfo);

    // Send message
    send_nlmsg(sock_fd, n);
    return 0;
}

// Prepare Network Namespace
static int prepare_network_ns(){
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
        exit_on_error("Failed to create Netlink socket");
    }

    if (create_veth(s, VETH0, VETH1)!= 0){
        exit_on_error("Could not create interface pair");
    }

    return 0;
}
