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
#include <rte_security.h>   // for rte_security_session_get_size()

#include <rte_spinlock.h>
#include "fprintbuff.h"
#include "fs_print_rte_mbuff.h"
#include "fs_extras.h"
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
#include "fs_net_common.h"
#include "fs_ethdev.h"

#include <rte_event_timer_adapter.h>   // for event timer

#include "fs_spinlock_test.h"
//#include "fs_print_structures.c"
// eth dev structures

#include "fs_extras.h"
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>
#include "fs_eventdev.h"
#include "fs_print_rte_ethdev_struct.h"
#include "fs_core.h"
#include "fs_eventdev_timer.h"
#include "fprintbuff.h"

#include "fs_global_vars.h"

//#include "fs_print_rte_structures.h"

extern uint64_t  g_core_messages;
extern int64_t g_print_interval;


extern int g_drop_all_traffic ;
uint32_t  g_IP_eth0 =  0xc0a80364 ;    // 192.168.3.100
MacAddr_t g_Mac_eth0 ={.addr[0]=0x00,
                       .addr[1]=0x0f,
                       .addr[2]=0xb7,
                       .addr[3]=0x06,
                       .addr[4]=0x10,
                       .addr[5]=0xf3};


void print_eth_setup(void);
void print_eth_setup(void)
{
printf("\n");
printf("%s\n",__FILE__);
printf("\n"
"  ---------------          ------------                                     \n"
"  |eth port_id 0| -------  |evt_que 0 | --                                  \n"
"  ---------------          ------------   \\     ------------     --------- \n"
"                                             >   | evt_prt 0| --> | core 0| \n" 
"                           ------------   /      ------------     --------- \n"
"                           |evt_que 1 | --                                  \n"
"                           ------------                                     \n"
"                                                                            \n"
"                                                                            \n"
"  ---------------          ------------                                     \n"
"  |eth port_id 1| -------  |evt_que 2 | --                                  \n"
"  ---------------          ------------   \\     ------------     --------- \n"
"                                             >   | evt_prt 1| --> | core 1| \n" 
"                           ------------   /      ------------     --------- \n"
"                           |evt_que 3 | --                                  \n"
"                           ------------                                     \n"
"                                                                            \n"
"                                                                            \n"
"                           ------------                                     \n"
"                           |evt_que 4 | --                                  \n"
"                           ------------   \\     ------------     --------- \n"
"                                             >   | evt_prt 2| --> | core 2| \n" 
"                           ------------   /      ------------     --------- \n"
"                           |evt_que 5 | --                                  \n"
"                           ------------                                     \n"
"                                                                            \n"
"                                                                            \n"
"                           ------------                                     \n"
"                           |evt_que 6 | --                                  \n"
"                           ------------   \\     ------------     --------- \n"
"                                             >   | evt_prt 3| --> | core 4| \n" 
"                           ------------   /      ------------     --------- \n"
"                           |evt_que 7 | --                                  \n"
"                           ------------                                     \n"
"                                                                            \n"
"  odd queues have higher priority \n"
"                         \n"
"\n");
}


#if 0
 in the future, I want to add different rte_flows and crypto and ....

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


//  forward reference for compiler
int        ethdev_setup( __attribute__((unused))void * arg);
int         ethdev_loop( __attribute__((unused))void * arg);
int        ethdev_print( __attribute__((unused))void * arg);
int      ethdev_cleanup( __attribute__((unused))void * arg);
void ethdev_description( void);


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//  Setup for test_app

int ethdev_setup( __attribute__((unused)) void * arg)
{

    WAI();
   print_eth_setup(); 

///////
/// Eth Dev Config info
//////

    g_glob.enabled_eth_port_mask = 0x03 ;            // cmd line -p argument - here I hardwired :-0    

   // number of rx and tx ports per queue
   // g_glob.eth_port_conf[port_id].nb_rx_queues=1 ;
    g_glob.eth_port_cfg_data[0].nb_rx_queues=1 ;    
    g_glob.eth_port_cfg_data[0].nb_tx_queues=1 ;   
    g_glob.eth_port_cfg_data[1].nb_rx_queues=1 ;    
    g_glob.eth_port_cfg_data[1].nb_tx_queues=1 ;   



///////
/// Event Dev Config info
//////

    g_glob.event_dev_id = 0;                         // event dev_id index/handle => SSO  0

    memset(&(g_glob.def_p_conf), 0, sizeof(struct rte_event_port_conf));  
         g_glob.def_p_conf.dequeue_depth =1;        
         g_glob.def_p_conf.enqueue_depth =1;
         g_glob.def_p_conf.new_event_threshold = -1;

    // event queues & ports
    g_glob.evq.nb_queues = 8 ;  // total number of event queues in my design
    g_glob.evp.nb_ports  = 4 ;  // total number of event ports in my design.  


    // adapters rx 
    g_glob.rx_adptr.nb_rx_adptr     = 1 ;  // total number of rx_adapters in my design
    g_glob.rx_adptr.nb_rx_adptr_add = 2 ;  // total number of rx_adapter_adds



    // adapters tx 
    g_glob.tx_adptr.nb_tx_adptr = 1 ;      // total number of tx_adapters in my design.  
    g_glob.tx_adptr.nb_tx_adptr_add = 2 ;  // total number of rx_adapter_adds



    // event dev queue to event dev port map
    
    // allocate storage arrays based on evp and evq , rx_adapters, and tx_adaptersabove.
     
/*  EVENT_QUEUE - memory allocate */
    g_glob.evq.event_q_cfg = (event_queue_cfg_t *)malloc(sizeof(event_queue_cfg_t) * g_glob.evq.nb_queues);
    if (!g_glob.evq.event_q_cfg) {
                rte_panic("Memory allocation failure \n");
    }
    memset( g_glob.evq.event_q_cfg , 0 , (sizeof(event_queue_cfg_t) * g_glob.evq.nb_queues));


/*  EVENT_PORTS - memory allocate */
    g_glob.evp.event_p_id = (q_id_and_priority_t *)malloc(sizeof(q_id_and_priority_t) * g_glob.evp.nb_ports);
    if (!g_glob.evp.event_p_id) {
                free( g_glob.evq.event_q_cfg);
                rte_panic("Memory allocation failure \n");
    }
    memset( g_glob.evp.event_p_id , 0 , (sizeof(q_id_and_priority_t) * g_glob.evp.nb_ports));

/*  RX_ADAPTERS - memory allocate */
    g_glob.rx_adptr.rx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.rx_adptr.nb_rx_adptr);
    if (!g_glob.rx_adptr.rx_adptr) {
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_cfg);
                rte_panic("Failed to allocate memery for Rx adapter\n");
    }

    g_glob.rx_adptr.rx_adptr_add = (event_rx_adapter_add_t *)malloc(sizeof(event_rx_adapter_add_t) *
                                        g_glob.rx_adptr.nb_rx_adptr_add);
    if (!g_glob.rx_adptr.rx_adptr_add) {
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_cfg);
                free(g_glob.rx_adptr.rx_adptr);
                rte_panic("Failed to allocate memery for Rx adapter\n");
    }

