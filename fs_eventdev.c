#include <stdint.h>
#include <stdint.h>



#include <rte_config.h>
#include <rte_spinlock.h>
#include <rte_eventdev.h>
#include <rte_ether.h>
#include <rte_event_eth_rx_adapter.h>



#include "fs_extras.h"
#include "fs_eventdev.h"
#include "fs_global_vars.h"







#if 1
 //void print_event_queue_cfg

 /**********************************
 A high level overview of the setup steps are:
 rte_event_dev_configure()
 rte_event_queue_setup()
 rte_event_port_setup()
 rte_event_port_link()

 rte_event_dev_start()    // start the device in a separate function

 #if 0
     event_dev_id = 0;                 // event dev_id index/handle => SSO  0
     nb_event_ports  = 0;         // event dev ports??
     memset(&(def_p_conf), 0, sizeof(struct rte_event_port_conf));
          def_p_conf.dequeue_depth =1;
          def_p_conf.enqueue_depth =1;
          def_p_conf.new_event_threshold = -1;
     sched_type = RTE_SCHED_TYPE_ATOMIC ;   //RTE_SCHED_TYPE_ORDERED  RTE_SCHED_TYPE_PARALLEL
 #endif

 ************************************/
 void Initialize_EventDev(void);
 void Initialize_EventDev(void)
 {
 int     result;
 uint8_t                       i;
 uint8_t                       nb_event_dev_devices = 0;
 struct  rte_event_dev_info    event_dev_capabilities;
 struct  rte_event_dev_config  event_dev_config = {0};

 uint8_t                       event_q_id = 0;
 struct  rte_event_queue_conf  def_q_conf = {0};      // queue
 struct  rte_event_queue_conf  event_q_conf = {0};
 uint32_t event_queue_cfg = 0;

     WAI();
    {
 //////////
 ////////// check there is an event dev device available
 //////////
 //////////
         /*uint8_t       rte_event_dev_count (void) */
         CALL_RTE("rte_event_dev_count()");
         nb_event_dev_devices = rte_event_dev_count();
         if ( nb_event_dev_devices  == 0)
         {
             printf(" no event dev devices found\n");
             printf(" did you forget to bind at least 1 event-dev device to vfio-pci\n");
             rte_exit(EXIT_FAILURE, "No Event Devices available");
         }
         WAI();
         printf(" Found %d event devices\n", nb_event_dev_devices);

 //  debug -print out all event dev devices - (my program only uses 1)
 //        int rte_event_dev_info_get (uint8_t dev_id, struct rte_event_dev_info *dev_info)
         for (i = 0 ; i < nb_event_dev_devices ; i++)
         {
            CALL_RTE("rte_event_dev_info_get()");
            result = rte_event_dev_info_get(i,&event_dev_capabilities);
            printf("  %d)  %s\n",i, event_dev_capabilities.driver_name);
         }

 //  My test- I expect just 1
         if( nb_event_dev_devices != 1)
         {
             printf(" ** WARNING:: Number of  event dev devices found is not what is expected \n");
         }
         // WHY at least there is at least 1  device,  Index=0;
         //  g_glob.event_dev_id = 0;     // assign dev_id  to 0 --> set at the top of the function

         printf(" g_glob.event_dev_id:   %d \n", g_glob.event_dev_id);
    }


 //////////
 //////////  rte_event_dev_configure()
 //////////
 //////////   Get default event_dev Setting and  Capabilities
    {
         CALL_RTE("rte_event_dev_info_get()");
         result = rte_event_dev_info_get( g_glob.event_dev_id , &event_dev_capabilities );
         printf(" result    %d \n",result);
         printf(" default: event_dev_capabilities \n");
         print_rte_event_dev_info(0,"event_dev_capabilities", &event_dev_capabilities);

         // check compatability - make sure queue will accept any type of RTE_SCHED_TYPE_* values:
         //                                 ordered,parallel,atomic,ethdev,ccryptodev,timer,...
         if ( event_dev_capabilities.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
              event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;

 //////  setup parameters for rte_event_dev_configure()

         event_dev_config.dequeue_timeout_ns          = 0;                      // uint32_t     0 use Default values
         event_dev_config.nb_events_limit             = -1;                     // int32_t ??? I started with 100 and got errors, but l3fwd has -1, so copy for notw;
         event_dev_config.nb_event_queues             = g_glob.evq.nb_queues ;  // uint8_t
         event_dev_config.nb_event_ports              = g_glob.evp.nb_ports  ;  // uint8_t
         event_dev_config.nb_event_queue_flows        = 1024;                  // uint32_t
         event_dev_config.nb_event_port_dequeue_depth = 1;                     // uint32_t   cpy l3fwd->needs to be 1 because we do in hw?
         event_dev_config.nb_event_port_enqueue_depth = 1;                     // uint32_t cpy l3fwd->needs to be 1 because we do in hw?
 //////    event_dev_config.event_dev_cfg           = dev_info.event_dev_cap;  // uint32_t
         event_dev_config.event_dev_cfg               = 0;                     // uint32_t   again copy what l3fwd has to see if we get the same respons


 #ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf(" int rte_event_dev_configure( uint8_t dev_id,\n"
                "                              const struct rte_event_dev_config * dev_conf \n"
                "                             )\n");
         printf(  " Call Args: g_glob.event_dev_id  %d  event_dev_config: \n",g_glob.event_dev_id );
         FONT_NORMAL();
 #endif
         print_rte_event_dev_config( 0, "event_dev_config",0,&event_dev_config);

         CALL_RTE("rte_event_dev_configure()");
         result = rte_event_dev_configure( g_glob.event_dev_id, &event_dev_config);
         if(result < 0)
         {
              printf(" rte_event_dev_configure() returned %d\n",result);
              rte_panic("Error in configuring event device\n");
         }
     }

 //////
 //////  rte_event_queue_setup()
 //////
 //////   Get default event_queue Setting and  Capabilities
     {
         int ret;
         event_queue_cfg_t *ptr;

 #ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();
         printf(" int rte_event_queue_default_conf_get( uint8_t dev_id,\n"
                "                                       uint8_t queue_id,\n"
                "                                       struct rte_event_queue_conf *    queue_conf \n"
                "                                       )\n");
         printf(  " Call Args: g_glob.event_dev_id:%d  event_q_id:%d  def_q_conf: \n",g_glob.event_dev_id,event_q_id );
         FONT_NORMAL();
 #endif
         CALL_RTE("rte_event_queue_default_conf_get()");
         rte_event_queue_default_conf_get( g_glob.event_dev_id , event_q_id, &def_q_conf);

         printf(" default: event dev queue info: event_q_id=%d \n",event_q_id);
         printf("      struct rte_event_queue_conf *\n") ;
         printf("          uint32_t nb_atomic_flows              %d (0x%08x) \n", def_q_conf.nb_atomic_flows, def_q_conf.nb_atomic_flows  );  //
         printf("          uint32_t nb_atomic_order_sequences    %d (0x%08x) \n", def_q_conf.nb_atomic_order_sequences, def_q_conf.nb_atomic_order_sequences );  //
         printf("          uint32_t event_queue_cfg              %d (0x%08x) \n", def_q_conf.event_queue_cfg , def_q_conf.event_queue_cfg );  //
         printf("                    values: #define RTE_EVENT_QUEUE_CFG_ALL_TYPES       (1ULL << 0)    \n");  //
         printf("                            #define RTE_EVENT_QUEUE_CFG_SINGLE_LINK     (1ULL << 1)    \n");  //
         printf("          uint8_t  schedule_type                %3d (0x%02x) (%s)\n", def_q_conf.schedule_type , def_q_conf.schedule_type
                                                              ,StringSched[def_q_conf.schedule_type] );  //
         printf("          uint8_t  priority                     %3d (0x%02x) \n", def_q_conf.priority , def_q_conf.priority );  //

         event_q_conf.nb_atomic_flows           = 1024;
         event_q_conf.nb_atomic_order_sequences = 1024;
         event_q_conf.event_queue_cfg           = event_queue_cfg;
         //     event_q_conf.schedule_type             = RTE_EVENT_QUEUE_CFG_ALL_TYPES ;
         event_q_conf.priority                  = RTE_EVENT_DEV_PRIORITY_NORMAL ;

         // bounds check
         if (def_q_conf.nb_atomic_flows < event_q_conf.nb_atomic_flows)
             event_q_conf.nb_atomic_flows = def_q_conf.nb_atomic_flows;

         // bounds check
         if (def_q_conf.nb_atomic_order_sequences <
                         event_q_conf.nb_atomic_order_sequences)
             event_q_conf.nb_atomic_order_sequences =
                         def_q_conf.nb_atomic_order_sequences;


        printf("***  Config: event queue config   *** \n");

        ptr = g_glob.evq.event_q_cfg;

        // walk through the number event queues created as part of the event structure
        for ( event_q_id= 0 ; event_q_id < event_dev_config.nb_event_queues ; event_q_id++)
        {
            // above I set teh number of queues and the number of ports = number of cores:
            //    So I should loop through each evnet_q_ID.
            // update per config changes in "Initialize "

            // event_q_conf.nb_atomic_flows
            // event_q_conf.nb_atomic_order_sequences
            event_q_conf.event_queue_cfg = ( ptr + event_q_id )->ev_q_conf.event_queue_cfg;
            event_q_conf.schedule_type   = ( ptr + event_q_id )->ev_q_conf.schedule_type ;
            event_q_conf.priority        = ( ptr + event_q_id )->ev_q_conf.priority;

 #ifdef PRINT_CALL_ARGUMENTS
            FONT_CALL_ARGUMENTS_COLOR();
            printf(  " Call Args: g_glob.event_dev_id:%d  event_q_id:%d  event_q_conf: \n",g_glob.event_dev_id,event_q_id );
            FONT_NORMAL();
 #endif
           // this is how I will configure each queue for this event dev:
            print_rte_event_queue_conf( 1 ,"event_q_id",event_q_id, &event_q_conf);
            CALL_RTE("call rte_event_queue_setup()");
            ret = rte_event_queue_setup(g_glob.event_dev_id , event_q_id,
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


        CALL_RTE("rte_event_port_default_conf_get()");
        rte_event_port_default_conf_get(g_glob.event_dev_id, 0, &def_p_conf);

        printf(" default: event port config  -> def_p_conf \n");
        printf("      struct rte_event_port_conf *\n") ;
        printf("          int32_t  new_event_threshold        %4d (0x%08x)\n", def_p_conf.new_event_threshold      , def_p_conf.new_event_threshold      );
        printf("          uint16_t dequeue_depth              %4d (0x%04x)\n", def_p_conf.dequeue_depth            , def_p_conf.dequeue_depth            );
        printf("          uint16_t enqueue_depth              %4d (0x%04x)\n", def_p_conf.enqueue_depth            , def_p_conf.enqueue_depth            );
        printf("          uint8_t  disable_implicit_release   %4d (0x%02x)\n", def_p_conf.disable_implicit_release , def_p_conf.disable_implicit_release );

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
        printf("          int32_t  new_event_threshold        %4d (0x%08x)\n", event_p_conf.new_event_threshold      , event_p_conf.new_event_threshold     );
        printf("          uint16_t dequeue_depth              %4d (0x%04x)\n", event_p_conf.dequeue_depth            , event_p_conf.dequeue_depth           );
        printf("          uint16_t enqueue_depth              %4d (0x%04x)\n", event_p_conf.enqueue_depth            , event_p_conf.enqueue_depth           );
        printf("          uint8_t  disable_implicit_release   %4d (0x%02x)\n", event_p_conf.disable_implicit_release , event_p_conf.disable_implicit_release);

        for (event_p_id = 0; event_p_id < g_glob.evp.nb_ports ; event_p_id++) {

 #ifdef PRINT_CALL_ARGUMENTS
            FONT_CALL_ARGUMENTS_COLOR();
            printf(  " Call Args: g_glob.event_dev_id:%d  event_p_id:%d  event_p_conf: \n",g_glob.event_dev_id,event_p_id );
            FONT_NORMAL();
 #endif
            print_rte_event_port_conf(1 , "event_p_conf", event_p_id, &event_p_conf);
            CALL_RTE("rte_event_port_setup()");
            ret = rte_event_port_setup( g_glob.event_dev_id , event_p_id,  &event_p_conf);
            if (ret < 0)
                rte_panic("Error in configuring event port %d\n", event_p_id);
        }
      }

 //////
 //////  rte_event_port_link()
 //////
 //////  connect an event dev port to array of queue_#, queue_priority
 //////
 //////  So we have n cores.   For this app, I want 1 core to have 1 port and 2 queues.
 /////                         we will use the Index to be the Index of the core??
     {
         int32_t ret;
         uint8_t port;
         uint8_t array_sz;
         uint8_t *queues_array;
         uint8_t *priorities_array;
         int      index;



         printf(" Linking event_ports to event_queues on event_dev_id:%d \n ",g_glob.event_dev_id);
         for ( port  = 0; port  < (g_glob.evp.nb_ports) ; port ++)
         {
              array_sz         =    (g_glob.evp.event_p_id + port)->nb_links ;
              queues_array     = &( (g_glob.evp.event_p_id + port)->q_id[0]);
              priorities_array = &( (g_glob.evp.event_p_id + port)->pri[0]);

 #ifdef PRINT_CALL_ARGUMENTS
         FONT_CALL_ARGUMENTS_COLOR();

              printf("       port# %d \n",port);
              printf("           number of queues  %d \n",array_sz);
              for (index = 0 ; index < array_sz  ; index++)
              {
                    printf("            %d) queue_id %d  queue_prioity %d\n",index
                                          , *( queues_array+index  )
                                          , *( priorities_array +index) );
              }

             printf("           int rte_event_port_link( uint8_t dev_id,    = %d  (g_glob.event_dev_id)\n", g_glob.event_dev_id);
             printf("                                    uint8_t port_id,   = %d  (port) \n",port);
             printf("                                    const uint8_t   queues[],   %p     \n",queues_array);
             printf("                                    const uint8_t   priorities[] %p \n", priorities_array);
             printf("                                    uint16_t nb_links  = %d  (array_sz) \n",array_sz);
             printf("                                    ) \n");


             FONT_NORMAL();
  #endif
             CALL_RTE("rte_event_port_link()");
             ret = rte_event_port_link ( g_glob.event_dev_id , port , queues_array, priorities_array, array_sz);
             if (ret != array_sz )
             {
                rte_panic("Error in rte_event_port_link() requested %d successfull %d\n",array_sz,ret);
             }
         }
     }


 {
         uint32_t  queue_count = 0xffffffff;
         uint32_t  port_count = 0xffffffff;

         rte_event_dev_attr_get(g_glob.event_dev_id, RTE_EVENT_DEV_ATTR_PORT_COUNT,&port_count);
         rte_event_dev_attr_get(g_glob.event_dev_id, RTE_EVENT_DEV_ATTR_QUEUE_COUNT,&queue_count);

         printf(" eventdev  Sanity Check for eventdev id: %d\n", g_glob.event_dev_id);
         printf("                  number of event ports: %u\n", port_count);
         printf("                 number of event queues: %u\n", queue_count);

 }




 //////
 //////
 //////   // not sure about this...
 //////
     {
         uint32_t  evdev_service_id = 0;
         int32_t ret;
         //  THis might be for SW version of event dev MGR???  But sure what this is and how it is used.
         CALL_RTE("rte_event_dev_service_id_get()");
         ret = rte_event_dev_service_id_get( g_glob.event_dev_id,
                                 &evdev_service_id);
         if (ret != -ESRCH && ret != 0) {
                 rte_panic("Error getting the service ID for sw eventdev\n");
         }
         printf(" service ID for event_dev (Device?) %d \n",evdev_service_id);
     }
    return ;

 }   // end  void Initialize_EventDev(void);

#endif




//////
//////  rte_event_dev_start()
//////
//////  So we have n cores.   For this app, I want 1 core to have 1 port and 1 queue.
/////                         we will use the Index to be the Index of the core??
void start_event_dev(void);
void start_event_dev(void)
   {
        int32_t ret;

        CALL_RTE("rte_event_dev_start()");
        ret = rte_event_dev_start( g_glob.event_dev_id );
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














/*****************************************************************************
 *  event dev print functions
 ****************************************************************************/

const char * event_type_string[] = {
       "RTE_EVENT_TYPE_ETHDEV",       //00
       "RTE_EVENT_TYPE_CRYPTODEV",
       "RTE_EVENT_TYPE_TIMER",
       "RTE_EVENT_TYPE_CPU",
       "RTE_EVENT_TYPE_ETH_RX_ADAPTER",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",                     //0x08  
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN",
       "UNKNOWN"
};


const char * sched_type_string[] = {
       "RTE_SCHED_TYPE_ORDERED",       //00
       "RTE_SCHED_TYPE_ATOMIC",
       "RTE_SCHED_TYPE_PARALLEL"
};

const char * event_op_string[] = {
       "RTE_EVENT_OP_NEW",           //00
       "RTE_EVENT_OP_FORWARD",       
       "RTE_EVENT_OP_RELEASE"
};

const char * event_queue_cfg_string[] = {
"-undefined-",           //00
"RTE_EVENT_QUEUE_CFG_ALL_TYPES",       
"RTE_EVENT_QUEUE_CFG_SINGLE_LINK"
};




void print_rte_event(int indent,const char * string,struct rte_event *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();

  printf("%sstruct rte_event: %s  \n",s,string);
  printf("%s    uint32_t    flow_id:20          0x%x \n" ,s,p->flow_id         );
  printf("%s    uint32_t    sub_event_type:8    0x%x \n" ,s,p->sub_event_type  );
  printf("%s    uint32_t    event_type:4        0x%x  -%s \n" ,s,p->event_type,event_type_string[p->event_type]);
  printf("%s    uint8_t     op:2                0x%x  -%s \n" ,s,p->op,event_op_string[p->op]);
  printf("%s    uint8_t     rsvd:4              0x%x \n" ,s,p->rsvd           );
  printf("%s    uint8_t     sched_type:2        0x%x  -%s \n" ,s,p->sched_type,sched_type_string[p->sched_type ]);
  printf("%s    uint8_t     queue_id            0x%x \n" ,s,p->queue_id       );
  printf("%s    uint8_t     priority            0x%x  0 high, 128 normal,255 low \n" ,s,p->priority       );
  printf("%s    uint8_t     impl_opaque         0x%x \n" ,s,p->impl_opaque    );
  printf("%s    union   u64/evt_ptr/mbuf_ptr   0x%lx \n" ,s,p->u64           );
  FONT_NORMAL();
}

#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_eth_rx_adapter_queue_conf(int indent,const char* string,int id,struct rte_event_eth_rx_adapter_queue_conf *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();
  printf("%sstruct rte_event_eth_rx_adapter_queue_conf  %s rx_adapter#=%d\n",s,string,id);
  printf("%s    uint32_t        rx_queue_flags        0x%x\n",s,p->rx_queue_flags              );
//  printf("%s    uint16_t      servicing_weight      %d\n"  ,s,p->service_weight              );
  print_rte_event(indent+1 ," ", &(p->ev));
//  printf("%s  uint8_t nb_single_link_event_port_queues  %d\n",s,p->nb_single_link_event_port_queues);
  FONT_NORMAL();
}
#endif

#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_dev_config(int indent,const char* string,int id,struct rte_event_dev_config *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();
  printf("%sstruct rte_event_dev_config  %s dev#=%d\n",s,string,id);
  printf("%s    uint32_t dequeue_timeout_ns               %d\n",s,p->dequeue_timeout_ns              );
  printf("%s    int32_t  nb_events_limit                  %d\n",s,p->nb_events_limit                 );
  printf("%s    uint8_t  nb_event_queues                  %d\n",s,p->nb_event_queues                 );
  printf("%s    uint8_t  nb_event_ports                   %d\n",s,p->nb_event_ports                  );
  printf("%s    uint32_t nb_event_queue_flows             %d\n",s,p->nb_event_queue_flows            );
  printf("%s    uint32_t nb_event_port_dequeue_depth      %d\n",s,p->nb_event_port_dequeue_depth     );
  printf("%s    uint32_t nb_event_port_enqueue_depth      %d\n",s,p->nb_event_port_enqueue_depth     );
  printf("%s    uint32_t event_dev_cfg                    %d\n",s,p->event_dev_cfg                   );
//  printf("%s  uint8_t nb_single_link_event_port_queues  %d\n",s,p->nb_single_link_event_port_queues);
  FONT_NORMAL();
}
#endif


#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_queue_conf(int indent,const char* string,int id, struct rte_event_queue_conf *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();
  printf("%sstruct rte_event_queue_conf %s queue#=%d\n",s,string,id);
  printf("%s    uint32_t nb_atomic_flows:            %d  (0x%08x)\n",s,p->nb_atomic_flows, p->nb_atomic_flows );
  printf("%s    uint32_t nb_atomic_order_sequences:  %d  (0x%08x)\n",s,p->nb_atomic_order_sequences,p->nb_atomic_order_sequences );
  printf("%s    uint32_t event_queue_cfg:            %d -%s\n",s,p->event_queue_cfg,event_queue_cfg_string[p->event_queue_cfg]);
  printf("%s    uint8_t  schedule_type:              %d  (0x%08x)\n",s,p->schedule_type, p->schedule_type);
  printf("%s    uint8_t  priority:                   %d  (0x%08x)\n",s,p->priority, p->priority);
  FONT_NORMAL();
}
#endif

