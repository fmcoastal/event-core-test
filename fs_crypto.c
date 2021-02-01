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
#include <rte_crypto.h>
#include <rte_cryptodev.h>
#include <rte_event_crypto_adapter.h>


#include <rte_spinlock.h>
#include "fs_tstamp.h"
#include "fs_lpm_test.h"
 //  #include "fs_core.h"
#include "fs_crypto.h"



#include <rte_event_eth_rx_adapter.h>
#include "fs_extras.h"
#include "fs_spinlock_test.h"

#include "fs_eventdev.h"


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


// extern fs_time_stamp g_per_core_time_stamp[32]__rte_cache_aligned; // per core time stamp
// extern uint64_t  g_per_core_result[]; // per core time stamp


#if 0
????  not sure what to do on this.....

  Hash algo:    sha256
  Hash value:   20c800b78a602e5e37a78a79222da6937a9cb22f9d14dad7137e7b09a4c5f907
  Sign algo:    sha256,rsa2048:dev
  Sign value:   0f490c9a3f16a146b52a131662c15e2147a276f8fbbed20c2dfb24e04dbecc306a38ebbe1061fa5777b8b30ad795b5a0c6dc564607e247dcc22dad85d7266c8cb1ce3bff0e0cb9c2005b28c7f680c2406ab07a1125de5a6e7737df281fc6cee2aee6379f164bfc5997a9a89873718c2f08a96f7e32254fee53162c922192a34cfc3708d81f3ee2e58b7bdec8da08a27e294f4f9b807301b647aeb095f20edec64ab61a5aeb72d967af72b4c5c3ece4288e5b2df9b0a9992366cc18747ecdd10e6a6dd2a967f23435c2942c413b012dbe1b920aabc2a97bbabad485eeac50f4ca88ea424854a3f9dc196ad617a82a21e4f15e9643132ce4734e7c5fa59c2a78ae



#endif
char  c_m0[] = { "Killroy"        };
char  c_m1[] = { "was"         };
char  c_m2[] = { "there"       };
char  c_m3[] = { ":-)"};
char * cpt_message[] = {c_m0,c_m1,c_m2,c_m3};

                                                                                    
/*********************************************************************              
 *********************************************************************              
 *           REGULAR SPINLOCK TEST                                   *              
 *********************************************************************              
 *********************************************************************/

// extern rte_spinlock_t g_spinlock_measure_lock;
#define SpinLockFunction()  rte_spinlock_lock( &g_spinlock_measure_lock); rte_spinlock_unlock( &g_spinlock_measure_lock);

//  forward reference for compiler
int        crypto_setup( __attribute__((unused))void * arg);
int         crypto_loop( __attribute__((unused))void * arg);
int        crypto_print( __attribute__((unused))void * arg);
int      crypto_cleanup( __attribute__((unused))void * arg);
void crypto_description( void);


// FIGURE OUT THE four VARIABLE BELOW LATER
extern const char * StringSched[];
extern uint8_t g_evt_dev_id;    // this variable should be moved out of the fs_core file to a global evnet_dev_id for 
int g_cdev_id=0;                  // device id of the crypto device.   
                                // all eveent devices using the same SSO instance....
uint8_t g_nb_crypto_PortsPerCore ; 
uint8_t g_nb_crypto_QueuesPerCore ; 

int event_crypto_init(void);
// I will use   43.1.1  RTE_EVENT_CRYPTO_ADAPTER_OP_NEW mode
//     RTE_EVENT_CRYPTO_ADAPTER_CAP_INTERNAL_PORT_OP_FWD = RTE_EVENT_CRYPTO_ADAPTER_OP_NEW
//   I will try to use a Symetric encryption algoritm
//   <dpdk>/examples/fips_validation/main.c"



