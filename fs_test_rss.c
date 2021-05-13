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
#include <rte_event_timer_adapter.h>   // for event timer
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>

#include "fprintbuff.h"
#include "fs_print_rte_mbuff.h"
#include "fs_extras.h"
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
#include "fs_net_common.h"
#include "fs_ethdev.h"


#include "fs_spinlock_test.h"
//#include "fs_print_structures.c"
// eth dev structures

#include "fs_extras.h"
#include "fs_eventdev.h"
#include "fs_print_rte_ethdev_struct.h"
#include "fs_eventdev_timer.h"
#include "fs_core.h"
#include "fs_net_common.h"
#include "fs_global_vars.h"

//#include "fs_print_rte_structures.h"

extern uint64_t g_core_messages;
extern int64_t  g_print_interval;
extern int      g_drop_all_traffic ;
int64_t         g_loopback_traffic =  1;   // if =1 will swap src <->det for l2,l3,and l4 headers


void print_test_setup(void);
void print_test_setup(void)
{
printf("\n"
" %s \n"
"                                                                          \n" 
"                                                                          \n" 
"                          ------------                                    \n"
"                          |evt_que 0 | \\                                 \n"
"                          ------------  \\                                \n"
"                                         \\                               \n"
"  ---------------         ------------      -------------     ----------- \n"
"  |eth port_id 0| ----->  |evt_que 1 | ---> | evt_prt 0 | --> | core 23 | \n"
"  ---------------         ------------      -------------     ----------- \n"
"               \\  \\ \\                                                     \n" 
"                \\  \\ \\    ------------      -------------     ----------- \n"
"                 \\  \\ \\   |evt_que 2 | ---> | evt_prt 1 | --> | core 22 | \n"
"                  \\  \\    ------------      -------------     ----------- \n"
"                   \\  \\                                                   \n" 
"                    \\  \\  ------------      -------------     ----------- \n"
"                     \\    |evt_que 3 | ---> | evt_prt 2 | --> | core 21 | \n"
"                      \\   ------------      -------------     ----------- \n"
"                       \\                                                  \n" 
"                        \\ ------------      -------------     ----------- \n"
"                          |evt_que 4 | ---> | evt_prt 3 | --> | core 20 | \n"
"                          ------------      -------------     ----------- \n"
"                                                                          \n" 
"                                                                          \n" 
"                                                                          \n"
"\n",__FILE__);
}


// I will start with 4 cores,  ports, and 4 queues.
//   goal is to enable RSS and have the packets sprayed to 4 cores


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
int        test_setup( __attribute__((unused))void * arg);
int         test_loop( __attribute__((unused))void * arg);
int        test_print( __attribute__((unused))void * arg);
int      test_cleanup( __attribute__((unused))void * arg);
void test_description( void);


