//Reference: Used ChatGPT for debugging and assistance in creating this test setup

#include "/usr/lib/lwip/src/include/lwip/init.h"
#include "/usr/lib/lwip/src/include/lwip/udp.h"
#include "/usr/lib/lwip/src/include/lwip/timeouts.h"
// #include "/usr/lib/lwip/contrib/examples/example_app/default_netif.h"
#include "/usr/lib/lwip/src/include/lwip/timeouts.h"
#include "/usr/lib/lwip/src/include/lwip/tcpip.h"
#include "/usr/lib/lwip/src/include/lwip/netif.h"
#include "/usr/lib/lwip/src/include/lwip/inet.h"
#include "/usr/lib/lwip/src/include/lwip/pbuf.h"
#include "/usr/lib/lwip/src/include/lwip/etharp.h"

//-------------------------------------------

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>

int tapfd;
struct netif my_netif;


int open_tap(char *dev_name) {
    struct ifreq ifr;
    tapfd = open("/dev/net/tun", O_RDWR);
    if (tapfd < 0) {
        perror("open /dev/net/tun");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI; // TAP device, no extra headers
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

    if (ioctl(tapfd, TUNSETIFF, (void *)&ifr) < 0) { //Ioctl called on the fd to actually tie it to the 'tap0' NW Interface and set it up
        perror("ioctl TUNSETIFF");
        close(tapfd);
        return -1;
    }
    return tapfd;
}


//"Loops through pbuf chain and writes packet data totapfd" - ChatGPT
err_t tap_linkoutput(struct netif *netif, struct pbuf *p) {
    struct pbuf *q;
    for(q = p; q != NULL; q = q->next) {
        if (write(tapfd, q->payload, q->len) < 0) {
            perror("write to tap");
            return ERR_IF;
        }
    }
    return ERR_OK;
}


void poll_tap() {
    char buf[1600];
    int n = read(tapfd, buf, sizeof(buf));
    if (n > 0) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, n, PBUF_RAM);
        if (p) {
            pbuf_take(p, buf, n);
            my_netif.input(p, &my_netif); // pass packet into lwIP
        }
    }
}


err_t tap_input(struct pbuf *p, struct netif *netif) {
    // just free p if you don't need it yet
    pbuf_free(p);
    return ERR_OK;
}


err_t example_netif_init(struct netif *netif) {
    netif->output = etharp_output;
    netif->linkoutput = tap_linkoutput; //ChatGPT Response: you need to implement a stub or real driver
    return ERR_OK;
}

void udpRecvCustom(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    // Echo the packet back
    udp_sendto(pcb, p, addr, port);
    pbuf_free(p);
}

//lwIP calls linkoutput() to send packets to the NIC

int main(void) {

    lwip_init();

    tapfd = open_tap("tap0");

    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;

    IP4_ADDR(&ipaddr, 192,168,1,10);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gateway, 192,168,1,1);

    netif_add(&my_netif, &ipaddr, &netmask, &gateway, NULL, example_netif_init, tap_input);
    netif_set_default(&my_netif);
    netif_set_up(&my_netif);

    struct udp_pcb *pcb = udp_new();

    //Args: pcb struct, const ip_addr_t* ipaddr, port#
    udp_bind(pcb, IP_ADDR_ANY, 9000);

    // udp_recv(pcb, udpRecvCustom, NULL);
    pcb->recv = udpRecvCustom;
    pcb->recv_arg = NULL;

    while(1) {
        poll_tap();
        sys_check_timeouts(); // Needed for lwIP timers
        usleep(1000);
    }
}