int  event_crypto_init(void)
{
struct rte_event_dev_info dev_info;
struct rte_event_port_conf conf = {0};
enum rte_event_crypto_adapter_mode mode;
int result;
uint32_t caps; 


   printf(" rte_event_dev_count() = %d \n",rte_event_dev_count() );
   if (!rte_event_dev_count())
   {
         /* If there is no hardware eventdev, or no software vdev was
          * specified on the command line, create an instance of
          * event_sw.
          */
          printf(" no event dev  device found:\n"); 
          printf("    did you create bind the SSO  to vfio-cpi?\n"); 
          return -1 ;
   }



   printf(" rte_crypto_dev_count() = %d \n",rte_cryptodev_count() );
   if ( rte_cryptodev_count() == 0 )
   {
      printf(" no crypto devices found:\ni"); 
      printf("    did you create vf's and  bind them to vfio-cpi?\n"); 
      printf("    did you add a -w 0002:10:00.x  to the app's rte arguments?\n");
      return -2 ;
   } 


   printf("**************************************************************\n");
   printf("***********crypto*********************************************\n");
   printf("**************************************************************\n");
   printf("**************************************************************\n");
   CALL_RTE("call rte_event_dev_info_get()");
   result  = rte_event_dev_info_get(g_evt_dev_id, &dev_info);

   if (result != 0)  printf( " rte_event_dev_info_get() failed: %d\n",result);

   print_rte_event_dev_info(0,"dev_info ",&dev_info);

   conf.new_event_threshold = dev_info.max_num_events;
   conf.dequeue_depth = dev_info.max_event_port_dequeue_depth;
   conf.enqueue_depth = dev_info.max_event_port_enqueue_depth;
   print_rte_event_port_conf( 1,"conf",g_evt_dev_id,  &conf);
 

   mode = RTE_EVENT_CRYPTO_ADAPTER_OP_FORWARD ;
   CALL_RTE("rte_event_crypto_adapter_create()");
   result = rte_event_crypto_adapter_create(g_evt_dev_id, g_cdev_id, &conf, mode);
   if (result != 0)  printf( " rte_event_crypto_adapter_create() failed: %d\n",result);

   printf(" rte_crypto_dev_count() = %d \n",rte_cryptodev_count() );

// get capabilities of crypto dev:
   CALL_RTE("call rte_event_crypto_adapter_caps_get()");
   result = rte_event_crypto_adapter_caps_get(g_evt_dev_id, g_cdev_id, &caps);
   if (result != 0)  printf( " rte_event_crypto_adapter_caps_get() failed: %d\n",result);

   printf(" crcaps = 0x%08x\n",caps);

//rte_event_crypto_adapter_create()

//   struct rte_cryptodev_qp_conf qp_conf = {128, NULL, NULL};

//   struct rte_cryptodev_config conf = {rte_socket_id(), 1, 0};
//   ret = rte_cryptodev_configure(env.dev_id, &conf);

   rte_event_dev_close (g_cdev_id);
   return 0;
}



