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
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>

#include "fprintbuff.h"
#include "fs_print_rte_mbuff.h"
#include "fs_extras.h"
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
#include "fs_net_common.h"
#include "fs_ethdev.h"
#include "fs_eventdev.h"

#include <rte_event_timer_adapter.h>   // for event timer

//#include "fs_print_structures.c"
// eth dev structures


#include "fs_print_rte_ethdev_struct.h"
#include "fs_eventdev_timer.h"

#include "fs_global_vars.h"

char  adapter_clk_sources[6][64]={"RTE_EVENT_TIMER_ADAPTER_CPU_CLK",
                                "RTE_EVENT_TIMER_ADAPTER_EXT_CLK0", 
                                "RTE_EVENT_TIMER_ADAPTER_EXT_CLK1", 
                                "RTE_EVENT_TIMER_ADAPTER_EXT_CLK2", 
                                "RTE_EVENT_TIMER_ADAPTER_EXT_CLK3",
                                "Illegal_Value"} ; 

char * get_clk_src( int v);
char * get_clk_src( int v)
{
   if( (v  > 4)||  ( v < 0 )) return (adapter_clk_sources[4]);
   return adapter_clk_sources[v];
}


//#include "fs_print_rte_structures.h"

extern uint64_t  g_core_messages;
extern int64_t g_print_interval;

void timer_print_setup(void);
void timer_print_setup(void)
{
printf("\n"
"  ------------                                     \n"
"  |evt_que 0 | --                                  \n"
"  ------------   \\     ------------     --------- \n"
"                    >   | evt_prt 0| --> | core 0| \n" 
"                        ------------     --------- \n"
"                                                   \n"
"                                                   \n"
"  ------------                                     \n"
"  |evt_que 1 | --                                  \n"
"  ------------   \\     ------------     --------- \n"
"                    >   | evt_prt 1| --> | core 1| \n" 
"                        ------------     --------- \n"
"                                                   \n"
"                                                   \n"
"  ------------                                     \n"
"  |evt_que 2 | --                                  \n"
"  ------------   \\     ------------     --------- \n"
"                    >   | evt_prt 2| --> | core 2| \n" 
"                        ------------     --------- \n"
"                                                   \n"
"                                                   \n"
"  ------------                                     \n"
"  |evt_que 3 | --                                  \n"
"  ------------   \\     ------------     --------- \n"
"                    >   | evt_prt 3| --> | core 3| \n" 
"                        ------------     --------- \n"
"                                                   \n"
"                         \n"
"\n");
}


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
int        timer_setup( __attribute__((unused))void * arg);
int         timer_loop( __attribute__((unused))void * arg);
int        timer_print( __attribute__((unused))void * arg);
int      timer_cleanup( __attribute__((unused))void * arg);
void timer_description( void);

/////
/////
/////  Configer event_timer  hw.
/////
/////   - must have event dev setup here.

// https://doc.dpdk.org/guides/prog_guide/event_timer_adapter.html?highlight=rte_event_timer
// "app/test/test_event_timer_adapter.c" 

