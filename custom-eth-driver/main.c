//Using XDP as a way to directly access the NIC buffers, as the BCMGENET Ethernet driver used on the RasPi4B is very large in scope
//This hooks into the BCMGENET driver. So I don't have to modify the network driver code directly or write my own fully from scratch

//Initial example base: https://github.com/xdp-project/xdp-tutorial/blob/main/basic01-xdp-pass/xdp_pass_kern.c

/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

SEC("xdp") //Tells kernel this function is an XDP program
int  xdp_prog_simple(struct xdp_md *ctx)
{
    //*ctx is the context from the kernel for the packet



    //XDP_PASS flag tells the packet to continue up the regular linux network stack
	// return XDP_PASS;


    //Redirect incoming packets to an AF_XDP socket instead of passing up the linux network stack

    //'xsks_map' = BPF map holding userspace AF_XDP sockets




}

char _license[] SEC("license") = "GPL";