#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
// #include <arpa/inet.h>

#include "/usr/lib/lwip/src/include/lwip/netif.h"
#include "/usr/lib/lwip/src/include/lwip/pbuf.h"
#include "/usr/lib/lwip/src/include/lwip/err.h"
#include "/usr/lib/lwip/src/include/lwip/etharp.h"

int raw_fd = -1;
static uint8_t hwaddr[6];

static err_t linkoutput(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    uint8_t buffer[1600];
    int offset = 0;

    // Copy the pbuf chain into a linear buffer
    for (q = p; q != NULL; q = q->next) {
        memcpy(buffer + offset, q->payload, q->len);
        offset += q->len;
    }

    send(raw_fd, buffer, offset, 0);
    return ERR_OK;
}

void linux_netif_init(struct netif *netif, const char *ifname)
{
    raw_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_fd < 0) { perror("socket"); return; }

    // Get interface index
    struct ifreq ifr = {0};
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ioctl(raw_fd, SIOCGIFINDEX, &ifr);
    int ifindex = ifr.ifr_ifindex;

    // Get MAC
    ioctl(raw_fd, SIOCGIFHWADDR, &ifr);
    memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);

    // Bind raw socket to the interface
    struct sockaddr_ll sll = {
        .sll_family   = AF_PACKET,
        .sll_protocol = htons(ETH_P_ALL),
        .sll_ifindex  = ifindex
    };

    if (bind(raw_fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind");
        return;
    }

    // Fill netif fields
    netif->hwaddr_len = 6;
    memcpy(netif->hwaddr, hwaddr, 6);
    netif->mtu = 1500;

    netif->flags = NETIF_FLAG_BROADCAST |
                   NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;

    netif->linkoutput = linkoutput;
    netif->output     = etharp_output;

    // Name (optional)
    netif->name[0] = 'e';
    netif->name[1] = 't';

    printf("linux_netif: bound to %s (ifindex=%d)\n", ifname, ifindex);
}
