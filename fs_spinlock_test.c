/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

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

#include <rte_spinlock.h>
#include "fs_tstamp.h"
#include "fs_lpm_test.h"



#include "main.h"
#include "fs_spinlock_test.h"



fs_time_stamp g_per_core_time_stamp[32]__rte_cache_aligned={0}; // per core time stamp
uint64_t  g_per_core_result[32]={0}; // per core time stamp


/*********************************************************************
 *********************************************************************
 *           REGULAR SPINLOCK TEST                                   *
 *   Test: use ret_spinlock_lock() and rte_spinlock_unlock()         *
 *   execute the functions and  measure the time it takes            *
 *    to run 1000000 calls.
 *********************************************************************
 *********************************************************************/

rte_spinlock_t g_spinlock_measure_lock;
#define SpinLockFunction()  rte_spinlock_lock( &g_spinlock_measure_lock); rte_spinlock_unlock( &g_spinlock_measure_lock);

//  forward reference for compiler
int        spinlock_setup( __attribute__((unused))void * arg);
int         spinlock_loop( __attribute__((unused))void * arg);
int        spinlock_print( __attribute__((unused))void * arg);
int      spinlock_cleanup( __attribute__((unused))void * arg);
void spinlock_description( void);

int spinlock_setup( __attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("spinlock_setup function %d\n",x);
     return 0;
}

#define LOCK_LOOPS (1000*1000)

int spinlock_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    char string[256];
    int  i;
//   int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy loop function %d\n",x);

    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    printf(" Launch code on Core: %d\n",lcore_id);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
    sprintf(string,"per_core_time_stamp[%d]",lcore_id);
    tstamp_init( p_per_core_time_stamp, string);

    tstamp_start( p_per_core_time_stamp);
    for( i = 0 ; (i < LOCK_LOOPS) && (!g_force_quit ) ; i ++)
    {
         SpinLockFunction();                            /*1*/
         SpinLockFunction();                            /*2*/
         SpinLockFunction();                            /*3*/
         SpinLockFunction();                            /*4*/
         SpinLockFunction();                            /*5*/

         SpinLockFunction();                            /*1*/
         SpinLockFunction();                            /*2*/
         SpinLockFunction();                            /*3*/
         SpinLockFunction();                            /*4*/
         SpinLockFunction();                            /*5*/

         SpinLockFunction();                            /*1*/
         SpinLockFunction();                            /*2*/
         SpinLockFunction();                            /*3*/
         SpinLockFunction();                            /*4*/
         SpinLockFunction();                            /*5*/

         SpinLockFunction();                            /*1*/
         SpinLockFunction();                            /*2*/
         SpinLockFunction();                            /*3*/
         SpinLockFunction();                            /*4*/
         SpinLockFunction();                            /*5*/

    }
    tstamp_end( p_per_core_time_stamp);
    
    // because we actually only measure 1 time,
    // the first interval record is the time.
    // save the interval so we can display the result for each core
    //       at the end. 
    g_per_core_result[lcore_id] = p_per_core_time_stamp->interval;

    rte_spinlock_lock( &g_fs_print_lock);
    printf(" core %d done\n",lcore_id);
    rte_spinlock_unlock( &g_fs_print_lock);

     // no looping while waiting for Ctrl-C funcion 
    if (g_verbose >= 3)
    {
        rte_spinlock_lock( &g_fs_print_lock);
        printf("************\n    Core   %d     \n***************\n",lcore_id);
        tstamp_print(p_per_core_time_stamp,1);
        rte_spinlock_unlock( &g_fs_print_lock);
    }
    return 0;
}

int spinlock_print(__attribute__((unused)) void * arg)
{
//    int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy print function %d\n",x);

    unsigned lcore_id;
    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    rte_spinlock_lock( &g_fs_print_lock);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
 
    printf("************\n    Core   %d     \n***************\n",lcore_id);
    tstamp_print(p_per_core_time_stamp,1);
    rte_spinlock_unlock( &g_fs_print_lock);

     return 0;
}

int spinlock_cleanup(__attribute__((unused)) void * arg)
{
     // print the results from each core
     int i;
     printf(" Spinlock Results\n");
     printf(" core     interval (ticks)\n");
     for ( i = 0 ; i < 32 ; i++)
     {
         if ( g_per_core_result[i] != 0 )
         {
              printf("  %2d       %ld\n",i, g_per_core_result[i]);
         }
     }
     return 0;
}

void  spinlock_description(void)
{
    printf(" \"spinlock\" - measures & reports the time it takes \n");
    printf("                to run 1,000,000  rte_spinlock_lcok()\n");
    printf("                and rte_spinlock_unlock()\n");
}



