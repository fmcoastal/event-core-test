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

#include "fs_eventdev.h"               // structures used in configuring the 
                                       // event dev components
#include "fs_eventdev.h"  // for printing ethaddrs (mac addresses)
#include "fs_print_rte_mbuff.h"  // for printing ethaddrs (mac addresses)

#include "fs_global_vars.h"

global_data_t g_glob={0};



void print_global_data (global_data_t *p)
{
char eth_addr[30];
int i,j;
   printf("%s",C_RED);
   printf(" uint16_t  enabled_port_mask = %d \n",p->enabled_port_mask);
   printf(" uint16_t  nb_ports_available = %d \n",p->nb_ports_available);
   printf(" uint16_t  event_d_id = %d \n",p->event_d_id);
   printf(" struct    rte_mempool* p_pktmbuf_pool  = %p \n",p->p_pktmbuf_pool);
   {
       printf(" struct rte_ether_addr      eth_addr[RTE_MAX_ETHPORTS];");
       for ( i = 0 ; i < p->nb_ports_available ; i++)
       {
          if ( i%4 == 0) printf("\n");
          printf("  %d) %s",i,format_mac_addr(eth_addr,&( p->eth_addr[i])));
       }
       printf("\n        }\n");
   }


    print_rte_event_port_conf(1,"def_p_conf",0,&(p->def_p_conf));
    printf("%s",C_RED);

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
          if ( i%4 == 0) printf("\n       ");
          printf("   %d) %2d",i,*(p->rx_adptr.rx_adptr+i));
       }
       printf("\n");
    }
    printf("        }\n");



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
          if ( i%4 == 0) printf("\n     ");
          printf("   %d) %2d",i,*(p->tx_adptr.tx_adptr+i));
       }
       printf("\n");
    }
    printf("        }\n");


    printf(" struct event_queues  evq : {\n");
    printf("         uint8_t nb_queues; %d \n",p->evq.nb_queues);
    if (p->evq.nb_queues == 0 )
    {
         printf("           -- no evq array malloc-ed yet\n");
    }
    else
    {
       printf("       event_queue_cfg_t  arrayi\n");
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
    printf("`}\n");

   printf("%s",C_WHITE);

}









