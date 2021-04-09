#ifndef _fs_print_mbuff_h
#define _fs_print_mbuff_h



/*
 *   COnverts an IP address into a printable string
 */
char * format_ip_addr(char * str,uint32_t ip);
/*
 *   COnverts an LE_IP address into a printable string
 */
char * format_LE_ip_addr(char * str,uint32_t ip);

/*
 *   converts a ethernet address into a printable String
 */
char * format_rte_mac_addr(char * str,struct rte_ether_addr *m );





/*
#define INDENT_SIZE 3
#define INDENT(x) \
    char s[64]= {0};\
    int i;\
    for ( i = 0 ; i < (INDENT_SIZE * x) ; i++) s[i] = ' ';
*/


/*
 *  dumps the packet data in an rte_mbuff to the screenxi
 *  http://doc.dpdk.org/api/structrte__mbuf.html
 *
 *  and:  https://doc.dpdk.org/guides/prog_guide/mbuf_lib.html
 */
void print_rte_mbuf(int indent, struct rte_mbuf *m);









/*
 *  dumps the packet data in an rte_mbuff to the screenxi
 *
 *  https://doc.dpdk.org/guides/prog_guide/mbuf_lib.html
 */
void print_rte_mbuf_pkt(int indent, struct rte_mbuf *m);

#endif