int test_setup( __attribute__((unused)) void * arg)
{

    WAI();
   print_test_setup(); 

///////
/// Eth Dev Config info
//////
    g_glob.enabled_eth_port_mask = 0x01 ;            // cmd line -p argument - here I hardwired :-0    

 // number of rx and tx ports per queue
 // g_glob.eth_port_cfg_data[port_id].nb_rx_queues=1 ;

    g_glob.eth_port_cfg_data[0].nb_rx_queues=3 ;
    g_glob.eth_port_cfg_data[0].nb_tx_queues=3 ;


///////
/// Event Dev Config info
//////
    g_glob.event_dev_id = 0;                         // event dev_id index/handle => SSO  0

    memset(&(g_glob.def_p_conf), 0, sizeof(struct rte_event_port_conf));  
         g_glob.def_p_conf.dequeue_depth =1;        
         g_glob.def_p_conf.enqueue_depth =1;
         g_glob.def_p_conf.new_event_threshold = -1;

    // event queues & ports
    g_glob.evq.nb_queues = 5 ;  // total number of event queues in my design
    g_glob.evp.nb_ports  = 4 ;  // total number of event ports in my design.  


    // adapters rx 
    g_glob.rx_adptr.nb_rx_adptr     = 1 ;  // total number of rx_adapters in my design
    g_glob.rx_adptr.nb_rx_adptr_add = 1 ;  // total number of rx_adapter_adds
//??

    // adapters tx 
    g_glob.tx_adptr.nb_tx_adptr = 1 ;      // total number of tx_adapters in my design.  
    g_glob.tx_adptr.nb_tx_adptr_add = 1 ;  // total number of rx_adapter_adds
//??
    
    // allocate storage arrays based on evp and evq , rx_adapters, and tx_adaptersabove.


////////////////
// Allocate structures based on values above
///////////////
     
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

/*  RX_ADAPTERS_ADD - memory allocate */
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

/*  TX_ADAPTERS_ADD - memory allocate */
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


/////////////////////////
//  Fill in the memory allocated above with data
////////////////////////

////////////////////////////////////////
// now map the evnet_queues, event_ports rx_adapters and tx_adapters
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
     ( ptr + 1 )->ev_q_conf.priority = 0x80;

#if 1
 // queue 2
     ( ptr + 2 )->event_q_id    = 2;         // event queue index
     ( ptr + 2 )->to_event_port = 1;         // event port this queue feeds
     ( ptr + 2 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
     ( ptr + 2 )->ev_q_conf.schedule_type = 0;  /*  dont care when
                                             event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
     ( ptr + 2 )->ev_q_conf.priority = 0x80;

 // queue 3
     ( ptr + 3 )->event_q_id    = 3;         // event queue index
     ( ptr + 3 )->to_event_port = 2;         // event port this queue feeds
     ( ptr + 3 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
     ( ptr + 3 )->ev_q_conf.schedule_type = 0;  /*  dont care when
                                             event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
     ( ptr + 3 )->ev_q_conf.priority = 0x80;

 // queue 4
     ( ptr + 4 )->event_q_id    = 4;         // event queue index
     ( ptr + 4 )->to_event_port = 3;         // event port this queue feeds
     ( ptr + 4 )->ev_q_conf.event_queue_cfg = RTE_EVENT_QUEUE_CFG_ALL_TYPES;
     ( ptr + 4 )->ev_q_conf.schedule_type = 0;  /*  dont care when
                                             event_queue_config = RTE_EVENT_QUEUE_CFG_ALL_TYPE*/
     ( ptr + 4 )->ev_q_conf.priority = 0x80;


#endif
}


// now link the event_queues to the event ports
//  to do this...   
//       the link command is port based and wants an array of
//       queues and the queue priority        


 // event port 0  - one event queues ("0"), queue Priority 0x40 
   (g_glob.evp.event_p_id + 0)->nb_links = 2;
   (g_glob.evp.event_p_id + 0)->q_id[0] = 0;   // event_queue_id
   (g_glob.evp.event_p_id + 0)->pri[0]  = 40;  // event_queue priority
 //                - one event queues ("1"), queue Priority 0x80 
   (g_glob.evp.event_p_id + 0)->q_id[1] = 1;   // event_queue_id
   (g_glob.evp.event_p_id + 0)->pri[1]  = 80;  // event_queue priority




#if 1

//  Below definition is for 3 queues on a eth_port,  
//     typically there is only 1 queue on an eth_port unless rte_flow 
//     is in use   
// event port 1  -  queue 1 ), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 1)->nb_links = 1;
   (g_glob.evp.event_p_id + 1)->q_id[0] = 2;
   (g_glob.evp.event_p_id + 1)->pri[0]  = 80;

// event port 2  -  queue 2 ), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 2)->nb_links = 1;
   (g_glob.evp.event_p_id + 2)->q_id[0] = 3;
   (g_glob.evp.event_p_id + 2)->pri[0]  = 80;

// event port 3  -  queue 3 ), queue Priority 0x80 and 0x40
   (g_glob.evp.event_p_id + 3)->nb_links = 1;
   (g_glob.evp.event_p_id + 3)->q_id[0] = 4;
   (g_glob.evp.event_p_id + 3)->pri[0]  = 80;


#endif

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
    g_glob.rx_adptr.rx_adptr_add[0].event_dev_queue = 1; // 
    g_glob.rx_adptr.rx_adptr_add[0].sched_type = RTE_SCHED_TYPE_ORDERED;        // 
    g_glob.rx_adptr.rx_adptr_add[0].priority = 0x80;        // 

#if 0
// rx_adapter 0  connects eth dev 1/port 0 to event queue 1
    g_glob.rx_adptr.rx_adptr_add[1].adapter_id = 0 ;     //
    g_glob.rx_adptr.rx_adptr_add[1].eth_dev_port = 1 ;   //
    g_glob.rx_adptr.rx_adptr_add[1].eth_dev_queue = 0 ;  //
    g_glob.rx_adptr.rx_adptr_add[1].event_dev_queue = 1; //
    g_glob.rx_adptr.rx_adptr_add[1].sched_type = RTE_SCHED_TYPE_ORDERED;        //
    g_glob.rx_adptr.rx_adptr_add[1].priority = 0x80;        //

// rx_adapter 0  connects eth dev 2/port 0 to event queue 2
    g_glob.rx_adptr.rx_adptr_add[2].adapter_id = 0 ;     //
    g_glob.rx_adptr.rx_adptr_add[2].eth_dev_port = 2 ;   //
    g_glob.rx_adptr.rx_adptr_add[2].eth_dev_queue = 0 ;  //
    g_glob.rx_adptr.rx_adptr_add[2].event_dev_queue = 2; //
    g_glob.rx_adptr.rx_adptr_add[2].sched_type = RTE_SCHED_TYPE_ORDERED;        //
    g_glob.rx_adptr.rx_adptr_add[2].priority = 0x80;        //

#endif

//    g_glob.tx_adptr.nb_tx_adptr_adds = 2;

    g_glob.tx_adptr.tx_adptr[0]  = 0 ; //  .

// tx_adapter 0  connects eth port_id(dev) 0/queue 0 to event queue 0
    g_glob.tx_adptr.tx_adptr_add[0].adapter_id = 0 ;     // 
    g_glob.tx_adptr.tx_adptr_add[0].eth_dev_port = 0 ;   // 
    g_glob.tx_adptr.tx_adptr_add[0].eth_dev_queue = 0 ;  // 

#if 0
 // tx_adapter 0  connects eth port_id(dev)1 / queue 0 to event queue 0
     g_glob.tx_adptr.tx_adptr_add[1].adapter_id = 0 ;     //
     g_glob.tx_adptr.tx_adptr_add[1].eth_dev_port = 1 ;   //
     g_glob.tx_adptr.tx_adptr_add[1].eth_dev_queue = 0 ;  //

 // tx_adapter 0  connects eth port_id(dev)2 / queue 0 to event queue 0
     g_glob.tx_adptr.tx_adptr_add[2].adapter_id = 0 ;     //
     g_glob.tx_adptr.tx_adptr_add[2].eth_dev_port = 2 ;   //
     g_glob.tx_adptr.tx_adptr_add[2].eth_dev_queue = 0 ;  //
#endif

// INFO
   printf("rte_event_dev_count() = %d \n",rte_event_dev_count());
   printf("rte_eth_dev_count_avail() = %d \n",rte_eth_dev_count_avail());

#if 0
   {
      int i;
      struct rte_eth_dev_info  	eth_dev_info;
      dev = &rte_eth_devices[port_id];
       
      for( i = 0 ; i < rte_eth_dev_count_avail(); i++)
      {
         RTE_ETH_VALID_PORTID_OR_ERR_RET(port_id, -ENODEV);
         rte_eth_dev_info_get (i,&eth_dev_info);
         print_rte_eth_dev_info( 0, "eth_dev_info",i, &eth_dev_info);
      }
   }
#endif

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

    rx_tx_adapter_setup_internal_port();


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

#define TIMER_EVENT_QUEUE 0

#define TIMER_ADAPTER
#ifdef TIMER_ADAPTER
      timer_event_init();
      timer_event_start();
#endif


     return 0;
}





