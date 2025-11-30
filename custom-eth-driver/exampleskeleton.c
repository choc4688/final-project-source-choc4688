//Source: https://mkmints.wixsite.com/embtech/post/writing-your-first-ethernet-driver-a-step-by-step-guide

#include <linux/module.h>
#include <linux/netdevice.h>

static int my_eth_init(struct net_device *dev) {
    // Initialization code goes here
    printk(KERN_INFO "Initializing My Ethernet driver\n");
    // Assign MAC address
    ether_setup(dev);
    dev->netdev_ops = &my_netdev_ops;
    dev->flags |= IFF_PROMISC;
    return 0;
}

static void my_eth_exit(struct net_device *dev) {
    // Cleanup code goes here
    printk(KERN_INFO "Exiting My Ethernet driver\n");
}

static struct net_device_ops my_netdev_ops = {
    .ndo_init = my_eth_init,
    .ndo_uninit = my_eth_exit,
    // Add more operations here
};

static int __init my_eth_init_module(void) {
    struct net_device *dev = alloc_netdev(0, "myeth%d", NET_NAME_UNKNOWN, ether_setup);
    if (!dev)
        return -ENOMEM;
    register_netdev(dev);
    printk(KERN_INFO "My Ethernet driver registered\n");
    return 0;
}

static void __exit my_eth_exit_module(void) {
    // Cleanup and unregister code goes here
    struct net_device *dev = dev_get_by_name(&init_net, "myeth0");
    if (!dev) {
        printk(KERN_ERR "Ethernet device not found\n");
        return;
    }
    unregister_netdev(dev);
    free_netdev(dev);
    printk(KERN_INFO "My Ethernet driver unregistered\n");
}

module_init(my_eth_init_module);
module_exit(my_eth_exit_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinuxLovers-mkmints");
MODULE_DESCRIPTION("Sample Ethernet driver for Linux");