#ifdef PRINT_DATA_STRUCTURES

void print_rte_event_port_conf(int indent,const char* string,int id, struct rte_event_port_conf *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();
  printf("%sstruct rte_event_port_conf %s port#=%d\n",s,string,id);
  printf("%s    int32_t         new_event_threshold :  %d\n",s,p->new_event_threshold );
  printf("%s    uint16_t        dequeue_depth       :  %d\n",s,p->dequeue_depth       );
  printf("%s    uint16_t        enqueue_depth       :  %d\n",s,p->enqueue_depth       );
  //printf("%s    uint32_t      event_port_cfg      :  %d\n",s,p->event_port_cfg      );
  FONT_NORMAL();
}
#endif



void print_rte_event_dev_info(int indent,const char* string,struct rte_event_dev_info * p)
{
   INDENT(indent);
   FONT_DATA_STRUCTURES_COLOR();

    printf("%sstruct rte_event_dev_info: %s \n",s,string);
    printf("%s    char *        driver_name  %s \n",s,p->driver_name);
    printf("%s    rte_device * dev          %p \n",s,p->dev);
    printf("%s    uint32_t     min_dequeue_timeout_ns           0x%08x \n",s,p->min_dequeue_timeout_ns         );
    printf("%s    uint32_t     max_dequeue_timeout_ns           0x%08x \n",s,p->max_dequeue_timeout_ns         );
    printf("%s    uint32_t     dequeue_timeout_ns               0x%08x \n",s,p->dequeue_timeout_ns             );
    printf("%s    uint8_t      max_event_queues                 0x%02x \n",s,p->max_event_queues               );
    printf("%s    uint32_t     max_event_queue_flows            0x%08x \n",s,p->max_event_queue_flows          );
    printf("%s    uint8_t      max_event_queue_priority_levels  0x%02x \n",s,p->max_event_queue_priority_levels);
    printf("%s    uint8_t      max_event_priority_levels        0x%02x \n",s,p->max_event_priority_levels      );
    printf("%s    uint8_t      max_event_ports                  0x%02x \n",s,p->max_event_ports                );
    printf("%s    uint8_t      max_event_port_dequeue_depth     0x%02x \n",s,p->max_event_port_dequeue_depth   );
    printf("%s    uint32_t     max_event_port_enqueue_depth     0x%08x \n",s,p->max_event_port_enqueue_depth   );
    printf("%s    int32_t      max_num_events                   0x%08x \n",s,p->max_num_events                 );
  FONT_NORMAL();
}





