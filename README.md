# dpdk_test_app

The dpdk_test_app is "test harness code".  The base code (see main.c) does 
      not set up any IO. It can be used to debug and profile code. Test code 
      files (fs_lpm.c and fs_spinlock.c) can be copied and modified to 
      enable the code you want to test. 

      Your test function must implement the following functions:
          
        // struct to hold test function parameters
        struct test_mode_struct {
                 int   (*setup)(void *);
                 int   (*main_loop)(void *);
                 int   (*print_results)(void *);
                 int   (*cleanup)(void *);
                 void  (*description)(void);
         };

        ** tm_dummy can be used as a template for building out your 
           fuctionality,  Feel free to modify the  main() function as 
           you need for your testing.


      To add your own test code, modify main.c.   Add your "test_mode_struct" the 
         "setup_test_functions()" and "usage()" functions.  Use the next test number 
         available.



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



Download dpdk_test_app

   * cd to the directory where you want the "dpdk_test_app" directory created.
   * execute the folloing command to download the Project:

      > git clone https://github.com/fmcoastal/dpdk_test_app.git

To Build 
  -- Standalone Environment 
  
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

  -- Buildroot Environment 

   * go to dpdk in the SDK output
       > cd <SDK_DIR>/<xxx_release_output>/build/dpdk/examples

   * get the files 
       > git clone https://github.com/fmcoastal/dpdk_test_app.git

   * Modify the dpdk envionment Makeifile
       > vim Makefile
 
      ** at the bottom of the file add "DIRS-y += dpdk_test_app".
         should look as follows:

           DIRS-y += measure_routine_time
           DIRS-y += dpdk_test_app

           include $(RTE_SDK)/mk/rte.extsubdir.mk


   * buld the code from the <SDK_DIR>/<xxx_release_output> directory
      
      > cd ../../../
      > make dpdk-rebuild
   
   * executible name is "test_app"
     ** executible can  be found in:
 
<SDK_DIR>/<xxx_release_output>/build/dpdk/examples/dpdk_test_app/build/test_app

Other Items:
    dpdk tools like dpdk_devbind.py can be found in <DPDK_RELEASE>/usertools/dpdk-*




