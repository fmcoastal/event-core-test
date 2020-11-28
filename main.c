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
#include "main.h"

//  TEST FUNCTION INCLUDES
#include "fs_tstamp.h"
#include "fs_spinlock_test.h"
#include "fs_lpm_test.h"
#include "fs_core.h"


// GLOBAL DEFINITIONS REFERENCED BY TEST FUNCTIONS

uint64_t      g_verbose = 0;          // Higher number more printouts 
volatile bool g_force_quit;           // Ctrl-C flag
rte_spinlock_t g_fs_print_lock = {0}; //  lock for printing to output if 
                                      //      you need clean output

// GLOBAL DEFINITION USED IN MAIN.C
struct test_mode_struct g_tst_func;   // which test algo. to run
int32_t    g_test_selection = -1;     // which test to run
                                      //     -1 none selected
int myapp_parse_args(int argc, char **argv);



//  DUMMY TEST FUNCTION
int        dummy_setup( __attribute__((unused))void * arg);
int         dummy_loop( __attribute__((unused))void * arg);
int        dummy_print( __attribute__((unused))void * arg);
int      dummy_cleanup( __attribute__((unused))void * arg);
void dummy_description( void);

int dummy_setup( __attribute__((unused)) void * arg)
{
     int32_t x = -1 ;
     if (arg != NULL ) x = *(int32_t *)arg;
     printf("dummy test function %d\n",x);
     return 0;
}
int dummy_loop( __attribute__((unused)) void * arg)
{
     int32_t x = -1 ;
     if (arg != NULL ) x = *(int32_t *)arg;
     printf("dummy loop function %d\n",x);
     return 0;
}
int dummy_print(__attribute__((unused)) void * arg)
{
     int32_t x = -1 ;
     if (arg != NULL ) x = *(int32_t *)arg;
     printf("dummy print function %d\n",x);
     return 0;
}
int dummy_cleanup(__attribute__((unused)) void * arg)
{
     int32_t x = -1 ;
     if (arg != NULL ) x = *(int32_t *)arg;
     printf("dummy cleanup function %d\n",x);
     return 0;
}
void  dummy_description(void)
{
    printf(" \"Dummy\" - template test which does noting\n");
}

struct test_mode_struct  tm_dummy = {
      .setup          = dummy_setup,
      .main_loop      = dummy_loop,
      .print_results  = dummy_print,
      .cleanup        = dummy_cleanup,
      .description    = dummy_description,
};



void usage(void);
void usage(void)
{

    printf("\n"); 
    printf("  Example:\n"); 
    printf("     myapp -c 0x0f -- -t 3\n"); 
    printf("\n"); 
    printf("  -c <coremash>\n");  
    printf("  -T <option>  \n\n");
    printf("  %d",1);   tm_spinlock.description();
    printf("  %d",2);   tm_rwspinlock.description();
    printf("  %d",3);   tm_rte_lcore_id.description();
    printf("  %d",4);   tm_lpm.description();
    printf("  %d",5);   tm_core.description();
    printf("  %d",0);   tm_dummy.description();
    printf("\n"); 
}


 /*
  * Setup test function  methods.
  */
 static void
 setup_test_funtions(void)
 {
         /* run the spinlock test. */
         if ( g_test_selection  == 1)
                 g_tst_func  = tm_spinlock;
         else if ( g_test_selection  == 2)
                 g_tst_func  = tm_rwspinlock;
         else if ( g_test_selection  == 3)
                 g_tst_func  = tm_rte_lcore_id;
         else if ( g_test_selection  == 4)
                 g_tst_func  = tm_lpm;
         else if ( g_test_selection  == 5)
                 g_tst_func  = tm_core;


         /* Setup dummy lookup functions. */
         else
                 g_tst_func = tm_dummy;
 }


static int
test_launch_one_lcore(__attribute__((unused)) void *dummy)
{
        if (g_tst_func.main_loop != NULL) g_tst_func.main_loop(dummy);
	return 0;
}



static void signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n",
				signum);
		g_force_quit = true;
	}
}


int
main(int argc, char **argv)
{
	int ret;
        unsigned  lcore_id;

        if(argc <= 1)
        {
            usage();
            return 1;
        }

	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

        /* init Signal functions */
	g_force_quit = false;
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* parse application arguments (after the EAL ones) */
	ret = myapp_parse_args(argc, argv);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE, "Invalid myapp arguments\n");
                usage();
                return -1;
        }

        if( g_test_selection < 0)
        {
                printf(" No Test Function Specified.\n");
                usage();
       		rte_exit(EXIT_FAILURE, "Invalid myapp arguments\n");
                return -1;
        }

        // configure for the test to run. 
        setup_test_funtions();

#include "fs_log.h"
        // run the setup functions for the particular test;
        if( g_tst_func.setup != NULL) g_tst_func.setup(NULL);


	ret = 0;
        lcore_id = rte_lcore_id();
	/* launch per-lcore init on every lcore */
	rte_eal_mp_remote_launch(test_launch_one_lcore, NULL, CALL_MASTER);
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		if (rte_eal_wait_lcore(lcore_id) < 0) {
			ret = -1;
			break;
		}
	}

