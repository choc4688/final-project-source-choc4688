//Reference: Used ChatGPT for debugging and assistance in creating this test setup

#include "/usr/lib/lwip/src/include/lwip/init.h"
#include "/usr/lib/lwip/src/include/lwip/udp.h" //Gives udp_new, udp_bind, udp_recv, udp_sendto, etc.
#include "/usr/lib/lwip/src/include/lwip/timeouts.h"
// #include "/usr/lib/lwip/contrib/examples/example_app/default_netif.h"
#include "/usr/lib/lwip/src/include/lwip/timeouts.h"
#include "/usr/lib/lwip/src/include/lwip/tcpip.h"
#include "/usr/lib/lwip/src/include/lwip/netif.h"
#include "/usr/lib/lwip/src/include/lwip/inet.h"
#include "/usr/lib/lwip/src/include/lwip/pbuf.h" //pbuf is lwIP's packet buffer type
#include "/usr/lib/lwip/src/include/lwip/etharp.h"
#include "/usr/lib/lwip/src/include/lwip/ip_addr.h" //Contains ip_addr_t definition, IP address structures, etc.

//-------------------------------------------

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

//LwIP Notes:
//PCB = Protocol Control Block, a socket-like object
//lwIP doesn't auto free packet buffers, have to call pbuf_free()
//'netif_add()' creates and registers a NW interface for lwIP use
    //Adds interface to netif_list with things like MAC address, etc.
    //Assigns IP addr, subnet mask, and gateway if provided
    //Calls ethernetif_init() initialization setup
        //Driver setup function initializing Ethernet HW, setting MAC address, fill in callbacks like linkoutput, set interface HW type
    //Install the receive function 'netif_input' as the RX path
        //Driver hands packet to lwIP by calling netif->input(pbuf, netif)
    //Returns pointer to the fully initialized netif
        //Can then call things like 'netif_set_default(&my_netif)' or netif_set_up(&my_netif)


//lwIP expects raw Ethernet frames************************************


extern void linux_netif_init(struct netif *netif, const char *ifname);
extern int raw_fd;




static void udp_echo_recv(void *arg, struct udp_pcb *pcb,
                          struct pbuf *p, const ip_addr_t *addr,
                          u16_t port)
{
    if (p != NULL) { //If a packet was received, send the same buffer back to the same address + port
        udp_sendto(pcb, p, addr, port);   // Echo packet back
        pbuf_free(p);                     // Free the packet buffer
    }
}

int main(void)
{
    lwip_init();

    struct netif netif;
    ip4_addr_t ipaddr, netmask, gw;

    IP4_ADDR(&ipaddr, 192,168,1,100);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,1,1);

    // Initialize netif using AF_PACKET
    linux_netif_init(&netif, "eth0");
    netif_add(&netif, &ipaddr, &netmask, &gw, NULL,
              NULL /*init done already*/, ethernet_input);

    netif_set_default(&netif);
    netif_set_up(&netif);

    // Create UDP echo server
    struct udp_pcb *pcb = udp_new();
    udp_bind(pcb, IP_ADDR_ANY, 9000);
    udp_recv(pcb, udp_echo_recv, NULL);

    printf("lwIP UDP echo server running on 192.168.1.100:9000\n");

    // Main loop: receive+deliver packets + lwIP timers
    uint8_t buf[2048];

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(raw_fd, &fds);

        struct timeval tv = {0, 10000}; // 10 ms
        int rv = select(raw_fd+1, &fds, NULL, NULL, &tv);

        if (rv > 0 && FD_ISSET(raw_fd, &fds)) {
            int len = recv(raw_fd, buf, sizeof(buf), 0);
            if (len > 0) {
                // allocate pbuf
                struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
                pbuf_take(p, buf, len);
                netif.input(p, &netif);  // send into lwIP
            }
        }

        sys_check_timeouts();  // lwIP timers
    }

    return 0;
}