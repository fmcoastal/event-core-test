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

void print_rte_eth_dev_portconf( int indent, struct rte_eth_dev_portconf *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_dev_portconf { \n",s);
   printf("%s    burst_size: 0x%02x\n",s,d->burst_size);
   printf("%s    ring_size: 0x%02x\n",s,d->ring_size);
   printf("%s    nb_queues: 0x%02x\n",s,d->nb_queues);
   printf("%s } \n",s);

}


void print_rte_eth_dev_info  (int indent, const char* string,int id, struct rte_eth_dev_info *p)
{
    INDENT(indent);
    printf("%sstruct rte_eth_dev_info %s dev#=%d \n",s,string,id);
    printf("%s    const char   driver_name                  :  %s\n",s,p->driver_name           );
    printf("%s    unsigned int if_index                     :  %d\n",s,p->if_index              );
    printf("%s    uint16_t     min_mtu                      :  %d\n",s,p->min_mtu               );
    printf("%s    uint16_t     max_mtu                      :  %d\n",s,p->max_mtu               );
    printf("%s    const uint32_t * dev_flags                :  %p  %x\n",s,p->dev_flags,*p->dev_flags);
    printf("%s    uint32_t     min_rx_bufsize               :  %d\n",s,p->min_rx_bufsize        );
    printf("%s    uint32_t     max_rx_pktlen                :  %d\n",s,p->max_rx_pktlen         );
    printf("%s    uint32_t     max_lro_pkt_size             :  %d\n",s,p->max_lro_pkt_size      );
    printf("%s    uint16_t     max_rx_queues                :  %d\n",s,p->max_rx_queues         );
    printf("%s    uint16_t     max_tx_queues                :  %d\n",s,p->max_tx_queues         );
    printf("%s    uint32_t     max_mac_addrs                :  %d\n",s,p->max_mac_addrs         );
    printf("%s    uint16_t     max_vfs                      :  %d\n",s,p->max_vfs               );
    printf("%s    uint16_t     max_vmdq_pools               :  %d\n",s,p->max_vmdq_pools        );
    printf("%s    struct rte_eth_rxseg_capa    rx_seg_capa  : -TBD-  \n",s          );
    printf("%s    uint64_t     rx_offload_capa              :  0x%lx\n",s,p->rx_offload_capa       );
    printf("%s    uint64_t     tx_offload_capa              :  0x%lx\n",s,p->tx_offload_capa       );
    printf("%s    uint64_t     rx_queue_offload_capa        :  0x%lx\n",s,p->rx_queue_offload_capa );
    printf("%s    uint64_t     tx_queue_offload_capa        :  0x%lx\n",s,p->tx_queue_offload_capa );
    printf("%s    uint16_t     reta_size                    :  %d\n",s,p->reta_size             );
    printf("%s    uint8_t      hash_key_size                :  %d\n",s,p->hash_key_size         );
    printf("%s    uint64_t     flow_type_rss_offloads       :  0x%lx\n",s,p->flow_type_rss_offloads);
    print_rte_eth_rxconf(indent+1 , &(p->default_rxconf));
    print_rte_eth_txconf(indent+1 , &(p->default_txconf));
    printf("%s    uint16_t     vmdq_queue_base              :  %d\n",s,p->vmdq_queue_base       );
    printf("%s    uint16_t     vmdq_queue_num               :  %d\n",s,p->vmdq_queue_num        );
    printf("%s    uint16_t     vmdq_pool_base               :  %d\n",s,p->vmdq_pool_base        );
    printf("%s    struct rte_eth_desc_lim      rx_desc_lim  :  -TBD-\n",s           );
    printf("%s    struct rte_eth_desc_lim      tx_desc_lim  :  -TBD-\n",s           );
    printf("%s    uint32_t     speed_capa                   :  %d\n",s,p->speed_capa            );
    printf("%s    uint16_t     nb_rx_queues                 :  %d\n",s,p->nb_rx_queues          );
    printf("%s    uint16_t     nb_tx_queues                 :  %d\n",s,p->nb_tx_queues          );
    print_rte_eth_dev_portconf( indent+1 ,&(p->default_rxportconf));
    print_rte_eth_dev_portconf( indent+1 , &(p->default_txportconf));
    printf("%s    uint64_t     dev_capa                     :  0x%lx\n",s,p->dev_capa              );
    printf("%s    struct rte_eth_switch_info   switch_info  :  -TBD-\n",s           );
    printf("%s    uint64_t     reserved_64s [0]             :  0x%lx\n",s,p->reserved_64s [0]      );
    printf("%s    uint64_t     reserved_64s [1]             :  0x%lx\n",s,p->reserved_64s [1]      );
    printf("%s    void *       reserved_ptrs [0]            :  %p\n",s,p->reserved_ptrs [0]      );
    printf("%s    void *       reserved_ptrs [1]            :  %p\n",s,p->reserved_ptrs [1]      );
}






/*****************************************************************************
 *  eth dev print functions
 ****************************************************************************/

void print_rte_eth_rxmode( int indent, struct rte_eth_rxmode *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}

void print_rte_eth_txmode( int indent, struct rte_eth_txmode *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}

void print_rte_eth_dcb_rx_conf(int indent, struct rte_eth_dcb_rx_conf  *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}

void print_rte_eth_rss_conf( int indent, struct rte_eth_rss_conf *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}

void print_rte_eth_vmdq_rx_conf(int indent, struct rte_eth_vmdq_rx_conf  *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}

void print_rte_eth_vmdq_dcb_conf (int indent, struct rte_eth_vmdq_dcb_conf *d)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
}


void print_rte_eth_thresh(int indent, struct rte_eth_thresh *d);
void print_rte_eth_thresh(int indent, struct rte_eth_thresh *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_thresh { \n",s);
   printf("%s    pthresh: 0x%02x\n",s,d->pthresh);
   printf("%s    hthresh: 0x%02x\n",s,d->hthresh);
   printf("%s    wthresh: 0x%02x\n",s,d->wthresh);
   printf("%s } \n",s);
}




void print_rte_eth_rxconf( int indent, struct rte_eth_rxconf *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_rxconf { \n"              ,s);
   print_rte_eth_thresh(indent+1,&(d->rx_thresh));
   printf("%s    rx_free_thresh:    0x%04x \n"   ,s,d->rx_free_thresh);
   printf("%s    rx_drop_en:        0x%02x \n"  ,s,d->rx_drop_en);
   printf("%s    rx_deferred_start: 0x%02x \n"  ,s,d->rx_deferred_start);
   printf("%s    offloads:          0x%016lx \n",s,d->offloads);
   printf("%s } \n",s);
}

void print_rte_eth_txconf( int indent, struct rte_eth_txconf *d)
{
   INDENT(indent);

   printf("%s struct rte_eth_txconf { \n"              ,s);
   print_rte_eth_thresh(indent+1, &(d->tx_thresh));
   printf("%s    tx_rs_thresh:      0x%04x\n"   ,s,d->tx_rs_thresh);
   printf("%s    tx_free_thresh:    0x%02x \n"  ,s,d->tx_free_thresh);
   printf("%s    tx_deferred_start: 0x%02x \n"  ,s,d->tx_deferred_start);
   printf("%s    offloads:          0x%016lx \n",s,d->offloads);
   printf("%s } \n",s);
}
 



#if 1
void print_rte_eth_conf  (int indent,const char* string,int id,struct rte_eth_conf *p)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

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
   FONT_NORMAL();
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