/*	if you change "CALL_MASTER" above to "SKIP_MASTER", the "Loop Code"
        will not be run on this core. create a loop here where you do 
        any processing you want.  Make sure to check g_force_quit to know when
        to break out.  you will want to implement like the rte_eal_wait_lcore()
        to know when all the "thread-cores" have terminated. 
 */

        if( g_tst_func.print_results != NULL)  g_tst_func.print_results(NULL);
	if( g_tst_func.cleanup != NULL)  g_tst_func.cleanup(NULL);

        printf("  thank you very much\n");
	printf("Bye...\n");

	return ret;
}



/***********************************************************
 *    cmd line options
 */
static const char short_options[] =
	"h"   /* help */
	"p:"  /* portmask */
	"q:"  /* number of queues */
	"t:"  /* test number */
	"v:"  /* verbose output */
	;

//#define CMD_LINE_OPT_MAC_UPDATING "mac-updating"
//#define CMD_LINE_OPT_NO_MAC_UPDATING "no-mac-updating"

enum {
	/* long options mapped to a short option */

	/* first long only option value must be >= 256, so that we won't
	 * conflict with short options */
	CMD_LINE_OPT_MIN_NUM = 256,
};

static const struct option lgopts[] = {
//	{ CMD_LINE_OPT_MAC_UPDATING, no_argument, &mac_updating, 1},
//	{ CMD_LINE_OPT_NO_MAC_UPDATING, no_argument, &mac_updating, 0},
	{NULL, 0, 0, 0}
};

/* Parse the argument given in the command line of the application */
int
myapp_parse_args(int argc, char **argv)
{
	int opt, ret;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];

	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, short_options,
				  lgopts, &option_index)) != EOF) {

		switch (opt) {
		/* portmask */
		case 'p':
			break;

		/* nqueue */
		case 'q':
			break;

		/* help */
		case 'h':
                        usage();
			break;

		/* test code to run */
		case 't':
                        printf(" -t option= %s \n",optarg);
                        g_test_selection = atoi(optarg);
			break;

		/* verbose */
		case 'v':
                        printf(" verbose = %s \n",optarg);
                        g_verbose = atoi(optarg);
			break;


		/* long options */
		case 0:
			break;

		default:
			//myapp_usage(prgname);
			return -1;
		}
	}

	if (optind >= 0)
		argv[optind-1] = prgname;

	ret = optind-1;
	optind = 1; /* reset getopt lib */
	return ret;
}






#if 0
// array of time stamps structures, 1 per core
// fs_time_stamp g_per_core_time_stamp[32]={0}; // per core time stamp


struct test_mode_struct  tm_rwlock = {
      .setup         = NULL,
      .main_loop     = NULL,
      .print_results = NULL,
      .cleanup       = NULL,
};


/***********************************************************
 *   Built in test function 
 *       this test a single lock structure being accessed by all 
 *       cores started as part of the application
 */
// rte_spinlock_t g_fs_measure_lock;
rte_rwlock_t g_fs_measure_lock;
//#define TestFunction()  rte_spinlock_lock( &g_fs_measure_lock); rte_spinlock_unlock( &g_fs_measure_lock);
#define TestFunction()     rte_rwlock_read_lock( &g_fs_measure_lock); rte_rwlock_read_unlock( &g_fs_measure_lock);
//#define TestFunction()     X=rte_lcore_id ();

/***********************************************************/





int test_main_loop(__attribute__((unused)) void *dummy);

/* main processing loop */
int test_main_loop(__attribute__((unused)) void *dummy)
{
        unsigned lcore_id;
 	char string[256];
        int  i;

        fs_time_stamp *p_per_core_time_stamp;

        lcore_id = rte_lcore_id();
        printf(" Launch code on Core: %d\n",lcore_id);


        p_per_core_time_stamp = &(g_per_core_time_stamp[lcore_id]);
        sprintf(string,"per_core_time_stamp[%d]",lcore_id);
        tstamp_init( p_per_core_time_stamp, string);


#define  MEASURE_LOCK_TIME
#ifdef  MEASURE_LOCK_TIME

       #define LOCK_LOOPS (1000*1000)
       {

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

          rte_spinlock_lock( &g_fs_print_lock);
          printf(" core %d done\n",lcore_id);
          rte_spinlock_unlock( &g_fs_print_lock);
      }
#endif

        while (!g_force_quit) {
            /*    PUT YOUR LOOPING TEST  CODE HERE */
            do_lpm_test();


       }

       rte_spinlock_lock( &g_fs_print_lock);
       printf("************\n    Core   %d     \n***************\n",lcore_id);
       tstamp_print(p_per_core_time_stamp,1);
       rte_spinlock_unlock( &g_fs_print_lock);


        return 0;
}
#endif


