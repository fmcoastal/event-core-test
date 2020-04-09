
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
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
//#include <rte_random.h>
#include <rte_debug.h>
//#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
//#include <rte_tcp.h>
//#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>
#include <rte_lpm.h>
#include <rte_errno.h>


#include <rte_lpm.h>
#include <rte_errno.h>
#include "fs_lpm_test.h"

#include "main.h"

static struct ipv4_fs_lpm_route RouteArray[] = {
         {RTE_IPV4(198, 18,  0, 0), 24,  10},  /* HF */
         {RTE_IPV4(198, 18,  1, 0), 24, 11},  /* HS */
         {RTE_IPV4(198, 18,  2, 0), 24, 12},  /* LF */
         {RTE_IPV4(198, 18,  3, 0), 24, 13},  /* LL */
};

static struct ipv4_fs_lpm_test  test[] = {

          {RTE_IPV4(198, 168, 1, 1), 0,  0},  /* fail */
          {RTE_IPV4(198, 18,  0, 0), 10, 1},  /* pass */
          {RTE_IPV4(198, 18,  1, 0), 11, 1},  /* pass */
          {RTE_IPV4(198, 18,  2, 0), 12, 1},  /* pass */
          {RTE_IPV4(198, 18,  3, 0), 13, 1},  /* pass */
 };



#if 0
// example of what printout looks like.  

******
  LPM TEST: create LPM structure
******
    Successfully created LPM Object:
    LPM Name:  fs_lpm
    test_config.max_rules   :  1024   (0x00000400)
    test_config.number_tbl8s:  256   (0x00000100)
    test_config.flags:  0   (0x0)
******
  LPM TEST: add 4 Routes
******
 0)  IP:198.18.0.0 (0xc6120000)  Depth:24,   next hop (value on Match): 0x0a
 1)  IP:198.18.1.0 (0xc6120100)  Depth:24,   next hop (value on Match): 0x0b
 2)  IP:198.18.2.0 (0xc6120200)  Depth:24,   next hop (value on Match): 0x0c
 3)  IP:198.18.3.0 (0xc6120300)  Depth:24,   next hop (value on Match): 0x0d
******
  LPM TEST: Test Rules were Added
******
0)  RouteArray{0] is present: SUCCESS  returned expected value:0xa
1)  RouteArray{1] is present: SUCCESS  returned expected value:0xb
2)  RouteArray{2] is present: SUCCESS  returned expected value:0xc
3)  RouteArray{3] is present: SUCCESS  returned expected value:0xd
******
  LPM TEST: Do lookup and check results
******
ENOENT:  2 0x2
EINVAL:  22 0x16
 0) Submit ip: 198.168.1.1 (0xc6a80101)ret:-2  N-H 0 lookup: not_found, expected not-found  **test PASSED**
 1) Submit ip: 198.18.0.0 (0xc6120000)ret:0  N-H a lookup: found, N-H matched. expected Pass **test PASSED**
 2) Submit ip: 198.18.1.0 (0xc6120100)ret:0  N-H b lookup: found, N-H matched. expected Pass **test PASSED**
 3) Submit ip: 198.18.2.0 (0xc6120200)ret:0  N-H c lookup: found, N-H matched. expected Pass **test PASSED**
 4) Submit ip: 198.18.3.0 (0xc6120300)ret:0  N-H d lookup: found, N-H matched. expected Pass **test PASSED**
******
  LPM TEST: Validate Rules are still in place...
******
0)  RouteArray{0] is present: SUCCESS  returned expected value:0xa
1)  RouteArray{1] is present: SUCCESS  returned expected value:0xb
2)  RouteArray{2] is present: SUCCESS  returned expected value:0xc
3)  RouteArray{3] is present: SUCCESS  returned expected value:0xd
******
  LPM TEST: delete LPM structure
******

#endif


static inline char * format_ip_addr(char * str,uint32_t ip);

static const char C_NORMAL[]   ={0x1b,'[','0','m',0x00};         /* NORMAL=$'\e[0m'    */
static const char C_GREEN[]    ={0x1b,'[','3','2','m',0x00};     /* GREEN =$'\e[32m' */

