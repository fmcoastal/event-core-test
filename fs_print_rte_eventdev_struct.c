#include <stdint.h>
#include <stdint.h>

#include <rte_eventdev.h>
#include <rte_event_eth_rx_adapter.h>

#include "fs_extras.h"
#include "fs_print_rte_eventdev_struct.h"

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

const char * event_queue_cfg_string[] = {
"-undefined-",           //00
"RTE_EVENT_QUEUE_CFG_ALL_TYPES",       
"RTE_EVENT_QUEUE_CFG_SINGLE_LINK"
};


#ifdef PRINT_DATA_STRUCTURES
void print_rte_event_queue_conf(int indent,const char* string,int id, struct rte_event_queue_conf *p)
{
  INDENT(indent);
  FONT_DATA_STRUCTURES_COLOR();
  printf("%sstruct rte_event_queue_conf %s queue#=%d\n",s,string,id);
  printf("%s    uint32_t nb_atomic_flows:  %d\n",s,p->nb_atomic_flows);
  printf("%s    uint32_t nb_atomic_order_sequences:  %d\n",s,p->nb_atomic_order_sequences);
  printf("%s    uint32_t event_queue_cfg:  %d -%s\n",s,p->event_queue_cfg,event_queue_cfg_string[p->event_queue_cfg]);
  printf("%s    uint8_t  schedule_type:    %d\n",s,p->schedule_type);
  printf("%s    uint8_t  priority:         %d\n",s,p->priority);
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