int crypto_setup( __attribute__((unused)) void * arg)
{
int     result;
uint8_t      i;
unsigned int nb_lcores = rte_lcore_count();

uint8_t                       nb_event_dev_devices = 0;
struct  rte_event_dev_info    dev_info;
struct  rte_event_dev_config  event_dev_config = {0};


uint8_t                       event_q_id = 0;
struct  rte_event_queue_conf  def_q_conf;        // queue
struct  rte_event_queue_conf  event_q_conf; 
uint32_t event_queue_cfg = 0;

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
        printf(" found %d event devices \n", nb_event_dev_devices);

//        int rte_event_dev_info_get (uint8_t dev_id, struct rte_event_dev_info *dev_info)
        for (i = 0 ; i < nb_event_dev_devices ; i++)
        {   
           result = rte_event_dev_info_get(i,&dev_info);
           printf("  %d)  %s\n",i, dev_info.driver_name); 
        }
   
        if( nb_event_dev_devices != 1)
        {
            printf(" ** WARNING:: Number of  event dev devices found is not what is expected \n"); 
        }
        // WHY at least there is at least 1  device,  Index=0;
        g_evt_dev_id = 0; 


        g_nb_crypto_PortsPerCore = 1   ; 
        g_nb_crypto_QueuesPerCore = 1  ; // should this be queues per port?  Should this be = number of cores??
   
        printf(" g_evt_dev_id:  %d \n",g_evt_dev_id);
        printf(" nb_lcores:      %d \n",nb_lcores);
        printf(" g_nb_crypto_PortsPerCore:   %d \n",g_nb_crypto_PortsPerCore);
        printf(" g_nb_crypto_QueuesPerCore:  %d \n",g_nb_crypto_QueuesPerCore);
   }
 
   { 
//////////
//////////  rte_event_dev_configure()
//////////
//////////   Get default event_dev Setting and  Capabilities
        CALL_RTE("call rte_event_dev_info_get()");
        result = rte_event_dev_info_get( g_evt_dev_id, &dev_info );
        printf(" result    %d \n",result);
        printf(" default: dev_info \n");
        printf("      struct rte_event_dev_info: \n");
        printf("          char *        driver_name  %s \n",dev_info.driver_name);
        printf("          rte_device * dev          %p \n",dev_info.dev);
        printf("          uint32_t     min_dequeue_timeout_ns           0x%08x \n",dev_info.min_dequeue_timeout_ns         );  	
        printf("          uint32_t     max_dequeue_timeout_ns           0x%08x \n",dev_info.max_dequeue_timeout_ns         );  	
        printf("          uint32_t     dequeue_timeout_ns               0x%08x \n",dev_info.dequeue_timeout_ns             );  	
        printf("          uint8_t      max_event_queues                 0x%02x \n",dev_info.max_event_queues               ); 	
        printf("          uint32_t     max_event_queue_flows            0x%08x \n",dev_info.max_event_queue_flows          );  	
        printf("          uint8_t      max_event_queue_priority_levels  0x%02x \n",dev_info.max_event_queue_priority_levels); 	
        printf("          uint8_t      max_event_priority_levels        0x%02x \n",dev_info.max_event_priority_levels      ); 	
        printf("          uint8_t      max_event_ports                  0x%02x \n",dev_info.max_event_ports                ); 	
        printf("          uint8_t      max_event_port_dequeue_depth     0x%02x \n",dev_info.max_event_port_dequeue_depth   ); 	
        printf("          uint32_t     max_event_port_enqueue_depth     0x%08x \n",dev_info.max_event_port_enqueue_depth   );  	
        printf("          int32_t      max_num_events                   0x%08x \n",dev_info.max_num_events                 ); 	
        printf("          uint32_t     event_dev_cap                    0x%08x \n",dev_info.event_dev_cap                  );  	
    
   // check compatability
        if (dev_info.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
             event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;



     //////  rte_event_dev_configure()
    
        event_dev_config.dequeue_timeout_ns          = 0;                      // uint32_t     0 use Default values
        event_dev_config.nb_events_limit             = -1;                     // int32_t ??? I started with 100 and got errors, but l3fwd has -1, so copy for notw;
        event_dev_config.nb_event_queues             = (uint8_t)g_nb_crypto_QueuesPerCore * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_ports              = (uint8_t)g_nb_crypto_PortsPerCore  * nb_lcores;  // uint8_t   
        event_dev_config.nb_event_queue_flows        = 1024;                  // uint32_t 
        event_dev_config.nb_event_port_dequeue_depth = 1;                     // uint32_t   cpy l3fwd->needs to be 1 because we do in hw?
        event_dev_config.nb_event_port_enqueue_depth = 1;                     // uint32_t cpy l3fwd->needs to be 1 because we do in hw?
//////    event_dev_config.event_dev_cfg           = dev_info.event_dev_cap;  // uint32_t 
        event_dev_config.event_dev_cfg               = 0;                     // uint32_t   again copy what l3fwd has to see if we get the same respons
    
    
        printf(" config values for event_dev_config \n");
        printf("      struct rte_event_dev_config:  \n");
        printf("          uint32_t dequeue_timeout_ns             0x%08x \n", event_dev_config.dequeue_timeout_ns           );   	
        printf("          int32_t  nb_events_limit     x          0x%08x \n", event_dev_config.nb_events_limit              );  //why int vs uint ? 	
        printf("          uint8_t  nb_event_queues     x          0x%02x \n", event_dev_config.nb_event_queues              );   	
        printf("          uint8_t  nb_event_ports                 0x%02x \n", event_dev_config.nb_event_ports               );   	
        printf("          uint32_t nb_event_queue_flows     x     0x%08x \n", event_dev_config.nb_event_queue_flows         );   	
        printf("          uint32_t nb_event_port_dequeue_depth  x 0x%08x \n", event_dev_config.nb_event_port_dequeue_depth  );   	
        printf("          uint32_t nb_event_port_enqueue_depth  x 0x%08x \n", event_dev_config.nb_event_port_enqueue_depth  );   	
        printf("          uint32_t event_dev_cfg                  0x%08x \n", event_dev_config.event_dev_cfg                );   	

        printf(" %scall rte_event_dev_configure() %s\n",C_GREEN,C_NORMAL);
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
//            event_q_conf.schedule_type = evt_rsrc->sched_type;
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
       event_p_conf.disable_implicit_release = 0 ; // WHAT IS THIS ????


            printf(" Config: event port config  -> event_p_conf \n");
            printf("      struct rte_event_port_conf *\n") ; 
            printf("          int32_t  new_event_threshold        0x%08x \n", event_p_conf.new_event_threshold      );    	
            printf("          uint16_t dequeue_depth              0x%04x \n", event_p_conf.dequeue_depth            );    	
            printf("          uint16_t enqueue_depth              0x%04x \n", event_p_conf.enqueue_depth            );    	
            printf("          uint8_t  disable_implicit_release   0x%02x \n", event_p_conf.disable_implicit_release );  


     for (event_p_id = 0; event_p_id < (g_nb_crypto_PortsPerCore * nb_lcores) ; event_p_id++) {
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

        for (event_p_id = 0; event_p_id < (g_nb_crypto_PortsPerCore * nb_lcores) ; event_p_id++) 
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





     event_crypto_init();


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



#define LOCK_LOOPS (10*10)
#define BATCH_SIZE  4


int crypto_loop( __attribute__((unused)) void * arg)
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
    dev_id = g_evt_dev_id;
    port_id = lcore_id; 

    if( lcore_id == 0 )
    {

        printf("*** Put an event into the Event Dev Scheduler ***\n");

        printf("        dev_id: %d  \n ",dev_id);
        printf("        port_id: %d \n ",port_id);
        printf("        lcore_id: %d \n ",lcore_id);
      
        ev.event=0;    // set "event union" to 0
        ev.event_ptr  = (void *) (cpt_message[lcore_id]) ;  // set the second 64 bits to point at a payload

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

        print_rte_event( 1, "ev",&ev);

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
        print_rte_event(1, "event[0]",&events[0]);
        printf(" Message:   %s\n",(char *)events[0].event_ptr);

        printf("  c%d) Send cpt_message to next Port (aka core for me)  %d events\n",lcore_id,next_core[lcore_id]);

        // set to forward to next core.
        events[0].queue_id       = next_core[lcore_id];
        // set operation to forward packet    
        //  events[0].op             = RTE_EVENT_OP_FORWARD;
        events[0].flow_id        += 1;
        events[0].event_ptr  = (void *)cpt_message[lcore_id]; 

 
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

int crypto_print(__attribute__((unused)) void * arg)
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

int crypto_cleanup(__attribute__((unused)) void * arg)
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

void  crypto_description(void)
{
    printf(" \"crypto\" - send a packt to the crypto engine to encrypt \n");
    printf("                then send the result to decrypt\n");
    printf("                loop\n");
}



struct test_mode_struct  tm_crypto = {
      .setup         = crypto_setup,
      .main_loop     = crypto_loop,
      .print_results = crypto_print,
      .cleanup       = crypto_cleanup,
      .description   = crypto_description,
};



