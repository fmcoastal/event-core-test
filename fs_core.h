#ifndef _fs_core_h
#define _fs_core_h


/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

#include "main.h"

extern struct test_mode_struct  tm_core;
extern char * core_message[];


// Stuff the event structure with the values I want
// and make it cleaner for other tests to generate timer events.

 struct rte_event * gen_ev( struct rte_event * p_ev,
                                        uint32_t flow_id,
                                        uint8_t  sched_type,
                                        uint8_t  evt_queue,
                                        uint8_t  evt_queue_priority,
                                        void *   evt_ptr);







#endif