/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////  Handle CPU Messages
/////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t  g_test_core_2_next_message[]      = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
uint8_t  g_test_core_2_evt_port_id[]       = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 3,2,1,0 };
uint8_t  g_test_core_2_next_evt_queue_id[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,4,3,2 };

static inline void Do_Event_Type_CPU( struct rte_event * p_event, unsigned lcore_id,uint8_t event_port_id );
static inline void Do_Event_Type_CPU( struct rte_event * p_event, unsigned lcore_id,uint8_t event_port_id )
{
     int ret;
     printf("****    c%d) Received CPU event from evt_port_id %d**** \n",lcore_id,event_port_id);
     print_rte_event( 0, "event[i]",p_event);
     printf(" Message:   %s\n",(char *)p_event->event_ptr);

     printf("  c%d) Send message to next Port (aka core for me)  next_queue: %d \n",lcore_id
                                             ,g_test_core_2_next_evt_queue_id[lcore_id]);

     // set to forward to next core.
     p_event->queue_id       = g_test_core_2_next_evt_queue_id[lcore_id];
     // set operation to forward packet
     p_event->op             = RTE_EVENT_OP_FORWARD;  // this for some reason stops the event form forwarding
     p_event->flow_id        += 1;
     p_event->priority        = 0x80 ;
     p_event->event_ptr       = (void *)core_message[g_test_core_2_next_message[lcore_id]] ;

     // print_rte_event( 0, "event[i]",&events[i]);
     rte_pause();
     usleep(500000);

     ret = rte_event_enqueue_burst(g_glob.event_dev_id, g_test_core_2_evt_port_id[lcore_id],
                         p_event , 1);
     if( ret != 1 )
     {
          printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
          printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
          rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
     }
}

