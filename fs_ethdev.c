/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>   // for usleep

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <rte_eventdev.h>

#include <rte_spinlock.h>
#include "fs_extras.h"
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
#include "fs_ethdev.h"



#include "fs_spinlock_test.h"
//#include "fs_print_structures.c"
// eth dev structures

#include "fs_extras.h"
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>
#include "fs_print_rte_eventdev_struct.h"
#include "fs_print_rte_ethdev_struct.h"


//#include "fs_print_rte_structures.h"



#if 0

              +-----------------+
                | +-------------+ |
   +-------+    | |    flow 0   | |
   |Packet |    | +-------------+ |
   |event  |    | +-------------+ |
   |       |    | |    flow 1   | |port_link(port0, queue0)
   +-------+    | +-------------+ |     |     +--------+
   +-------+    | +-------------+ o-----v-----o        |dequeue +------+
   |Crypto |    | |    flow n   | |           | event  +------->|Core 0|
   |work   |    | +-------------+ o----+      | port 0 |        |      |
   |done ev|    |  event queue 0  |    |      +--------+        +------+
   +-------+    +-----------------+    |
   +-------+                           |
   |Timer  |    +-----------------+    |      +--------+
   |expiry |    | +-------------+ |    +------o        |dequeue +------+
   |event  |    | |    flow 0   | o-----------o event  +------->|Core 1|
   +-------+    | +-------------+ |      +----o port 1 |        |      |
  Event enqueue | +-------------+ |      |    +--------+        +------+
o-------------> | |    flow 1   | |      |
   enqueue(     | +-------------+ |      |
   queue_id,    |                 |      |    +--------+        +------+
   flow_id,     | +-------------+ |      |    |        |dequeue |Core 2|
   sched_type,  | |    flow n   | o-----------o event  +------->|      |
   event_type,  | +-------------+ |      |    | port 2 |        +------+
   subev_type,  |  event queue 1  |      |    +--------+
   event)       +-----------------+      |    +--------+
                                         |    |        |dequeue +------+
   +-------+    +-----------------+      |    | event  +------->|Core n|
   |Core   |    | +-------------+ o-----------o port n |        |      |
   |(SW)   |    | |    flow 0   | |      |    +--------+        +--+---+
   |event  |    | +-------------+ |      |                         |
   +-------+    | +-------------+ |      |                         |
       ^        | |    flow 1   | |      |                         |
       |        | +-------------+ o------+                         |
       |        | +-------------+ |                                |
       |        | |    flow n   | |                                |
       |        | +-------------+ |                                |
       |        |  event queue n  |                                |
       |        +-----------------+                                |
       |                                                           |
       +-----------------------------------------------------------+
#endif

// I will start with 4 cores,  ports, and 4 queues.
//       maybe later, i will increase the number of queues and try to inject a timer event.


extern const char * StringSched[];    // defined in fs_core.c


extern fs_time_stamp g_per_core_time_stamp[32]__rte_cache_aligned; // per core time stamp
extern uint64_t  g_per_core_result[]; // per core time stamp

///////////////////
//ethdev
////////////////
#define RTE_ETHDEV_RX_DESC_DEFAULT 1024     /* not sure what this is - is it an event  */
#define RTE_ETHDEV_TX_DESC_DEFAULT 1024
#define MEMPOOL_CACHE_SIZE 256
#define MAX_PKT_BURST 32

#define RTE_RX_DESC_DEFAULT 1024
#define RTE_TX_DESC_DEFAULT 1024



/*********************************************************************
 *********************************************************************
 *           REGULAR SPINLOCK TEST                                   *
 *********************************************************************
 *********************************************************************/

extern rte_spinlock_t g_spinlock_measure_lock;
#define SpinLockFunction()  rte_spinlock_lock( &g_spinlock_measure_lock); rte_spinlock_unlock( &g_spinlock_measure_lock);

//  forward reference for compiler
int        ethdev_setup( __attribute__((unused))void * arg);
int         ethdev_loop( __attribute__((unused))void * arg);
int        ethdev_print( __attribute__((unused))void * arg);
int      ethdev_cleanup( __attribute__((unused))void * arg);
void ethdev_description( void);

//#define RTE_MAX_ETHPORTS 8

struct event_rx_adptr {
        uint32_t service_id;
        uint8_t nb_rx_adptr;
        uint8_t *rx_adptr;
};

struct event_tx_adptr {
        uint32_t service_id;
        uint8_t nb_tx_adptr;
        uint8_t *tx_adptr;
};

struct event_queues {
        uint8_t *event_q_id;
        uint8_t nb_queues;
};

struct event_ports {
        uint8_t *event_p_id;
        uint8_t nb_ports;
        rte_spinlock_t lock;
};





typedef struct global_data_struct {
   uint16_t enabled_port_mask   ;   //two ports handled by vfio-pci  
   uint16_t nb_ports_available  ;
   uint16_t event_d_id;             // event dev device ID index.
   uint16_t sched_type;
   struct rte_mempool*   p_pktmbuf_pool ;
   struct rte_ether_addr eth_addr[RTE_MAX_ETHPORTS]; 
   struct rte_event_port_conf def_p_conf;
   struct event_rx_adptr rx_adptr;
   struct event_tx_adptr tx_adptr;
   struct event_queues evq;
   struct event_ports evp;


} global_data;  

global_data g_glob;

//uint8_t g_evt_dev_id;  g_glob.event_d_id
uint8_t g_nb_PortsPerCore ; 
uint8_t g_nb_QueuesPerCore ; 


