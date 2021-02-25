#ifndef _fs_eventdev_timer_h
#define _fs_eventdev_timer_h



#include "main.h"

#define NSECPERSEC 1E9 // No of ns in 1 sec
 

extern struct test_mode_struct  tm_timer;


void  timer_event_init(void);


void print_rte_event_timer_adapter_conf (int indent,const char* string,int id,struct rte_event_timer_adapter_conf *p);
void print_rte_event_timer_adapter_info (int indent,const char* string,int id,struct rte_event_timer_adapter_info  *p);



#endif

