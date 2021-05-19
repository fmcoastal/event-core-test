# evemt-core-test

The event-core-test is built on the "test harness code".  The base code (see main.c) does 
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


      Special Files:
             x       - use this as a reference for starting the dpdk application
             f_fixes - use this to attach devices to vfio-pci
           
      Notes:
          The test are very hard wired with respect to cores, number on interfaces.
              Incorrect interface cli parameters will cause execution to fail



To Get Help Output on test_app:

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
      ./huge_mem_cfg
      sudo dpdk_cfg
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


Cross Compile Application:

export FS_TOOLCHAIN_REVISION=marvell-tools-265.0
export EXTRA_CFLAGS='-O0 -g3'
export CROSS=$FS_BASE_DIR/sdk/$FS_SDK_REVISION/toolchain/$FS_TOOLCHAIN_REVISION/bin/aarch64-marvell-linux-gnu-

export RTE_LINUX_KERNEL=$FS_BASE_DIR/sdk/$FS_SDK_REVISION/$FS_TARGET_DEVICE-release-output/build/linux-custom
export RTE_SDK=$FS_BASE_DIR/sdk/$FS_SDK_REVISION/$FS_TARGET_DEVICE-release-output/build/dpdk
export RTE_TARGET=build



Need to understand:
 cat /sys/bus/pci/devices/0002\:04\:00.0/limits/nix    as it relates to resources..


=================================================================
 DEBUG_FS_NOTES

 RSRC_ALLOC:

    root@cn96xx:/home/fsmith# cat /sys/kernel/debug/octeontx2/rsrc_alloc
    pcifunc     NPA         NIX0       SSO GROUP   SSOWS       TIM         CPT0
    PF0:VF0     8           8
    PF0:VF1     9           9
    PF0:VF2     10          10
    PF0:VF3     11          11
    PF0:VF4     12          12
    PF0:VF5     13          13
    PF0:VF6     14          14
    PF0:VF7     15          15
    PF0:VF8     16          16
    PF0:VF9     17          17
    PF0:VF10    18          18
    PF0:VF11    19          19
    PF0:VF12    20          20
    PF0:VF13    21          21
    PF0:VF14    22          22
    PF0:VF15    23          23
    PF1         0           0
    PF2         1           1
    PF5         4           4
    PF6         5           5
    PF7         6           6
    PF8         7           7
    PF12        24
    root@cn96xx:/home/fsmith/depot/event-core-test#
   



 RVU_PF_CGX_MAP:

    root@cn96xx:/home/fsmith# cat /sys/kernel/debug/octeontx2/rvu_pf_cgx_map
    PCI dev         RVU PF Func     NIX block       CGX     LMAC
    0002:02:00.0    0x400           NIX0            CGX0    LMAC0
    0002:03:00.0    0x800           NIX0            CGX1    LMAC0
    0002:04:00.0    0xc00           NIX0            CGX1    LMAC1
    0002:05:00.0    0x1000          NIX0            CGX1    LMAC2
    0002:06:00.0    0x1400          NIX0            CGX2    LMAC0
    0002:07:00.0    0x1800          NIX0            CGX2    LMAC1
    0002:08:00.0    0x1c00          NIX0            CGX2    LMAC2
    0002:09:00.0    0x2000          NIX0            CGX2    LMAC3
    root@cn96xx:/home/fsmith#  



 SSO:
   echo all > /sys/kernel/debug/octeontx2/sso/hwgrp/sso_hwgrp_pc 
       Output is to dmesg, not the console  so...

       dmesg -c
       echo  all >  /sys/kernel/debug/octeontx2/sso/hwgrp/sso_hwgrp_pc
       dmesg >sso_hwgrp_ient_walk.txt
       dmesg -c



    cat  /sys/kernel/debug/octeontx2/sso/hwgrp/sso_hwgrp_ient_walk
       Output is to dmesg, not the console  so...

       dmesg -c
       cat  /sys/kernel/debug/octeontx2/sso/hwgrp/sso_hwgrp_ient_walk
       dmesg >sso_hwgrp_ient_walk.txt
       dmesg -c 

    echo all >  /sys/kernel/debug/octeontx2/sso/hws/sso_hws_info
       Output is to dmesg, not the console so...

       dmesg -c
       echo all >  /sys/kernel/debug/octeontx2/sso/hwgrp/sso_hws_info
       dmesg >sso_hws_info.txt
       dmesg -c 

    cat  /sys/kernel/debug/octeontx2/sso/sso_pc
       Output is to dmesg, not the console  so...

       dmesg -c
       cat  /sys/kernel/debug/octeontx2/sso/sso_pc
       dmesg >sso_pc.txt
       dmesg -c

    echo all >  /sys/kernel/debug/octeontx2/sso/hws/sso_hws_info
       Output is to dmesg, not the console  so...

       dmesg -c
       echo all > /sys/kernel/debug/octeontx2/sso/hws/sso_hws_info
       dmesg >sso_hws_info.txt
       dmesg -c



 CGX:    ( packet count at interface )
    cat /sys/kernel/debug/octeontx2/cgx/cgx1/lmac1/stats

 NIX:    (???)
     cat /sys/kernel/debug/octeontx2/nix/rq_ctx
     cat /sys/kernel/debug/octeontx2/nix/sq_ctx
     cat /sys/kernel/debug/octeontx2/nix/cq_ctx
     cat /sys/kernel/debug/octeontx2/nix/ndc_rx_cache
     cat /sys/kernel/debug/octeontx2/nix/ndc_rx_hits_miss
     cat /sys/kernel/debug/octeontx2/nix/ndc_tx_cache
     cat /sys/kernel/debug/octeontx2/nix/ndc_tx_hits_miss

sso_info
