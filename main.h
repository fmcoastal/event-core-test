#ifndef _main_h
#define _main_h


// flag to printout more data  Larger number prints more
extern uint64_t g_verbose;

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