/*  TX_ADAPTERS - memory allocate */
    g_glob.tx_adptr.tx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.tx_adptr.nb_tx_adptr);
    if (!g_glob.tx_adptr.tx_adptr) {
                free(g_glob.evq.event_q_cfg);
                free(g_glob.evp.event_p_id);
                free(g_glob.rx_adptr.rx_adptr);
                free(g_glob.rx_adptr.rx_adptr_add);
                rte_panic("Failed to allocate memery for Rx adapter\n");
    }

    g_glob.tx_adptr.tx_adptr_add = (event_tx_adapter_add_t *)malloc(sizeof(event_tx_adapter_add_t) *
                                        g_glob.tx_adptr.nb_tx_adptr_add);
    if (!g_glob.tx_adptr.tx_adptr_add) {
                free(g_glob.evq.event_q_cfg);
                free(g_glob.evp.event_p_id);
                free(g_glob.rx_adptr.rx_adptr);
                free(g_glob.rx_adptr.rx_adptr_add);
                free(g_glob.tx_adptr.tx_adptr);
                rte_panic("Failed to allocate memery for Rx adapter\n");
     } 


////////////////////////////////////////
// now map the queues, ports and adapters.
//    - set prioritites for the queues.


{
    event_queue_cfg_t *ptr = g_glob.evq.event_q_cfg;

// queue 0
    ( ptr + 0 )->event_q_id    = 0;         // event queue index
    ( ptr + 0 )->to_event_port = 0;         // event port this queue feeds
    ( ptr + 0 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 0 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 0 )->ev_q_conf.priority = 0x80;

// queue 1
    ( ptr + 1 )->event_q_id    = 1;         // event queue index
    ( ptr + 1 )->to_event_port = 0;         // event port this queue feeds
    ( ptr + 1 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 1 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 1 )->ev_q_conf.priority = 0x40;

// queue 2
    ( ptr + 2 )->event_q_id    = 2;         // event queue index
    ( ptr + 2 )->to_event_port = 1;         // event port this queue feeds
    ( ptr + 2 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 2 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 2 )->ev_q_conf.priority = 0x80;

// queue 3
    ( ptr + 3 )->event_q_id    = 3;         // event queue index
    ( ptr + 3 )->to_event_port = 1;         // event port this queue feeds
    ( ptr + 3 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 3 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 3 )->ev_q_conf.priority = 0x40;

// queue 4
    ( ptr + 4 )->event_q_id    = 4;         // event queue index
    ( ptr + 4 )->to_event_port = 2;         // event port this queue feeds
    ( ptr + 4 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 4 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 4 )->ev_q_conf.priority = 0x80;

// queue 5
    ( ptr + 5 )->event_q_id    = 5;         // event queue index
    ( ptr + 5 )->to_event_port = 2;         // event port this queue feeds
    ( ptr + 5 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 5 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 5 )->ev_q_conf.priority = 0x40;

// queue 6
    ( ptr + 6 )->event_q_id    = 6;         // event queue index
    ( ptr + 6 )->to_event_port = 3;         // event port this queue feeds
    ( ptr + 6 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 6 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 6 )->ev_q_conf.priority = 0x80;

// queue 7
    ( ptr + 7 )->event_q_id    = 7;         // event queue index
    ( ptr + 7 )->to_event_port = 3;         // event port this queue feeds
    ( ptr + 7 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
    ( ptr + 7 )->ev_q_conf.schedule_type = 0;  /*  dont care when   
                                            event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
    ( ptr + 7 )->ev_q_conf.priority = 0x40;
}




// now link the event_queues to the event ports
//  to do this...   
//       the link command is port based and wants an array of
//       queues and the queue priority        


 // port 0  - two queues (0 and 1), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 0)->nb_links = 2;
   (g_glob.evp.event_p_id + 0)->q_id[0] = 0;
   (g_glob.evp.event_p_id + 0)->pri[0]  = 80;
   (g_glob.evp.event_p_id + 0)->q_id[1] = 1;
   (g_glob.evp.event_p_id + 0)->pri[1]  = 40;
 
 // port 1  - two queues (2 and 3), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 1)->nb_links = 2;
   (g_glob.evp.event_p_id + 1)->q_id[0] = 2;
   (g_glob.evp.event_p_id + 1)->pri[0]  = 80;
   (g_glob.evp.event_p_id + 1)->q_id[1] = 3;
   (g_glob.evp.event_p_id + 1)->pri[1]  = 40;

  // port 2  - two queues (4 and 5), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 2)->nb_links = 2;
   (g_glob.evp.event_p_id + 2)->q_id[0] = 4;
   (g_glob.evp.event_p_id + 2)->pri[0]  = 80;
   (g_glob.evp.event_p_id + 2)->q_id[1] = 5;
   (g_glob.evp.event_p_id + 2)->pri[1]  = 40;

  // port 3  - two queues (0 and 1), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 3)->nb_links = 2;
   (g_glob.evp.event_p_id + 3)->q_id[0] = 6;
   (g_glob.evp.event_p_id + 3)->pri[0]  = 80;
   (g_glob.evp.event_p_id + 3)->q_id[1] = 7;
   (g_glob.evp.event_p_id + 3)->pri[1]  = 40;
 

//  You should only really need one adapter.  

/* I am going to cheese out here on the initialization code.  It will look like this..
   not efficient, but easier to manage global data structure mallocs

   for ( i = 0 ; i < nb_xx_adptr ;i++)

         rte_event_eth_rx_adapter_create()
         for( j = 0 ; j < nb_rx_adapter_add; j++)
         {
            if(rx_adapter[i] == rx_adapter_add[j].adapter_id
            {
                setup and call
                rte_event_eth_rx_adapter_add()
            }
         }
     }
*/

//  
    g_glob.rx_adptr.rx_adptr[0]  = 0 ;     // 

// rx_adapter 0  connects eth dev 0/port 0 to event queue 0
    g_glob.rx_adptr.rx_adptr_add[0].adapter_id = 0 ;     // 
    g_glob.rx_adptr.rx_adptr_add[0].eth_dev_port = 0 ;   // 
    g_glob.rx_adptr.rx_adptr_add[0].eth_dev_queue = 0 ;  // 
    g_glob.rx_adptr.rx_adptr_add[0].event_dev_queue = 0; // 
    g_glob.rx_adptr.rx_adptr_add[0].sched_type = RTE_SCHED_TYPE_ORDERED;        // 
    g_glob.rx_adptr.rx_adptr_add[0].priority = 0x80;        // 


// rx_adapter 0  connects eth dev 1/port 0 to event queue 2
    g_glob.rx_adptr.rx_adptr_add[1].adapter_id = 0 ;     // 
    g_glob.rx_adptr.rx_adptr_add[1].eth_dev_port = 1 ;   // 
    g_glob.rx_adptr.rx_adptr_add[1].eth_dev_queue = 0 ;  // 
    g_glob.rx_adptr.rx_adptr_add[1].event_dev_queue = 2; // 
    g_glob.rx_adptr.rx_adptr_add[1].sched_type = RTE_SCHED_TYPE_ORDERED;        // 
    g_glob.rx_adptr.rx_adptr_add[1].priority = 0x80;        // 

#if 0
//   example of using to rx_adapters as opposed to using 1
    g_glob.rx_adptr.rx_adptr[1]  = 1 ;     // 

// rx_adapter 0  connects eth dev 0/port 0 to event queue 0
    g_glob.rx_adptr.rx_adptr_add[3].adapter_id = 1 ;     // 
    g_glob.rx_adptr.rx_adptr_add[3].eth_dev_port = 2 ;   // 
    g_glob.rx_adptr.rx_adptr_add[3].eth_dev_queue = 0 ;  // 
    g_glob.rx_adptr.rx_adptr_add[3].event_dev_queue = 4; // 
    g_glob.rx_adptr.rx_adptr_add[3].sched_type = RTE_SCHED_TYPE_ORDERED;        // 
    g_glob.rx_adptr.rx_adptr_add[3].priority = 0x80;        // 

// rx_adapter 0  connects eth dev 1/port 0 to event queue 2
    g_glob.rx_adptr.rx_adptr_add[4].adapter_id = 0 ;     // 
    g_glob.rx_adptr.rx_adptr_add[4].eth_dev_port = 3 ;   // 
    g_glob.rx_adptr.rx_adptr_add[4].eth_dev_queue = 0 ;  // 
    g_glob.rx_adptr.rx_adptr_add[4].event_dev_queue = 6; // 
    g_glob.rx_adptr.rx_adptr_add[4].sched_type = RTE_SCHED_TYPE_ORDERED;        // 
#endif


//    g_glob.tx_adptr.nb_tx_adptr_adds = 2;

    g_glob.tx_adptr.tx_adptr[0]  = 0 ; //  .

// rx_adapter 0  connects eth dev 0/port 0 to event queue 0
    g_glob.tx_adptr.tx_adptr_add[0].adapter_id = 0 ;     // 
    g_glob.tx_adptr.tx_adptr_add[0].eth_dev_port = 0 ;   // 
    g_glob.tx_adptr.tx_adptr_add[0].eth_dev_queue = 0 ;  // 

// rx_adapter 0  connects eth dev 1/port 0 to event queue 2
    g_glob.tx_adptr.tx_adptr_add[1].adapter_id = 0 ;     // 
    g_glob.tx_adptr.tx_adptr_add[1].eth_dev_port = 1 ;   // 
    g_glob.tx_adptr.tx_adptr_add[1].eth_dev_queue = 0 ;  // 




// INFO
   printf("rte_event_dev_count() = %d \n",rte_event_dev_count());
   printf("rte_eth_dev_count_avail() = %d \n",rte_eth_dev_count_avail());

/////////////////////  
/////////////////////  
/////////////////////  
/////////////////////  
//  Set up ethdev interfaces
//    attache to and configure Etherenet Devices
//    create tx queue
//    create tx queue
//    enable promiscuous mode

    initialize_eth_dev_ports();


/////////////////////  
/////////////////////  
/////////////////////  
/////////////////////  
//  Set up Event Dev  device. 
//    attache to the event dev device    rte_event_dev_configure()
//    create queue                       rte_event_queue_setup()
//    create ports                       rte_event_port_setup()
//    connect queues to ports.           rte_event_port_link()
 
    Initialize_EventDev();

/////
/////
/////  Start event_dev subsystem 
/////
/////

    start_event_dev();

/////////////////////  
/////////////////////  
/////////////////////  
/////////////////////  
//  connect "event dev"  to "ethernet devices" 
//    create rx_adapters    
//    associate adapter to eth queue and event queue
//    create tx_adapters    
//    associate adapter to eth queue and event queue
//
//  per dpdk docs (40.1.5. Starting the Adapter Instance)
//      event dev should be started before starting adapterss

 
    print_global_data(&g_glob);

    rx_tx_adapter_setup_internal_port();;


/////
/////
/////  Start the ethernet interfaces
/////
/////
     ethdev_start();

#if  1 
    {
    //     uint32_t service_id;
         uint32_t caps,i,ret ;
         for(i = 0 ; i < g_glob.rx_adptr.nb_rx_adptr ; i++)
         {
             CALL_RTE("rte_event_eth_rx_adapter_caps_get()");
             ret = rte_event_eth_rx_adapter_caps_get(g_glob.event_dev_id,
                                      g_glob.rx_adptr.rx_adptr[i], &caps);
             if( ret != 0)
             {
                  printf("ERR: rte_event_eth_rx_adapter_caps_get returned %d\n",ret);
             }
             printf("rte_event_eth_rx_adapter_caps: id:%d   caps:  0x%08x \n",
                                                     g_glob.rx_adptr.rx_adptr[i], 
                                                                         caps );
         }
         for(i = 0 ; i < g_glob.tx_adptr.nb_tx_adptr ; i++)
         {
 
             CALL_RTE("rte_event_eth_tx_adapter_caps_get()");
             ret = rte_event_eth_tx_adapter_caps_get(g_glob.event_dev_id,
                                      g_glob.tx_adptr.tx_adptr[i], &caps);
             if( ret != 0)
             {
                  printf("ERR: rte_event_eth_tx_adapter_caps_get returned %d\n",ret);
             }

             printf("rte_event_eth_tx_adapter_caps: id:%d   caps: 0x%08x \n",
                                                     g_glob.tx_adptr.tx_adptr[i], 
                                                                            caps);
          }
    }

#endif

/////
/////
/////  Configer event_timer  hw.   
/////
/////
#define TIMER_ADAPTER
#ifdef TIMER_ADAPTER

#define TIMER_EVENT_QUEUE  1
      timer_event_init();
      timer_event_start();
#endif


     return 0;
}





//  Each test function "init"  defines the number of ethernet devices
//    this function displays all the interfaces DPDK sees and make sure
//    the number of ethdev is consistent with what the "test function"
//    expects.
//   We expect the "INIT" functions have configuation information which
//     is consistent with dpdk.
void eth_dev_check_config(void);
void eth_dev_check_config(void)
{

    int16_t  eth_port_id;
    int16_t  nb_ports = 0;
    char     string[64];
    uint64_t i;

    WAI();
    g_glob.nb_eth_ports_available = 0;

    printf("rte_eal_init() thinks I have the following ethdev device(s)\n");

    // print eth port ids  that rte_init sees
    RTE_ETH_FOREACH_DEV(eth_port_id) 
    {
        /* get (-ENODEV) or (-EINVAL) if port_id is invalid. */
        if ( rte_eth_dev_get_name_by_port( eth_port_id , string) == 0)
        {
            printf("  ----> portid 0x%x enabled PCIe:  %s \n",eth_port_id,string );
            nb_ports++;
        }
    }

    // Now check what the program initialization wants for a port mask. 
    for ( i = 0 ; i < (8*sizeof(uint16_t)) ; i++ )   // size of g_glob.enabled_port_mask
    {
       if ( (g_glob.enabled_eth_port_mask  & (0x01 << i))  != 0) 
       { 
          g_glob.nb_eth_ports_available++; 
          printf(".");
       }
    }
    printf("\n");
    if( g_glob.nb_eth_ports_available  > nb_ports )
    {
         printf("ERR: rte_eth_dev %d    g_glob.nb_eth_ports_available=%d   \n",nb_ports,g_glob.nb_eth_ports_available);
         rte_exit(EXIT_FAILURE, "Expected eth ports does not match available eth ports");
    }



    printf("  g_glob.enabled_eth_port_mask   (0x%04x)  \n"
           "  g_glob.nb_eth_ports_available  %d \n"
           "  nb_ports                       %d\n",
                                            g_glob.enabled_eth_port_mask,
                                            g_glob.nb_eth_ports_available,
                                            nb_ports );

    printf("  port_id    PCIe address    nb_rx_queues    nb_tx_queues\n ");
   // check that the Number of Rx and Tx queues are greater than 0. (missed in initialization??)
   for( i = 0 ; i <  g_glob.nb_eth_ports_available ; i++)
   {
       rte_eth_dev_get_name_by_port( eth_port_id , string);
       printf("  %ld      %s   %d     %d \n ", i, string, 
                                          g_glob.eth_port_cfg_data[0].nb_rx_queues,
                                          g_glob.eth_port_cfg_data[0].nb_tx_queues );
       if ( ( g_glob.eth_port_cfg_data[0].nb_rx_queues == 0 ) ||
            ( g_glob.eth_port_cfg_data[0].nb_rx_queues == 0 ))
       {
                printf(" -- ERROR - Eth port defined with qty 0 rx or tx queues \n ");
               rte_exit(EXIT_FAILURE, "Eth port %ld defined with qty 0 rx or tx queues \n",i);
       }
   }


}



  


// initialize each ethernet interface
//      rte_eth_dev_info_get()             Get Default cfg from dpdk
//      Adjust default config to what we need.
//      rte_eth_dev_configure()            Create the interface - based on cfg
//      rte_eth_dev_adjust_nb_rx_tx_desc() adjust the number of descriptors
//      rte_eth_macaddr_get()              ---
//      rte_eth_rx_queue_setup()           allocate rx queues
//      rte_eth_tx_queue_setup()           allocate tx Queues
//      prte_eth_promiscuous_enable()      put interface in Promiscuous mode.
void   initialize_eth_dev_ports(void)
{         
    uint16_t port_id;
    int ret;
    uint16_t nb_rxd = RTE_RX_DESC_DEFAULT;
    uint16_t nb_txd = RTE_TX_DESC_DEFAULT;
    uint32_t nb_mbufs;
    uint16_t nb_ports = 0;
    int      i;

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
    
    // count the number of ports, and print out what rte_eal thinks we have. 
    eth_dev_check_config();
    nb_ports = g_glob.nb_eth_ports_available;


// for event mode 
// note the RSS reference    
    port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
    port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
    port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP;

    print_rte_eth_conf(0,"default: port_conf",0,&port_conf);    

//   allocate mbuffs for the eth-dev interface HW.
//     first, how many ports, and confirm against eal_init


    // allocate memory
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
        

///////////////////////////////////////
///////////////////////////////////////
    RTE_ETH_FOREACH_DEV(port_id) {
         struct rte_eth_conf local_port_conf = port_conf;
         struct rte_eth_dev_info dev_info;
         struct rte_eth_rxconf rxq_conf;
         struct rte_eth_txconf txq_conf;

          /* skip ports that are not enabled */
          if ((g_glob.enabled_eth_port_mask & (1 << port_id)) == 0) {
                  printf("Skipping disabled port %u\n", port_id);
                  continue;
          }

         /* init port */
          printf("%sInitializing port %u... %s\n",C_RED, port_id,C_WHITE);
          printf ("%s %s  portid: %d %s\n",C_RED,__FUNCTION__,port_id,C_WHITE);
          fflush(stdout);

#ifdef PRINT_CALL_ARGUMENTS
               FONT_CALL_ARGUMENTS_COLOR();
               printf(  " Call Args: port_id %d  dev_info: \n",port_id);
               FONT_NORMAL();
#endif
          CALL_RTE("rte_eth_dev_info_get()");    
          ret = rte_eth_dev_info_get(port_id, &dev_info);
          if (ret != 0)
                  rte_panic("Error during getting device (port %u) info: %s\n",
                            port_id, strerror(-ret));

// check for rss support on this eth port
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
          FONT_CALL_ARGUMENTS_COLOR();
          printf(  " Call Args: port_id %d  rx_queuess:%d  tx_queues %d  local_port_conf: \n",
                                port_id,
                                 g_glob.eth_port_cfg_data[port_id].nb_rx_queues,
                                 g_glob.eth_port_cfg_data[port_id].nb_tx_queues);
          FONT_NORMAL();
#endif
          CALL_RTE("rte_eth_dev_configure()");    
          ret = rte_eth_dev_configure(port_id, 
                                      g_glob.eth_port_cfg_data[port_id].nb_rx_queues, 
                                      g_glob.eth_port_cfg_data[port_id].nb_tx_queues, 
                                      &local_port_conf);
          if (ret < 0)
                  rte_panic("Cannot configure device: err=%d, port=%u\n",
                            ret, port_id);

 #ifdef PRINT_CALL_ARGUMENTS
          FONT_CALL_ARGUMENTS_COLOR();
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
          rte_eth_macaddr_get(port_id, &g_glob.eth_port_cfg_data[port_id].eth_addr);


// FIX THIS LATER --  will need more than 1 queue on Rx and 1 queue on tx.
// Fixed??
          /*******************************************/ 
          /****  init one RX queue on each port   ****/
          /*******************************************/ 
          fflush(stdout);
          printf("port_id = %d\n",port_id);
          printf("nb_rxd = %d\n",nb_rxd);
          printf("rte_eth_dev_socket_id(port_id) = %d\n",rte_eth_dev_socket_id(port_id));
          rxq_conf = dev_info.default_rxconf;
          rxq_conf.offloads = local_port_conf.rxmode.offloads;

          for( i = 0 ; i < g_glob.eth_port_cfg_data[port_id].nb_rx_queues ; i++)
          {
#ifdef PRINT_CALL_ARGUMENTS
              FONT_CALL_ARGUMENTS_COLOR();
              printf(" int rte_eth_rx_queue_setup( uint16_t port_id, \n"
                     "                             uint16_t rx_queue_id,\n"
                     "                             uint16_t 	nb_rx_desc,\n"
                     "                             unsigned int 	socket_id,\n"
                     "                             const struct rte_eth_rxconf * 	rx_conf,\n"
                     "                             struct rte_mempool * 	mb_pool \n"
                     "                             )\n");
              printf( " Call Args: port_id %d  rx_queue_id:%d  nb_rx_desc:%d  socketid:%d  rx_conf:   rte_mempool* \n",
                                     port_id,           i,      nb_rxd,  rte_eth_dev_socket_id(port_id));
              FONT_NORMAL();
              print_rte_eth_rxconf( 0, &rxq_conf);
#endif
              CALL_RTE("rte_eth_rx_queue_setup()");    
              ret = rte_eth_rx_queue_setup(port_id, i , nb_rxd,
                                            rte_eth_dev_socket_id(port_id),
                                            &rxq_conf,
                                            g_glob.p_pktmbuf_pool);
              if (ret < 0)
                    rte_panic("rte_eth_rx_queue_setup:err=%d, port=%u\n",
                              ret, port_id);
          }

          /**********************************/ 
          /* init one TX queue on each port */
          /**********************************/ 
          fflush(stdout);
          txq_conf = dev_info.default_txconf;
          txq_conf.offloads = local_port_conf.txmode.offloads;

          print_rte_eth_txconf( 0, &txq_conf);

          for( i = 0 ; i < g_glob.eth_port_cfg_data[port_id].nb_tx_queues ; i++)
          {
              CALL_RTE("rte_eth_tx_queue_setup()");    
              ret = rte_eth_tx_queue_setup(port_id, i , nb_txd,
                              rte_eth_dev_socket_id(port_id),
                              &txq_conf);
              if (ret < 0)
                    rte_panic("rte_eth_tx_queue_setup:err=%d, port=%u\n",
                            ret, port_id);
          }

          CALL_RTE("rte_eth_promiscuous_enable()\n");    
          rte_eth_promiscuous_enable(port_id);

          printf("Port %u,MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                  port_id,
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[0],
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[1],
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[2],
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[3],
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[4],
                  g_glob.eth_port_cfg_data[port_id].eth_addr.addr_bytes[5]);

    }  // FOR_EACH_ETHDEV

}


//////////////////////////////////////////////
//////////////////////////////////////////////
//  Start all the ethernet interfaces.
void ethdev_start(void)
{
    uint16_t port_id;
    int ret = 0;

    RTE_ETH_FOREACH_DEV(port_id) {
         /* skip ports that are not enabled */
         if ((g_glob.enabled_eth_port_mask & (1 << port_id)) == 0)
                         continue;



#ifdef PRINT_CALL_ARGUMENTS
    FONT_CALL_ARGUMENTS_COLOR();
    printf(  " int rte_eth_dev_start( uint16_t port_id)\n");
    printf(  " Call Args: port_id %d \n",port_id);
    FONT_NORMAL();
#endif
    CALL_RTE("rte_eth_dev_start()");
    ret = rte_eth_dev_start(port_id);
    if (ret < 0)
         rte_panic("rte_eth_dev_start:err=%d, port=%u\n", ret, port_id);
    }

}

//////////////////////////////////////////////
//////////////////////////////////////////////
/* Check the link status of all ports in up to 9s, and print them finally */
void check_ports_link_status(uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
        uint16_t port_id;
        uint8_t count, all_ports_up, print_flag = 0;
        struct rte_eth_link link;
        int ret;

        WAI();
        printf("\nChecking link status...");
        fflush(stdout);
        for (count = 0; count <= MAX_CHECK_TIME; count++) {
                if (g_force_quit)
                        return;
                all_ports_up = 1;
                RTE_ETH_FOREACH_DEV(port_id) {
                        if (g_force_quit)
                                return;
                        if ((port_mask & (1 << port_id)) == 0)
                                continue;
                        memset(&link, 0, sizeof(link));
                        ret = rte_eth_link_get_nowait(port_id, &link);
                        if (ret < 0) {
                                all_ports_up = 0;
                                if (print_flag == 1)
                                        printf("Port %u link get failed: %s\n",
                                                port_id, rte_strerror(-ret));
                                continue;
                        }
                        /* print link status if flag set */
                        if (print_flag == 1) {
                                if (link.link_status)
                                        printf(
                                        "Port%d Link Up. Speed %u Mbps - %s\n",
                                                port_id, link.link_speed,
                                (link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
                                        ("full-duplex") : ("half-duplex\n"));
                                else
                                        printf("Port %d Link Down\n", port_id);
                                continue;
                        }
                        /* clear all_ports_up flag if any link down */
                        if (link.link_status == ETH_LINK_DOWN) {
                                all_ports_up = 0;
                                break;
                        }
                }
                /* after finally printing all link status, get out */
                if (print_flag == 1)
                        break;

                if (all_ports_up == 0) {
                        printf(".");
                        fflush(stdout);
                        rte_delay_ms(CHECK_INTERVAL);
                }

                /* set the print_flag if all ports up or timeout */
                if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
                        print_flag = 1;
                        printf("done\n");
                }
        }
}




#define MAX_STRING_LEN 256

/* show border */
static char bdr_str[MAX_STRING_LEN];

#define STATS_BDR_FMT "========================================"
#define STATS_BDR_STR(w, s) printf("%.*s%s%.*s\n", w, \
        STATS_BDR_FMT, s, w, STATS_BDR_FMT)
  


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// show_port
void show_port(void)
{
        uint16_t i = 0;
        int ret = 0, j, k;

        snprintf(bdr_str, MAX_STRING_LEN, " show - Port PMD %"PRIu64,
                        rte_get_tsc_hz());
        STATS_BDR_STR(10, bdr_str);

        RTE_ETH_FOREACH_DEV(i) {
                uint16_t mtu = 0;
                struct rte_eth_link link;
                struct rte_eth_dev_info dev_info;
                struct rte_eth_rxq_info queue_info;
                struct rte_eth_rss_conf rss_conf;

                memset(&rss_conf, 0, sizeof(rss_conf));

                snprintf(bdr_str, MAX_STRING_LEN, " Port (%u)", i);

                printf("\t  -- Socket %d\n", rte_eth_dev_socket_id(i));
                ret = rte_eth_link_get(i, &link);
                if (ret < 0) {
                        printf("Link get failed (port %u): %s\n",
                               i, rte_strerror(-ret));
                } else {
                        printf("\t  -- link speed %d duplex %d,"
                                        " auto neg %d status %d\n",
                                        link.link_speed,
                                        link.link_duplex,
                                        link.link_autoneg,
                                        link.link_status);
                }
                printf("\t  -- promiscuous (%d)\n",
                                rte_eth_promiscuous_get(i));
                ret = rte_eth_dev_get_mtu(i, &mtu);
                if (ret == 0)
                        printf("\t  -- mtu (%d)\n", mtu);

                ret = rte_eth_dev_info_get(i, &dev_info);
                if (ret != 0) {
                        printf("Error during getting device (port %u) info: %s\n",
                                i, strerror(-ret));
                        return;
                }

                printf("  - queue\n");
                for (j = 0; j < dev_info.nb_rx_queues; j++) {
                        ret = rte_eth_rx_queue_info_get(i, j, &queue_info);
                        if (ret == 0) {
                                printf("\t  -- queue %d rx scatter %d"
                                                " descriptors %d"
                                                " offloads 0x%"PRIx64
                                                " mempool socket %d\n",
                                                j,
                                                queue_info.scattered_rx,
                                                queue_info.nb_desc,
                                                queue_info.conf.offloads,
                                                queue_info.mp->socket_id);
                        }
                }

                ret = rte_eth_dev_rss_hash_conf_get(i, &rss_conf);
                if (ret == 0) {
                        if (rss_conf.rss_key) {
                                printf("  - RSS\n");
                                printf("\t  -- RSS len %u key (hex):",
                                                rss_conf.rss_key_len);
                                for (k = 0; k < rss_conf.rss_key_len; k++)
                                        printf(" %x", rss_conf.rss_key[k]);
                                printf("\t  -- hf 0x%"PRIx64"\n",
                                                rss_conf.rss_hf);
                        }
                }

                printf("  - cyrpto context\n");
#ifdef RTE_LIBRTE_SECURITY
                void *p_ctx = rte_eth_dev_get_sec_ctx(i);
                printf("\t  -- security context - %p\n", p_ctx);

                if (p_ctx) {
                        printf("\t  -- size %u\n",
                                        rte_security_session_get_size(p_ctx));
                        const struct rte_security_capability *s_cap =
                                rte_security_capabilities_get(p_ctx);
                        if (s_cap) {
                                printf("\t  -- action (0x%x), protocol (0x%x),"
                                                " offload flags (0x%x)\n",
                                                s_cap->action,
                                                s_cap->protocol,
                                                s_cap->ol_flags);
//                                printf("\t  -- capabilities - oper type %x\n",
//                                                s_cap->crypto_capabilities->op);
                        }
                }
#endif
        }

        STATS_BDR_STR(50, "");
}







