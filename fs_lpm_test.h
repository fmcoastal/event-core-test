#ifndef _fs_lpm_test
#define fs_lpm_test
/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

#include "main.h"

// my copy of  
struct ipv4_fs_lpm_route {
         uint32_t ip;
         uint8_t  depth;
         uint8_t  ResultOnMatch;
 };


// structure to create an array of lpm test imputs.
struct ipv4_fs_lpm_test {
     uint32_t ip;               /* IP address to pass to the rte_lpm_lookup() function */
     uint8_t  expected_result;  /* this should match the "ResultOMatch */
     uint8_t  pass_fail;        /* should the expected "lookup" result be pass or fail */ 
 };


extern struct test_mode_struct  tm_lpm;



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

#endif




