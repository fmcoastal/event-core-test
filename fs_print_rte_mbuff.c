#include <stdio.h> 


#include <rte_ether.h> 
#include <rte_mbuf_core.h> 
#include <rte_ethdev.h> 

//#include "../../lib/librte_ethdev/rte_ethdev_core.h"
//#include "../../drivers/net/thunderx/base/nicvf_plat.h"

#include "fprintbuff.h"
#include "fs_extras.h"
#include "fs_print_rte_mbuff.h"

#define INDENT_SIZE 3
#define INDENT(x) \
    char s[64]= {0};\
    int i;\
    for ( i = 0 ; i < (INDENT_SIZE * x) ; i++) s[i] = ' ';



/** Information for a given RSS type. */
struct rss_type_info {
	const char *str; /**< Type name. */
	uint64_t rss_type; /**< Type value. */
};



#if 0
static const struct rss_type_info rss_type_table[] = {
        { "all", ETH_RSS_IP | ETH_RSS_TCP |
                        ETH_RSS_UDP | ETH_RSS_SCTP |
                        ETH_RSS_L2_PAYLOAD },
        { "none", 0 },
        { "ipv4", ETH_RSS_IPV4 },
        { "ipv4-frag", ETH_RSS_FRAG_IPV4 },
        { "ipv4-tcp", ETH_RSS_NONFRAG_IPV4_TCP },
        { "ipv4-udp", ETH_RSS_NONFRAG_IPV4_UDP },
        { "ipv4-sctp", ETH_RSS_NONFRAG_IPV4_SCTP },
        { "ipv4-other", ETH_RSS_NONFRAG_IPV4_OTHER },
        { "ipv6", ETH_RSS_IPV6 },
        { "ipv6-frag", ETH_RSS_FRAG_IPV6 },
        { "ipv6-tcp", ETH_RSS_NONFRAG_IPV6_TCP },
        { "ipv6-udp", ETH_RSS_NONFRAG_IPV6_UDP },
        { "ipv6-sctp", ETH_RSS_NONFRAG_IPV6_SCTP },
        { "ipv6-other", ETH_RSS_NONFRAG_IPV6_OTHER },
        { "l2-payload", ETH_RSS_L2_PAYLOAD },
        { "ipv6-ex", ETH_RSS_IPV6_EX },
        { "ipv6-tcp-ex", ETH_RSS_IPV6_TCP_EX },
        { "ipv6-udp-ex", ETH_RSS_IPV6_UDP_EX },
        { "port", ETH_RSS_PORT },
        { "vxlan", ETH_RSS_VXLAN },
        { "geneve", ETH_RSS_GENEVE },
        { "nvgre", ETH_RSS_NVGRE },
        { "ip", ETH_RSS_IP },
        { "udp", ETH_RSS_UDP },
        { "tcp", ETH_RSS_TCP },
        { "sctp", ETH_RSS_SCTP },
        { "tunnel", ETH_RSS_TUNNEL },
        { NULL, 0 },
};
#endif

/*
 *   COnverts an IP address into a printable string
 */
char * format_ip_addr(char * str,uint32_t ip)
{
   sprintf(str,"%d.%d.%d.%d",((ip >> 24)&0xff),((ip >> 16)&0xff),((ip >> 8)&0xff),((ip >> 0)&0xff));
   return str;
}
/*
 *   COnverts an LE_IP address into a printable string
 */
char * format_LE_ip_addr(char * str,uint32_t ip)
{
   sprintf(str,"%d.%d.%d.%d",((ip >> 0)&0xff),((ip >> 8)&0xff),((ip >> 16)&0xff),((ip >> 24)&0xff));
   return str;
}


char BAD_POINTER[]={"BAD POINTER"};
/*
 *   converts a ethernet address into a printable String
 */
char * format_mac_addr(char * str,struct rte_ether_addr *m )
{
   if ( str == NULL) return BAD_POINTER;
   sprintf(str,"%02x:%02x:%02x:%02x:%02x:%02x",m->addr_bytes[0]
                                              ,m->addr_bytes[1]
                                              ,m->addr_bytes[2]
                                              ,m->addr_bytes[3]
                                              ,m->addr_bytes[4]
                                              ,m->addr_bytes[5]);
   return str;
}






/*
 *  dumps the packet data in an rte_mbuff to the screenxi
 *  http://doc.dpdk.org/api/structrte__mbuf.html
 *
 *  and:  https://doc.dpdk.org/guides/prog_guide/mbuf_lib.html
 */