////////////////////////////////////////////////////
////////////////////////////////////////////////////
//  Connect  interface rx/tx queues (ethdev)  to event-dev queues
//
//  malloc()                           -allocates memory to save an index to each rx_adapter
//  rte_event_eth_rx_adapter_create()  -create the adapter 

void  rx_tx_adapter_setup_internal_port(void)
{
    struct   rte_event_eth_rx_adapter_queue_conf eth_q_conf;
    uint16_t adapter_id = 0;
    int      ret;
    int      i,j;

    WAI();

//////////////////////////////////////////////
/////////// configure rx adapters ////////////
////////////////////////////////////////////// 

     memset(&eth_q_conf, 0, sizeof(eth_q_conf));

//  walk through the aray of adapter Indexes
   for ( i = 0 ; i < g_glob.rx_adptr.nb_rx_adptr ; i++ )
   {

//       Create the adpater
         adapter_id = g_glob.rx_adptr.rx_adptr[i]; 
#ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf("int rte_event_eth_rx_adapter_create( uint8_t   id,\n"
                "                                     uint8_t 	dev_id,\n"
                "                                     struct rte_event_port_conf * port_config\n" 
                "                                     )\n");	
         printf(  " Call Args: adapter_id %d  event_dev_id %d  rte_event_port_conf: \n",
                                                               adapter_id,
                                                               g_glob.event_dev_id);
         FONT_NORMAL();
#endif

         print_rte_event_port_conf(1,"---create--- def_p_conf",adapter_id,&g_glob.def_p_conf);
         CALL_RTE("rte_event_eth_rx_adapter_create()");
         ret = rte_event_eth_rx_adapter_create(adapter_id,
                                                g_glob.event_dev_id,
                                                &g_glob.def_p_conf);
         if (ret)
               rte_panic("Failed to create rx adapter[%d]\n", adapter_id);

//       now add connections between eth dev and event dev
         for( j = 0 ; j <  g_glob.rx_adptr.nb_rx_adptr_add; j++)
         {
            if(g_glob.rx_adptr.rx_adptr[i] == g_glob.rx_adptr.rx_adptr_add[j].adapter_id)
            {
         /* Configure user requested sched type*/
                 eth_q_conf.ev.event_type     = RTE_EVENT_TYPE_ETHDEV  ;        // uint32_t event_type:4;
                 eth_q_conf.ev.sub_event_type = RTE_EVENT_TYPE_ETH_RX_ADAPTER ; // uint32_t sub_event_type:8;
                 eth_q_conf.ev.sched_type     = g_glob.rx_adptr.rx_adptr_add[j].sched_type ;     // handle incomming traffic as ordered
                 eth_q_conf.ev.op             = RTE_EVENT_OP_NEW;                                // uint8_t op:2; NEW,FORWARD or RELEASE  Really Create a flow, reuse a flow, delete a few.
                 eth_q_conf.ev.priority       = g_glob.rx_adptr.rx_adptr_add[j].priority;        //  0x80 - set priority above eth traffic (0x80) 
                 eth_q_conf.ev.queue_id       = g_glob.rx_adptr.rx_adptr_add[j].event_dev_queue; //  the event queue the WQE will be put in
        
#ifdef PRINT_CALL_ARGUMENTS
                 FONT_CALL_ARGUMENTS_COLOR();
                 printf(" int rte_event_eth_rx_adapter_queue_add( uint8_t  id,\n"
                        "                                         uint16_t eth_dev_id,\n"
                        "                                         int32_t 	rx_queue_id,\n"
                        "                                         const struct rte_event_eth_rx_adapter_queue_conf * 	conf\n" 
                        "                                        )\n");	
                 printf(" Call Args: id:rx_adapter_id %d  eth_dev_id:port_id %d  rx_queue_id: %d struct rte_event_eth_rx_adapter_queue_conf:\n",
                                      g_glob.rx_adptr.rx_adptr_add[j].adapter_id, 
                                      g_glob.rx_adptr.rx_adptr_add[j].eth_dev_port,
                                      g_glob.rx_adptr.rx_adptr_add[j].eth_dev_queue);
                 FONT_NORMAL();
#endif
                 print_rte_event_eth_rx_adapter_queue_conf(1,"--add-- eth_q_conf",
                                                    g_glob.rx_adptr.rx_adptr_add[j].adapter_id,
                                                    &eth_q_conf);
                 CALL_RTE("rte_event_eth_rx_adapter_queue_add()");
                 ret = rte_event_eth_rx_adapter_queue_add(  g_glob.rx_adptr.rx_adptr_add[j].adapter_id, 
                                                            g_glob.rx_adptr.rx_adptr_add[j].eth_dev_port,
                                                            g_glob.rx_adptr.rx_adptr_add[j].eth_dev_queue, 
                                                            &eth_q_conf);
         if (ret)
              rte_panic("Failed to add queues to Rx adapter\n");

            }
         }

//       Start the adpater
         CALL_RTE("rte_event_eth_rx_adapter_start(uint8_t id)");
         ret = rte_event_eth_rx_adapter_start(adapter_id);
         if (ret)
              rte_panic("Rx adapter[%d] start Failed\n", adapter_id);

    }  // loop next adapter