static inline void NORMAL(void) { printf("%s",C_NORMAL);}
static inline void GREEN(void)  { printf("%s",C_GREEN);}



struct rte_lpm * g_p_test_lpm = NULL ;// handle to LPM structure


//  DUMMY TEST FUNCTION
int    lpm_setup( __attribute__((unused))void * arg);
int     lpm_loop( __attribute__((unused))void * arg);
int    lpm_print( __attribute__((unused))void * arg);
int  lpm_cleanup( __attribute__((unused))void * arg);
void lpm_description( void);

int lpm_setup( __attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("dummy test function %d\n",x);
    int i;

    int result;
    int nb_rules = sizeof(RouteArray)/sizeof(struct ipv4_fs_lpm_route) ;
//    int nb_tests = sizeof(test)/sizeof(struct ipv4_fs_lpm_test) ;
    uint32_t tmp;
    char  scratch[64];
    const char  test_lpm_name[] =  {"fs_lpm "};
    struct  rte_lpm_config test_cfg;    // config struture for LPM table

    GREEN();
    printf("******\n");
    printf("  LPM TEST: create LPM structure \n");
    printf("******\n");
 

    test_cfg.max_rules = 1024;          // use value in L3fwd
                                        //   24 bit table??
    test_cfg.number_tbl8s = ( 1 << 8 ); // use value in L3fwd
                                        //    8 bit tables???
    test_cfg.flags = 0;                 // documentation says unused

      // 0 is numa socket
      g_p_test_lpm = rte_lpm_create(test_lpm_name, 0, &test_cfg);

      if(g_p_test_lpm == NULL)
      {
         printf(" failed to create LPM search structure\n");
         printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
         return -1;
      }
      else
      {
         printf("    Successfully created LPM Object:\n");
         printf("    LPM Name:  %s\n",test_lpm_name);
         printf("    test_config.max_rules   :  %d   (0x%08x)\n",test_cfg.max_rules,test_cfg.max_rules );
         printf("    test_config.number_tbl8s:  %d   (0x%08x)\n",test_cfg.number_tbl8s,test_cfg.number_tbl8s );
         printf("    test_config.flags:  %d   (0x%0x)\n",test_cfg.flags,test_cfg.flags );
      }

     printf("******\n");
     printf("  LPM TEST: add %d Routes \n",nb_rules);
     printf("******\n");
     for( i = 0 ; i < nb_rules ; i++)
     {
       result = rte_lpm_add(g_p_test_lpm, RouteArray[i].ip,RouteArray[i].depth, RouteArray[i].ResultOnMatch);
       if(result != 0)
       {
         printf(" failed to create LPM search structure\n");
         printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
       }
       else
       {
         printf(" %d)  IP:%s (0x%08x)  Depth:%d,   next hop (value on Match): 0x%02x\n",i
                                     ,format_ip_addr(scratch, RouteArray[i].ip)
                                     ,RouteArray[i].ip
                                     ,RouteArray[i].depth
                                     ,RouteArray[i].ResultOnMatch );
       }
     }
     printf("******\n");
     printf("  LPM TEST: Test Rules were Added \n");
     printf("******\n");
     for( i = 0 ; i < nb_rules ; i++)
     {
       result = rte_lpm_is_rule_present (g_p_test_lpm, RouteArray[i].ip,RouteArray[i].depth, &tmp  );
       if(result < 0)
       {
         printf("%d) error when calling  rte_lpm_is_rule_present() \n",i);
         printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
       }
       else if ( result == 1)
       {
         printf("%d)  RouteArray{%d] is present: ",i,i);
         if( RouteArray[i].ResultOnMatch == tmp)
               printf("SUCCESS  returned expected value:0x%x\n",tmp);
         else
               printf("ERROR  returned 0x%x not expceted 0x%x:  \n",tmp,RouteArray[i].ResultOnMatch);
       }
       else if (result == 0)
       {
         printf("%d)  RouteArray{%d] is NOT present: ",i,i);
       }
       else
       {
         printf("%d)  RouteArray{%d]  UNRECOGNIZED RESPONSE %d: ",i,i,result);
       }
     }
     return 0;
}

