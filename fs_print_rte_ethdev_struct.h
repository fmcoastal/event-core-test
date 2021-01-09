#ifndef _fs_print_rte_ethdev_struct_h
#define _fs_print_rte_ethdev_struct_h

#include "fs_extras.h"





/*****************************************************************************
 *  eth dev print functions
 ****************************************************************************/

#ifdef PRINT_DATA_STRUCTURES

void print_rte_eth_conf  (int indent,const char* string,int id,struct rte_eth_conf *p);

void print_rte_eth_rxmode( int indent, struct rte_eth_rxmode *d);
void print_rte_eth_txmode( int indent, struct rte_eth_txmode *d);
void print_rte_eth_dcb_rx_conf(int indent, struct rte_eth_dcb_rx_conf  *d);
void print_rte_eth_rss_conf( int indent, struct rte_eth_rss_conf *d);
void print_rte_eth_vmdq_rx_conf(int indent, struct rte_eth_vmdq_rx_conf  *d);
void print_rte_eth_vmdq_dcb_conf (int indent, struct rte_eth_vmdq_dcb_conf *d);

#else

#define print_rte_eth_conf( w,x,y,z);

#define print_rte_eth_rxmode( x, y )
#define print_rte_eth_txmode( x, y )
#define print_rte_eth_dcb_rx_conf( x, y);
#define print_rte_eth_rss_conf( x,y );
#define print_rte_eth_vmdq_rx_conf( x,y);
#define print_rte_eth_vmdq_dcb_conf (x,y);


#endif

#endif
