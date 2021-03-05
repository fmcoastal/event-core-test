#ifndef _fs_ethdev_h
#define _fs_ethdev_h


/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

#include "main.h"

extern struct test_mode_struct  tm_ethdev;
extern uint64_t g_rx_packet_cnt[];

extern  uint32_t  g_IP_eth0 ;   // my IP address for arp  
extern  MacAddr_t g_Mac_eth0 ;  // my MAC address for arp 






void   initialize_eth_dev_ports(void);
void ethdev_start(void);
void check_ports_link_status(uint32_t port_mask);
void  rx_tx_adapter_setup_internal_port(void);



#endif

