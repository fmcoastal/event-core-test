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
#include <rte_event_timer_adapter.h>   // for event timer
#include <rte_event_eth_rx_adapter.h>
#include <rte_event_eth_tx_adapter.h>


#include <rte_spinlock.h>
#include "fs_extras.h"
#include "fs_net_common.h"

#include "fs_ethdev.h"           
#include "fs_eventdev.h"           // structures used in configuring the 
                                   // event dev components
#include "fs_print_rte_mbuff.h"    // for printing ethaddrs (mac addresses)

#include "fs_global_vars.h"

global_data_t g_glob={0};



void print_global_data (global_data_t *p)
{
char string[32];
int i,j;

   printf("%s",C_RED);
   printf(" uint16_t  enabled_eth_port_mask            = %d \n",p->enabled_eth_port_mask);
   printf(" uint16_t  nb_eth_ports_available           = %d \n",p->nb_eth_ports_available);
   printf(" uint16_t  event_dev_id                   = %d \n",p->event_dev_id);
   printf(" struct    rte_mempool* p_pktmbuf_pool  = %p \n",p->p_pktmbuf_pool);

   printf("\n");
   printf("------------ eth_dev config data  -----------\n");
   {
      printf(" struct rte_eth_conf eth_port_cfg_data[RTE_MAX_ETHPORTS];");
      for ( i = 0 ; i < p->nb_eth_ports_available ; i++)
      {
         rte_eth_dev_get_name_by_port( i , string);
         printf("    uint16_t eth port_id                 %d   (%s)\n",i,string);
         printf("    struct   rte_ether_addr  eth_addr:   %s  \n",format_rte_mac_addr(string,&(p->eth_port_cfg_data[i].eth_addr)));
         printf("    uint16_t nb_rx_queues:               %d  \n", p->eth_port_cfg_data[i].nb_rx_queues);
         printf("    uint16_t nb_tx_queues:               %d  \n", p->eth_port_cfg_data[i].nb_tx_queues);
      }
      printf("\n");
   }

   printf("\n");
   printf("------------ eventdev queues -----------\n");
   printf(" struct event_queues  evq : {\n");
   printf("         uint8_t nb_queues; %d \n",p->evq.nb_queues);
   if (p->evq.nb_queues == 0 )
   {
         printf("           -- no evq array malloc-ed yet\n");
   }
   else
   {
       printf("       event_queue_cfg_t  array\n");
       printf("    queue    to_port  event_queue_cfg   schedule_type   priority\n");
       for ( i = 0 ; i < p->evq.nb_queues ; i++)
       {
          printf("     %2d)      %2d         0x%08x           0x%02x         0x%02x \n",
                     ((p->evq.event_q_cfg) + i)->event_q_id       ,
                     ((p->evq.event_q_cfg) + i)->to_event_port    ,
                     ((p->evq.event_q_cfg) + i)->ev_q_conf.event_queue_cfg ,
                     ((p->evq.event_q_cfg) + i)->ev_q_conf.schedule_type ,
                     ((p->evq.event_q_cfg) + i)->ev_q_conf.priority );
       }
       printf("\n");
   }
   printf("        }\n");

   printf("\n");
   printf("------------ eventdev ports -----------\n");
   printf(" struct event_ports  evp : {\n");
   printf("         uint8_t nb_ports; %d \n",p->evp.nb_ports);
   if (p->evp.nb_ports == 0 )
   {
         printf("           -- no evp array malloc-ed yet\n");
   }
   else
   {
       for ( i = 0 ; i < p->evp.nb_ports ; i++)
       {
          printf("           port# %d \n",i);
          printf("              number of queues  %d \n", (g_glob.evp.event_p_id + i)->nb_links);
          for ( j = 0 ; j < (g_glob.evp.event_p_id + i)->nb_links ; j++)
          {
               printf("                %d) queue_id %d  queue_prioity %d\n",j
                                     ,(g_glob.evp.event_p_id + i)->q_id[j]
                                     , (g_glob.evp.event_p_id + i)->pri[j]  );
          }
       }
       printf("\n");
   }
   printf("        }\n");

   printf("             struct rte_spinlock_t lock : {\n");
   printf("               -tbd- \n");
   printf("              }\n");

    printf("\n");
    printf("---------- event_dev  adapter default config  --------\n");
    print_rte_event_port_conf(1,"def_p_conf",0,&(p->def_p_conf));
    printf("%s",C_RED);


    printf("\n");
    printf("------------ eventdev rx_adapters -----------\n");
    printf(" struct event_rx_adapter rx_adptr : {\n");
    printf("         uint32_t service_id; %d \n",p->rx_adptr.service_id);
    printf("         uint8_t nb_rx_adptr; %d \n",p->rx_adptr.nb_rx_adptr);
    if (p->rx_adptr.nb_rx_adptr == 0 )
    {
         printf("           -- no rx_adptr array malloc-ed yet\n");
    }
    else
    {
       printf("         uint8_t * rx_adptr array");
       for ( i = 0 ; i < p->rx_adptr.nb_rx_adptr ; i++)
       {
          if ( i%4 == 0) printf("\n           ");
          printf("   %d) %2d",i,*(p->rx_adptr.rx_adptr+i));
       }
       printf("\n");
    }
    printf("\n");
    printf("         uint8_t nb_rx_adptr_add; %d \n",p->rx_adptr.nb_rx_adptr_add);
    if (p->rx_adptr.nb_rx_adptr_add == 0 )
    {
         printf("           -- no rx_adptr_add array malloc-ed yet\n");
    }
    else
    {
       printf("         uint8_t * rx_adptr_add array\n");
       printf("             uint32_t  uint8_t  uint8_t  uint8_t    uint8_t  uint8_t \n");
       printf("             adapter   eth_dev  eth_dev  event_dev  sched    Priority\n");
       printf("             id        port     queue    queue      type     \n");
       for ( i = 0 ; i < p->rx_adptr.nb_rx_adptr_add ; i++)
       {
       printf("            %2d        %2d       %2d       %2d         %2d        %3d \n",
                                       p->rx_adptr.rx_adptr_add[i].adapter_id,   
                                       p->rx_adptr.rx_adptr_add[i].eth_dev_port,   
                                       p->rx_adptr.rx_adptr_add[i].eth_dev_queue,   
                                       p->rx_adptr.rx_adptr_add[i].event_dev_queue,   
                                       p->rx_adptr.rx_adptr_add[i].sched_type,   
                                       p->rx_adptr.rx_adptr_add[i].priority   
                                        );
       }
       printf("\n");
    }
    printf("        }\n");


    printf("\n");
    printf("------------ eventdev tx_adapters -----------\n");
    printf(" struct event_tx_adapter tx_adptr : {\n");
    printf("         uint32_t service_id; %d \n",p->tx_adptr.service_id);
    printf("         uint8_t nb_tx_adptr; %d \n",p->tx_adptr.nb_tx_adptr);
    if (p->tx_adptr.nb_tx_adptr == 0 )
    {
         printf("           -- no tx_adptr array malloc-ed yet\n");
    }
    else
    {
       printf("         uint8_t * tx_adptr array");
       for ( i = 0 ; i < p->tx_adptr.nb_tx_adptr ; i++)
       {
          if ( i%4 == 0) printf("\n         ");
          printf("   %d) %2d",i,*(p->tx_adptr.tx_adptr+i));
       }
       printf("\n");
    }
    printf("         uint8_t nb_tx_adptr_add; %d \n",p->tx_adptr.nb_tx_adptr_add);
    if (p->tx_adptr.nb_tx_adptr_add == 0 )
    {
         printf("           -- no tx_adptr_add array malloc-ed yet\n");
    }
    else
    {
       printf("         uint8_t * tx_adptr_add array\n");
       printf("            uint32_t    uint8_t       uint8_t      \n");
       printf("            adapter_id  eth_dev_port  eth_dev_queue\n");
       for ( i = 0 ; i < p->rx_adptr.nb_rx_adptr_add ; i++)
       {
       printf("               %2d         %2d          %2d            \n",
                                       p->rx_adptr.rx_adptr_add[i].adapter_id,   
                                       p->rx_adptr.rx_adptr_add[i].eth_dev_port,   
                                       p->rx_adptr.rx_adptr_add[i].eth_dev_queue   
                                        );
       }
       printf("\n");
    }
    printf("        }\n");

   printf("%s",C_WHITE);

}