//////////////////////////////////////////////
/////////// configure tx adapters ////////////
////////////////////////////////////////////// 
 
//  walk through the aray of adapter Indexes
   for ( i = 0 ; i < g_glob.tx_adptr.nb_tx_adptr ; i++ )
   {
//       Create the adpater
         adapter_id = g_glob.tx_adptr.tx_adptr[i]; 

#ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf("int rte_event_eth_tx_adapter_create( uint8_t id, \n"
                "                                     uint8_t dev_id,\n"
                "                                     struct rte_event_port_conf * 	port_config \n"
                "                                     )\n");	
         printf(" Call Args: adpater_id: %d, event_dev_id:%d, struct def_p_conf)\n",
                                 adapter_id,
                                 g_glob.event_dev_id );
         FONT_NORMAL();
#endif
         print_rte_event_port_conf(1,"---create--- def_p_conf",adapter_id ,&g_glob.def_p_conf);
         CALL_RTE("rte_event_eth_tx_adapter_create()");
         ret = rte_event_eth_tx_adapter_create(adapter_id , 
                                        g_glob.event_dev_id ,
                                        &(g_glob.def_p_conf) );
         if (ret)
                rte_panic("Failed to create tx adapter[%d]\n",adapter_id);

//       now add connections between eth dev and event dev
         for( j = 0 ; j <  g_glob.tx_adptr.nb_tx_adptr_add; j++)
         {
            if(g_glob.tx_adptr.tx_adptr[i] == g_glob.tx_adptr.tx_adptr_add[j].adapter_id)
            {
// zzzzzzzzzzzzzzzzzzzzzzzzzzzz
        
#ifdef PRINT_CALL_ARGUMENTS
                 FONT_CALL_ARGUMENTS_COLOR();
                 printf(" int rte_event_eth_tx_adapter_queue_add( uint8_t  id,\n"
                        "                                         uint16_t eth_dev_id,\n"
                        "                                         int32_t  queue,\n"  /* this is the eth_queue */
                        "                                        )\n");	
                 printf(" Call Args: id:tx_adapter_id %d  eth_dev_id:port_id %d  tx_queue_id: %d \n",
                                      g_glob.tx_adptr.tx_adptr_add[j].adapter_id, 
                                      g_glob.tx_adptr.tx_adptr_add[j].eth_dev_port,
                                      g_glob.tx_adptr.tx_adptr_add[j].eth_dev_queue);
                 FONT_NORMAL();
#endif
                 CALL_RTE("rte_event_eth_tx_adapter_queue_add()");
                 ret = rte_event_eth_tx_adapter_queue_add(  g_glob.tx_adptr.tx_adptr_add[j].adapter_id, 
                                                            g_glob.tx_adptr.tx_adptr_add[j].eth_dev_port,
                                                            g_glob.tx_adptr.tx_adptr_add[j].eth_dev_queue 
                                                            );
                 if (ret)
                     rte_panic("Failed to add queues to Tx adapter\n");
            }
         }