int lpm_loop( __attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("dummy loop function %d\n",x);


   char scratch[128];
   int32_t i;
   uint32_t tmp;
   int result;
   int nb_tests = sizeof(test)/sizeof(struct ipv4_fs_lpm_test) ;

printf("******\n");
printf("  LPM TEST: Do lookup and check results \n");
printf("******\n");

printf("ENOENT:  %d 0x%x\n",ENOENT,ENOENT);
printf("EINVAL:  %d 0x%x\n",EINVAL,EINVAL);

printf("  LPM TEST: enter Ctrl-C to exit \n");
printf("******\n");


    while (!g_force_quit)
    {
//Do lookup and check results
        for( i = 0 ; i < nb_tests ; i++)
        {
          printf(" %d) Submit ip: %s (0x%08x)",i
                                        ,format_ip_addr(scratch, test[i].ip)
                                        ,test[i].ip );
          tmp=0;
          result = rte_lpm_lookup (g_p_test_lpm, test[i].ip , &tmp  );
          printf("ret:%d  N-H %x",result,tmp);
          if(result ==  0)    /* found match */
          {
             if( tmp == test[i].expected_result ) /* get back proper N-H response value  */
             {
                 if(test[i].pass_fail == 1)  // Expect a match, get match, get correct data, we expect good result -  PASS.
                 {
                     printf(" lookup: found, N-H matched. expected Pass **test PASSED** \n");
                 }
                 else
                 {
                     printf(" lookup: found, N-H matched, expeced Fail, **test FAILED** (should have been pass)  \n");
                 }
             }
             else /* did not get back the expected N-H  value */
             {
                 printf(" lookup: found,  expecteed H-N was %x  **test FAILED** \n",test[i].expected_result);
             }
          }
          else if (result == -2)   // no match found
          {
             if(test[i].pass_fail == 1)  // Is this the expected result?
             {
                 printf(" lookup: not_found, expected found,  **test FAILED**\n");
             }
             else
             {
                 printf(" lookup: not_found, expected not-found  **test PASSED**\n");
             }
          }
          else
          {
              printf(" lookup returned unexpected results **test FAILED**  %d\n",result);
          }
       }
     }
     return 0;
}

int lpm_print(__attribute__((unused)) void * arg)
{
     int32_t x = -1 ;
     if (arg != NULL ) x = *(int32_t *)arg;
     printf("dummy print function %d\n",x);
     return 0;
}

int lpm_cleanup(__attribute__((unused)) void * arg)
{
//     int32_t x = -1 ;
//     if (arg != NULL ) x = *(int32_t *)arg;
//     printf("dummy cleanup function %d\n",x);
   int result;
   int i;
   uint32_t tmp;
   int nb_rules = sizeof(RouteArray)/sizeof(struct ipv4_fs_lpm_route) ;
printf("******\n");
printf("  LPM TEST: Validate Rules are still in place... \n");
printf("******\n");
     for( i = 0 ; i < nb_rules ; i++)
     {
       result = rte_lpm_is_rule_present (g_p_test_lpm, RouteArray[i].ip,RouteArray[i].depth, &tmp  );
       if(result < 0)
       {
         printf("%d) error when calling  rte_lpm_is_rule_present() \n",i);
         printf("    errno:%d  %s\n",rte_errno,rte_strerror(rte_errno));
       }
       else if ( result == 1)
       {
         printf("%d)  RouteArray{%d] is present: ",i,i);
         if( RouteArray[i].ResultOnMatch == tmp)
               printf("SUCCESS  returned expected value:0x%x\n",tmp);
         else
               printf("ERROR  returned 0x%x not expceted 0x%x:  \n",tmp,RouteArray[i].ResultOnMatch);
       }
       else if (result == 0)
       {
         printf("%d)  RouteArray{%d] is NOT present: ",i,i);
       }
       else
       {
         printf("%d)  RouteArray{%d]  UNRECOGNIZED RESPONSE %d: ",i,i,result);
       }

     }

// could test deleting rule.

//   Destroy what we created
printf("******\n");
printf("  LPM TEST: delete LPM structure \n");
printf("******\n");
      rte_lpm_free(g_p_test_lpm);

      NORMAL();

     return 0;
}

