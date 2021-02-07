#ifndef  _fs_eventdev_h
#define  _fs_eventdev_h



// structures to configure varaious 



/* I am going to cheese out here.
   I am going initialze by

   f0r ( i = 0 ; i < nb_xx_adptr ;i++)
         create the adapte()
         for( j = 0 ; j < nb_rx_adapter_add; j++)
         {
            if(rx_adapter[i] == rx_adapter_add[j].adapter_id
            {
                setup and call
                event_rx_adapter_add()
            }
         }
     }


*/

////////////////////////////////////
//   parameters to configue event rx_adapters 
//      ties eth queues to event queues
//        also sets atomic/ordered/parallel
typedef struct event_rx_adapter_add_struct
{
    uint32_t adapter_id;        // index of rx_adapter to tye "add" function to
    uint8_t eth_dev_port;       // eth_dev_port
    uint8_t eth_dev_queue;      // eth_dev_queue
    uint8_t event_dev_queue;    // event_dev_queue for this eth queue /rte_flow queue 
    uint8_t sched_type;           // atomic/ordered/paralle;
    uint8_t priority;           // Per Eng, priority will actuall be set by priority
                                //    of event dev queue, not this.  I added here
                                //    to help me keep things straight when I
                                //    manuall fill the structures. 
} event_rx_adapter_add_t;


struct event_rx_adptr {
        uint32_t service_id;

        uint8_t nb_rx_adptr;
        uint8_t *rx_adptr;

        uint8_t nb_rx_adptr_add;
        event_rx_adapter_add_t *rx_adptr_add;
};




////////////////////////////////////
//   parameters to configue event rx_adapters 
//      ties eth queues to event queues
//        also sets atomic/ordered/parallel
typedef struct event_tx_adapter_add_struct
{
    uint32_t adapter_id;        // index of rx_adapter to tye "add" function to
    uint8_t eth_dev_port;       // eth_dev_port
    uint8_t eth_dev_queue;      // eth_dev_queue

} event_tx_adapter_add_t;


struct event_tx_adptr {
        uint32_t service_id;

        uint8_t nb_tx_adptr;
        uint8_t *tx_adptr;

        uint8_t nb_tx_adptr_add;
        event_tx_adapter_add_t *tx_adptr_add;
};


////////////////////////////////////
//   parameters to configue event queues and
//    event queue to port mapping. 
typedef struct  event_queue_cfg_struct {
     int                           event_q_id;
     int                           to_event_port;
     struct rte_event_queue_conf   ev_q_conf ;
   } event_queue_cfg_t;



struct event_queues {
        event_queue_cfg_t *event_q_cfg;
        uint8_t            nb_queues;
};

////////////////////////////////////
//   parameters to configue event ports and
//      q_id to port mapping. 
// event_q_id = q_id   - I added event later 
//               because of confusion between
//               eth->queue  and event->queue
typedef struct event_port_link_struct
       {
          uint8_t nb_links;
          uint8_t q_id[32];
          uint8_t pri[32];
       } q_id_and_priority_t;

struct event_ports {
        q_id_and_priority_t  *event_p_id;
        uint8_t            nb_ports;
        rte_spinlock_t    lock;
};



 /*****************************************************************************
 *  event dev print functions
 ****************************************************************************/

#ifdef PRINT_DATA_STRUCTURES

void  print_rte_event(int indent,const char * string,struct rte_event *p);
void  print_rte_event_dev_config(int,const char*,int,struct rte_event_dev_config*);
void  print_rte_event_queue_conf(int,const char*,int,struct rte_event_queue_conf*);
void  print_rte_event_port_conf(int,const char*,int,struct rte_event_port_conf*);
void  print_rte_event_eth_rx_adapter_queue_conf(int,const char*,int,struct rte_event_eth_rx_adapter_queue_conf*);
void  print_rte_event_dev_info(int indent,const char* string, struct rte_event_dev_info * p);

#else

#define  print_rte_event(x,y)
#define  print_rte_event_dev_config(w,x,y,z );
#define  print_rte_event_queue_conf(w,x,y,z );
#define  print_rte_event_port_conf(w,x,y,z );
#define  print_rte_event_eth_rx_adapter_queue_conf(w,x,y,z );

#endif




#endif
