#ifndef fs_log_h
#define fs_log_h
       #include <rte_log.h>

/* NOTES:
      From DPDK.org 

      Log messages will be for the LogLevel and Higher messages
             so, if the log level is set to Debug, any and all RTE_LOG
             messages will be printed and logged.

      TO enable log messages,
         1) Enable log messages in the module you want output from. 
         1) Set the global level to the level you want to see
          
   How to enable the module output ....
      Major Modules have macros.  Use something like

          rte_log_set_level ( RTE_LOGTYPE_EVENTDEV , RTE_LOG_INFO);

      For Finer grane, look at the output of " "DetailedLoggingOptions ".
          then use the function rte_log_set_level_regexp 

*/

        // fs - play with logging
{
    FILE *foo1 = NULL;
    int id = 0;
    static char C_GREEN[]   ={0x1b,'[','3','2','m',0x00};  // GREEN=$'\e[32m' 
    static char C_CYAN[]   ={0x1b,'[','3','6','m',0x00};  // GREEN=$'\e[32m' 
    static char C_WHITE[]   ={0x1b,'[','3','7','m',0x00};  // GREEN=$'\e[32m' 
    static char C_YELLOW[]   ={0x1b,'[','3','3','m',0x00};  // GREEN=$'\e[32m' 
 
    printf ("%s",C_GREEN);
    printf (" rte_log_get_stream:  %p \n",rte_log_get_stream());
    printf (" STDOUT:  %p \n",stdout);
    printf (" rte_gloable_log_level: %d\n",rte_log_get_global_level());
    printf (" rte_log_get_level(RTE_LOGTYPE_MBUF) : %d\n",rte_log_get_level(RTE_LOGTYPE_MBUF));
    rte_log_set_global_level(RTE_LOG_DEBUG);

//  rte_log_set_level (uint32_t logtype, uint32_t level)
    rte_log_set_level ( RTE_LOGTYPE_MEMPOOL  , RTE_LOG_DEBUG);
    printf (" rte_log_get_level(RTE_LOGTYPE_MEMPOOL) : %d\n",rte_log_get_level(RTE_LOGTYPE_MEMPOOL));

    rte_log_set_level ( RTE_LOGTYPE_MBUF     , RTE_LOG_DEBUG);
    printf (" rte_log_get_level(RTE_LOGTYPE_MBUF)    : %d\n",rte_log_get_level(RTE_LOGTYPE_MBUF));

    rte_log_set_level ( RTE_LOGTYPE_USER1    , RTE_LOG_DEBUG);
    printf (" rte_log_get_level(RTE_LOGTYPE_USER1)   : %d\n",rte_log_get_level(RTE_LOGTYPE_USER1));

    rte_log_set_level ( RTE_LOGTYPE_EVENTDEV , RTE_LOG_INFO);
    printf (" rte_log_get_level(RTE_LOGTYPE_EVENTDEV): %d\n",rte_log_get_level(RTE_LOGTYPE_EVENTDEV));

/*
    rte_log_set_level ( RTE_LOGTYPE_PMD      , RTE_LOG_DEBUG);
    printf (" rte_log_get_level(RTE_LOGTYPE_PMD): %d\n",rte_log_get_level(RTE_LOGTYPE_PMD));
*/

    printf(" rte_log_cur_msg_loglevel per_lcore? %d\n",rte_log_cur_msg_loglevel());	
    printf(" rte_log_cur_msg_logtype  per_lcore? %d\n",rte_log_cur_msg_logtype());	

    printf ("%s",C_WHITE);

    // Example of how to set and check a Logging Level

    printf ("%s",C_CYAN);
    printf("Set \"pmd.net.octeontx.init\"  to RTE_LOG_DEBUG\n" );
    rte_log_set_level_regexp("pmd.net.octeontx.init"  ,RTE_LOG_DEBUG);
    id = rte_log_register("pmd.net.octeontx.init");    
    printf("  Id assigned to  \"pmd.net.octeontx.init\"  : %d \n",id);
    printf("  Verify \"pmd.net.octeontx.init\" Level: %d \n", rte_log_get_level(id ));
    printf ("%s",C_WHITE);

    // Example of how to set log level based on a Regex
    rte_log_set_level_regexp("pmd.net.octeontx.driver",RTE_LOG_DEBUG) ;  // specific
    rte_log_set_level_regexp(".port."                 ,RTE_LOG_DEBUG) ;  // anything with port
    rte_log_set_level_regexp(".thunderx."             ,RTE_LOG_DEBUG) ;  // anything with Thunderx

    // generate log events.   note RTE_LOG macro expands DEBUG to RTE_LOG_DEBUG
    printf ("%s",C_YELLOW);
    RTE_LOG( DEBUG , MEMPOOL , " Log Message Test to MEMPOOL fs_test \n");
    RTE_LOG( DEBUG , USER1   , " Log Message Test to USER1: fs_test\n");
    printf ("%s",C_WHITE);


//#define FIGURE_OUT_THE_BEHAVIOUR
#ifdef FIGURE_OUT_THE_BEHAVIOUR
    {
        static char C_RED[]   ={0x1b,'[','3','1','m',0x00};  // GREEN=$'\e[32m' 
        int last_global_log_level = rte_log_get_global_level();
        unsigned i;    

        printf ("%s",C_RED);
        for ( i = RTE_LOG_EMERG; i <= RTE_LOG_DEBUG  ; i++)
        {
            rte_log_set_global_level(i);
            printf("\n");	
            printf("*** current rte global log level %d           ****\n",rte_log_get_global_level());	
            printf("***  - Now generate log messaes at each level ****\n");	
            printf("***  - and see what prints                    ****\n");	

//          the emergency message below generates messages to systemd.journal.  
//            RTE_LOG( EMERG   , USER1 , " Log to USER1:  EMERG  (%d) \n",i);
            RTE_LOG( ALERT   , USER1 , " Log to USER1:  ALERT   (%d) \n",RTE_LOG_ALERT);
            RTE_LOG( CRIT    , USER1 , " Log to USER1:  CRIT    (%d) \n",RTE_LOG_CRIT);
            RTE_LOG( ERR     , USER1 , " Log to USER1:  ERR     (%d) \n",RTE_LOG_ERR);
            RTE_LOG( WARNING , USER1 , " Log to USER1:  WARNING (%d) \n",RTE_LOG_WARNING);
            RTE_LOG( NOTICE  , USER1 , " Log to USER1:  NOTICE  (%d) \n",RTE_LOG_NOTICE);
            RTE_LOG( INFO    , USER1 , " Log to USER1:  INFO    (%d) \n",RTE_LOG_INFO);
            RTE_LOG( DEBUG   , USER1 , " Log to USER1:  DEBUG   (%d) \n",RTE_LOG_DEBUG);
        }
        // restore the original Log level
        rte_log_set_global_level(last_global_log_level);
        printf ("%s",C_WHITE);
    }
#endif

    foo1 = fopen("DetailedLoggingOptions","w+");
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