void  timer_event_init(void);
void  timer_event_init(void)
{
    // only crating 1 timer instance of 100ms with 40 possible entries/events ??
    struct rte_event_timer_adapter_info  timer_adapter_info;
    uint32_t caps = 0;
    int ret = 0;
    uint8_t flags ;
    flags = RTE_EVENT_TIMER_ADAPTER_F_ADJUST_RES;
//    flags = 0;


    //  create an event timer adapter instance
struct rte_event_timer_adapter_conf time_adapter_config = {
     .event_dev_id        = g_glob.event_dev_id,
     .timer_adapter_id    = 0,
     .socket_id           = rte_socket_id(),    // numa socket
     .clk_src             = RTE_EVENT_TIMER_ADAPTER_CPU_CLK,
     .timer_tick_ns       = 1E7,                  // 10ms  
     .max_tmo_ns          = 5*1E9,                  // 5 sec   
     .nb_timers           = 4000,                 // fs reduced from 40000 to 40
     .flags               = flags
    };

    WAI();

    ret = rte_event_timer_adapter_caps_get(g_glob.event_dev_id, &caps);
    if ( ret != 0 )
    {
       printf("failed to get adapter capabilities  %d\n",ret);
       printf(" Did you bind a timer event to pci-vfio and is it listed in dpdk args? \n");
       rte_panic("rte_event_timer_adapter_caps_get() failed   \n");
    }

    print_rte_event_timer_adapter_conf(0, "timer_100ms",0 ,&time_adapter_config);
    CALL_RTE("rte_event_timer_adapter_create()" );
    g_glob.timer_100ms = rte_event_timer_adapter_create(&time_adapter_config);
    if ( g_glob.timer_100ms == NULL)
    {
         // EVOL - memory leak here all the buffers allocated for glob structure are hanging
         printf(" Did you bind a timer event to pci-vfio and is it listed in dpdk args? \n");
         rte_panic("Cannot init timer_adapter: rte_event_timer_adapter_create()   \n");
    }
    else
    {
         printf(" successfully created timer_adapter  %p \n", g_glob.timer_100ms);
    }
    // Call rte_event_timer_adapter_get_info()  to get this timer's
    //     min_resolution_ns
    //    max_tmo_ns
    //   adapter_conf

    CALL_RTE("rte_event_timer_adapter_get_info()  " );
    rte_event_timer_adapter_get_info(  g_glob.timer_100ms ,&timer_adapter_info);
    print_rte_event_timer_adapter_info ( 0 , "timer_adapter_info" , 0 , &timer_adapter_info);

    return;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
void      timer_event_start(void)
{
   WAI();
rte_event_timer_adapter_start( g_glob.timer_100ms);
}




int timer_setup( __attribute__((unused)) void * arg)
{

    WAI();
    timer_print_setup();

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
    g_glob.evq.nb_queues = 4 ;  // total number of event queues in my design
    g_glob.evp.nb_ports  = 4 ;  // total number of event ports in my design.  


    // adapters rx & tx
    g_glob.rx_adptr.nb_rx_adptr = 0 ;  // total number of rx_adapters in my design
    g_glob.tx_adptr.nb_tx_adptr = 0 ;  // total number of tx_adapters in my design.  



    // event dev queue to event dev port map
    
    // allocate storage arrays based on evp and evq , rx_adapters, and tx_adaptersabove.
     
    g_glob.evq.event_q_cfg = (event_queue_cfg_t *)malloc(sizeof(event_queue_cfg_t) * g_glob.evq.nb_queues);
    if (!g_glob.evq.event_q_cfg) {
                rte_panic("Memory allocation failure \n");
    }
    memset( g_glob.evq.event_q_cfg , 0 , (sizeof(event_queue_cfg_t) * g_glob.evq.nb_queues));


    g_glob.evp.event_p_id = (q_id_and_priority_t *)malloc(sizeof(q_id_and_priority_t) * g_glob.evp.nb_ports);
    if (!g_glob.evp.event_p_id) {
                free( g_glob.evq.event_q_cfg);
                rte_panic("Memory allocation failure \n");
    }
    memset( g_glob.evp.event_p_id , 0 , (sizeof(q_id_and_priority_t) * g_glob.evp.nb_ports));

    g_glob.rx_adptr.rx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.rx_adptr.nb_rx_adptr);
    if (!g_glob.rx_adptr.rx_adptr) {
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_cfg);
                rte_panic("Failed to allocate memery for Rx adapter\n");
    }

    g_glob.tx_adptr.tx_adptr = (uint8_t *)malloc(sizeof(uint8_t) *
                                        g_glob.tx_adptr.nb_tx_adptr);
    if (!g_glob.tx_adptr.tx_adptr) {
                free(g_glob.rx_adptr.rx_adptr);
                free(g_glob.evp.event_p_id);
                free(g_glob.evq.event_q_cfg);
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
    ( ptr + 1 )->ev_q_conf.priority = 0x80;

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
    ( ptr + 3 )->ev_q_conf.priority = 0x80;

}