void  lpm_description(void)
{
    printf(" \"spinlock\" - is an example of how to uese\n");
    printf("                the rte_lpm_xxx functions.  \n");
    printf("                This test will loop doing lookups  \n");
    printf("                Use Ctrl-C to terminate  \n");
}

struct test_mode_struct  tm_lpm = {
      .setup         = lpm_setup,
      .main_loop     = lpm_loop,
      .print_results = lpm_print,
      .cleanup       = lpm_cleanup,
      .description   = lpm_description,
};




/*
 *   COnverts an IP address into a printable string
 */
static inline char * format_ip_addr(char * str,uint32_t ip)
{
   sprintf(str,"%d.%d.%d.%d",((ip >> 24)&0xff),((ip >> 16)&0xff),((ip >> 8)     &0xff),((ip >> 0)&0xff));
   return str;
}


#if 0

static void
 signal_handler(int signum)
 {
         if (signum == SIGINT || signum == SIGTERM) {
                 printf("\n\nSignal %d received, preparing to exit...\n",
                                 signum);
                 g_force_quit = true;
         }
 }



int
main(int  argc, char **argv)
{
        struct l3fwd_event_resources *evt_rsrc;
        struct lcore_conf *qconf;
        struct rte_eth_dev_info dev_info;
        struct rte_eth_txconf *txconf;
        int i, ret;
        unsigned nb_ports;
        uint16_t queueid, portid;
        unsigned lcore_id;
        uint32_t n_tx_queue, nb_lcores;
        uint8_t nb_rx_queue, queue, socketid;

        /* init EAL */
        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_exit(EXIT_FAILURE, "Invalid EAL parameters\n");
        argc -= ret;
        argv += ret;

        g_force_quit = false;
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

#if 0
        // fs - play with logging
        {
        FILE *foo1 = NULL;
        int x = 0;
        printf (" rte_log_get_stream:  %p \n",rte_log_get_stream());
        printf (" STDOUT:  %p \n",stdout);
        printf (" rte_gloable_log_level: %d\n",rte_log_get_global_level());
        printf (" rte_log_get_level(RTE_LOGTYPE_MBUF) : %d\n",rte_log_get_level(RTE_LOGTYPE_MBUF));
        rte_log_set_global_level(RTE_LOG_DEBUG);

//        rte_log_set_level (uint32_t logtype, uint32_t level)
        rte_log_set_level ( RTE_LOGTYPE_MEMPOOL , RTE_LOG_DEBUG);
        rte_log_set_level ( RTE_LOGTYPE_MBUF , RTE_LOG_DEBUG);
        rte_log_set_level ( RTE_LOGTYPE_USER1 , RTE_LOG_DEBUG);
//        rte_log_set_level ( RTE_LOGTYPE_PMD , RTE_LOG_DEBUG);

        x = (rte_log_set_level_regexp("pmd.net.octeontx.init"  ,RTE_LOG_DEBUG) == 0) ? (x << 1) | 0x01 : (x << 1) ;
        x = (rte_log_set_level_regexp("pmd.net.octeontx.driver",RTE_LOG_DEBUG) == 0) ? (x << 1) | 0x01 : (x << 1) ;
//      x = (rte_log_set_level_regexp(".port."                 ,RTE_LOG_DEBUG) == 0) ? (x << 1) | 0x01 : (x << 1) ;
//      x = (rte_log_set_level_regexp(".thunderx."             ,RTE_LOG_DEBUG) == 0) ? (x << 1) | 0x01 : (x << 1) ;
        printf(" Log event result 0x%08x \n",x);

        RTE_LOG( DEBUG , MEMPOOL , " Log to MEMPOOL fs_test \n");
        RTE_LOG( DEBUG , USER1 , " Log to USER1: fs_test\n");

        foo1 = fopen("TST_LOG","w+");
        if ( foo1 != NULL )
           {
              rte_log_dump(foo1);  // this tells the log level of the perticular logging type.
              fclose (foo1);
           }
        else
           {
              printf(" failed to open file TST_LOG\n");
           }
        }
#endif

       if( lpm_setup( NULL ) != 0) return -1;
       lpm_loop( NULL );
       lpm_print( NULL );
       lpm_cleanup( NULL );



   return 0;
}


#endif  /* end standalone */