//       Start the tx adpater
         CALL_RTE("rte_event_eth_tx_adapter_start(uint8_t id)");
         ret = rte_event_eth_tx_adapter_start(adapter_id);
         if (ret)
              rte_panic("Tx adapter[%d] start Failed\n", adapter_id);

    }  // loop next adapter

}





#define BATCH_SIZE  4

char  ethdev_m0[] = { "Another"        };
char  ethdev_m1[] = { "Brick"         };
char  ethdev_m2[] = { "In the"       };
char  ethdev_m3[] = { "wall"};
char * ethdev_message[] = {ethdev_m0,ethdev_m1,ethdev_m2,ethdev_m3};


int g_drop_all_traffic = 0;

struct rte_event         g_ev        __rte_cache_aligned;          // use this to encode an event.

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//     _loop Function

int ethdev_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    int  i;
    // struct rte_event   ev;
    uint8_t event_dev_id ;
    uint8_t event_port_id ;
   
    struct rte_event events[BATCH_SIZE];
    uint16_t nb_rx; 
    // because my event device is 1 core to 1 port to 1 queue,  the table below is a
    //     a simple bit ot
    uint8_t  core_2_message[]           = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t  core_2_next_message[]      = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,2,3,0 };
    uint8_t  core_2_evt_port_id[]       = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t  core_2_next_evt_queue_id[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 3,5,7,1 };
    uint16_t ret;
    int core_counter = 0;
    struct rte_event_timer *p_ev_timer;   // pointer to timer event
    struct rte_event       *p_ev;         // pointer to an event


    if( ! ( rte_lcore_is_enabled(20)  &&
            rte_lcore_is_enabled(21)  &&
            rte_lcore_is_enabled(22)  &&
            rte_lcore_is_enabled(23) ))
    {
        printf(" this function expects you to use cores 20,21,22,24i (-c 0xf00000) ");
        rte_panic(" use the correct cores, or fix the core_2_port/message/queue mappings");
    }

    lcore_id       = rte_lcore_id();             // my core index
    event_dev_id   = g_glob.event_dev_id;          // id of my event device
    event_port_id  = core_2_evt_port_id[ lcore_id ]; // for now, I have 1 port associated with 1 core.  

    if (lcore_id == 23 )  
    {
 #ifdef TIMER_ADAPTER
         // Set up timer event structure to send a timerout via eventdev to a core
  
         p_ev_timer = &g_ev_timer;
  
         p_ev_timer = gen_timer_ev( p_ev_timer,                 /* struct rte_event_timer *   */
                                     0x07734 ,                  /* uint32_t flow_id           */
                                     RTE_SCHED_TYPE_PARALLEL,   /* uint8_t  sched_type        */
                                     1,                         /* uint8_t  evt_queue         */
                                     0x40,                      /* uint8_t evt_queue_priority */
                                     (void *)p_ev_timer );      /* void * evt_ptr */
  
   #ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf(" Call Args: event_dev_id:%d, lcore_id:%d ev: \n",event_dev_id,lcore_id);
         FONT_NORMAL();
   #endif
         print_rte_event_timer ( 1, "g_ev_timer",&(g_ev_timer));
         CALL_RTE("rte_event_timer_arm_burst() ");
         ret = rte_event_timer_arm_burst( g_glob.timer_100ms , &p_ev_timer, 1 );
         if( ret != 1 )
         {
              printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
              printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
              rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
         }

#endif  // event timer

    }

    if( (lcore_id == 20 )  && ( g_core_messages == 1))
    {
        // cmd line option to send inter-core operations
 
        printf("*** Put an event into the Event Dev Scheduler ***\n");
        printf("***     first message should come back to this core ***\n");
        // because of my event dev definition:  want to enqueue to ev_queue 1 and ev_port 0, priority 0x40

        printf("        event_dev_id:   %d \n",event_dev_id);
        printf("        event_port_id:  %d \n",event_port_id);
        printf("        lcore_id:       %d \n",lcore_id);
        printf("        print every :   %ldth  \n",g_print_interval);


       //// Set up event structure to send an event to a core using event_dev sso
        p_ev = &g_ev;

        // fill in my rte_event with the field data I need.  note- op=RTE_EVENT_OP_NEW
        p_ev = gen_ev( p_ev,                               /* struct rte_event * p_ev     */
                       0xdead,                             /* uint32_t flow_id            */
                       RTE_SCHED_TYPE_PARALLEL,            /* uint8_t  sched_type         */
                       TIMER_EVENT_QUEUE ,     /* core 20 message queue */ /* uint8_t  evt_queue          */
                       0x40 ,  /* core 20 msg queue pri */ /* uint8_t  evt_queue_priority */
                       (void *) core_message[core_2_message[lcore_id]] /* void *evt_ptr) */
                       );

        // printout what I created

#ifdef PRINT_CALL_ARGUMENTS
        FONT_CALL_ARGUMENTS_COLOR();
        printf(" Call Args: event_dev_id:%d, evt_port_id :%d ev: \n",event_dev_id,event_port_id);
        FONT_NORMAL();
#endif
        print_rte_event( 1, "p_ev",p_ev);
        CALL_RTE("rte_event_enqueue_burst() ");
        /* static uint16_t rte_event_enqueue_burst (uint8_t dev_id,
                                            uint8_t port_id,
                                            const struct rte_event ev[]
                                            ,uint16_t nb_events)          */
        ret = rte_event_enqueue_burst(event_dev_id, event_port_id, p_ev, 1 );
        if( ret != 1 )
        {
             printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
             printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
             rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
        }



   } // end lcore id == 0 && put a message in


