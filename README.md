# dpdk_test_app

The dpdk_test_app is "test harness code".  THe base code (see main.c) does not set up any IO. 
      It can be used to debug and profile code. Test code files (fs_lpm.c and fs_spinlock.c) 
      can be copied and modified to enable the code you want to test. Adding your own test code
      is as simple aadding your test_mode_struct "name" to the list at the top of "main.c". 

        // array of tests
        struct test_mode_struct test_array[] = {
                        tm_spinlock,
                        tm_rwspinlock,
                        tm_rte_lcore_id,
                        tm_lpm,
                        YOUR_TEST_STRUCT_HERE,
        };

       Your test function must implement the following functions:
          
        // struct to hold test function parameters
        struct test_mode_struct {
                 int   (*setup)(void *);
                 int   (*main_loop)(void *);
                 int   (*print_results)(void *);
                 int   (*cleanup)(void *);
                 void  (*description)(void);
         };

        tm_dummy can be used as a template for building out your fuctionality,  Feel free to modify the 
        main() function as you need for your testing.


To Get Help Output:
         root@localhost:/home/fsmith/dpdk_test_app# ./build/test_app

  Example:
     myapp -c 0x0f -- -t 3

           -c <coremash>
           -T <option>

           1 "spinlock" - measures & reports the time it takes
                         to run 1,000,000  rte_spinlock_lcok()
                         and rte_spinlock_unlock()
           2 "rw_spinlock" - measures & reports the time it takes
                         to run 1,000,000 rte_rwlock_read_lock()
                         and rte_rwlock_read_unlock()
           3 "lcore_id" - measures & reports the time it
                         takes to run 1,000,000 rte_lcore_id()
           4 "spinlock" - is an example of how to uese
                         the rte_lpm_xxx functions.
                         This test will loop doing lookups
                         Use Ctrl-C to terminate
           0 "Dummy" - template test which does noting

         root@localhost:/home/fsmith/dpdk_test_app#



Download the dpdk_test_app

   * cd to the directory where you want the "dpdk_test_app" directory created.
   * execute the folloing command to download the Project:

      > git clone https://github.com/fmcoastal/dpdk_test_app

To Build 
  - Standalone Environment 
  
   * cd to the dpdk_test_app directory 
   * execute make 
      > make
   * if successful,  the program test_app can be found in 
      ./build/test_app

   ** You may need the following Environment variables set:

      > export RTE_TARGET=build
      > export RTE_KERNELDIR=/home/fsmith/linux
      > export RTE_SDK=/home/fsmith/dpdk/dpdk-19.11

    * to run
      sudo ./build/test_app -c 0x0001  -- -t 1

  - Buildroot Environment 