void print_rte_mbuf(int indent, struct rte_mbuf *m)
{
    INDENT(indent);

//    m->tx_offload=0xdeadbeef;

    printf("%s struct rte_mbuf:  %p { \n"  ,s,m);

    printf( "buf_addr is offset 0x%lx bytes intothe header \n",offsetof(struct rte_mbuf, buf_addr) );
    printf("%s   (void *)   buf_addr:       0x%p\n",s,m->buf_addr);
    printf( "refcnt is offset 0x%lx bytes intothe header \n",offsetof(struct rte_mbuf, refcnt) );
    printf("%s   (uint16_t) refcnt          0x%04x\n",s,m->refcnt);
    printf("%s   (uint16_t) nb_segs         0x%04x\n",s,m->nb_segs);
    printf("%s   (uint16_t) port            0x%04x\n",s,m->port);
    printf("%s   (uint64_t) ol_flags        0x%016lx\n",s,m->ol_flags);
    printf("%s   (uint32_t) pkt_len         0x%08x\n",s,m->pkt_len);
    printf("%s   (uint16_t) data_len        0x%04x\n",s,m->data_len);
    printf("%s   (uint16_t) vlan_tci        0x%04x\n",s,m->vlan_tci);
    printf("%s   (uint16_t) vlan_tci_outer  0x%04x\n",s,m->vlan_tci_outer);
    printf("%s   (uint16_t) buf_len         0x%04x\n",s,m->buf_len);
    printf("%s   (uint64_t) timestamp       0x%016lx\n",s,m->timestamp);
    printf("%s   (struct rte_mempool*) pool 0x%p\n",s,m->pool);
    printf("%s   (struct rte_mbuf*) next    0x%p\n",s,m->next);
    printf("%s   (uint16_t) priv_size       0x%08x\n",s,m->priv_size);
    printf("%s   (uint16_t) timesync        0x%08x\n",s,m->timesync);
    printf("%s   (uint32_t) seqn            0x%08x\n",s,m->seqn);
    printf("%s   (struct rte_mbuf_ext_shared_info *) shinfo 0x%p\n",s,m->shinfo);

    printf("%s   (uint64_t) dynfield1[0]           0x%016lx\n",s,m->dynfield1[0]);
    printf("%s   (uint64_t) dynfield1[1]           0x%016lx\n",s,m->dynfield1[1]);
    printf("%s   (uint32_t) packet_type            0x%08x\n",s,m->packet_type);


    printf("%s   (uint8_t:4) l2_type               0x%01x\n",s,m->l2_type);
    printf("%s   (uint8_t:4) l3_type               0x%01x\n",s,m->l3_type);
    printf("%s   (uint8_t:4) l4_type               0x%01x\n",s,m->l4_type);
    printf("%s   (uint8_t:4) tun_type              0x%01x\n",s,m->tun_type);
    printf("%s   (uint8_t) inner_esp_next_proto    0x%02x\n",s,m->inner_esp_next_proto);
    printf("%s   (uint8_t:4) inner_l2_type         0x%01x\n",s,m->inner_l2_type);
    printf("%s   (uint8_t:4) inner_l3_type         0x%01x\n",s,m->inner_l3_type);
    printf("%s   (uint8_t:4) inner_l4_type         0x%01x\n",s,m->inner_l4_type);

//    printf("%s   (uint32_t) rss        0x%08x\n",s,m->rss);
//    printf("%s   (uint32_t) lo         0x%08x\n",s,m->lo);
//    printf("%s   (uint32_t) hi         0x%08x\n",s,m->hi);

    printf("%s   (void *) userdata                 0x%p\n",s,m->userdata);
    printf("%s   (uint64_t) udata64                0x%016lx\n",s,m->udata64);
    printf("%s   (uint64_t) tx_offload             0x%016lx\n",s,m->tx_offload);
    printf("%s   (uint64_t:%d) l2_len                 0x%016x\n",s,RTE_MBUF_L2_LEN_BITS,m->l2_len);
   printf("%s   (uint64_t:%d) l3_len                 0x%016x\n",s,RTE_MBUF_L3_LEN_BITS,m->l3_len);
    printf("%s   (uint64_t:%d) l4_len                 0x%016x\n",s,RTE_MBUF_L4_LEN_BITS,m->l4_len);
    printf("%s   (uint64_t:%d) tso_segsz              0x%016x\n",s,RTE_MBUF_TSO_SEGSZ_BITS,m->tso_segsz);
    printf("%s   (uint64_t:%d) outer_l3_len           0x%016x\n",s,RTE_MBUF_OUTL3_LEN_BITS,m->outer_l3_len);
    printf("%s   (uint64_t:%d) outer_l2_len           0x%016x\n",s,RTE_MBUF_OUTL2_LEN_BITS,m->outer_l2_len);

    printf( "Txoffload is offset 0x%lx bytes intothe header \n",offsetof(struct rte_mbuf, tx_offload) );

}










