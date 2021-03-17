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
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
#include "fs_core.h"



#include "fs_extras.h"
#include "fs_spinlock_test.h"

#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>
#include "fs_eventdev.h"


void print_core_setup(void);
void print_core_setup(void)
{
printf("\n"
"    ------------          -------------      ----------- \n"
"    |evt_que 0 | -->      | evt_prt 0 | -->  | core 20 | \n"
"    ------------          -------------      ----------- \n"
"                                                         \n"
"    ------------          -------------      ----------- \n"
"    |evt_que 1 | -->      | evt_prt 1 | -->  | core 21 | \n"
"    ------------          -------------      ----------- \n"
"                                                         \n"
"    ------------          -------------      ----------- \n"
"    |evt_que 2 | -->      | evt_prt 2 | -->  | core 22 | \n"
"    ------------          -------------      ----------- \n"
"                                                         \n"
"    ------------          -------------      ----------- \n"
"    |evt_que 3 | -->      | evt_prt 3 | -->  | core 23 | \n"
"    ------------          -------------      ----------- \n"
"                                                         \n"
"                                                         \n"
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


/********************************************************************/
//  forward reference for compiler
int        core_setup( __attribute__((unused))void * arg);
int         core_loop( __attribute__((unused))void * arg);
int        core_print( __attribute__((unused))void * arg);
int      core_cleanup( __attribute__((unused))void * arg);
void core_description( void);


uint8_t g_evt_dev_id;
uint8_t g_nb_PortsPerCore ; 
uint8_t g_nb_QueuesPerCore ; 


int core_setup( __attribute__((unused)) void * arg)
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
    print_core_setup();

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
        printf(" found %d event devices \n", nb_event_dev_devices);
 
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
        g_evt_dev_id = 0;     // assign dev_id  to 0

        g_nb_PortsPerCore = 1   ; 
        g_nb_QueuesPerCore = 1  ; // should this be queues per port?  Should this be = number of cores??
   
        printf(" g_evt_dev_id:  %d \n",g_evt_dev_id);
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
        result = rte_event_dev_info_get( g_evt_dev_id, &dev_info );
        printf(" result    %d \n",result);
        printf(" default: dev_info \n");
        print_rte_event_dev_info(0,"dev_info", &dev_info );   
 
   // check compatability - make sure queue will accept any type of RTE_SCHED_TYPE_* values: 
   //                                 ordered,parallel,atomic,ethdev,ccryptodev,timer,...
        if (dev_info.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
             event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;

     //////  rte_event_dev_configure()
    
        event_dev_config.dequeue_timeout_ns          = 0;            // uint32_t     0 use Default values
        event_dev_config.nb_events_limit             = -1;           // int32_t ??? I started with 100 and got errors, but l3fwd has -1, so copy for notw;
        event_dev_config.nb_event_queues             = (uint8_t)g_nb_QueuesPerCore * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_ports              = (uint8_t)g_nb_PortsPerCore  * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_queue_flows        = 1024;         // uint32_t 
        event_dev_config.nb_event_port_dequeue_depth = 1;            // uint32_t   cpy l3fwd->needs to be 1 because we do in hw?
        event_dev_config.nb_event_port_enqueue_depth = 1;            // uint32_t cpy l3fwd->needs to be 1 because we do in hw?
//////    event_dev_config.event_dev_cfg           = dev_info.event_dev_cap;  // uint32_t 
        event_dev_config.event_dev_cfg               = 0;            // uint32_t   again copy what l3fwd has to see if we get the same respons


#ifdef PRINT_CALL_ARGUMENTS
        FONT_CALL_ARGUMENTS_COLOR();
        printf(  " Call Args: g_evt_dev_id  %d  event_dev_config: \n",g_evt_dev_id );
        FONT_NORMAL();
#endif
        print_rte_event_dev_config( 0, "config values for event_dev_config",0,&event_dev_config);   

        CALL_RTE("rte_event_dev_configure()");
        result = rte_event_dev_configure(g_evt_dev_id, &event_dev_config);
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

            printf(" %scall rte_event_queue_default_conf_get() %s\n",C_GREEN,C_NORMAL);    
            rte_event_queue_default_conf_get(g_evt_dev_id, event_q_id, &def_q_conf);
            
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
//            event_q_conf.schedule_type = g_glob.sched_type;
            event_q_conf.schedule_type = RTE_SCHED_TYPE_ATOMIC ;  /*RTE_SCHED_TYPE_ATOMIC,RTE_SCHED_TYPE_ORDERED,RTE_SCHED_TYPE_PARALLEL */
/*            evt_rsrc->evq.event_q_id = (uint8_t *)malloc(sizeof(uint8_t) * evt_rsrc->evq.nb_queues);

            if (!evt_rsrc->evq.event_q_id)
                rte_panic("Memory allocation failure\n");
            
            for (event_q_id = 0; event_q_id < evt_rsrc->evq.nb_queues;
                                        event_q_id++) {
                ret = rte_event_queue_setup(event_d_id, event_q_id,
                                &event_q_conf);
                if (ret < 0)
                    rte_panic("Error in configuring event queue\n");
                evt_rsrc->evq.event_q_id[event_q_id] = event_q_id;
           }  //  loop throug the queue Info. */

           // this is how I will configure each queue for this event dev:
            printf(" Config: event queue config  -> event_q_conf \n");
            printf("      struct rte_event_queue_conf *\n") ; 
            printf("          uint32_t nb_atomic_flows              0x%08x \n", event_q_conf.nb_atomic_flows           );    	
            printf("          uint32_t nb_atomic_order_sequences    0x%08x \n", event_q_conf.nb_atomic_order_sequences );    	
            printf("          uint32_t event_queue_cfg              0x%08x \n", event_q_conf.event_queue_cfg           );    	
            printf("          uint8_t  schedule_type                0x%02x (%s)\n", event_q_conf.schedule_type
                                                                 ,StringSched[event_q_conf.schedule_type]             );    	
            printf("          uint8_t  priority                     0x%02x \n", event_q_conf.priority                  );    	


           // walk through the number event queues created as part of the event structure  
            for ( event_q_id= 0 ; event_q_id < event_dev_config.nb_event_queues ; event_q_id++)
           {
             // above I set teh number of queues and the number of ports = number of cores:
             //    So I should loop through each evnet_q_ID.
                 printf(" %scall rte_event_queue_setup() for event_q_id %d %s\n",C_GREEN,event_q_id, C_NORMAL);
                 ret = rte_event_queue_setup(g_evt_dev_id , event_q_id,
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
      
            printf(" %scall rte_event_port_default_conf_get() %s\n",C_GREEN,C_NORMAL); 
            rte_event_port_default_conf_get(g_evt_dev_id, 0, &def_p_conf);

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
         printf(" %scall rte_event_port_setup() for event_p_id %d %s\n",C_GREEN,event_p_id,C_NORMAL);
         ret = rte_event_port_setup(g_evt_dev_id, event_p_id,  &event_p_conf);
         if (ret < 0)
             rte_panic("Error in configuring event port %d\n",
                   event_p_id);

/*         ret = rte_event_port_link(g_evt_dev_id, event_p_id, NULL,
                       NULL, 0);
         if (ret < 0)
             rte_panic("Error in linking event port %d to queue\n",
                   event_p_id);
         evt_rsrc->evp.event_p_id[event_p_id] = event_p_id;
*/

//131         /* init spinlock */  // spin lock on what???
//132         rte_spinlock_init(&evt_rsrc->evp.lock);
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

        for (event_p_id = 0; event_p_id < (g_nb_PortsPerCore * nb_lcores) ; event_p_id++) 
        {
           queues_array[0] = event_p_id ;
           priorities_array[0] = RTE_EVENT_DEV_PRIORITY_NORMAL ;

           printf(" Linking Port %d to queue %d \n",event_p_id,event_p_id);

           printf(" %scall rte_event_port_link() for event_p_id %d %s\n",C_GREEN,event_p_id,C_NORMAL);
           ret = rte_event_port_link ( g_evt_dev_id , event_p_id, queues_array, priorities_array, nb_links);
           if (ret != nb_links )
           {
              rte_panic("Error in rte_event_port_link() requested %d successfull %d\n",nb_links,ret);
           }
        }
    }

    {
        uint32_t  evdev_service_id = 0;
        int32_t ret;
        //  THis might be for SW version of event dev MGR???  But sure what this is and how it is used.
        printf(" %scall  rte_event_dev_service_id_get()  %s\n",C_GREEN,C_NORMAL);
        ret = rte_event_dev_service_id_get(g_evt_dev_id,
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
        ret = rte_event_dev_start( g_evt_dev_id );
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





// Stuff the event structure with the values I want
// and make it cleaner for other tests to generate timer events.
//   NOTE+ by default, I set op= RTE_EVENT_OP_NEW
 struct rte_event * gen_ev( struct rte_event * p_ev,
                                        uint32_t flow_id,
                                        uint8_t  sched_type,
                                        uint8_t  evt_queue,
                                        uint8_t  evt_queue_priority,
                                        void *   evt_ptr)
{
     if( p_ev == NULL)
     {
         printf( " ERROR-- I DO NOT TAKE NULL AT THIS TIME %s\n",__FILE__);
         // future, allocate a structure from heap or pool.
     }
     memset(p_ev,0, sizeof(struct rte_event));

    // In order to get sso to deliver, I have to add the following to the WQE
    // p_ev->ev.event=0;    // set event "union" fields to 0

    p_ev->flow_id        = flow_id;                // uint32_t flow_id:20;
    p_ev->sub_event_type = RTE_EVENT_TYPE_CPU   ;  // uint32_t sub_event_type:8;
    p_ev->event_type     = RTE_EVENT_TYPE_CPU   ;  // uint32_t event_type:4;
    p_ev->op             = RTE_EVENT_OP_NEW;       // uint8_t op:2; NEW,FORWARD or RELEASE
    //p_ev->ev.rsvd           =  ;                     // uint8_t rsvd:4;
    p_ev->sched_type     = sched_type;             // uint8_t  sched_type:2;  RTE_SCHED_TYPE_PARALLEL
    p_ev->queue_id       = evt_queue ;             // uint8_t  queue_id; event queue message will be placed in
    p_ev->priority       = evt_queue_priority ;    // uint8_t  priority;
    //p_evr->impl_opaque    =  ;                    // uint8_t  impl_opaque;

    p_ev->event_ptr  = (void *) evt_ptr ;      // set the second 64 bits to point this event structure

    return p_ev;

}



struct rte_event   g_ev;

#define BATCH_SIZE  4

char  m0[] = { "Eat"        };
char  m1[] = { "At"         };
char  m2[] = { "Joes"       };
char  m3[] = { "Bar & Grill"};
char * core_message[] = {m0,m1,m2,m3};



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//     _loop Function
int core_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
//    int  i;
    uint8_t event_dev_id ;
    uint8_t event_port_id ;
    struct rte_event  * p_ev;
 
    uint16_t nb_rx;
    uint16_t i;
    struct rte_event events[BATCH_SIZE];

    uint16_t ret;
    // because my event device is 1 core to 1 port to 1 queue,  the table below is a
    //     a simple bit ot  
    uint8_t  core_2_message[]       = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t  core_2_next_message[]  = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,2,3,0 };
    uint8_t  core_2_evt_port_id[]       = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,2,3 };
    uint8_t  core_2_next_evt_queue_id[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,2,3,0 };

    if( ! ( rte_lcore_is_enabled(20)  && 
            rte_lcore_is_enabled(21)  &&
            rte_lcore_is_enabled(22)  &&
            rte_lcore_is_enabled(23) ))
    {
        printf(" this function expects you to use cores 20,21,22,24i (-c 0xf00000) ");
        rte_panic(" use the correct cores, or fix the core_2_port/message/queue mappings");
    }	

    lcore_id       = rte_lcore_id();             // my core index
    event_dev_id   = g_evt_dev_id;               // id of my event device
    event_port_id  = core_2_evt_port_id[ lcore_id ]; // for now, I have 1 port associated with 1 core.

    if( lcore_id == 20 )
    {

        printf("*** Put an event into the Event Dev Scheduler ***\n");
        printf("***     first message should come back to this core ***\n");

        printf("        event_dev_id:  %d \n",event_dev_id);
        printf("        event_port_id: %d \n",event_port_id);
        printf("        lcore_id:      %d \n",lcore_id);

        p_ev = &g_ev;      

        // fill in my rte_event with the field data I need.  note- op=RTE_EVENT_OP_NEW 
        p_ev = gen_ev( p_ev,                               /* struct rte_event * p_ev     */
                       0xdead,                             /* uint32_t flow_id            */
                       RTE_SCHED_TYPE_PARALLEL,            /* uint8_t  sched_type         */
                       core_2_evt_port_id[lcore_id],       /* uint8_t  evt_queue          */
                       RTE_EVENT_DEV_PRIORITY_NORMAL,      /* uint8_t  evt_queue_priority */
                       (void *) core_message[core_2_message[lcore_id]] /* void *evt_ptr) */
                       );

        // printout what I created
        print_rte_event( 1,"ev",p_ev);


/* static uint16_t rte_event_enqueue_burst (uint8_t dev_id, 
                                            uint8_t port_id,
                                            const struct rte_event ev[]
                                            ,uint16_t nb_events)	  */
        ret = rte_event_enqueue_burst(event_dev_id, core_2_evt_port_id[lcore_id], p_ev, 1 );
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
                print_rte_event(1, "event[0]",&events[0]);
                printf(" Message:   %s\n",(char *)events[0].event_ptr);
               
                printf("  c%d) Send message to next Port (aka core for me) port_id %d \n",lcore_id,core_2_evt_port_id[lcore_id]);
       
                // set to forward to next core.
                events[0].queue_id       = core_2_next_evt_queue_id[lcore_id] ;
                // set operation to forward packet    
                events[0].op             = RTE_EVENT_OP_FORWARD;  // this for some reason stops the event form forwarding
                events[0].flow_id        += 1;
                events[0].event_ptr  = (void *)core_message[core_2_next_message[lcore_id]]; 
    
        
                print_rte_event(1, " about to stuff this  event[0]",&events[0]);
                rte_pause();
                usleep(1000000);     
        
                ret = rte_event_enqueue_burst(event_dev_id, core_2_evt_port_id[lcore_id],
                                      events, nb_rx);
                if( ret != 1 )
                {
                     printf("ERR: rte_event_enqueue_burst returned %d \n",ret);
                     printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
                     rte_exit(EXIT_FAILURE, "Error failed to enqueue startup event");
                }
    
             } // end event - RTE_EVENT_TYPE_CPU
             else
             {
    
                  printf("  c%d) unrecognized event  %d \n",lcore_id,events[i].event_type);
                  printf("  THIS IS A BUG, NOT SURE WHAT TO DO.\n");
                  print_rte_event(0,"unrecognized event",&events[i] );
      
    
             }  // end event - unrecognized

       } // end nb_rx

    } // end force quit
    return 0;
}

int core_print(__attribute__((unused)) void * arg)
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

int core_cleanup(__attribute__((unused)) void * arg)
{
     // print the results from each core

     rte_event_dev_dump( g_evt_dev_id  ,stdout);


// Stop the device

    printf("************\n    stopping the event_dev device  sso   \n***************\n");
     rte_event_dev_stop (g_evt_dev_id);

//   CLose the event dev device




/* int rte_event_dev_close (   uint8_t     dev_id  )   
Close an event device. The device cannot be restarted! */
    printf("************\n    closing the event_dev device  sso   \n***************\n");
     rte_event_dev_close( g_evt_dev_id   );
 
     return 0;
}

void  core_description(void)
{
    printf(" \"core\" - try to set up event device to pass events \n");
    printf("                from core to core\n");
}



struct test_mode_struct  tm_core = {
      .setup         = core_setup,
      .main_loop     = core_loop,
      .print_results = core_print,
      .cleanup       = core_cleanup,
      .description   = core_description,
};