static char C_RED[]   ={0x1b,'[','3','1','m',0x00};  // GREEN=$'\e[32m'
//static char C_GREEN[]   ={0x1b,'[','3','2','m',0x00};  // GREEN=$'\e[32m'
//static char C_CYAN[]   ={0x1b,'[','3','6','m',0x00};  // GREEN=$'\e[32m'
static char C_WHITE[]   ={0x1b,'[','3','7','m',0x00};  // GREEN=$'\e[32m'


// initialize each port
void      initialize_eth_dev_ports(void);
void     initialize_eth_dev_ports(void)
{         
    uint16_t port_id;
    int ret;
    uint16_t nb_rxd = RTE_RX_DESC_DEFAULT;
    uint16_t nb_txd = RTE_TX_DESC_DEFAULT;
    uint32_t nb_mbufs;
    uint16_t nb_ports;


    struct rte_eth_conf port_conf = {
           .rxmode = {
                  .max_rx_pkt_len = RTE_ETHER_MAX_LEN,
                  .split_hdr_size = 0,
           },
           .txmode = {
                   .mq_mode = ETH_MQ_TX_NONE,
           },
    };

    WAI();
// for event mode 
// note the RSS reference    
    port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
    port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
    port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP;

    print_rte_eth_conf(0,"default: port_conf",0,&port_conf);    
/*   printf(" struct rte_eth_conf     \n");
    printf(" port_conf.rxmode.mq_mode         %d \n", port_conf.rxmode.mq_mode);
    printf(" port_conf.rxmode.max_rx_pkt_len  %d \n", port_conf.rxmode.max_rx_pkt_len );
    printf(" port_conf.rxmode.split_hdr_size  %d \n", port_conf.rxmode.split_hdr_size);
    printf(" port_conf.txmode.mq_mode         %d \n", port_conf.txmode.mq_mode);
    printf(" port_conf.rx_adv_conf.rss_conf.rss_key  0x%p \n", port_conf.rx_adv_conf.rss_conf.rss_key);
    printf(" port_conf.rx_adv_conf.rss_conf.rss_hf   %ld \n", port_conf.rx_adv_conf.rss_conf.rss_hf);
*/

//   allocate mbuffs for the eth-dev interface HW.
    nb_ports = 2;  // FS -EVOL  I set this to 2 because I am going to use 2 etherent connections.  

    nb_mbufs = RTE_MAX(nb_ports * (RTE_ETHDEV_RX_DESC_DEFAULT +
                                   RTE_ETHDEV_TX_DESC_DEFAULT +
                                   MAX_PKT_BURST + rte_lcore_count() *
                                   MEMPOOL_CACHE_SIZE), 8192U);

    printf(" We think we need  %d mbufs\n",nb_mbufs );
    /* create the mbuf pool */

   
    CALL_RTE("rte_pktmbuf_pool_create()");    
    g_glob.p_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool",

                    nb_mbufs, MEMPOOL_CACHE_SIZE, 0,
                    RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (g_glob.p_pktmbuf_pool == NULL)
            rte_panic("Cannot init mbuf pool (g_p_pktmbuf_pool)  \n");
        


    RTE_ETH_FOREACH_DEV(port_id) {
         struct rte_eth_conf local_port_conf = port_conf;
         struct rte_eth_dev_info dev_info;
         struct rte_eth_rxconf rxq_conf;
         struct rte_eth_txconf txq_conf;

          /* skip ports that are not enabled */
          if ((g_glob.enabled_port_mask & (1 << port_id)) == 0) {
                  printf("Skipping disabled port %u\n", port_id);
                  continue;
          }
          g_glob.nb_ports_available++;
         /* init port */
          printf("Initializing port %u... ", port_id);
          printf ("%s %s  portid: %d %s\n",C_RED,__FUNCTION__,port_id,C_WHITE);
          fflush(stdout);

#ifdef PRINT_CALL_ARGUMENTS
               FONT_CYAN();
               printf(  " Call Args: port_id %d  dev_info: \n",port_id);
               FONT_NORMAL();
#endif
          CALL_RTE("rte_eth_dev_info_get()");    
          ret = rte_eth_dev_info_get(port_id, &dev_info);
          if (ret != 0)
                  rte_panic("Error during getting device (port %u) info: %s\n",
                            port_id, strerror(-ret));
          local_port_conf.rx_adv_conf.rss_conf.rss_hf &= dev_info.flow_type_rss_offloads;
          if (local_port_conf.rx_adv_conf.rss_conf.rss_hf != port_conf.rx_adv_conf.rss_conf.rss_hf) {
                  printf("Port %u modified RSS hash function based on hardware support,"
                        "requested:%#"PRIx64" configured:%#"PRIx64"",
                          port_id,
                          port_conf.rx_adv_conf.rss_conf.rss_hf,
                          local_port_conf.rx_adv_conf.rss_conf.rss_hf );
          }

          if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
                  local_port_conf.txmode.offloads |=
                          DEV_TX_OFFLOAD_MBUF_FAST_FREE;
 
#ifdef PRINT_CALL_ARGUMENTS
          FONT_CYAN();
          printf(  " Call Args: port_id %d  rx_queuess:%d  tx_queues %d  local_port_conf: \n",port_id,1,1);
          FONT_NORMAL();
#endif
          CALL_RTE("rte_eth_dev_configure()");    
          ret = rte_eth_dev_configure(port_id, 1, 1, &local_port_conf);
          if (ret < 0)
                  rte_panic("Cannot configure device: err=%d, port=%u\n",
                            ret, port_id);

 #ifdef PRINT_CALL_ARGUMENTS
          FONT_CYAN();
          printf(  " Call Args: port_id %d  nb_rxd :%d  nb_txd:%d \n",port_id,nb_rxd,nb_txd);
          FONT_NORMAL();
#endif
          CALL_RTE("call rte_eth_dev_adjust_nb_rx_tx_desc()");    
          ret = rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rxd,
                                                 &nb_txd);
          if (ret < 0)
                  rte_panic("Cannot adjust number of descriptors: err=%d, port=%u\n",
                            ret, port_id);

          CALL_RTE("call rte_eth_macaddr_get()");    
          rte_eth_macaddr_get(port_id, &g_glob.eth_addr[port_id]);

          /* init one RX queue */
          fflush(stdout);
          printf("port_id = %d\n",port_id);
          printf("nb_rxd = %d\n",nb_rxd);
          printf("rte_eth_dev_socket_id(port_id) = %d\n",rte_eth_dev_socket_id(port_id));
          rxq_conf = dev_info.default_rxconf;
          rxq_conf.offloads = local_port_conf.rxmode.offloads;

#ifdef PRINT_CALL_ARGUMENTS
          FONT_CYAN();
          printf(  " Call Args: port_id %d  rx_queue_id:%d  nb_rx_desc:%d  socketid:%d  rx_conf:   rte_mempool* \n",
                                     port_id,           0,      nb_rxd,  rte_eth_dev_socket_id(port_id));
          FONT_NORMAL();
#endif
          CALL_RTE("rte_eth_rx_queue_setup()");    
          ret = rte_eth_rx_queue_setup(port_id, 0, nb_rxd,
                                       rte_eth_dev_socket_id(port_id),
                                       &rxq_conf,
                                       g_glob.p_pktmbuf_pool);
          if (ret < 0)
                  rte_panic("rte_eth_rx_queue_setup:err=%d, port=%u\n",
                            ret, port_id);

          /* init one TX queue on each port */
          fflush(stdout);
          txq_conf = dev_info.default_txconf;
          txq_conf.offloads = local_port_conf.txmode.offloads;
          CALL_RTE("rte_eth_tx_queue_setup()");    
          ret = rte_eth_tx_queue_setup(port_id, 0, nb_txd,
                          rte_eth_dev_socket_id(port_id),
                          &txq_conf);
          if (ret < 0)
                  rte_panic("rte_eth_tx_queue_setup:err=%d, port=%u\n",
                            ret, port_id);

          CALL_RTE("rte_eth_promiscuous_enable()\n");    
          rte_eth_promiscuous_enable(port_id);

          printf("Port %u,MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                  port_id,
                  g_glob.eth_addr[port_id].addr_bytes[0],
                  g_glob.eth_addr[port_id].addr_bytes[1],
                  g_glob.eth_addr[port_id].addr_bytes[2],
                  g_glob.eth_addr[port_id].addr_bytes[3],
                  g_glob.eth_addr[port_id].addr_bytes[4],
                  g_glob.eth_addr[port_id].addr_bytes[5]);
    }  // FOR_EACH_ETHDEV


}



