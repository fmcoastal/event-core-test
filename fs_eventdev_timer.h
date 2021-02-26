#ifndef _fs_eventdev_timer_h
#define _fs_eventdev_timer_h



#include "main.h"

#define NSECPERSEC 1E9 // No of ns in 1 sec
 

extern struct test_mode_struct  tm_timer;
extern char * timer_message[];
extern struct rte_event_timer   g_ev_timer;    // use this to encode a timer event.

void  timer_event_init(void);

void  timer_event_start(void);

struct rte_event_timer * gen_timer_ev( struct rte_event_timer * p_evt_timer,
                                         uint32_t flow_id,
                                         uint8_t  sched_type,
                                         uint8_t  evt_queue,
                                         uint8_t evt_queue_priority,
                                         void * evt_ptr);






void print_rte_event_timer_adapter_conf (int indent,const char* string,int id,struct rte_event_timer_adapter_conf *p);
void print_rte_event_timer_adapter_info (int indent,const char* string,int id,struct rte_event_timer_adapter_info  *p);
void print_rte_event_timer  (int indent,const char* string,struct rte_event_timer *p);




#endif