#define BATCH_SIZE  4


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//     _loop Function

int test_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    int  i;
    // struct rte_event   ev;
    uint8_t event_dev_id ;
    uint8_t event_port_id ;
   
    struct rte_event events[BATCH_SIZE];
    uint16_t nb_rx; 
//    uint16_t ret;
    int core_counter = 0;
#ifdef TIMER_ADAPTER
    struct rte_event_timer *p_ev_timer;   // pointer to timer event
    uint16_t ret = 0;
#endif

    if( ! ( rte_lcore_is_enabled(20)  &&
            rte_lcore_is_enabled(21)  &&
            rte_lcore_is_enabled(22)  &&
            rte_lcore_is_enabled(23) ))
    {
        printf(" this function expects you to use cores 23 (-c 0x800000) ");
        rte_panic(" use the correct cores, or fix the core_2_port/message/queue mappings");
    }

    lcore_id       = rte_lcore_id();             // my core index
    event_dev_id   = g_glob.event_dev_id;          // id of my event device
    event_port_id  = g_test_core_2_evt_port_id[ lcore_id ]; // for now, I have 1 port associated with 1 core.  


    if(  lcore_id == 23 )  
    {
#ifdef TIMER_ADAPTER
     //// Set up timer event structure and add  event_dev sso

       p_ev_timer = &g_ev_timer;

       p_ev_timer = gen_timer_ev( p_ev_timer,                 /* struct rte_event_timer *   */
                                   0x07734 ,                  /* uint32_t flow_id           */
                                   RTE_SCHED_TYPE_PARALLEL,   /* uint8_t  sched_type        */
                                   TIMER_EVENT_QUEUE,                         /* uint8_t  evt_queue         */
                                   0x80,                      /* uint8_t evt_queue_priority */
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
            rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event\n");
       }
#endif
    }

    if( (lcore_id == 23 )  && ( g_core_messages == 1))
    {
        // cmd line option to send inter-core operations
 
        printf("*** Put an event into the Event Dev Scheduler ***\n");
        printf("***     first message should come back to this core ***\n");
        // because of my event dev definition:  want to enqueue to ev_queue 1 and ev_port 0, priority 0x40

        printf("        event_dev_id:   %d \n",event_dev_id);
        printf("        event_port_id:  %d \n",event_port_id);
        printf("        lcore_id:       %d \n",lcore_id);
        printf("        print every :   %ldth  \n",g_print_interval);

    
#ifdef CORE_MESSAGE
       //// Set up event structure to send an event to a core using event_dev sso
        p_ev = &g_ev;

        // fill in my rte_event with the field data I need.  note- op=RTE_EVENT_OP_NEW
        p_ev = gen_ev( p_ev,                                         /* struct rte_event * p_ev     */
                       0xdead,                                       /* uint32_t flow_id            */
                       RTE_SCHED_TYPE_PARALLEL,                      /* uint8_t  sched_type         */
                       0 ,     /* core 23 message queue */           /* uint8_t  evt_queue          */
                       0x40 ,  /* core 23 msg queue pri */           /* uint8_t  evt_queue_priority */
                       (void *) core_message[0] /*core 20 0th msg */ /* void *evt_ptr) */
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

#endif

   } // end lcore id == 23 && put a message in


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
            VERBOSE_M( EVENT_MESSAGES ) printf("%s Received Event: 0x%02x  (events[i].event_type) %s\n",C_BLUE,events[i].event_type,     C_NORMAL );
            VERBOSE_M( EVENT_DETAILS )  print_rte_event(0,"RTE_EVENT: ",&events[i] );
            g_core_stats[lcore_id].rx_event_cnt++ ; // inc stats
            
//////$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
////// switch through the event types
//////$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
            switch (events[i].event_type) {
//$$$$$$$$
//$$$$$$$$  Event type - CPU
//$$$$$$$$
#if 0
                case RTE_EVENT_TYPE_CPU:
                     Do_Event_Type_CPU( &events[i] , lcore_id, event_port_id );

                     break;
#endif
//$$$$$$$$
//$$$$$$$$  Event type - TIMER
//$$$$$$$$
                case RTE_EVENT_TYPE_TIMER:

                      print_g_core_stats();   // print counts to the display

                      p_ev_timer = events[i].event_ptr;
                      // Reset the timer state and requeue.
                      p_ev_timer->state          = RTE_EVENT_TIMER_NOT_ARMED;
                      p_ev_timer->ev.queue_id    = TIMER_EVENT_QUEUE ;  // all timer events handled by core 23 evt queue 0
                      // set operation to forward packet
                      // p_ev_timer->ev.op          = RTE_EVENT_OP_FORWARD;
                      p_ev_timer->ev.flow_id     += 1;

                      printf("****    c%d) Received %d Timer events from evt_port_id %d  %d**** \n",lcore_id,
                                                              nb_rx,
                                                              event_port_id,
                                                              p_ev_timer->ev.flow_id);
//                    print_rte_event( 0, "event[i]",&events[i]);

                      ret= rte_event_timer_arm_burst( g_glob.timer_100ms , &p_ev_timer, 1 );
                      if( ret != 1 )
                      {
                           printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
                           printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
                           rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
                      }
                     break;

//$$$$$$$$
//$$$$$$$$  Event type - ETHDEV
//$$$$$$$$
                case RTE_EVENT_TYPE_ETHDEV:
                     {
                         struct rte_mbuf *  p_mbuff = events[i].mbuf ;    // fish out the mbuff pointer.
                         uint8_t *          l4_ptr;
                         uint8_t *          l3_ptr;
                         uint8_t *          l2_ptr;   // pointer to the l2 header.
                         MAC_Hdr_t          l2_hdr;   // MAC_HDR_T is not memory aligned to an l2 header.
                                                      //   the structure has header size and saves vlans
                                                      //   if it find them in the header.
                         g_core_stats[lcore_id].rx_packet_cnt++ ;  // increment stats
                         core_counter--;
              ////////////////////
              // Print Packet Info
              ////////////////////
                         if ((  core_counter <= 0 ) && (g_print_interval > 0))
                         {
                             core_counter = g_print_interval;
                             VERBOSE_M( EVENT_ETH_MESSAGES )  print_rte_event(0,"RTE_EVENT_TYPE_ETHDEV",&events[i] );

                             printf("%d< Received RTE_EVENT_TYPE_ETHDEV   flow_id: 0x%05x  queue_id: %d, port_id %d\n",
                                                                          lcore_id,
                                                                          events[i].flow_id,
                                                                          events[i].queue_id,
                                                                          event_port_id );

                             VERBOSE_M(EVENT_ETH_DETAILS )
                             {
                                 l2_ptr = (uint8_t *) rte_pktmbuf_mtod(p_mbuff, struct rte_ether_hdr *);
                                 PrintBuff((uint8_t*) l2_ptr, 0x100 , l2_ptr ,"rte_eth_header:");
                                 PrintBuff((uint8_t*) p_mbuff, (0x100 + p_mbuff->pkt_len + 0x10) , (unsigned char *)p_mbuff ,"mbuff Raw Buffer");
                                 print_rte_mbuf(0,p_mbuff);
                             }
                             VERBOSE_M( EVENT_ETH_MESSAGES)
                             {
                                 printf(" rx_pkt_cnt: %lu \n",g_core_stats[lcore_id].rx_packet_cnt);
                             }
                         }  // end print packet info
               ////////////////////////
               // parse out packet info
               /////////////////////////
                         // fish out some packet data
                         l2_ptr = (uint8_t *) rte_pktmbuf_mtod(p_mbuff, struct rte_ether_hdr *);

                         GetMacData(l2_ptr,&l2_hdr);                            // fish the field info out of the data
                         l3_ptr =  l2_ptr +  l2_hdr.hdr_sz;                     // mover pointer forward to point to l3 info

                         VERBOSE_M( ETH_L2_ETH_TYPE ) printf("%s Received  L2.EthType: 0x%04x %s\n",C_RED,(htons(l2_hdr.EtherType)), C_NORMAL);
                         VERBOSE_M( ETH_L2_DETAILS )
                         {
                              printMAC_Hdr_t(&l2_hdr);  // print the header
                         }
                         // L2 payload - EthType
                         switch (htons(l2_hdr.EtherType)) {
                             case 0x0001:    /* ARP   */
                                 {

                                     ArpPktData_t * p_arp = (ArpPktData_t *)l3_ptr  ;
                                     VERBOSE_M ( ETH_L3_MESSAGES )
                                         printf("%s Received  L2.EthType: 0x%04x     ICMP Packet!! %s \n",C_RED, 0x0806, C_NORMAL);
                                      VERBOSE_M ( ETH_L3_DETAILS )
                                     {
                                         PrintBuff((uint8_t*) l3_ptr, sizeof(ArpPktData_t) , l3_ptr ,"ArpPktData_t:");
                                         printArpPktData_t(p_arp);
                                     }

                                     if ( ntohl(p_arp->TarProtocolAddr) == g_IP_eth0)
                                     {
                                         ArpPktData_t  d;

                                         VERBOSE_M( ETH_L3_MESSAGES ) printf("%s IT is for ME!! %s \n",C_RED, C_NORMAL);

                                         // save off the original
                                          memcpy(&d , p_arp, sizeof(ArpPktData_t));
                                         // move the macs
                                          SetMAC( &(p_arp->SrcHwAddr), &d.TarHwAddr);
                                          SetMAC( &(p_arp->TarHwAddr), &g_Mac_eth0 );
                                          // move the IP address
                                          p_arp->SrcProtocolAddr = d.TarProtocolAddr;
                                          p_arp->TarProtocolAddr = d.SrcProtocolAddr;
                                          // set opcode to response
                                          p_arp->OpCode = htons(0x0002);

                                          // swap L2_hdr mac addresses
                                          SetMAC(  (MacAddr_t *) l2_ptr       , &d.SrcHwAddr);
                                          SetMAC(  (MacAddr_t *) (l2_ptr + 6 ), &g_Mac_eth0);

                                          VERBOSE_M( ETH_L3_DETAILS )  PrintBuff((uint8_t*) l2_ptr, sizeof(ArpPktData_t)+ l2_hdr.hdr_sz , l2_ptr ,"Arp Response:");
                                     }
                                 }
                                 break;
                             case 0x0800:    /* IPV4  */
                                {
                                   IPv4_Hdr_t ipV4;
                                   ICMPPktData_t * p_ICMP;
                                   GetIPv4Data( l3_ptr, &ipV4 ) ;

                                   VERBOSE_M( ETH_L3_DETAILS )    printIPv4_Hdr_t(  &ipV4);

                                   l4_ptr = l3_ptr + (ipV4.HeaderLength * 4)  ;   // point to the L3 Data type
                                   switch ( ipV4.Protocol ) {
                                        case 0x01:                       // ICMP - assume for now ECHO  - EVOL EVOL EVOL  assume it is an echo for now ??
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES)  printf(" IPV4 Protocol:  ICMP  0x%04x\n",ipV4.Protocol );
                                            p_ICMP = ((ICMPPktData_t *) l4_ptr);
                                            printICMPPktData_t( p_ICMP  );
                                            switch (p_ICMP->Type) {
                                            case 0x00:
                                                printf("ICMP echo Reply\n");
                                                // BUG HERE- need to figure out how to fix the CRC.  
                                                //           then change
                                                //           ((ICMPPktData_t *)(l4_ptr))->Type =0; 
                                                // swap the MAC addrss
                                                SwapMAC((MAC_Hdr_t *) l2_ptr );                                                 
                                                // swap the IP addrss
                                                SwapIPv4_Hdr_IPaddr( ( IPv4_Hdr_t *) l3_ptr);  
                                               break;
                                            case 0x03:
                                               printf("ICMP Destination Unreachable 0x03\n");  
                                               break;
                                            case 0x08:
                                               printf("ICMP echo request\n");  
                                                // BUG HERE- need to figure out how to fix the CRC. in case 00 above.  
                                                //           Then requst will only need to forward packes.  
                                                //           Then, delete the swaps,and just forward the packet.  
                                                // swap the MAC addrss
                                                SwapMAC((MAC_Hdr_t *) l2_ptr );                                                 
                                                // swap the IP addrss
                                                SwapIPv4_Hdr_IPaddr( ( IPv4_Hdr_t *) l3_ptr);  
 
                                               break;
                                            default:
                                               printf("ICMP unrecognized \"Type\": %d\n", p_ICMP->Type );
                                               break;
                                            }
                                           break;
                                        case 0x06:                       // TCP
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES)  printf(" IPV4 Protocol:  TCP 0x%04x \n",ipV4.Protocol );
                                            break;
                                        case 0x11:                       // UDP
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES)printf(" IPV4 Protocol:  UDP 0x%04x  event_port_id %d \n",ipV4.Protocol,events[i].queue_id );
                                            {
                                                UDP_Hdr_t *p_udp_hdr;
                                                p_udp_hdr = ( UDP_Hdr_t *) l4_ptr;
                                                VERBOSE_M(ETH_L4_UDP_DETAILS ) printUDP_Hdr(p_udp_hdr);
                                                if( g_loopback_traffic == 1)
                                                {
                                                    // swap the MAC addrss
                                                    SwapMAC((MAC_Hdr_t *) l2_ptr );                                                 
                                                    // swap the IP addrss
                                                    SwapIPv4_Hdr_IPaddr( ( IPv4_Hdr_t *) l3_ptr);  
                                                    // swap the des Port ??
                                                    SwapUDP_Hdr_Ports(  (UDP_Hdr_t *) l4_ptr  );          
                                                 } // end g_loopback-traffic  
                                            }
                                            break;
                                        case 0x2f:                       // GRE
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES) printf(" IPV4 Protocol:  GRE 0x%04x \n",ipV4.Protocol );
                                            break;
                                        case 0x32:                       // IPSEC
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES) printf(" IPV4 Protocol:  IPSEC 0x%04x\n",ipV4.Protocol );
                                            break;
                                        default:                       // TCP
                                            VERBOSE_M(ETH_L3_IPV4_MESSAGES)  printf(" Unrecognized IPV4 Protocol:  %d\n",htons(ipV4.Protocol ));
                                            break;
                                       } // end switch - IPV4 protocol

                                 }  // end ipv4 packet
                                 break;
                             case 0x86dd:    /* IPV6 */
                             default:
                                 VERBOSE_M (ETH_L3_MESSAGES ) printf("%s Received  L2.EthType: 0x%04x  -- UNKNOWN / Unhandled -- %s \n",C_RED,htons(l2_hdr.EtherType), C_NORMAL);
                                 break;
                         } // end switch EthType
                         // Disposition the packet.
                         if ( g_drop_all_traffic  == 1)
                         {

                             rte_pktmbuf_free( events[i].mbuf  );
                         }
                         else
                         {
#ifdef LOOP_CALLSSSS
                             CALL_RTE("rte_event_eth_tx_adapter_enqueue()");
#endif
                             while (!rte_event_eth_tx_adapter_enqueue(event_dev_id,
                                                                      event_port_id,   // here is where I map to other interface??
                                                            &(events[i]), 1, 0) &&
                                                           ( g_force_quit != true ) ) ;
                         }




                     }
                     break;  // end event typ ETH

//////$$$$$$$$
//////$$$$$$$$  Event type - UNRECOGNIZED
//////$$$$$$$$
                default:
                     printf("  c%d) unrecognized event  %d \n",lcore_id,events[i].event_type);
                     printf("  THIS IS A BUG, NOT SURE WHAT TO DO.\n");
                     print_rte_event(0,"unrecognized event",&events[i] );
                     break;
               } // end switch -> *event type )

            }   // end for i -> nb_events

    }  // end while( g_force != 0 }
    return 0;
}

int test_print(__attribute__((unused)) void * arg)
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

int test_cleanup(__attribute__((unused)) void * arg)
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


/* clear time resources*/

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

void  test_description(void)
{
    printf(" \"test\" - framework to try customer configurations. \n");
    printf("                 \n");
}



struct test_mode_struct  tm_test = {
      .setup         = test_setup,
      .main_loop     = test_loop,
      .print_results = test_print,
      .cleanup       = test_cleanup,
      .description   = test_description,
};