// now link the event_queues to the event ports
//  to do this...   
//       the link command is port based and wants an array of
//       queues and the queue priority        


 // port 0  - one  queues (0 ), queue Priority 0x80 
   (g_glob.evp.event_p_id + 0)->nb_links = 1;
   (g_glob.evp.event_p_id + 0)->q_id[0] = 0;
   (g_glob.evp.event_p_id + 0)->pri[0]  = 80;
 
 // port 1  - one queues (1), queue Priority 0x80 
   (g_glob.evp.event_p_id + 1)->nb_links = 1;
   (g_glob.evp.event_p_id + 1)->q_id[0] = 1;
   (g_glob.evp.event_p_id + 1)->pri[0]  = 80;

  // port 2  - one queues (2), queue Priority 0x80
   (g_glob.evp.event_p_id + 2)->nb_links = 2;
   (g_glob.evp.event_p_id + 2)->q_id[0] = 2;
   (g_glob.evp.event_p_id + 2)->pri[0]  = 80;

  // port 3  - one queues (3), queue Priority 0x80
   (g_glob.evp.event_p_id + 3)->nb_links = 2;
   (g_glob.evp.event_p_id + 3)->q_id[0] = 3;
   (g_glob.evp.event_p_id + 3)->pri[0]  = 80;
 


#ifdef  ENABLE_ETHERNET  // no ethernet adapters
// what is missing from below is portID to go with the queue_id
    g_glob.rx_adptr.rx_adptr[0]  = 0 ; //  rx_adapter 0 assigned to queue 0.
    g_glob.rx_adptr.rx_adptr[1]  = 0 ; //  rx_adapter 1 assigned to queue 0. what is missing here is port_id

    g_glob.tx_adptr.tx_adptr[0]  = 0 ; //  tx_adapter 0 assigned to queue 0.
    g_glob.tx_adptr.tx_adptr[1]  = 0 ; //  tx_adapter 1 assigned to queue 0.
#endif


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

//    initialize_eth_dev_ports();


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

/////////////////////  
/////////////////////  
/////////////////////  
/////////////////////  
//  connect "event dev"  to "ethernet devices" 
//    create rx_adapters    
//    associate adapter to eth queue and event queue
//    create tx_adapters    
//    associate adapter to eth queue and event queue
 
//    rx_tx_adapter_setup_internal_port();;

/////
/////
/////  Start event_dev subsystem 
/////
/////

    start_event_dev();

/////
/////
/////  Start the ethernet interfaces
/////
/////
#ifdef   ENABLE_ETHERNET 
     ethdev_start();

    {
    //     uint32_t service_id;
         uint32_t caps,i,ret ;
         for(i = 0 ; i < 2 ; i++)
         {
          
         CALL_RTE("rte_event_eth_rx_adapter_caps_get()");
         ret = rte_event_eth_rx_adapter_caps_get(g_glob.event_dev_id,
                                  g_glob.rx_adptr.rx_adptr[i], &caps);
         if( ret != 0)
         {
             printf("ERR: rte_event_eth_rx_adapter_caps_get returned %d\n",ret);
         }
         printf("rte_event_eth_rx_adapter_caps:0x%8x \n",caps );

         ret = rte_event_eth_tx_adapter_caps_get(g_glob.event_dev_id,
                                  g_glob.tx_adptr.tx_adptr[i], &caps);
         if( ret != 0)
         {
             printf("ERR: rte_event_eth_rx_adapter_caps_get returned %d\n",ret);
         }
 

         printf("rte_event_eth_rx_adapter_caps:0x%8x \n,",caps);
         }

    }

#endif

/////
/////
/////  Start timer events
/////
/////

    timer_event_init();
    timer_event_start();

    return 0;
}