// struct worker_data *data = (struct worker_data *)arg;

    printf(" Launch code on Core: %d event_port %d \n",lcore_id, event_port_id);
    while (!g_force_quit)
    {
       // lots of handwaving here.  there should be only one event 
       //    put in before the Event loop
 
        nb_rx = rte_event_dequeue_burst(event_dev_id, event_port_id,
                                events, RTE_DIM(events), 0);

        if (nb_rx == 0) 
        {
            rte_pause();
            continue;
        }
   
        for(i = 0 ; i < nb_rx ; i++)
        {
            g_core_stats[lcore_id].rx_event_cnt++ ; // inc stats
            if( events[i].event_type == RTE_EVENT_TYPE_CPU )
            {     
 
                 printf("****    c%d) Received %d CPU events from evt_port_id %d**** \n",lcore_id,nb_rx,event_port_id);
                 print_rte_event( 0, "event[i]",&events[i]);
                 printf(" Message:   %s\n",(char *)events[i].event_ptr);
                 
                 printf("  c%d) Send message to next Port (aka core for me)  next_queue: %d \n",lcore_id
                                                         ,core_2_evt_port_id[lcore_id]);
                 
                 // set to forward to next core.
                 events[i].queue_id       = core_2_next_evt_queue_id[lcore_id];
                 // set operation to forward packet    
                 events[i].op             = RTE_EVENT_OP_FORWARD;  // this for some reason stops the event form forwarding
                 events[i].flow_id        += 1;
                 events[i].priority        = 0x40 ; 
                 events[i].event_ptr       = (void *)ethdev_message[core_2_next_message[lcore_id]] ; 
                 
                 // print_rte_event( 0, "event[i]",&events[i]);
                 rte_pause();
                 usleep(500000);     

                 ret = rte_event_enqueue_burst(event_dev_id, core_2_evt_port_id[lcore_id],
                                     &(events[i]) , 1);
                 if( ret != 1 )
                 {
                      printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
                      printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
                      rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
                 }
            }
            else if (events[i].event_type == RTE_EVENT_TYPE_TIMER   )   // evemt -> timer  
            {

 #ifdef TIMER_ADAPTER
                 printf("****    c%d) Received %d Timer events from evt_port_id %d**** \n",lcore_id,nb_rx,event_port_id);

                 print_g_core_stats();

                 p_ev_timer = events[i].event_ptr;
                 // set to forward to next core.
                 p_ev_timer->state          = RTE_EVENT_TIMER_NOT_ARMED;
                 // send timer event to same core.
                 p_ev_timer->ev.queue_id    = TIMER_EVENT_QUEUE ;
                 // set operation to forward packet
                 //  p_ev_timer->ev.op             = RTE_EVENT_OP_FORWARD;
                 p_ev_timer->ev.flow_id        += 1;

                 ret= rte_event_timer_arm_burst( g_glob.timer_100ms , &p_ev_timer, 1 );

                 if( ret != 1 )
                 {
                      printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
                      printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
                      rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
                 }
#endif
                    
            }
            else if (events[i].event_type == RTE_EVENT_TYPE_ETHDEV )  // event -> etherent packet 
            {
//<FS>
                 struct rte_mbuf *  p_mbuff = events[i].mbuf ;    // fish out the mbuff pointer. 
                 uint8_t *          buf;             
                 MAC_Hdr_t          l2_hdr;

                 g_core_stats[lcore_id].rx_packet_cnt++ ;
                 core_counter--;
                 buf = (uint8_t *) rte_pktmbuf_mtod(p_mbuff, struct rte_ether_hdr *);
                 GetMacData(buf,&l2_hdr);
                 
                 if( l2_hdr.EtherType == 0x0806 ) /* ARP  */
                 {
                      printf("%s GOT AN ARP Packet!! %s \n",C_RED, C_NORMAL);
                 } 

                 // only fall in if p_print_interval is >0 else do not print
                 if ((  core_counter <= 0 ) && (g_print_interval > 0))
                 {
                     core_counter = g_print_interval;

                     printf("%d< got packet  Stream  flow_id: 0x%05x  queue_id: %d, port_id %d\n",
                                                                      lcore_id,
                                                                      events[i].flow_id,
                                                                      events[i].queue_id,
                                                                      event_port_id );

                     VERBOSE_M( ETH_L2_DETAILS )
                     {
                         PrintBuff((uint8_t*) buf, 0x40 , NULL ,"mbuff Raw Buffer");
                     }
                     VERBOSE_M( ETH_L3_DETAILS )
                     {
                         PrintBuff((uint8_t*) p_mbuff, (0x100 + p_mbuff->pkt_len + 0x10) , NULL ,"mbuff Raw Buffer");
                         print_rte_mbuf(0,p_mbuff);
                     }
                     if(g_verbose > 2)
                     {
                         printf(" rx_pkt_cnt: %lu \n", g_core_stats[lcore_id].rx_packet_cnt );
                     } 
                 }
//<FS>
                 if ( g_drop_all_traffic  == 1)
                 {
                       
                       rte_pktmbuf_free( events[i].mbuf  );
                 }
                 else
                 {
#ifdef LOOP_CALLS
                     CALL_RTE("rte_event_eth_tx_adapter_enqueue()");
#endif
                     while (!rte_event_eth_tx_adapter_enqueue(event_dev_id,
                                                              event_port_id,   // here is where I map to other interface??
                                                    &(events[i]), 1, 0) &&
                                                   ( g_force_quit != true ) ) 
                                     ;
                 }    

            } // end event - packet
            else
            {
                       
                 printf("  c%d) unrecognized event  %d \n",lcore_id,events[i].event_type);
                 printf("  THIS IS A BUG, NOT SURE WHAT TO DO.\n");
                 print_rte_event(0,"unrecognized event",&events[i] );
                 
                 //  rte_pktmbuf_free( events[i].mbuf  );
 
            }
        }   // end for i -> nb_events 
    }  // end while
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

     rte_event_dev_dump( g_glob.event_dev_id  ,stdout);


// Stop the device

    printf("************\n    stopping the event_dev device  sso   \n***************\n");
     rte_event_dev_stop (g_glob.event_dev_id);

//   CLose the event dev device




/* int rte_event_dev_close (   uint8_t     dev_id  )   
Close an event device. The device cannot be restarted! */
    printf("************\n    closing the event_dev device  sso   \n***************\n");
     rte_event_dev_close( g_glob.event_dev_id );


/* clear ethdev resources*/
    if ( g_glob.evq.event_q_cfg  != NULL)       free(g_glob.evq.event_q_cfg);
    if ( g_glob.evp.event_p_id != NULL)         free(g_glob.evp.event_p_id);
    if ( g_glob.rx_adptr.rx_adptr != NULL)      free(g_glob.rx_adptr.rx_adptr);
    if ( g_glob.rx_adptr.rx_adptr_add != NULL)  free(g_glob.rx_adptr.rx_adptr_add);
    if ( g_glob.tx_adptr.tx_adptr != NULL)      free(g_glob.tx_adptr.tx_adptr);
    if ( g_glob.tx_adptr.tx_adptr_add != NULL)  free(g_glob.tx_adptr.tx_adptr_add);



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





