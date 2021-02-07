#include <stdint.h>
#include <stdint.h>



#include <rte_spinlock.h>
#include <rte_eventdev.h>
#include <rte_event_eth_rx_adapter.h>

#include "fs_extras.h"
#include "fs_eventdev.h"


#if 0
 568 //void print_event_queue_cfg
 569
 570 /**********************************
 571 A high level overview of the setup steps are:
 572 rte_event_dev_configure()
 573 rte_event_queue_setup()
 574 rte_event_port_setup()
 575 rte_event_port_link()
 576
 577 rte_event_dev_start()    // start the device in a separate function
 578
 579 #if 0
 580     event_dev_id = 0;                 // event dev_id index/handle => SSO  0
 581     nb_event_ports  = 0;         // event dev ports??
 582     memset(&(def_p_conf), 0, sizeof(struct rte_event_port_conf));
 583          def_p_conf.dequeue_depth =1;
 584          def_p_conf.enqueue_depth =1;
 585          def_p_conf.new_event_threshold = -1;
 586     sched_type = RTE_SCHED_TYPE_ATOMIC ;   //RTE_SCHED_TYPE_ORDERED  RTE_SCHED_TYPE_PARALLEL
 587 #endif
 588
 589 ************************************/
 590 void Initialize_EventDev(void);
 591 void Initialize_EventDev(void)
 592 {
 593 int     result;
 594 uint8_t                       i;
 595 uint8_t                       nb_event_dev_devices = 0;
 596 struct  rte_event_dev_info    event_dev_capabilities;
 597 struct  rte_event_dev_config  event_dev_config = {0};
 598
 599 uint8_t                       event_q_id = 0;
 600 struct  rte_event_queue_conf  def_q_conf = {0};      // queue
 601 struct  rte_event_queue_conf  event_q_conf = {0};
 602 uint32_t event_queue_cfg = 0;
 603
 604     WAI();
 605    {
 606 //////////
 607 ////////// check there is an event dev device available
 608 //////////
 609 //////////
 610         /*uint8_t       rte_event_dev_count (void) */
 611         CALL_RTE("rte_event_dev_count()");
 612         nb_event_dev_devices = rte_event_dev_count();
 613         if ( nb_event_dev_devices  == 0)
 614         {
 615             printf(" no event dev devices found\n");
 616             printf(" did you forget to bind at least 1 event-dev device to vfio-pci\n");
 617             rte_exit(EXIT_FAILURE, "No Event Devices available");
 618         }
 619         WAI();
 620         printf(" Found %d event devices\n", nb_event_dev_devices);
 621
 622 //  debug -print out all event dev devices - (my program only uses 1)
 623 //        int rte_event_dev_info_get (uint8_t dev_id, struct rte_event_dev_info *dev_info)
 624         for (i = 0 ; i < nb_event_dev_devices ; i++)
 625         {
 626            CALL_RTE("rte_event_dev_info_get()");
 627            result = rte_event_dev_info_get(i,&event_dev_capabilities);
 628            printf("  %d)  %s\n",i, event_dev_capabilities.driver_name);
 629         }
 630
 631 //  My test- I expect just 1
 632         if( nb_event_dev_devices != 1)
 633         {
 634             printf(" ** WARNING:: Number of  event dev devices found is not what is expected \n");
 635         }
 636         // WHY at least there is at least 1  device,  Index=0;
 637         //  g_glob.event_dev_id = 0;     // assign dev_id  to 0 --> set at the top of the function
 638
 639         printf(" g_glob.event_dev_id:   %d \n", g_glob.event_dev_id);
 640    }
 641
 642
 643 //////////
 644 //////////  rte_event_dev_configure()
 645 //////////
 646 //////////   Get default event_dev Setting and  Capabilities
 647    {
 648         CALL_RTE("rte_event_dev_info_get()");
 649         result = rte_event_dev_info_get( g_glob.event_dev_id , &event_dev_capabilities );
 650         printf(" result    %d \n",result);
 651         printf(" default: event_dev_capabilities \n");
 652         print_rte_event_dev_info(0,"event_dev_capabilities", &event_dev_capabilities);
 653
 654         // check compatability - make sure queue will accept any type of RTE_SCHED_TYPE_* values:
 655         //                                 ordered,parallel,atomic,ethdev,ccryptodev,timer,...
 656         if ( event_dev_capabilities.event_dev_cap & RTE_EVENT_DEV_CAP_QUEUE_ALL_TYPES)
 657              event_queue_cfg |= RTE_EVENT_QUEUE_CFG_ALL_TYPES;
 658
 659 //////  setup parameters for rte_event_dev_configure()
 660
 661         event_dev_config.dequeue_timeout_ns          = 0;                      // uint32_t     0 use Default values
 662         event_dev_config.nb_events_limit             = -1;                     // int32_t ??? I started with 100 and got errors, but l3fwd has -1, so copy for notw;
 663         event_dev_config.nb_event_queues             = g_glob.evq.nb_queues ;  // uint8_t
 664         event_dev_config.nb_event_ports              = g_glob.evp.nb_ports  ;  // uint8_t
 665         event_dev_config.nb_event_queue_flows        = 1024;                  // uint32_t
 666         event_dev_config.nb_event_port_dequeue_depth = 1;                     // uint32_t   cpy l3fwd->needs to be 1 because we do in hw?
 667         event_dev_config.nb_event_port_enqueue_depth = 1;                     // uint32_t cpy l3fwd->needs to be 1 because we do in hw?
 668 //////    event_dev_config.event_dev_cfg           = dev_info.event_dev_cap;  // uint32_t
 669         event_dev_config.event_dev_cfg               = 0;                     // uint32_t   again copy what l3fwd has to see if we get the same respons
 670
 671
 672 #ifdef PRINT_CALL_ARGUMENTS
 673         FONT_CALL_ARGUMENTS_COLOR();
 674         printf(" int rte_event_dev_configure( uint8_t dev_id,\n"
 675                "                              const struct rte_event_dev_config * dev_conf \n"
 676                "                             )\n");
 677         printf(  " Call Args: g_glob.event_dev_id  %d  event_dev_config: \n",g_glob.event_dev_id );
 678         FONT_NORMAL();
 679 #endif
 680         print_rte_event_dev_config( 0, "event_dev_config",0,&event_dev_config);
 681
 682         CALL_RTE("rte_event_dev_configure()");
 683         result = rte_event_dev_configure( g_glob.event_dev_id, &event_dev_config);
 684         if(result < 0)
 685         {
 686              printf(" rte_event_dev_configure() returned %d\n",result);
 687              rte_panic("Error in configuring event device\n");
 688         }
 689     }
 690
 691 //////
 692 //////  rte_event_queue_setup()
 693 //////
 694 //////   Get default event_queue Setting and  Capabilities
 695     {
 696         int ret;
 697         event_queue_cfg_t *ptr;
 698
 699 #ifdef PRINT_CALL_ARGUMENTS
 700         FONT_CALL_ARGUMENTS_COLOR();
 701         printf(" int rte_event_queue_default_conf_get( uint8_t dev_id,\n"
 702                "                                       uint8_t queue_id,\n"
 703                "                                       struct rte_event_queue_conf *    queue_conf \n"
 704                "                                       )\n");
 705         printf(  " Call Args: g_glob.event_dev_id:%d  event_q_id:%d  def_q_conf: \n",g_glob.event_dev_id,event_q_id );
 706         FONT_NORMAL();
 707 #endif
 708         CALL_RTE("rte_event_queue_default_conf_get()");
 709         rte_event_queue_default_conf_get( g_glob.event_dev_id , event_q_id, &def_q_conf);
 710
 711         printf(" default: event dev queue info: event_q_id=%d \n",event_q_id);
 712         printf("      struct rte_event_queue_conf *\n") ;
 713         printf("          uint32_t nb_atomic_flows              %d (0x%08x) \n", def_q_conf.nb_atomic_flows, def_q_conf.nb_atomic_flows  );  //
 714         printf("          uint32_t nb_atomic_order_sequences    %d (0x%08x) \n", def_q_conf.nb_atomic_order_sequences, def_q_conf.nb_atomic_order_sequences );  //
 715         printf("          uint32_t event_queue_cfg              %d (0x%08x) \n", def_q_conf.event_queue_cfg , def_q_conf.event_queue_cfg );  //
 716         printf("                    values: #define RTE_EVENT_QUEUE_CFG_ALL_TYPES       (1ULL << 0)    \n");  //
 717         printf("                            #define RTE_EVENT_QUEUE_CFG_SINGLE_LINK     (1ULL << 1)    \n");  //
 718         printf("          uint8_t  schedule_type                %3d (0x%02x) (%s)\n", def_q_conf.schedule_type , def_q_conf.schedule_type
 719                                                              ,StringSched[def_q_conf.schedule_type] );  //
 720         printf("          uint8_t  priority                     %3d (0x%02x) \n", def_q_conf.priority , def_q_conf.priority );  //
 721
 722         event_q_conf.nb_atomic_flows           = 1024;
 723         event_q_conf.nb_atomic_order_sequences = 1024;
 724         event_q_conf.event_queue_cfg           = event_queue_cfg;
 725         //     event_q_conf.schedule_type             = RTE_EVENT_QUEUE_CFG_ALL_TYPES ;
 726         event_q_conf.priority                  = RTE_EVENT_DEV_PRIORITY_NORMAL ;
 727
 728         // bounds check
 729         if (def_q_conf.nb_atomic_flows < event_q_conf.nb_atomic_flows)
 730             event_q_conf.nb_atomic_flows = def_q_conf.nb_atomic_flows;
 731
 732         // bounds check
 733         if (def_q_conf.nb_atomic_order_sequences <
 734                         event_q_conf.nb_atomic_order_sequences)
 735             event_q_conf.nb_atomic_order_sequences =
 736                         def_q_conf.nb_atomic_order_sequences;
 737
 738
 739        printf("***  Config: event queue config   *** \n");
 740
 741        ptr = g_glob.evq.event_q_cfg;
 742
 743        // walk through the number event queues created as part of the event structure
 744        for ( event_q_id= 0 ; event_q_id < event_dev_config.nb_event_queues ; event_q_id++)
 745        {
 746            // above I set teh number of queues and the number of ports = number of cores:
 747            //    So I should loop through each evnet_q_ID.
 748            // update per config changes in "Initialize "
 749
 750            // event_q_conf.nb_atomic_flows
 751            // event_q_conf.nb_atomic_order_sequences
 752            event_q_conf.event_queue_cfg = ( ptr + event_q_id )->ev_q_conf.event_queue_cfg;
 753            event_q_conf.schedule_type   = ( ptr + event_q_id )->ev_q_conf.schedule_type ;
 754            event_q_conf.priority        = ( ptr + event_q_id )->ev_q_conf.priority;
 755
 756 #ifdef PRINT_CALL_ARGUMENTS
 757            FONT_CALL_ARGUMENTS_COLOR();
 758            printf(  " Call Args: g_glob.event_dev_id:%d  event_q_id:%d  event_q_conf: \n",g_glob.event_dev_id,event_q_id );
 759            FONT_NORMAL();
 760 #endif
 761           // this is how I will configure each queue for this event dev:
 762            print_rte_event_queue_conf( 1 ,"event_q_id",event_q_id, &event_q_conf);
 763            CALL_RTE("call rte_event_queue_setup()");
 764            ret = rte_event_queue_setup(g_glob.event_dev_id , event_q_id,
 765                                 &event_q_conf);
 766            if (ret < 0)
 767                 rte_panic("Error in configuring event queue\n");
 768 //                evt_rsrc->evq.event_q_id[event_q_id] = event_q_id;
 769        }  //  loop throug the queue Info. */
 770    }
 771
 771
 772
 773
 774 //////
 775 //////  rte_event_port_setup()
 776 //////
 777 //////   Get default event_port Setting and  Capabilities
 778    {
 779        struct rte_event_port_conf event_p_conf;
 780        struct rte_event_port_conf def_p_conf;
 781        uint8_t event_p_id;
 782        int32_t ret;
 783
 784
 785        CALL_RTE("rte_event_port_default_conf_get()");
 786        rte_event_port_default_conf_get(g_glob.event_dev_id, 0, &def_p_conf);
 787
 788        printf(" default: event port config  -> def_p_conf \n");
 789        printf("      struct rte_event_port_conf *\n") ;
 790        printf("          int32_t  new_event_threshold        %4d (0x%08x)\n", def_p_conf.new_event_threshold      , def_p_conf.new_event_threshold      );
 791        printf("          uint16_t dequeue_depth              %4d (0x%04x)\n", def_p_conf.dequeue_depth            , def_p_conf.dequeue_depth            );
 792        printf("          uint16_t enqueue_depth              %4d (0x%04x)\n", def_p_conf.enqueue_depth            , def_p_conf.enqueue_depth            );
 793        printf("          uint8_t  disable_implicit_release   %4d (0x%02x)\n", def_p_conf.disable_implicit_release , def_p_conf.disable_implicit_release );
 794
 795        // set up defaults for config
 796        event_p_conf.dequeue_depth = 32,
 797        event_p_conf.enqueue_depth = 32,
 798        event_p_conf.new_event_threshold = 4096;
 799
 800        if (def_p_conf.new_event_threshold < event_p_conf.new_event_threshold)
 801             event_p_conf.new_event_threshold =
 802                          def_p_conf.new_event_threshold;
 803
 804        if (def_p_conf.dequeue_depth < event_p_conf.dequeue_depth)
 805             event_p_conf.dequeue_depth = def_p_conf.dequeue_depth;
 806
 807        if (def_p_conf.enqueue_depth < event_p_conf.enqueue_depth)
 808             event_p_conf.enqueue_depth = def_p_conf.enqueue_depth;
 809
 810 /*
 811 113     event_p_conf.disable_implicit_release =
 812 114         evt_rsrc->disable_implicit_release;
 813 115
 814 */
 815        event_p_conf.disable_implicit_release = 0 ; // marvell extension  see:
 816                                                    //  "<dpdk>/lib/librte_eventdev/rte_eventdev.h
 817
 818        printf(" Config: event port config  -> event_p_conf \n");
 819        printf("      struct rte_event_port_conf *\n") ;
 820        printf("          int32_t  new_event_threshold        %4d (0x%08x)\n", event_p_conf.new_event_threshold      , event_p_conf.new_event_threshold     );
 821        printf("          uint16_t dequeue_depth              %4d (0x%04x)\n", event_p_conf.dequeue_depth            , event_p_conf.dequeue_depth           );
 822        printf("          uint16_t enqueue_depth              %4d (0x%04x)\n", event_p_conf.enqueue_depth            , event_p_conf.enqueue_depth           );
 823        printf("          uint8_t  disable_implicit_release   %4d (0x%02x)\n", event_p_conf.disable_implicit_release , event_p_conf.disable_implicit_release);
 824
 825        for (event_p_id = 0; event_p_id < g_glob.evp.nb_ports ; event_p_id++) {
 826
 827 #ifdef PRINT_CALL_ARGUMENTS
 828            FONT_CALL_ARGUMENTS_COLOR();
 829            printf(  " Call Args: g_glob.event_dev_id:%d  event_p_id:%d  event_p_conf: \n",g_glob.event_dev_id,event_p_id );
 830            FONT_NORMAL();
 831 #endif
 832            print_rte_event_port_conf(1 , "event_p_conf", event_p_id, &event_p_conf);
 833            CALL_RTE("rte_event_port_setup()");
 834            ret = rte_event_port_setup( g_glob.event_dev_id , event_p_id,  &event_p_conf);
 835            if (ret < 0)
 836                rte_panic("Error in configuring event port %d\n", event_p_id);
 837        }
 838      }
 839
 840 //////
 841 //////  rte_event_port_link()
 842 //////
 843 //////  connect an event dev port to array of queue_#, queue_priority
 844 //////
 845 //////  So we have n cores.   For this app, I want 1 core to have 1 port and 2 queues.
 846 /////                         we will use the Index to be the Index of the core??
 847     {
 848         int32_t ret;
 849         uint8_t port;
 850         uint8_t array_sz;
 851         uint8_t *queues_array;
 852         uint8_t *priorities_array;
 853         int      index;
 854
 855
 856
 857         printf(" Linking event_ports to event_queues on event_dev_id:%d \n ",g_glob.event_dev_id);
 858         for ( port  = 0; port  < (g_glob.evp.nb_ports) ; port ++)
 859         {
 860              array_sz         =    (g_glob.evp.event_p_id + port)->nb_links ;
 861              queues_array     = &( (g_glob.evp.event_p_id + port)->q_id[0]);
 862              priorities_array = &( (g_glob.evp.event_p_id + port)->pri[0]);
 863
 864 #ifdef PRINT_CALL_ARGUMENTS
 865         FONT_CALL_ARGUMENTS_COLOR();
 866
 867              printf("       port# %d \n",port);
 868              printf("           number of queues  %d \n",array_sz);
 869              for (index = 0 ; index < array_sz  ; index++)
 870              {
 871                    printf("            %d) queue_id %d  queue_prioity %d\n",index
 872                                          , *( queues_array+index  )
 873                                          , *( priorities_array +index) );
 874              }
 875
 876             printf("           int rte_event_port_link( uint8_t dev_id,    = %d  (g_glob.event_dev_id)\n", g_glob.event_dev_id);
 877             printf("                                    uint8_t port_id,   = %d  (port) \n",port);
 878             printf("                                    const uint8_t   queues[],   %p     \n",queues_array);
 879             printf("                                    const uint8_t   priorities[] %p \n", priorities_array);
 880             printf("                                    uint16_t nb_links  = %d  (array_sz) \n",array_sz);
 881             printf("                                    ) \n");
 882
 883
 884             FONT_NORMAL();
 885  #endif
 886             CALL_RTE("rte_event_port_link()");
 887             ret = rte_event_port_link ( g_glob.event_dev_id , port , queues_array, priorities_array, array_sz);
 888             if (ret != array_sz )
 889             {
 890                rte_panic("Error in rte_event_port_link() requested %d successfull %d\n",array_sz,ret);
 891             }
 892         }
 893     }
 894
 895
 896 {
 897         uint32_t  queue_count = 0xffffffff;
 898         uint32_t  port_count = 0xffffffff;
 899
 900         rte_event_dev_attr_get(g_glob.event_dev_id, RTE_EVENT_DEV_ATTR_PORT_COUNT,&port_count);
 901         rte_event_dev_attr_get(g_glob.event_dev_id, RTE_EVENT_DEV_ATTR_QUEUE_COUNT,&queue_count);
 902
 903         printf(" eventdev  Sanity Check for eventdev id: %d\n", g_glob.event_dev_id);
 904         printf("                  number of event ports: %u\n", port_count);
 905         printf("                 number of event queues: %u\n", queue_count);
 906
 907 }
 908
 909
 910
 911
 912 //////
 913 //////
 914 //////   // not sure about this...
 915 //////
 916     {
 917         uint32_t  evdev_service_id = 0;
 918         int32_t ret;
 919         //  THis might be for SW version of event dev MGR???  But sure what this is and how it is used.
 920         CALL_RTE("rte_event_dev_service_id_get()");
 921         ret = rte_event_dev_service_id_get( g_glob.event_dev_id,
 922                                 &evdev_service_id);
 923         if (ret != -ESRCH && ret != 0) {
 924                 rte_panic("Error getting the service ID for sw eventdev\n");
 925         }
 926         printf(" service ID for event_dev (Device?) %d \n",evdev_service_id);
 927     }
 928    return ;
 929
 930 }   // end  void Initialize_EventDev(void);
 931
#endif
















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