void  rx_tx_adapter_setup_internal_port(void);
void  rx_tx_adapter_setup_internal_port(void)
{
	        struct rte_event_eth_rx_adapter_queue_conf eth_q_conf;
        uint8_t event_d_id = g_glob.event_d_id;   
        uint16_t adapter_id = 0;
        uint16_t nb_adapter = 0;
        uint16_t port_id;
        uint8_t q_id = 0;
        int ret;
WAI();

        memset(&eth_q_conf, 0, sizeof(eth_q_conf));
        eth_q_conf.ev.priority = RTE_EVENT_DEV_PRIORITY_NORMAL;

        RTE_ETH_FOREACH_DEV(port_id) {
                if ((g_glob.enabled_port_mask & (1 << port_id)) == 0)
                        continue;
                nb_adapter++;
        }

       g_glob.rx_adptr.nb_rx_adptr = nb_adapter;

        CALL_RTE("malloc()");
        g_glob.rx_adptr.rx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.rx_adptr.nb_rx_adptr);
        if (!g_glob.rx_adptr.rx_adptr) {
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_id);
                rte_panic("Failed to allocate memery for Rx adapter\n");
        }


        RTE_ETH_FOREACH_DEV(port_id) {
                if ((g_glob.enabled_port_mask & (1 << port_id)) == 0)
                        continue;

#ifdef PRINT_CALL_ARGUMENTS
               FONT_CYAN();
               printf(  " Call Args: adapter_id %d  event_d_id %d  rte_event_port_conf: \n",adapter_id,event_d_id);
               FONT_NORMAL();
#endif
                print_rte_event_port_conf(1,"---create--- def_p_conf",adapter_id,&g_glob.def_p_conf);
                CALL_RTE("rte_event_eth_rx_adapter_create()");
                ret = rte_event_eth_rx_adapter_create(adapter_id, event_d_id,
                                                &g_glob.def_p_conf);
                if (ret)
                        rte_panic("Failed to create rx adapter[%d]\n",
                                  adapter_id);


                /* Configure user requested sched type*/
                eth_q_conf.ev.sched_type = g_glob.sched_type;
                eth_q_conf.ev.queue_id =  g_glob.evq.event_q_id[q_id];

#ifdef PRINT_CALL_ARGUMENTS
               FONT_CYAN();
               printf(" Call Args: id:rx_adapter_id %d  eth_dev_id:port_id %d   rx_queue_id: %d  struct rte_event_eth_rx_adapter_queue_conf: \n",adapter_id,port_id,-1);
               FONT_NORMAL();
#endif
                print_rte_event_eth_rx_adapter_queue_conf(1,"--add-- eth_q_conf",adapter_id,&eth_q_conf);
                CALL_RTE("rte_event_eth_rx_adapter_queue_add()");
                ret = rte_event_eth_rx_adapter_queue_add(adapter_id, port_id,
                                                         -1, &eth_q_conf);
                if (ret)
                        rte_panic("Failed to add queues to Rx adapter\n");

                CALL_RTE("rte_event_eth_rx_adapter_start(uint8_t id)");
                ret = rte_event_eth_rx_adapter_start(adapter_id);
                if (ret)
                        rte_panic("Rx adapter[%d] start Failed\n", adapter_id);

                g_glob.rx_adptr.rx_adptr[adapter_id] = adapter_id;
                adapter_id++;
                if (q_id < g_glob.evq.nb_queues)
                        q_id++;
        }

        g_glob.tx_adptr.nb_tx_adptr = nb_adapter;
        CALL_RTE("malloc()");
        g_glob.tx_adptr.tx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.tx_adptr.nb_tx_adptr);
        if (!g_glob.tx_adptr.tx_adptr) {
                free(g_glob.rx_adptr.rx_adptr);
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_id);
                rte_panic("Failed to allocate memery for Rx adapter\n");
        }

        adapter_id = 0;
        RTE_ETH_FOREACH_DEV(port_id) {
                if ((g_glob.enabled_port_mask & (1 << port_id)) == 0)
                        continue;

                print_rte_event_port_conf(1,"---create--- def_p_conf",port_id,&g_glob.def_p_conf);
#ifdef PRINT_CALL_ARGUMENTS
                FONT_CYAN();
                printf(" Call Args: adpater_id: %d, event_d_id:%d, struct def_p_conf)\n",
                                        adapter_id,
                                        event_d_id );
                FONT_NORMAL();
#endif
                 print_rte_event_port_conf(1,"---create--- def_p_conf",adapter_id ,&g_glob.def_p_conf);

                CALL_RTE("rte_event_eth_tx_adapter_create()");
                ret = rte_event_eth_tx_adapter_create(adapter_id, event_d_id,
                                                &(g_glob.def_p_conf));
                if (ret)
                        rte_panic("Failed to create tx adapter[%d]\n",
                                  adapter_id);

#ifdef PRINT_CALL_ARGUMENTS
                FONT_CYAN();
                printf( " Call Args:  id:%d,uint16_t eth_dev_id:%d,int32_t queue:%d)\n",
                            adapter_id, port_id, -1);
                FONT_NORMAL();
#endif
                CALL_RTE("rte_event_eth_tx_adapter_queue_add()");
                ret = rte_event_eth_tx_adapter_queue_add(adapter_id, port_id,
                                                         -1);
                if (ret)
                        rte_panic("Failed to add queues to Tx adapter\n");
                CALL_RTE("rte_event_eth_tx_adapter_start()");
                ret = rte_event_eth_tx_adapter_start(adapter_id);
                if (ret)
                        rte_panic("Tx adapter[%d] start Failed\n", adapter_id);

                g_glob.tx_adptr.tx_adptr[adapter_id] = adapter_id;
                adapter_id++;
        }

}







