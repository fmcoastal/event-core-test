#ifndef  _fs_global_data_h
#define _fs_global_data_h




//////////////////////////////////////
typedef struct global_data_struct {
   uint16_t  enabled_eth_port_mask   ;   //two ports handled by vfio-pci
   uint16_t  nb_eth_ports_available  ;
   struct    rte_ether_addr  eth_addr[RTE_MAX_ETHPORTS];
   uint16_t  event_dev_id;             // event dev device ID index.
   struct    rte_mempool*        p_pktmbuf_pool ;
   struct    event_queues        evq;
   struct    event_ports         evp;
   struct    rte_event_port_conf def_p_conf;
   struct    event_rx_adptr      rx_adptr;
   struct    event_tx_adptr      tx_adptr;

   struct rte_event_timer_adapter *timer_100ms;
} global_data_t;

extern global_data_t g_glob;

void print_global_data (global_data_t *p);


#endif
