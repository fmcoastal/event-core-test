#include <stdint.h>
#include <stdint.h>

#include <rte_ethdev.h>

#include "fs_extras.h"
#include "fs_print_rte_ethdev_struct.h"


void print_rte_eth_rxmode( int indent, struct rte_eth_rxmode *d);
void print_rte_eth_txmode( int indent, struct rte_eth_txmode *d);
void print_rte_eth_dcb_rx_conf(int indent, struct rte_eth_dcb_rx_conf  *d);
void print_rte_eth_rss_conf( int indent, struct rte_eth_rss_conf *d);
void print_rte_eth_vmdq_rx_conf(int indent, struct rte_eth_vmdq_rx_conf  *d);
void print_rte_eth_vmdq_dcb_conf (int indent, struct rte_eth_vmdq_dcb_conf *d);

#ifdef PRINT_DATA_STRUCTURES

/*****************************************************************************
 *  eth dev print functions
 ****************************************************************************/

void print_rte_eth_rxmode( int indent, struct rte_eth_rxmode *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_rxmode { \n",s);
   printf("%s    mq_mode:          0x%02x\n",s,d->mq_mode);
   printf("%s    max_rx_pkt_len:   0x%08x\n",s,d->max_rx_pkt_len);
   printf("%s    max_lro_pkt_size: 0x%08x\n",s,d->max_lro_pkt_size);
   printf("%s    split_hdr_size    0x%04x\n",s,d->split_hdr_size);
   printf("%s    offloads        0x%016lx\n",s,d->offloads);
   printf("%s    reserved_64s[0]        0x%016lx\n",s,d->reserved_64s[0]);
   printf("%s    reserved_64s[1]        0x%016lx\n",s,d->reserved_64s[1]);
   printf("%s    reserved_ptrs[0]        %p\n",s,d->reserved_ptrs[0]);
   printf("%s    reserved_ptrs[1]        %p\n",s,d->reserved_ptrs[1]);
   printf("%s } \n",s);
}

void print_rte_eth_txmode( int indent, struct rte_eth_txmode *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_txmode { \n",s);
   printf("%s    mq_mode:          0x%02x\n",s,d->mq_mode);
   printf("%s    offloads        0x%016lx\n",s,d->offloads);
   printf("%s    pvid              0x%04x\n",s,d->pvid);

   printf("%s    hw_vlan_reject_tagged:   0x%02x\n",s,d->hw_vlan_reject_tagged);

   printf("%s    reserved_64s[0]        0x%016lx\n",s,d->reserved_64s[0]);
   printf("%s    reserved_64s[1]        0x%016lx\n",s,d->reserved_64s[1]);
   printf("%s    reserved_ptrs[0]        %p\n",s,d->reserved_ptrs[0]);
   printf("%s    reserved_ptrs[1]        %p\n",s,d->reserved_ptrs[1]);
   printf("%s } \n",s);
}

void print_rte_eth_dcb_rx_conf(int indent, struct rte_eth_dcb_rx_conf  *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_dcb_rx_conf { \n"              ,s);
   printf("%s    inb_tcs:                       %d\n"   ,s,d->nb_tcs);
   printf("%s    dcb_tc    size       0x%02x\n"   ,s,ETH_DCB_NUM_USER_PRIORITIES);
    {
      int i;
      for ( i = 0 ; i < ETH_DCB_NUM_USER_PRIORITIES ; i++)
      {
          if( (  i % 20) == 0 ) printf("\n%s      ",s);
          printf("%02x",d->dcb_tc[i]);
      }
      printf("\n");
   }
   printf("%s } \n",s);
}

void print_rte_eth_rss_conf( int indent, struct rte_eth_rss_conf *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_rss_conf { \n"              ,s);
   printf("%s    rss_key:           %p\n"   ,s,d->rss_key);
   if(d->rss_key != NULL)
   {
      int i;
      for ( i = 0 ; i < 40 ; i++)
      {
          if( (  i % 20) == 0 ) printf("\n%s",s);
          printf("%02x",d->rss_key[i]);
      }
      printf("\n");
   }
   printf("%s    rss_key_len:       0x%02x\n"   ,s,d->rss_key_len);
   printf("%s    rss_hf:            0x%016lx\n"   ,s,d->rss_hf);
   printf("%s } \n",s);
}

void print_rte_eth_vmdq_rx_conf(int indent, struct rte_eth_vmdq_rx_conf  *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_vmdq_rx_conf { \n"              ,s);
   printf("%s    nb_queue_pools:        %d\n"   ,s,d->nb_queue_pools);
   printf("%s    enable_default_pool    0x%02x\n"   ,s,d->enable_default_pool);
   printf("%s    default_pool           0x%02x\n"   ,s,d->default_pool);
   printf("%s    enable_loop_back       0x%02x\n"   ,s,d->enable_loop_back);
   printf("%s    nb_pool_maps           0x%02x\n"   ,s,d->nb_pool_maps);
   printf("%s    rx_mode                0x%08x\n"   ,s,d->rx_mode);
   printf("%s    pool_map    size       0x%02x\n"   ,s,ETH_VMDQ_MAX_VLAN_FILTERS);
   printf("%s       pool_map[0].vlan_id          0x%04x\n"   ,s,d->pool_map[0].vlan_id);
   printf("%s       pool_map[0].pools            0x%016lx\n"  ,s,d->pool_map[0].pools);
   printf("%s } \n",s);
}

void print_rte_eth_vmdq_dcb_conf (int indent, struct rte_eth_vmdq_dcb_conf *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_vmdq_dcb_conf { \n"              ,s);
   printf("%s    nb_queue_pools:           %d\n"   ,s,d->nb_queue_pools);
   printf("%s    enable_default_pool:      0x%02x\n"   ,s,d->enable_default_pool);
   printf("%s    default_pool:             0x%02x\n"   ,s,d->default_pool);
   printf("%s    nb_pool_maps:             0x%02x\n"   ,s,d->nb_pool_maps);
   printf("%s    pool_map array size       0x%02x\n"   ,s,ETH_VMDQ_MAX_VLAN_FILTERS);
   printf("%s        pool_map[0].vlan_id          0x%04x\n"   ,s,d->pool_map[0].vlan_id);
   printf("%s        pool_map[0].pools            0x%016lx\n"  ,s,d->pool_map[0].pools);
   printf("%s    dcb_tc  array size        0x%02x\n"   ,s,ETH_DCB_NUM_USER_PRIORITIES);
   printf("%s        dcb_tc[0]:                0x%02x\n"   ,s,d->dcb_tc[0]);
   printf("%s } \n",s);
}


#if 1
 void print_rte_eth_conf  (int indent,const char* string,int id,struct rte_eth_conf *p)
 {
     INDENT(indent);
     printf("%sstruct rte_eth_conf %s eth#=%d {\n",s,string,id);
     printf("%s    uint32_t     link_speeds    :  %d\n"   ,s,p->link_speeds  );
     print_rte_eth_rxmode( indent+1 , &(p->rxmode));
     print_rte_eth_txmode( indent+1 , &(p->txmode));
     printf("%s    uint32_t     lpbk_mode      :  %d\n"   ,s,p->lpbk_mode    );
     printf("%s    struct rx_adv_conf             -below\n",s   );
     print_rte_eth_rss_conf     ( indent+2 , &(p->rx_adv_conf.rss_conf      ));
     print_rte_eth_vmdq_dcb_conf( indent+2 , &(p->rx_adv_conf.vmdq_dcb_conf ));
     print_rte_eth_dcb_rx_conf  ( indent+2 , &(p->rx_adv_conf.dcb_rx_conf   ));
     print_rte_eth_vmdq_rx_conf ( indent+2 , &(p->rx_adv_conf.vmdq_rx_conf  ));


     printf("%s    struct tx_adv_conf             -TBD-\n",s   );
     printf("%s    uint32_t dcb_capability_en:    %d\n"   ,s,p->dcb_capability_en   );
     printf("%s    struct fdir_conf:              -TBD-\n",s   );
     printf("%s    struct intr_conf:              -TBD-\n",s   );

     printf("%s}\n",s);
/*
uint32_t        link_speeds
struct rte_eth_rxmode   rxmode
struct rte_eth_txmode   txmode
uint32_t        lpbk_mode

struct {
   struct rte_eth_rss_conf   rss_conf
   struct rte_eth_vmdq_dcb_conf   vmdq_dcb_conf
   struct rte_eth_dcb_rx_conf   dcb_rx_conf
   struct rte_eth_vmdq_rx_conf   vmdq_rx_conf
}       rx_adv_conf

union {
   struct rte_eth_vmdq_dcb_tx_conf   vmdq_dcb_tx_conf
   struct rte_eth_dcb_tx_conf   dcb_tx_conf
   struct rte_eth_vmdq_tx_conf   vmdq_tx_conf
}       tx_adv_conf
uint32_t        dcb_capability_en
struct rte_fdir_conf    fdir_conf
struct rte_intr_conf    intr_conf
*/

 }
#endif

#endif