int ethdev_setup( __attribute__((unused)) void * arg)
{
int     result;
unsigned int nb_lcores = rte_lcore_count();

uint8_t                       i;
uint8_t                       nb_event_dev_devices = 0;
struct  rte_event_dev_info    dev_info;
struct  rte_event_dev_config  event_dev_config = {0};


uint8_t                       event_q_id = 0;
struct  rte_event_queue_conf  def_q_conf;        // queue
struct  rte_event_queue_conf  event_q_conf; 
uint32_t event_queue_cfg = 0;

   WAI();

    g_glob.enabled_port_mask = 0x03 ;
    g_glob.nb_ports_available = 0;
    g_glob.event_d_id = 0;
    memset(&(g_glob.def_p_conf), 0, sizeof(struct rte_event_port_conf));
    g_glob.def_p_conf.dequeue_depth =1;        
    g_glob.def_p_conf.enqueue_depth =1;
    g_glob.def_p_conf.new_event_threshold = -1;
    g_glob.sched_type = RTE_SCHED_TYPE_ATOMIC ;   //RTE_SCHED_TYPE_ORDERED  RTE_SCHED_TYPE_PARALLEL



// INFO
   printf("rte_event_dev_count() = %d \n",rte_event_dev_count());
   printf("rte_eth_dev_count_avail() = %d \n",rte_eth_dev_count_avail());


/////////////////////  
/////////////////////  
/////////////////////  
/////////////////////  
//  Set up ethdev interfaces

    initialize_eth_dev_ports();
   


//uint8_t                       event_p_id = 0;
//struct  rte_event_port_conf
//struct  	rte_event_dev_xstats_name

//  https://doc.dpdk.org/api/rte__eventdev_8h.html#ac087670e6e885abcdf046c8e74844cd4 
// configure it, setup its queues, ports and start it), to

//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("spinlock_setup function %d\n",x);


   {
//////////
////////// check there is an event dev device available
//////////
//////////
        /*uint8_t 	rte_event_dev_count (void) */
        nb_event_dev_devices = rte_event_dev_count();
        if ( nb_event_dev_devices  == 0) 
        {
            printf(" no event dev devices found\n");
            printf(" did you forget to bind at least 1 to vfio-pci\n");
            rte_exit(EXIT_FAILURE, "No Event Devices available");   
        }
        WAI();
	printf(" Found %d event devices\n", nb_event_dev_devices);
 
//  debug -print out all event dev devices
//        int rte_event_dev_info_get (uint8_t dev_id, struct rte_event_dev_info *dev_info)
        for (i = 0 ; i < nb_event_dev_devices ; i++)
        {
           result = rte_event_dev_info_get(i,&dev_info);
           printf("  %d)  %s\n",i, dev_info.driver_name);
        }
//  My test- I expect just 1    
        if( nb_event_dev_devices != 1)
        {
            printf(" ** WARNING:: Number of  event dev devices found is not what is expected \n"); 
        }
        // WHY at least there is at least 1  device,  Index=0;
         g_glob.event_d_id = 0;     // assign dev_id  to 0


        g_nb_PortsPerCore = 1   ; 
        g_nb_QueuesPerCore = 1  ; // should this be queues per port?  Should this be = number of cores??
   
        printf(" g_glob.event_d_id:  %d \n", g_glob.event_d_id);
        printf(" nb_lcores:      %d \n",nb_lcores);
        printf(" g_nb_PortsPerCore:   %d \n",g_nb_PortsPerCore);
        printf(" g_nb_QueuesPerCore:  %d \n",g_nb_QueuesPerCore);
   }
 
   { 
//////////
//////////  rte_event_dev_configure()
//////////
//////////   Get default event_dev Setting and  Capabilities
        CALL_RTE("rte_event_dev_info_get()");
        result = rte_event_dev_info_get( g_glob.event_d_id , &dev_info );
        printf(" result    %d \n",result);
	printf(" default: dev_info \n");
        print_rte_event_dev_info( &dev_info );
    
   // check compatability - make sure queue will accept any type of RTE_SCHED_TYPE_* values: 
   //                                 ordered,parallel,atomic,ethdev,ccryptodev,timer,...
        if (dev_info.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
             event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;

     //////  rte_event_dev_configure()
    
        event_dev_config.dequeue_timeout_ns          = 0;                      // uint32_t     0 use Default values
        event_dev_config.nb_events_limit             = -1;                     // int32_t ??? I started with 100 and got errors, but l3fwd has -1, so copy for notw;
        event_dev_config.nb_event_queues             = (uint8_t)g_nb_QueuesPerCore * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_ports              = (uint8_t)g_nb_PortsPerCore  * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_queue_flows        = 1024;                  // uint32_t 
        event_dev_config.nb_event_port_dequeue_depth = 1;                     // uint32_t   cpy l3fwd->needs to be 1 because we do in hw?
        event_dev_config.nb_event_port_enqueue_depth = 1;                     // uint32_t cpy l3fwd->needs to be 1 because we do in hw?
//////    event_dev_config.event_dev_cfg           = dev_info.event_dev_cap;  // uint32_t 
        event_dev_config.event_dev_cfg               = 0;                     // uint32_t   again copy what l3fwd has to see if we get the same respons
    
    
#ifdef PRINT_CALL_ARGUMENTS
        FONT_CYAN();
        printf(  " Call Args: g_glob.event_d_id  %d  event_dev_config: \n",g_glob.event_d_id );
        FONT_NORMAL();
#endif
        print_rte_event_dev_config( 0, "config values for event_dev_config",0,&event_dev_config);   
    
        CALL_RTE("rte_event_dev_configure()"); 
        result = rte_event_dev_configure( g_glob.event_d_id, &event_dev_config);
        if(result < 0)
        {
             printf(" rte_event_dev_configure() returned %d\n",result);
             rte_panic("Error in configuring event device\n");
        }
    }

    {
//////
//////  rte_event_queue_setup()
//////
//////   Get default event_queue Setting and  Capabilities
        int ret;
#ifdef PRINT_CALL_ARGUMENTS
            FONT_CYAN();
            printf(  " Call Args: g_glob.event_d_id:%d  event_q_id:%d  def_q_conf: \n",g_glob.event_d_id,event_q_id );
            FONT_NORMAL();
#endif
            CALL_RTE("rte_event_queue_default_conf_get()");    
            rte_event_queue_default_conf_get( g_glob.event_d_id , event_q_id, &def_q_conf);
            
            printf(" default: event dev -> queue info event_q_id=%d \n",event_q_id);
            printf("      struct rte_event_queue_conf *\n") ; 
            printf("          uint32_t nb_atomic_flows              0x%08x \n", def_q_conf.nb_atomic_flows           );  //  	
            printf("          uint32_t nb_atomic_order_sequences    0x%08x \n", def_q_conf.nb_atomic_order_sequences );  //  	
            printf("          uint32_t event_queue_cfg              0x%08x \n", def_q_conf.event_queue_cfg           );  //  	
            printf("          uint8_t  schedule_type                0x%02x (%s)\n", def_q_conf.schedule_type
                                                                 ,StringSched[def_q_conf.schedule_type]             );  //  	
            printf("          uint8_t  priority                     0x%02x \n", def_q_conf.priority                  );  //  	
            
            event_q_conf.nb_atomic_flows           = 1024;
            event_q_conf.nb_atomic_order_sequences = 1024;
            event_q_conf.event_queue_cfg           = event_queue_cfg;
       //     event_q_conf.schedule_type             = RTE_EVENT_QUEUE_CFG_ALL_TYPES ;
            event_q_conf.priority                  = RTE_EVENT_DEV_PRIORITY_NORMAL ;
            
            if (def_q_conf.nb_atomic_flows < event_q_conf.nb_atomic_flows)
                event_q_conf.nb_atomic_flows = def_q_conf.nb_atomic_flows;
            
            if (def_q_conf.nb_atomic_order_sequences <
                            event_q_conf.nb_atomic_order_sequences)
                event_q_conf.nb_atomic_order_sequences =
                            def_q_conf.nb_atomic_order_sequences;
            
            event_q_conf.event_queue_cfg = event_queue_cfg;
            event_q_conf.schedule_type = g_glob.sched_type;
//            event_q_conf.schedule_type = RTE_SCHED_TYPE_ATOMIC ;  /*RTE_SCHED_TYPE_ATOMIC,RTE_SCHED_TYPE_ORDERED,RTE_SCHED_TYPE_PARALLEL */
            CALL_RTE("malloc()");
            g_glob.evq.event_q_id = (uint8_t *)malloc(sizeof(uint8_t) * g_glob.evq.nb_queues);

            if (!g_glob.evq.event_q_id)
                rte_panic("Memory allocation failure\n");
            
            for (event_q_id = 0; event_q_id < g_glob.evq.nb_queues;
                                        event_q_id++) {
#ifdef PRINT_CALL_ARGUMENTS
               FONT_CYAN();
               printf(  " Call Args: g_glob.event_d_id:%d  event_q_id:%d  event_q_conf:\n",g_glob.event_d_id,event_q_id );
               FONT_NORMAL();
#endif
                CALL_RTE("rte_event_queue_setup()");
                ret = rte_event_queue_setup(g_glob.event_d_id, event_q_id,
                                &event_q_conf);
                if (ret < 0)
                    rte_panic("Error in configuring event queue\n");
                g_glob.evq.event_q_id[event_q_id] = event_q_id;
           }  //  loop throug the queue Info. 

           // this is how I will configure each queue for this event dev:
            printf("***  Config: event queue config  -> event_q_conf *** \n");
            printf("         struct rte_event_queue_conf *\n") ; 
            printf("             uint32_t nb_atomic_flows              0x%08x \n", event_q_conf.nb_atomic_flows           );    	
            printf("             uint32_t nb_atomic_order_sequences    0x%08x \n", event_q_conf.nb_atomic_order_sequences );    	
            printf("             uint32_t event_queue_cfg              0x%08x \n", event_q_conf.event_queue_cfg           );    	
            printf("             uint8_t  schedule_type                0x%02x (%s)\n", event_q_conf.schedule_type
                                                                 ,StringSched[event_q_conf.schedule_type]             );    	
            printf("             uint8_t  priority                     0x%02x \n", event_q_conf.priority                  );    	


           // walk through the number event queues created as part of the event structure  
            for ( event_q_id= 0 ; event_q_id < event_dev_config.nb_event_queues ; event_q_id++)
           {
             // above I set teh number of queues and the number of ports = number of cores:
             //    So I should loop through each evnet_q_ID.
#ifdef PRINT_CALL_ARGUMENTS
                 FONT_CYAN();
                 printf(  " Call Args: g_glob.event_d_id:%d  event_q_id:%d  event_q_conf: \n",g_glob.event_d_id,event_q_id );
                 FONT_NORMAL();
#endif
                 print_rte_event_queue_conf( 1 ,"event_q_id",event_q_id, &event_q_conf);
                 CALL_RTE("call rte_event_queue_setup()");
                 ret = rte_event_queue_setup(g_glob.event_d_id , event_q_id,
                                &event_q_conf);
                if (ret < 0)
                    rte_panic("Error in configuring event queue\n");
//                evt_rsrc->evq.event_q_id[event_q_id] = event_q_id;
           }  //  loop throug the queue Info. */
       }


//////
//////  rte_event_port_setup()
//////
//////   Get default event_port Setting and  Capabilities
       {
            struct rte_event_port_conf event_p_conf;
            struct rte_event_port_conf def_p_conf;
            uint8_t event_p_id;
            int32_t ret;
       
           /* evt_rsrc->evp.event_p_id = (uint8_t *)malloc(sizeof(uint8_t) *
                            evt_rsrc->evp.nb_ports);
            if (!evt_rsrc->evp.event_p_id)
                rte_panic("Failed to allocate memory for Event Ports\n");
           */
      
            CALL_RTE("rte_event_port_default_conf_get()"); 
            rte_event_port_default_conf_get(g_glob.event_d_id, 0, &def_p_conf);

            printf(" default: event port config  -> def_p_conf \n");
            printf("      struct rte_event_port_conf *\n") ; 
            printf("          int32_t  new_event_threshold        0x%08x \n", def_p_conf.new_event_threshold      );    	
            printf("          uint16_t dequeue_depth              0x%04x \n", def_p_conf.dequeue_depth            );    	
            printf("          uint16_t enqueue_depth              0x%04x \n", def_p_conf.enqueue_depth            );    	
            printf("          uint8_t  disable_implicit_release   0x%02x \n", def_p_conf.disable_implicit_release );  

           // set up defaults for config
            event_p_conf.dequeue_depth = 32,
            event_p_conf.enqueue_depth = 32,
            event_p_conf.new_event_threshold = 4096;
 

     if (def_p_conf.new_event_threshold < event_p_conf.new_event_threshold)
         event_p_conf.new_event_threshold =
                         def_p_conf.new_event_threshold;

     if (def_p_conf.dequeue_depth < event_p_conf.dequeue_depth)
        event_p_conf.dequeue_depth = def_p_conf.dequeue_depth;

     if (def_p_conf.enqueue_depth < event_p_conf.enqueue_depth)
         event_p_conf.enqueue_depth = def_p_conf.enqueue_depth;

/*
113     event_p_conf.disable_implicit_release =
114         evt_rsrc->disable_implicit_release;
115
*/
       event_p_conf.disable_implicit_release = 0 ; // marvell extension  see:
                                                   //  "<dpdk>/lib/librte_eventdev/rte_eventdev.h
                                                     
            printf(" Config: event port config  -> event_p_conf \n");
            printf("      struct rte_event_port_conf *\n") ; 
            printf("          int32_t  new_event_threshold        0x%08x \n", event_p_conf.new_event_threshold      );    	
            printf("          uint16_t dequeue_depth              0x%04x \n", event_p_conf.dequeue_depth            );    	
            printf("          uint16_t enqueue_depth              0x%04x \n", event_p_conf.enqueue_depth            );    	
            printf("          uint8_t  disable_implicit_release   0x%02x \n", event_p_conf.disable_implicit_release );  

     for (event_p_id = 0; event_p_id < (g_nb_PortsPerCore * nb_lcores) ; event_p_id++) {

#ifdef PRINT_CALL_ARGUMENTS
         FONT_CYAN();
         printf(  " Call Args: g_glob.event_d__id:%d  event_p_id:%d  event_p_conf: \n",g_glob.event_d_id,event_p_id );
         FONT_NORMAL();
#endif
         print_rte_event_port_conf(1 , "event_p_conf", event_p_id, &event_p_conf);         
         CALL_RTE("rte_event_port_setup()");
         ret = rte_event_port_setup( g_glob.event_d_id , event_p_id,  &event_p_conf);
         if (ret < 0)
             rte_panic("Error in configuring event port %d\n",
                   event_p_id);


       }

//135     evt_rsrc->def_p_conf = event_p_conf;

     }

//////
//////  rte_event_port_link()
//////
//////  So we have n cores.   For this app, I want 1 core to have 1 port and 1 queue. 
/////                         we will use the Index to be the Index of the core??
    {
       int32_t ret;
       uint8_t event_p_id;
       uint8_t 	queues_array[32];
       uint8_t 	priorities_array[32];
       uint16_t nb_links = 1; 
       int      index;

        for (event_p_id = 0; event_p_id < (g_nb_PortsPerCore * nb_lcores) ; event_p_id++) 
        {
           queues_array[0] = event_p_id ;
           priorities_array[0] = RTE_EVENT_DEV_PRIORITY_NORMAL ;

           printf(" Linking Ports  event_d_id:%d, event_port_id:%d to event_queue_id %d \n",g_glob.event_d_id,event_p_id,event_p_id);
#ifdef PRINT_CALL_ARGUMENTS
          FONT_CYAN();

          printf(" Call Args: event_d_id:%d, event_p_id:%d   <queue array>  <priority array> nb_links%d \n",g_glob.event_d_id,event_p_id,nb_links);
          printf(  "     Index:     queue_array   prioritys_array \n" );
          printf(  "                             (high:%d  low:%d)  \n",RTE_EVENT_DEV_PRIORITY_HIGHEST, RTE_EVENT_DEV_PRIORITY_LOWEST );
          for ( index = 0 ; index < nb_links ; index++)
          printf(  "       %d             %d             %d \n",index, *(queues_array+index),*(priorities_array+index) );
          FONT_NORMAL();
 #endif
           CALL_RTE("rte_event_port_link()");
           ret = rte_event_port_link ( g_glob.event_d_id , event_p_id, queues_array, priorities_array, nb_links);
           if (ret != nb_links )
           {
              rte_panic("Error in rte_event_port_link() requested %d successfull %d\n",nb_links,ret);
           }
        }
    }

/////
/////
/////  Configure rte_event_eth_rx/tx_adapters
/////
/////
     rx_tx_adapter_setup_internal_port();






    {
        uint32_t  evdev_service_id = 0;
        int32_t ret;
        //  THis might be for SW version of event dev MGR???  But sure what this is and how it is used.
        CALL_RTE("rte_event_dev_service_id_get()");
        ret = rte_event_dev_service_id_get( g_glob.event_d_id,
                                &evdev_service_id);
        if (ret != -ESRCH && ret != 0) {
                printf("Error getting the service ID for sw eventdev\n");
                return -1;
        }
        printf(" service ID for event_dev (Device?) %d \n",evdev_service_id);
    }


//////
//////  rte_event_dev_start()
//////
//////  So we have n cores.   For this app, I want 1 core to have 1 port and 1 queue.
/////                         we will use the Index to be the Index of the core??
     {
        int32_t ret;
 
        printf(" %scall rte_event_dev_start() %s\n",C_GREEN,C_NORMAL);
        ret = rte_event_dev_start( g_glob.event_d_id );
        if (  ret  < 0)
        {
                printf(" Event Dev Device Start failed:  %d \n",ret);
                rte_exit(EXIT_FAILURE, "Error starting eventdev");
        }
        else
        {    
            printf(" Event Dev Device Started successfully \n");
        }
     }



     return 0;
}

extern void print_struct_rte_event( const char * string,struct rte_event *p);


#define LOCK_LOOPS (10*10)
#define BATCH_SIZE  4

char  ethdev_m0[] = { "Another"        };
char  ethdev_m1[] = { "Brick"         };
char  ethdev_m2[] = { "In the"       };
char  ethdev_m3[] = { "wall"};
char * ethdev_message[] = {ethdev_m0,ethdev_m1,ethdev_m2,ethdev_m3};


int ethdev_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
//    char string[256];
//    int  i;
    struct rte_event   ev;

    uint8_t dev_id ;
    uint8_t port_id ;
//    size_t sent = 0, received = 0;
   
    struct rte_event events[BATCH_SIZE];
    uint16_t nb_rx; 
    uint8_t  next_core[] = {1,2,3,0};  
    uint16_t ret;

    lcore_id = rte_lcore_id();
    dev_id = g_glob.event_d_id;
    port_id = lcore_id; 

    if( lcore_id == 0 )
    {

        printf("*** Put an event into the Event Dev Scheduler ***\n");

        printf("        dev_id: %d  \n ",dev_id);
        printf("        port_id: %d \n ",port_id);
        printf("        lcore_id: %d \n ",lcore_id);
      
        ev.event=0;    // set "event union" to 0
        ev.event_ptr  = (void *) (ethdev_message[lcore_id]) ;  // set the second 64 bits to point at a payload

// In order to get sso to deliver, I had to add the following to the WQE
        ev.flow_id        = 0xDEAD;                 // uint32_t flow_id:20;
        ev.sub_event_type = RTE_EVENT_TYPE_CPU ;    // uint32_t sub_event_type:8;
        ev.event_type     = RTE_EVENT_TYPE_CPU ;    // uint32_t event_type:4;
        ev.op             = RTE_EVENT_OP_NEW;         // uint8_t op:2; NEW,FORWARD or RELEASE
        //ev.rsvd           =  ;                      // uint8_t rsvd:4;
        ev.sched_type     = RTE_SCHED_TYPE_PARALLEL ;       // uint8_t  sched_type:2;
        ev.queue_id       = 1 ;                             // uint8_t  queue_id;
        ev.priority       = RTE_EVENT_DEV_PRIORITY_NORMAL ; // uint8_t  priority;
        //ev.impl_opaque    =  ; // uint8_t  impl_opaque;

        print_struct_rte_event( "ev",&ev);

        ret = rte_event_enqueue_burst(dev_id, lcore_id,&ev, 1 );
        if( ret != 1 )
        {
             printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
             printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
             rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
        }

   }


// struct worker_data *data = (struct worker_data *)arg;

    printf(" Launch code on Core: %d\n",lcore_id);
    while (!g_force_quit)
    {
       // lots of handwaving here.  there should be only one event 
       //    put in before the Event loop
 
        nb_rx = rte_event_dequeue_burst(dev_id, port_id,
                                events, RTE_DIM(events), 0);

        if (nb_rx == 0) 
        {
            rte_pause();
            continue;
        }
        printf("****    c%d) Received %d events **** \n",lcore_id,nb_rx);
        print_struct_rte_event( "event[0]",&events[0]);
        printf(" Message:   %s\n",(char *)events[0].event_ptr);

        printf("  c%d) Send message to next Port (aka core for me)  %d events\n",lcore_id,next_core[lcore_id]);

        // set to forward to next core.
        events[0].queue_id       = next_core[lcore_id];
        // set operation to forward packet    
        //  events[0].op             = RTE_EVENT_OP_FORWARD;
        events[0].flow_id        += 1;
        events[0].event_ptr  = (void *)ethdev_message[lcore_id]; 

 
        rte_pause();
        usleep(1000000);     
 
        ret = rte_event_enqueue_burst(dev_id, port_id,
                                events, nb_rx);
        if( ret != 1 )
        {
             printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
             printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
             rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
        }

//        if (lcore_id == 3) g_force_quit = true;

    }
    return 0;
}