// Stuff the timer event structure with the values I want
// and make it cleaner for other tests to generate timer events.

 struct rte_event_timer * gen_timer_ev( struct rte_event_timer * p_evt_timer,
                                        uint32_t flow_id,
                                        uint8_t  sched_type,
                                        uint8_t  evt_queue,
                                        uint8_t  evt_queue_priority,
                                        void *   evt_ptr) 
{
     if( p_evt_timer == NULL)
     {
         printf( " ERROR-- I DO NOT TAKE NULL AT THIS TIME \n");
         // future, allocate a structure from heap or pool.  
     }  
     memset(p_evt_timer,0, sizeof(struct rte_event_timer)); 
     p_evt_timer->state  =  RTE_EVENT_TIMER_NOT_ARMED;  /* set this per documentation*/  
     p_evt_timer->timeout_ticks = 100;                  /*time in ns (5 seconds for now ???)*/ 
    // p_evt_timer->impl_opaque[2] ;  per documentation, do not touch
    // p_evt_timer->user_meta[0] = 0;                        // my application defined data

    // In order to get sso to deliver, I have to add the following to the WQE
    // p_evt_timer->ev.event=0;    // set event "union" fields to 0

    p_evt_timer->ev.flow_id        = flow_id;                // uint32_t flow_id:20;
    p_evt_timer->ev.sub_event_type = RTE_EVENT_TYPE_TIMER ;  // uint32_t sub_event_type:8;
    p_evt_timer->ev.event_type     = RTE_EVENT_TYPE_TIMER ;  // uint32_t event_type:4;
    p_evt_timer->ev.op             = RTE_EVENT_OP_NEW;       // uint8_t op:2; NEW,FORWARD or RELEASE
    //p_evt_timer-ev.rsvd           =  ;                     // uint8_t rsvd:4;
    p_evt_timer->ev.sched_type     = sched_type;             // uint8_t  sched_type:2;  RTE_SCHED_TYPE_PARALLEL
    p_evt_timer->ev.queue_id       = evt_queue ;             // uint8_t  queue_id; event queue message will be placed in
    p_evt_timer->ev.priority       = evt_queue_priority ;    // uint8_t  priority;
    //p_evt_timer->ev.impl_opaque    =  ;                    // uint8_t  impl_opaque;

    p_evt_timer->ev.event_ptr  = (void *) evt_ptr ;      // set the second 64 bits to point this event structure

    return p_evt_timer;

}    


#define BATCH_SIZE  4

char  timer_m0[] = { "As"        };
char  timer_m1[] = { "time"      };
char  timer_m2[] = { "goes"      };
char  timer_m3[] = { "by"        };
char * timer_message[] = {timer_m0,timer_m1,timer_m2,timer_m3};


extern int g_drop_all_traffic ;

extern struct rte_event       g_ev;          // use this to encode an event.
struct rte_event_timer g_ev_timer;    // use this to encode a timer event.

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//     _loop Function

