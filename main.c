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

static volatile bool force_quit;

int myapp_parse_args(int argc, char **argv);


static void
signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n",
				signum);
		force_quit = true;
	}
}

//  lock for printing to output if you need clean output
rte_spinlock_t g_fs_print_lock = {0};

// array of time stamps structures, 1 per core
fs_time_stamp g_per_core_time_stamp[32]={0}; // per core time stamp


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
          for( i = 0 ; (i < LOCK_LOOPS) && (!force_quit ) ; i ++)
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

        while (!force_quit) {
            continue;


       }

       rte_spinlock_lock( &g_fs_print_lock);
       printf("************\n    Core   %d     \n***************\n",lcore_id);
       tstamp_print(p_per_core_time_stamp,1);
       rte_spinlock_unlock( &g_fs_print_lock);


        return 0;
}


static int
test_launch_one_lcore(__attribute__((unused)) void *dummy)
{
	test_main_loop(dummy);
	return 0;
}






int
main(int argc, char **argv)
{
	int ret;
        unsigned  lcore_id;

	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

	force_quit = false;
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* parse application arguments (after the EAL ones) */
	ret = myapp_parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid myapp arguments\n");



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

//	RTE_ETH_FOREACH_DEV(portid) {
//		if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
//			continue;
//		printf("Closing port %d...", portid);
//		rte_eth_dev_stop(portid);
//		rte_eth_dev_close(portid);
//		printf(" Done\n");
//	}






        printf("  thank you very much\n");
        printf("  mash any key to continue\n");


	printf("Bye...\n");

	return ret;
}



/***********************************************************
 *    cmd line options
 */
static const char short_options[] =
	"p:"  /* portmask */
	"q:"  /* number of queues */
	"T:"  /* timer period */
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

		/* timer period */
		case 'T':
                        printf(" %s \n",optarg);
//			timer_secs = l2fwd_parse_timer_period(optarg);
//			if (timer_secs < 0) {
//				printf("invalid timer period\n");
//				l2fwd_usage(prgname);
//				return -1;
//			}
//			timer_period = timer_secs;
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







