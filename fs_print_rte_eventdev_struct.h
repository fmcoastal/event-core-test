#ifndef _fs_print_rte_eventdev_struct_h
#define _fs_print_rte_eventdev_struct_h


/*****************************************************************************
 *  event dev print functions
 ****************************************************************************/

#ifdef PRINT_DATA_STRUCTURES

void  print_rte_event(int indent,struct rte_event *p);
void  print_rte_event_dev_config(int,const char*,int,struct rte_event_dev_config*);
void  print_rte_event_queue_conf(int,const char*,int,struct rte_event_queue_conf*);
void  print_rte_event_port_conf(int,const char*,int,struct rte_event_port_conf*);
void  print_rte_event_eth_rx_adapter_queue_conf(int,const char*,int,struct rte_event_eth_rx_adapter_queue_conf*);
void  print_rte_event_dev_info(struct rte_event_dev_info * p);

#else

#define  print_rte_event(x,y)
#define  print_rte_event_dev_config(w,x,y,z );
#define  print_rte_event_queue_conf(w,x,y,z );
#define  print_rte_event_port_conf(w,x,y,z );
#define  print_rte_event_eth_rx_adapter_queue_conf(w,x,y,z );

#endif


#endif



