#!/bin/bash


RUN_CODE=0
if [ "$1" == "" ] ; then
   echo " test 0:  ??  "
   echo " test 1:  Spin Lock test "
   echo " test 2:  ???  "
   echo " test 3:  ???  "
   echo " test 4:  Longest prefix match test code "
   echo " test 5:  event dev,  message to next core   "
   echo " test 6:  event dev crypto  "
   echo " test 7:  event dev ethdev  "
   echo " test 8:  event dev timer  "
   echo " test 9:  test - customer specific  "
   exit
else
   RUN_CODE=$1
fi


########################################################################################################
# 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9  
########################################################################################################
if  [ "$RUN_CODE" == "9" ] ; then

#./build/core_test_app -c 0x800000 -w 0002:0e:00.0,xae_cnt=32768 -w 0002:10:00.1 -w 0002:05:00.0 –log-level=’pmd.octeontx2.*,debug’ -- -t 9 -v 4 -p 100000

#COMPILED=fs_test_test_parse
COMPILED=fs_test_rss
#COMPILED=fs_test_multi_evt_port

    if [ "$COMPILED" == "fs_test_test_parse" ] ; then
       #scappy input  ( ln -s  fs_test_test_parse.[ch] fs_test.[ch]    )
       ./build/core_test_app -c 0x800000 -w 0002:0e:00.0,xae_cnt=32768 -w 0002:10:00.1 -w 0002:07:00.0 –log-level=’eth.octeontx2.*,debug’ -- -t 9 -v 4 -p 1

    elif [ "$COMPILED" == "fs_test_multi_evt_port" ] ; then

       # 1 cpu polling 3 event-ports  ( ln -s  fs_test_multi_evt_port.[ch] fs_test.[ch]    )
       ./build/core_test_app -c 0x800000 -w 0002:0e:00.0,xae_cnt=32768,single_ws=1 -w 0002:10:00.1 -w 0002:04:00.0 -w 0002:05:00.0 -w 0002:06:00.0  -w 0002:07:00.0 –log-level=’eth.octeontx2.*,debug’  -- -t 9  -p 10000

    elif [ "$COMPILED" == "fs_test_rss" ] ; then

       # 1 cpu polling 3 event-ports  ( ln -s  fs_test_multi_evt_port.[ch] fs_test.[ch]    )
       ./build/core_test_app -c 0xf00000 -w 0002:0e:00.0,xae_cnt=32768  -w 0002:10:00.1 -w 0002:04:00.0  –log-level=’eth.octeontx2.*,debug’  -- -t 9  -p 10000
 

    else
       echo " unrecognized test application variant -  Please Fix !!"
       exit
 
    fi



########################################################################################################
# 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 8 
########################################################################################################
elif  [ "$RUN_CODE" == "8" ] ; then
./build/core_test_app -c 0xf00000 -w 0002:0e:00.0,xae_cnt=32768  -w 0002:10:00.1  -w 0002:04:00.0 -w 0002:05:00.0   –log-level=’pmd.crypto.octeontx2,8’  -- -t 8  -p 1000000

########################################################################################################
# 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7
########################################################################################################
elif  [ "$RUN_CODE" == "7" ] ; then


./build/core_test_app -c 0xf00000 -w 0002:0e:00.0,xae_cnt=32768  -w 0002:10:00.1  -w 0002:04:00.0 -w 0002:05:00.0   –log-level=’pmd.octeontx2.base,8’  -- -t 7 -v 0xffff -p 1000
# ./build/core_test_app -c 0xf00000 -w 0002:0e:00.0,xae_cnt=32768  -w 0002:10:00.1  -w 0002:04:00.0 -w 0002:05:00.0   –log-level=’pmd.crypto.octeontx2,8’  -- -t 7 -m  -p 1

########################################################################################################
#  6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6  6 6 6 6 6 
########################################################################################################
elif  [ "$RUN_CODE" == "6" ] ; then

# https://doc.dpdk.org/guides/cryptodevs/octeontx2.html
# lets try to run crypto code
#  REFERENCE CODE :  <DPDK>/app/test/test_event_crypto_adapter.c

./build/core_test_app -c 0x0f -w 0002:0e:00.0,xae_cnt=32768  -w 0002:10:00.1 –log-level=’pmd.crypto.octeontx2,8’  -- -t 6

#create vf instance of crypto driver
# https://doc.dpdk.org/guides/cryptodevs/octeontx2.html
# lspci -d:a0fd
# echo 1 > /sys/bus/pci/drivers/octeontx2-cpt/0002:10:00.0/sriov_numvfs


#./build/core_test_app -c 0x0f -w 0002:0e:00.0,xae_cnt=32768  --vdev "cryptodev_aesni_mb_pmd"  -- -t 6

# https://doc.dpdk.org/guides-16.04/sample_app_ug/l2_forward_crypto.html
# http://doc.dpdk.org/guides/prog_guide/cryptodev_lib.html#device-features-and-capabilities
# http://doc.dpdk.org/guides/prog_guide/event_crypto_adapter.html
# https://doc.dpdk.org/guides/cryptodevs/
# http://doc.dpdk.org/guides/prog_guide/rte_security.html 

########################################################################################################
#   5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
########################################################################################################
elif [ "$RUN_CODE" == "5" ] ; then
# code to pass messages from one core to the next

# ./build/core_test_app -c 0x0f --log-level otx2_logtype_sso:debug -- -t 5
# ./build/core_test_app -c 0x0f -- -t 5
   ./build/core_test_app -c 0xf00000 -w 0002:0e:00.0,xae_cnt=32768 -w 0002:04:00.0 -w 0002:05:00.0    -- -t 5
# ./build/core_test_app -c 0x0f  -w 0002:0e:00.0,xae_cnt=32768  --log-level otx2_logtype_sso:debug -- -t 5



elif [ "$RUN_CODE" == "0" ] ; then
       echo "cmd 0"
   ./build/core_test_app -c 0x0f -n 4  -- -t 0

elif [ "$RUN_CODE" == "1" ] ; then
       echo "Spin-lock test"
   ./build/core_test_app -c 0x0f -n 4  -- -t 1

elif  [ "$RUN_CODE" == "2" ] ; then
       echo "cmd 2"
   ./build/core_test_app -c 0x0f -n 4  -- -t 2

elif  [ "$RUN_CODE" == "3" ] ; then
       echo "cmd 3"
   ./build/core_test_app -c 0x0f -n 4  -- -t 3

elif  [ "$RUN_CODE" == "4" ] ; then
       echo "Longest Prefix match test code cmd 4"
   ./build/core_test_app -c 0x0f -n 4  -- -t 4




else
echo "unrecognized script to run: $1"
echo ""

echo " 5 - send a message between cores"
echo " 6 - crypto test"

fi