struct test_mode_struct  tm_spinlock = {
      .setup         = spinlock_setup,
      .main_loop     = spinlock_loop,
      .print_results = spinlock_print,
      .cleanup       = spinlock_cleanup,
      .description   = spinlock_description,
};





/*********************************************************************
 *********************************************************************
 *          READ WRITE SPINLOCK TEST
 *     same test as above, but uses rte_rw_read_lock()  and
 *                                  rte_rw_read_unlock()            
 *********************************************************************
 *********************************************************************/

rte_rwlock_t   g_rw_measure_lock;
#define RW_SpinLockFunction()     rte_rwlock_read_lock( &g_rw_measure_lock); rte_rwlock_read_unlock( &g_rw_measure_lock);

//  forward reference for compiler
int        rwspinlock_setup( __attribute__((unused))void * arg);
int         rwspinlock_loop( __attribute__((unused))void * arg);
int        rwspinlock_print( __attribute__((unused))void * arg);
int      rwspinlock_cleanup( __attribute__((unused))void * arg);
void rwspinlock_description( void);
int rwspinlock_setup( __attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("spinlock_setup function %d\n",x);
     return 0;
}

#define LOCK_LOOPS (1000*1000)

int rwspinlock_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    char string[256];
    int  i;
//   int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy loop function %d\n",x);

    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    printf(" Launch code on Core: %d\n",lcore_id);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
    sprintf(string,"per_core_time_stamp[%d]",lcore_id);
    tstamp_init( p_per_core_time_stamp, string);

    tstamp_start( p_per_core_time_stamp);
    for( i = 0 ; (i < LOCK_LOOPS) && (!g_force_quit ) ; i ++)
    {
         RW_SpinLockFunction();                            /*1*/
         RW_SpinLockFunction();                            /*2*/
         RW_SpinLockFunction();                            /*3*/
         RW_SpinLockFunction();                            /*4*/
         RW_SpinLockFunction();                            /*5*/

         RW_SpinLockFunction();                            /*1*/
         RW_SpinLockFunction();                            /*2*/
         RW_SpinLockFunction();                            /*3*/
         RW_SpinLockFunction();                            /*4*/
         RW_SpinLockFunction();                            /*5*/

         RW_SpinLockFunction();                            /*1*/
         RW_SpinLockFunction();                            /*2*/
         RW_SpinLockFunction();                            /*3*/
         RW_SpinLockFunction();                            /*4*/
         RW_SpinLockFunction();                            /*5*/

         RW_SpinLockFunction();                            /*1*/
         RW_SpinLockFunction();                            /*2*/
         RW_SpinLockFunction();                            /*3*/
         RW_SpinLockFunction();                            /*4*/
         RW_SpinLockFunction();                            /*5*/

    }
    tstamp_end( p_per_core_time_stamp);

    // because we actually only measure 1 time,
    // the first interval record is the time.
    // save the interval so we can display the result for each core
    //       at the end. 
    g_per_core_result[lcore_id] = p_per_core_time_stamp->interval;

    rte_spinlock_lock( &g_fs_print_lock);
    printf(" core %d done\n",lcore_id);
    rte_spinlock_unlock( &g_fs_print_lock);

     // no looping while waiting for Ctrl-C funcion 

    if (g_verbose >= 3)
    { 
        rte_spinlock_lock( &g_fs_print_lock);
        printf("************\n    Core   %d     \n***************\n",lcore_id);
        tstamp_print(p_per_core_time_stamp,1);
        rte_spinlock_unlock( &g_fs_print_lock);
    }
    return 0;
}

int rwspinlock_print(__attribute__((unused)) void * arg)
{
//    int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy print function %d\n",x);

    unsigned lcore_id;
    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    rte_spinlock_lock( &g_fs_print_lock);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
 
    printf("************\n    Core   %d     \n***************\n",lcore_id);
    tstamp_print(p_per_core_time_stamp,1);
    rte_spinlock_unlock( &g_fs_print_lock);

     return 0;
}


int rwspinlock_cleanup(__attribute__((unused)) void * arg)
{
     // print the results from each core
     int i;
     printf(" RW Spinlock Results\n");
     printf(" core     interval (ticks)\n");
     for ( i = 0 ; i < 32 ; i++)
     {
         if ( g_per_core_result[i] != 0 )
         {
              printf("  %2d       %ld\n",i, g_per_core_result[i]);
         }
     }

    return 0;
}

void  rwspinlock_description(void)
{
    printf(" \"rw_spinlock\" - measures & reports the time it takes \n");
    printf("                to run 1,000,000 rte_rwlock_read_lock()\n");
    printf("                and rte_rwlock_read_unlock()\n");
}