/*
 *  dumps the packet data in an rte_mbuff to the screenxi
 *
 *  https://doc.dpdk.org/guides/prog_guide/mbuf_lib.html
 */
void print_rte_mbuf_pkt(int indent, struct rte_mbuf *m)
{
    struct rte_ether_hdr *eth_hdr;
    char  src_mac[32];
    char  dst_mac[32];
    uint16_t  eth_hdr_type;    // the 16 bits after the Mac Address
    void * l3;
    struct rte_ipv4_hdr *ipv4_hdr;
    int shortflag=1;    // when dumping unrecognized packet types, limits size to 0x200
    INDENT(indent);

//  the packet date
     eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);
     l3 = (uint8_t *)eth_hdr + sizeof(struct rte_ether_hdr);

     printf("%s L2/MAC Hdr):  \n",s);
     rte_ether_format_addr(dst_mac,32,&eth_hdr->d_addr);
     rte_ether_format_addr(src_mac,32,&eth_hdr->s_addr);
     eth_hdr_type = RTE_STATIC_BSWAP16(eth_hdr->ether_type);
     printf("%s dest:  %s\n",s,dst_mac);
     printf("%s src:   %s\n",s,src_mac);
     if(eth_hdr_type == RTE_ETHER_TYPE_VLAN )  // we have a packet with a vlan (0x8100)
     {
         printf("%s BUG- WE HAVE A VLAN NEED TO ADD CODE. \n",s);
         printf("%s ether_type:  0x%04x  86dd-IPV6  \n",s,eth_hdr_type);
     }
     else if (eth_hdr_type == RTE_ETHER_TYPE_ARP)
     {
          printf("%s ether_type: arp  0x%04x\n",s,eth_hdr_type);
          PrintBuff((uint8_t *)l3, 32,0,"buf");  // dump the next 32 bytes


     }
     else if (eth_hdr_type == RTE_ETHER_TYPE_IPV4)
     {
          char  src_ip[32];
          char  dst_ip[32];

          printf("%s ether_type: IPV4  0x%04x\n",s,eth_hdr_type);

          ipv4_hdr = (struct rte_ipv4_hdr *)l3;

          PrintBuff((uint8_t *) l3, 32,0,"buf");  // dump the next 32 bytes

          printf("%s L3/IP Hdr:  \n",s);
          format_ip_addr(src_ip,RTE_STATIC_BSWAP32(ipv4_hdr->src_addr));
          format_ip_addr(dst_ip,RTE_STATIC_BSWAP32(ipv4_hdr->dst_addr));
          printf("%s version ihl:     0x%02x \n",s,ipv4_hdr->version_ihl);
          printf("%s type of service: 0x%02x \n",s,ipv4_hdr->type_of_service);
      }
      else
      {
          if(m->next == NULL)
          {
              printf("%s unrecognized packet type: 0x%04x  size: %d \n",s,eth_hdr_type,rte_pktmbuf_data_len(m));
              if( (rte_pktmbuf_data_len(m) > 0x200 ) && (shortflag == 1 ))
                  PrintBuff((uint8_t *)eth_hdr,0x200,0,"buf");
              else
                  PrintBuff((uint8_t *)eth_hdr,rte_pktmbuf_data_len(m),0,"buf");
          }
          else
          {

              printf("%s unrecognized packet type: 0x%04x  1st data size: %d \n",s,eth_hdr_type,rte_pktmbuf_pkt_len(m));
              if( (rte_pktmbuf_data_len(m) > 0x200 ) && (shortflag == 1 ))
                  PrintBuff((uint8_t *)eth_hdr,0x200,0,"buf");
              else
                  PrintBuff((uint8_t *)eth_hdr,rte_pktmbuf_data_len(m),0,"buf");

          }
      }
}


