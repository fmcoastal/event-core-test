#ifndef _main_h
#define _main_h


// flag to printout more data  Larger number prints more

// MESSAGES:  1 line helping you with where you are at
// DETAILS    HEADERS or BUFFERS,
// CALLS      Print the names of the functions called
// CALL_ARGS  Prints out the argument values of the called functions


extern uint64_t g_verbose;
#define  EVENT_MESSAGES         0x01
#define  EVENT_DETAILS          0x02

#define  EVENT_CORE_DETAILS    0x04
#define  EVENT_TIMER_DETAILS   0x08

// command line argument to print eth output every -p <arg> packets received
#define  EVENT_ETH_MESSAGES    0x0010  // verbosity level associate with argc -p
#define  EVENT_ETH_DETAILS     0x0020  // verbosity level associate with argc -p


#define      ETH_L2_ETH_TYPE    0x0040 //print out the l2.ethtype value 
#define      ETH_L2_DETAILS     0x0080 //Print the entire L2 header


#define      ETH_L3_MESSAGES        0x0100  //Print 1 lines, ARP,... 
#define      ETH_L3_DETAILS         0x0200  //Print the perticular L3 header 
//                                                (whatever it is, ARP,IPV4,IPV6...)  
#define      ETH_L3_IPV4_MESSAGES   0x0400
#define      ETH_L4_UDP_DETAILS     0x0800


#define VERBOSE_M(x)  if( (g_verbose & x) == x) 
#define VERBOSE(x)    if(   g_verbose >=  x )  



// variable ctrl-c uses to terminate program
extern volatile bool g_force_quit;

// struct to hold test function parameters
struct test_mode_struct {
         int   (*setup)(void *);
         int   (*main_loop)(void *);
         int   (*print_results)(void *);
         int   (*cleanup)(void *);
         void  (*description)(void);
 };


 //  lock for printing to output if you need clean output
extern rte_spinlock_t g_fs_print_lock ;


#endif