int ethdev_print(__attribute__((unused)) void * arg)
{
//    int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy print function %d\n",x);

//    unsigned lcore_id;
//    lcore_id = rte_lcore_id();
    rte_spinlock_lock( &g_fs_print_lock);

    // put your print statements here.

    rte_spinlock_unlock( &g_fs_print_lock);

     return 0;
}

int ethdev_cleanup(__attribute__((unused)) void * arg)
{
     // print the results from each core

     rte_event_dev_dump( g_glob.event_d_id  ,stdout);


// Stop the device

    printf("************\n    stopping the event_dev device  sso   \n***************\n");
     rte_event_dev_stop (g_glob.event_d_id);

//   CLose the event dev device




/* int rte_event_dev_close (   uint8_t     dev_id  )   
Close an event device. The device cannot be restarted! */
    printf("************\n    closing the event_dev device  sso   \n***************\n");
     rte_event_dev_close( g_glob.event_d_id );


/* clear ethdev resourcesi*/
/* free the mbuff pool */
#if 0
    USED L2FWD_EVENT as example and this buffer is never freed
    printf("************\n    free teh pktmbuf_pool  event_dev device  sso   \n***************\n");
          rte_pktmbuf_free(g_p_pktmbuf_pool);
#endif
     return 0;
}

void  ethdev_description(void)
{
    printf(" \"ethdev\" - try to set up event mode with event_ethdev to pass traffic\n");
    printf("                in and out of board.  Later add RSS to spread acroos \n");
    printf("                mulitple cores \n");
}



struct test_mode_struct  tm_ethdev = {
      .setup         = ethdev_setup,
      .main_loop     = ethdev_loop,
      .print_results = ethdev_print,
      .cleanup       = ethdev_cleanup,
      .description   = ethdev_description,
};