int timer_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    uint8_t  event_dev_id ;
   
    struct rte_event events[BATCH_SIZE];
    uint16_t nb_rx; 
    uint16_t ret;
    int i;
    uint8_t core_2_message[]           = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t core_2_evt_port_id[]       = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t core_2_next_evt_queue_id[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,2,3,0 };
    uint8_t event_port_id ;
    struct rte_event_timer *p_ev_timer;   // pointer to timer event

    int core_counter = 0;

    lcore_id   = rte_lcore_id();      // my core index
    event_dev_id     = g_glob.event_dev_id;   // id of my event device

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
         //// Set up timer event structure and add  event_dev sso
         p_ev_timer = &g_ev_timer;
    
         p_ev_timer = gen_timer_ev( p_ev_timer,                 /* struct rte_event_timer *   */
                                     0x07734 ,                  /* uint32_t flow_id           */
                                     RTE_SCHED_TYPE_PARALLEL,   /* uint8_t  sched_type        */
                                     0,                         /* uint8_t  evt_queue         */
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
    }

    if( (lcore_id == 23 )  && ( g_core_messages == 1))
    {
         printf("*** Put an event into the Event Dev Scheduler ***\n");

         printf("        event_dev_id:  %d \n ",event_dev_id);
         printf("        event_port_id: %d \n ",event_port_id);
         printf("        lcore_id:      %d \n ",lcore_id);
         printf("        print every :  %ldth  \n ",g_print_interval);

#ifdef ENABLE_CORE_MESSAGES
         memset(&g_ev,0, sizeof(struct rte_event)); 
         g_ev.event=0;    // set event "union" fields to 0
         g_ev.event_ptr  = (void *) (timer_message[lcore_id]) ;  // set the second 64 bits to point at a payload
     
// In order to get sso to deliver, I have to add the following to the WQE
         g_ev.flow_id        = 0xDEAD;                 // uint32_t flow_id:20;
         g_ev.sub_event_type = RTE_EVENT_TYPE_CPU ;    // uint32_t sub_event_type:8;
         g_ev.event_type     = RTE_EVENT_TYPE_CPU ;    // uint32_t event_type:4;
         g_ev.op             = RTE_EVENT_OP_NEW;         // uint8_t op:2; NEW,FORWARD or RELEASE
         //g_ev.rsvd           =  ;                      // uint8_t rsvd:4;
         g_ev.sched_type     = RTE_SCHED_TYPE_PARALLEL ;       // uint8_t  sched_type:2;
         g_ev.queue_id       = 0 ;                             // uint8_t  queue_id;
         g_ev.priority       = RTE_EVENT_DEV_PRIORITY_NORMAL ; // uint8_t  priority;
         //g_ev.impl_opaque    =  ; // uint8_t  impl_opaque;
     
#ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf(" Call Args: event_dev_id:%d, lcore_id:%d ev: \n",event_dev_id,lcore_id);
         FONT_NORMAL();
#endif
         print_rte_event( 1, "g_ev",&g_ev);
         CALL_RTE("rte_event_enqueue_burst() ");
         ret = rte_event_enqueue_burst(event_dev_id, lcore_id  ,&g_ev, 1 );
         if( ret != 1 )
         {
              printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
              printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
              rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event\n");
         }
#endif
    }

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
            if( events[i].event_type == RTE_EVENT_TYPE_CPU )
            {     
 
                 printf("****    c%d) Received %d events **** \n",lcore_id,nb_rx);
                 print_rte_event( 0, "event[i]",&events[i]);
                 printf(" Message:   %s\n",(char *)events[i].event_ptr);
                 
                 printf("  c%d) Send message to next evt_queue (aka core for me)  %d queue\n",lcore_id,core_2_next_evt_queue_id[lcore_id]);
                 
                 // set to forward to next core.
                 events[i].queue_id       = core_2_next_evt_queue_id[lcore_id];
                 // set operation to forward packet    
                 //  events[0].op             = RTE_EVENT_OP_FORWARD;
                 events[i].flow_id        += 1;
                 events[i].event_ptr  = (void *)timer_message[lcore_id]; 
                 
                 rte_pause();
                 usleep(500000);     

                 ret = rte_event_enqueue_burst(event_dev_id, event_port_id,
                                     &(events[i]) , 1);
                 if( ret != 1 )
                 {
                      printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
                      printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
                      rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
                 }
            }
            if( events[i].event_type == RTE_EVENT_TYPE_TIMER )
            {     
                 printf("****    c%d) Received %d Timer events from evt_port_id %d**** \n",lcore_id,nb_rx,event_port_id);
                 print_rte_event( 0, "event[i]",&events[i]);
                 printf(" Message:   %s\n", timer_message[core_2_message [lcore_id] ] );
                 
                 printf("  c%d) Send timer message to  next evt_queue (aka core for me)  %d queue\n",lcore_id,core_2_next_evt_queue_id[lcore_id]);
 
                 p_ev_timer = events[i].event_ptr;
                 // set to forward to next core.
                 p_ev_timer->state          = RTE_EVENT_TIMER_NOT_ARMED;
                 p_ev_timer->ev.queue_id    = core_2_next_evt_queue_id[lcore_id];
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
            }
            else if (events[i].event_type == RTE_EVENT_TYPE_ETHDEV )  // event -> etherent packet
            {
//<FS>
                 core_counter--;
                 // only fall in if p_print_interval is >0 else do not print
                 if ((  core_counter < 0 ) && (g_print_interval > 0))
                 {
                     struct rte_mbuf *  p_mbuff;
                     uint8_t *          buf;

                     core_counter = g_print_interval;
                     p_mbuff = events[i].mbuf;

                     printf("%d< got packet  Stream  flow_id: 0x%05x  evt_queue_id: %d, evt_port_id %d\n",lcore_id,events[i].flow_id,events[i].queue_id,event_port_id );
                     if(g_verbose > 3)
                     {
                         buf = (uint8_t *) rte_pktmbuf_mtod(p_mbuff, struct rte_ether_hdr *);
                         PrintBuff((uint8_t*) buf, 0x40 , NULL ,"mbuff Raw Buffer");
                     }
                     
                     PrintBuff((uint8_t*) p_mbuff, (0x100 + p_mbuff->pkt_len + 0x10) , NULL ,"mbuff Raw Buffer");
                     print_rte_mbuf(0,p_mbuff);
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
                                                              event_port_id,
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
            } // unrecognized event type.  
        }   // end for i -> nb_events 
    }  // end while
    return 0;
}

int timer_print(__attribute__((unused)) void * arg)
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

int timer_cleanup(__attribute__((unused)) void * arg)
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


