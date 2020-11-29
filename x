#!/bin/bash


RUN_CODE=0
if [ "$1" != "" ] ; then
 echo " input : $1 "
 RUN_CODE=$1
fi


if [ "$RUN_CODE" == "5" ] ; then
# code to pass messages from one core to the next

# ./build/core_test_app -c 0x0f --log-level otx2_logtype_sso:debug -- -t 5
# ./build/core_test_app -c 0x0f -- -t 5
   ./build/core_test_app -c 0x0f -w 0002:0e:00.0,xae_cnt=32768  -- -t 5

elif  [ "$RUN_CODE" == "6" ] ; then

# lets try to run crypto code
./build/core_test_app -c 0x0f -w 0002:0e:00.0,xae_cnt=32768  -- -t 6


else
echo "unrecognized script to run: $1"
echo ""

echo " 5 - send a message between cores"
echo " 6 - crypto test"

fi




