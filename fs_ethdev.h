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




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// show_port
void show_port(void);



void   initialize_eth_dev_ports(void);
void ethdev_start(void);
void check_ports_link_status(uint32_t port_mask);
void  rx_tx_adapter_setup_internal_port(void);

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//RSS notes

// https://doc.dpdk.org/api/examples_2ip_pipeline_2link_8c-example.html#_a4
//   https://doc.dpdk.org/api/rte__ethdev_8h_source.html


#if 0
rte_flow_action_rss Struct Reference
#include <rte_flow.h>

Data Fields
enum rte_eth_hash_function 	func
uint32_t 	level
uint64_t 	types
uint32_t 	key_len
uint32_t 	queue_num
const uint8_t * 	key
const uint16_t * 	queue
#endif


#if 0 

 3870 int rte_eth_dev_rss_reta_update(uint16_t port_id,
 3871                 struct rte_eth_rss_reta_entry64 *reta_conf,
 3872                 uint16_t reta_size);
 3873 
 3892 int rte_eth_dev_rss_reta_query(uint16_t port_id,
 3893                    struct rte_eth_rss_reta_entry64 *reta_conf,
 3894                    uint16_t reta_size);
 3895 

 3870 int rte_eth_dev_rss_reta_update(uint16_t port_id,
 3871                 struct rte_eth_rss_reta_entry64 *reta_conf,
 3872                 uint16_t reta_size);
 3873 
 3892 int rte_eth_dev_rss_reta_query(uint16_t port_id,
 3893                    struct rte_eth_rss_reta_entry64 *reta_conf,
 3894                    uint16_t reta_size);
 3895 


struct rte_eth_rss_conf {
  459     uint8_t *rss_key;    
  460     uint8_t rss_key_len; 
  461     uint64_t rss_hf;     
  462 };

A structure used to configure the Receive Side Scaling (RSS) feature of an 
 Ethernet port. If not NULL, the rss_key pointer of the rss_conf structure 
 points to an array holding the RSS key to use for hashing specific header 
 fields of received packets. The length of this array should be indicated 
 by rss_key_len below. Otherwise, a default random hash key is used by the 
 device driver.

The rss_key_len field of the rss_conf structure indicates the length in 
 bytes of the array pointed by rss_key. To be compatible, this length will 
 be checked in i40e only. Others assume 40 bytes to be used as before.

The rss_hf field of the rss_conf structure indicates the different types 
 of IPv4/IPv6 packets to which the RSS hashing must be applied. Supplying 
 an rss_hf equal to zero disables the RSS feature.




struct rte_eth_rss_reta_entry64 {
  851     uint64_t mask;
  853     uint16_t reta[RTE_RETA_GROUP_SIZE];
  855 };



  712 #define ETH_RSS_IP ( \
  713     ETH_RSS_IPV4 | \
  714     ETH_RSS_FRAG_IPV4 | \
  715     ETH_RSS_NONFRAG_IPV4_OTHER | \
  716     ETH_RSS_IPV6 | \
  717     ETH_RSS_FRAG_IPV6 | \
  718     ETH_RSS_NONFRAG_IPV6_OTHER | \
  719     ETH_RSS_IPV6_EX)


static struct rte_eth_conf port_conf_default = {
    .link_speeds = 0,
    .rxmode = {
        .mq_mode = ETH_MQ_RX_NONE,
        .max_rx_pkt_len = 9000, /* Jumbo frame max packet len */
        .split_hdr_size = 0, /* Header split buffer size */
    },
    .rx_adv_conf = {
        .rss_conf = {
            .rss_key = NULL,
            .rss_key_len = 40,
            .rss_hf = 0,
        },
    },
    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
    },
    .lpbk_mode = 0,
};


static int
rss_setup(uint16_t port_id,
    uint16_t reta_size,
    struct link_params_rss *rss)
{
    struct rte_eth_rss_reta_entry64 reta_conf[RETA_CONF_SIZE];
    uint32_t i;
    int status;
    /* RETA setting */
    memset(reta_conf, 0, sizeof(reta_conf));
    for (i = 0; i < reta_size; i++)
        reta_conf[i / RTE_RETA_GROUP_SIZE].mask = UINT64_MAX;
    for (i = 0; i < reta_size; i++) {
        uint32_t reta_id = i / RTE_RETA_GROUP_SIZE;
        uint32_t reta_pos = i % RTE_RETA_GROUP_SIZE;
        uint32_t rss_qs_pos = i % rss->n_queues;
        reta_conf[reta_id].reta[reta_pos] =
            (uint16_t) rss->queue_id[rss_qs_pos];
    }
    /* RETA update */
    status = rte_eth_dev_rss_reta_update(port_id,
        reta_conf,
        reta_size);
    return status;
}





#endif




#endif