/* clear ethdev resourcesi*/
/* free the mbuff pool */
#if 0
    USED L2FWD_EVENT as example and this buffer is never freed
    printf("************\n    free teh pktmbuf_pool  event_dev device  sso   \n***************\n");
          rte_pktmbuf_free(g_p_pktmbuf_pool);
#endif
     return 0;
}

void  timer_description(void)
{
    printf(" \"ethdev\" - try to set up event mode with event_ethdev to pass traffic\n");
    printf("                in and out of board.  Later add RSS to spread acroos \n");
    printf("                mulitple cores \n");
}



struct test_mode_struct  tm_timer = {
      .setup         = timer_setup,
      .main_loop     = timer_loop,
      .print_results = timer_print,
      .cleanup       = timer_cleanup,
      .description   = timer_description,
};






#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_timer_adapter_conf  (int indent,const char* string,int id,struct rte_event_timer_adapter_conf *p)
{
    INDENT(indent);
    FONT_DATA_STRUCTURES_COLOR();

    printf("%sstruct rte_event_timer_adapter_conf %s id=%d  addr:%p  {\n",s,string,id,p);
    printf("%s    uint8_t   event_dev_id                 :  %d\n",s,p->event_dev_id );
    printf("%s    uint16_t  timer_adapter_id             :  %d\n",s,p->timer_adapter_id );
    printf("%s    uint32_t  socket_id                    :  %d\n",s,p->socket_id  );
    printf("%s    enum rte_event_timer_adapter clk_src   :  %d   %s \n",s,p->clk_src,get_clk_src(p->clk_src)  );
    printf("%s    uint64_t  timer_tick_ns                :  %ld\n",s,p->timer_tick_ns  );
    printf("%s    uint64_t  max_tmo_ns                   :  %ld\n",s,p->max_tmo_ns  );
    printf("%s    uint64_t  nb_timers                    :  %ld\n",s,p->nb_timers  );
    printf("%s    uint64_t  flags                        :  %ld\n",s,p->flags  );
    printf("%s}\n",s );
    FONT_NORMAL();
}
#else
#define print_rte_event_timer_adapter_conf  (w,x,y,z)
#endif



#ifdef PRINT_DATA_STRUCTURES

void print_rte_event_timer_adapter_info   (int indent,const char* string,int id,struct rte_event_timer_adapter_info  *p);
void print_rte_event_timer_adapter_info   (int indent,const char* string,int id,struct rte_event_timer_adapter_info  *p)
{
    INDENT(indent);
    FONT_DATA_STRUCTURES_COLOR();

    printf("%sstruct rte_event_timer_adapter_info  %s id=%d  addr:%p {\n",s,string,id,p );
    printf("%s   uint64_t      min_resolution_ns         %ld\n",s,p->min_resolution_ns  );
    printf("%s   uint64_t      max_tmo_ns                %ld\n",s,p->max_tmo_ns  );
    print_rte_event_timer_adapter_conf( indent+1, "conf ",0,&(p->conf));
    printf("%s   uint32_t      caps                      %d\n",s,p->caps  );
    printf("%s   int16_t       event_dev_port_id     %d\n",s,p->event_dev_port_id  );
    printf("%s}\n",s );
    FONT_NORMAL();
}

#else
#define print_rte_event_timer_adapter_info   (w,x,y,z)
#endif



#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_timer  (int indent,const char* string,struct rte_event_timer *p)
{
    INDENT(indent);
    FONT_DATA_STRUCTURES_COLOR();

    printf("%sstruct rte_event_timer  %s  add:%p    { \n",s,string,p);
    print_rte_event( indent+1 , "ev",&(p->ev));
    printf("%s    enum      state                        :  %d\n",s,p->state );
    printf("%s    uint64_t  timeout_ticks                :  %ld\n",s,p->timeout_ticks  );
    printf("%s    uint64_t  impl_opaque[0]               :  %ld\n",s,p->impl_opaque[0]  );
    printf("%s    uint64_t  impl_opaque[1]               :  %ld\n",s,p->impl_opaque[1]  );
    printf("%s    uint8_t   user_meta[0]                 :  %d\n",s,p->user_meta[0] );
    printf("%s} \n",s );
    FONT_NORMAL();
}
#else
#define print_rte_event_timer  (w,x,y,)
#endif