struct test_mode_struct  tm_rwspinlock = {
      .setup         = rwspinlock_setup,
      .main_loop     = rwspinlock_loop,
      .print_results = rwspinlock_print,
      .cleanup       = spinlock_cleanup,
      .description   = rwspinlock_description,
};


/*********************************************************************
 *********************************************************************
 *          rte_lcore_id TEST
 *     same test as above,  ret_lcore_id()
 *********************************************************************
 *********************************************************************/

#define TestFunction()     x=rte_lcore_id ();

//  forward reference for compiler
int        lcore_id_setup( __attribute__((unused))void * arg);
int         lcore_id_loop( __attribute__((unused))void * arg);
int        lcore_id_print( __attribute__((unused))void * arg);
int      lcore_id_cleanup( __attribute__((unused))void * arg);
void lcore_id_description( void);

int lcore_id_setup( __attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("spinlock_setup function %d\n",x);
     return 0;
}

#define LOCK_LOOPS (1000*1000)

int lcore_id_loop( __attribute__((unused)) void * arg)
{
    unsigned lcore_id;
    char string[256];
    int  i;
    int  x;
//   int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy loop function %d\n",x);

    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    printf(" Launch code on Core: %d\n",lcore_id);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
    sprintf(string,"per_core_time_stamp[%d]",lcore_id);
    tstamp_init( p_per_core_time_stamp, string);

    tstamp_start( p_per_core_time_stamp);
    for( i = 0 ; (i < LOCK_LOOPS) && (!g_force_quit ) ; i ++)
    {
         TestFunction();                            /*1*/
         TestFunction();                            /*2*/
         TestFunction();                            /*3*/
         TestFunction();                            /*4*/
         TestFunction();                            /*5*/

         TestFunction();                            /*1*/
         TestFunction();                            /*2*/
         TestFunction();                            /*3*/
         TestFunction();                            /*4*/
         TestFunction();                            /*5*/

         TestFunction();                            /*1*/
         TestFunction();                            /*2*/
         TestFunction();                            /*3*/
         TestFunction();                            /*4*/
         TestFunction();                            /*5*/

         TestFunction();                            /*1*/
         TestFunction();                            /*2*/
         TestFunction();                            /*3*/
         TestFunction();                            /*4*/
         TestFunction();                            /*5*/

    }
    tstamp_end( p_per_core_time_stamp);

    // because we actually only measure 1 time,
    // the first interval record is the time.
    // save the interval so we can display the result for each core
    //       at the end. 
    g_per_core_result[lcore_id] = p_per_core_time_stamp->interval;


    rte_spinlock_lock( &g_fs_print_lock);
    printf(" core %d done\n",x);
    rte_spinlock_unlock( &g_fs_print_lock);

     // no looping while waiting for Ctrl-C funcion 

    if (g_verbose >= 3)
    {  
        rte_spinlock_lock( &g_fs_print_lock);
        printf("************\n    Core   %d     \n***************\n",lcore_id);
        tstamp_print(p_per_core_time_stamp,1);
        rte_spinlock_unlock( &g_fs_print_lock);
    }
    return 0;
}

int lcore_id_print(__attribute__((unused)) void * arg)
{
//    int32_t x = -1 ;
//    if (arg != NULL ) x = *(int32_t *)arg;
//    printf("dummy print function %d\n",x);

    unsigned lcore_id;
    fs_time_stamp *p_per_core_time_stamp;

    lcore_id = rte_lcore_id();
    rte_spinlock_lock( &g_fs_print_lock);

    p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
 
    printf("************\n    Core   %d     \n***************\n",lcore_id);
    tstamp_print(p_per_core_time_stamp,1);
    rte_spinlock_unlock( &g_fs_print_lock);

     return 0;
}


int lcore_id_cleanup(__attribute__((unused)) void * arg)
{
     // print the results from each core
     int i;
     printf(" rte_lcore_id Results\n");
     printf(" core     interval (ticks)\n");
     for ( i = 0 ; i < 32 ; i++)
     {
         if ( g_per_core_result[i] != 0 )
         {
              printf("  %2d       %ld\n",i, g_per_core_result[i]);
         }
     }

     return 0;
}
void  lcore_id_description(void)
{
    printf(" \"lcore_id\" - measures & reports the time it \n");
    printf("                takes to run 1,000,000 rte_lcore_id()\n");
}


struct test_mode_struct  tm_rte_lcore_id = {
      .setup         = lcore_id_setup,
      .main_loop     = lcore_id_loop,
      .print_results = lcore_id_print,
      .cleanup       = lcore_id_cleanup,
      .description       = lcore_id_description,
};




