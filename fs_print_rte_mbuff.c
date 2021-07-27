#include <stdio.h> 


#include <rte_ether.h> 
#include <rte_mbuf_core.h> 
#include <rte_ethdev.h> 

//#include "../../lib/librte_ethdev/rte_ethdev_core.h"
//#include "../../drivers/net/thunderx/base/nicvf_plat.h"

#include "fprintbuff.h"
#include "fs_extras.h"
#include "fs_print_rte_mbuff.h"

#if 0
#define INDENT_SIZE 3
#define INDENT(x) \
    char s[64]= {0};\
    int i;\
    for ( i = 0 ; i < (INDENT_SIZE * x) ; i++) s[i] = ' ';
#endif


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
char * format_rte_mac_addr(char * str,struct rte_ether_addr *m )
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
    uint8_t * p; 
    char string[128]; 
    int i; 
    int done;
    INDENT(indent);

//    m->tx_offload=0xdeadbeef;

    printf("%s struct rte_mbuf:  address:%p { \n"  ,s,m);

    printf("%s   (void *)   buf_addr:       0x%p       offset:0x%lx \n",s,m->buf_addr   ,offsetof(struct rte_mbuf, buf_addr));
    printf("%s   (rte_iova_t) buf_iova:     0x%016lx  offset:0x%lx \n",s,m->buf_iova   ,offsetof(struct rte_mbuf, buf_iova));
    printf("%s   (uint16_t) data_off        0x%04x              offset:0x%lx \n",s,m->data_off   ,offsetof(struct rte_mbuf, data_off));
    printf("%s   (uint16_t) refcnt          0x%04x              offset:0x%lx \n",s,m->refcnt     ,offsetof(struct rte_mbuf, refcnt));
    printf("%s   (uint16_t) nb_segs         0x%04x              offset:0x%lx \n",s,m->nb_segs    ,offsetof(struct rte_mbuf,nb_segs ));
    printf("%s   (uint16_t) port            0x%04x              offset:0x%lx \n",s,m->port       ,offsetof(struct rte_mbuf,port ));
    printf("%s   (uint64_t) ol_flags        0x%016lx  offset:0x%lx \n"         ,s,m->ol_flags   ,offsetof(struct rte_mbuf,ol_flags ));
    printf("%s   (uint32_t) pkt_len         0x%08x          offset:0x%lx \n"    ,s,m->pkt_len    ,offsetof(struct rte_mbuf,pkt_len ));
    printf("%s   (uint16_t) data_len        0x%04x              offset:0x%lx \n",s,m->data_len   ,offsetof(struct rte_mbuf,data_len ));
    printf("%s   (uint16_t) vlan_tci        0x%04x              offset:0x%lx \n",s,m->vlan_tci   ,offsetof(struct rte_mbuf,vlan_tci ));
    printf("%s   (uint16_t) vlan_tci_outer  0x%04x              offset:0x%lx \n",s,m->vlan_tci_outer   ,offsetof(struct rte_mbuf,vlan_tci_outer ));
    printf("%s   (uint16_t) buf_len         0x%04x              offset:0x%lx \n",s,m->buf_len    ,offsetof(struct rte_mbuf,buf_len ));
    printf("%s   (uint64_t) timestamp       0x%016lx  offset:0x%lx \n"         ,s,m->timestamp  ,offsetof(struct rte_mbuf,timestamp ));
    printf("%s   (struct rte_mempool*) pool 0x%p       offset:0x%lx \n"        ,s,m->pool       ,offsetof(struct rte_mbuf, pool));
    printf("%s   (struct rte_mbuf*) next    0x%p       offset:0x%lx \n"        ,s,m->next       ,offsetof(struct rte_mbuf, next));
    printf("%s   (uint16_t) priv_size       0x%08x          offset:0x%lx \n",s,m->priv_size  ,offsetof(struct rte_mbuf,priv_size ));
    printf("%s   (uint16_t) timesync        0x%08x          offset:0x%lx \n"    ,s,m->timesync   ,offsetof(struct rte_mbuf,timesync ));
    printf("%s   (uint32_t) seqn            0x%08x          offset:0x%lx \n"    ,s,m->seqn       ,offsetof(struct rte_mbuf,seqn ));
    printf("%s   (struct rte_mbuf_ext_shared_info *) shinfo 0x%p  offset:0x%lx \n",s,m->shinfo   ,offsetof(struct rte_mbuf,shinfo ));

    printf("%s   (uint64_t) dynfield1[0]           0x%016lx        offset:0x%lx \n",s,m->dynfield1[0], offsetof(struct rte_mbuf,dynfield1[0] ));
    printf("%s   (uint64_t) dynfield1[1]           0x%016lx        offset:0x%lx \n",s,m->dynfield1[1], offsetof(struct rte_mbuf,dynfield1[1] ));
    printf("%s   (uint64_t) dynfield1[8]           0x%016lx        offset:0x%lx \n",s,m->dynfield1[8], offsetof(struct rte_mbuf,dynfield1[8] ));
    printf("%s   (uint32_t) packet_type            0x%08x          offset:0x%lx \n",s,m->packet_type,  offsetof(struct rte_mbuf,packet_type ));


    printf("%s   (uint8_t:4) l2_type               0x%01x          offset:0x bit \n",s,m->l2_type   );
    printf("%s   (uint8_t:4) l3_type               0x%01x          offset:0x bit \n",s,m->l3_type    );
    printf("%s   (uint8_t:4) l4_type               0x%01x          offset:0x bit \n",s,m->l4_type    );
    printf("%s   (uint8_t:4) tun_type              0x%01x\n",s,m->tun_type);
    printf("%s   (uint8_t) inner_esp_next_proto    0x%02x\n",s,m->inner_esp_next_proto);
    printf("%s   (uint8_t:4) inner_l2_type         0x%01x\n",s,m->inner_l2_type);
    printf("%s   (uint8_t:4) inner_l3_type         0x%01x\n",s,m->inner_l3_type);
    printf("%s   (uint8_t:4) inner_l4_type         0x%01x\n",s,m->inner_l4_type);

//    printf("%s   (uint32_t) rss        0x%08x\n",s,m->rss);
//    printf("%s   (uint32_t) lo         0x%08x\n",s,m->lo);
//    printf("%s   (uint32_t) hi         0x%08x\n",s,m->hi);

    printf("%s   (void *) userdata                 0x%p             offset:0x%lx \n",s,m->userdata, offsetof(struct rte_mbuf,userdata ));
    printf("%s   (uint64_t) udata64                0x%016lx         offset:0x%lx \n",s,m->udata64 , offsetof(struct rte_mbuf,udata64  ));
    printf("%s   (uint64_t) tx_offload             0x%016lx         offset:0x%lx \n",s,m->tx_offload, offsetof(struct rte_mbuf,tx_offload));
    printf("%s   (uint64_t:%d) l2_len               0x%016x\n",s,RTE_MBUF_L2_LEN_BITS,m->l2_len);
    printf("%s   (uint64_t:%d) l3_len               0x%016x\n",s,RTE_MBUF_L3_LEN_BITS,m->l3_len);
    printf("%s   (uint64_t:%d) l4_len               0x%016x\n",s,RTE_MBUF_L4_LEN_BITS,m->l4_len);
    printf("%s   (uint64_t:%d) tso_segsz            0x%016x\n",s,RTE_MBUF_TSO_SEGSZ_BITS,m->tso_segsz);
    printf("%s   (uint64_t:%d) outer_l3_len         0x%016x\n",s,RTE_MBUF_OUTL3_LEN_BITS,m->outer_l3_len);
    printf("%s   (uint64_t:%d) outer_l2_len         0x%016x\n",s,RTE_MBUF_OUTL2_LEN_BITS,m->outer_l2_len);
//    printf("%s   (struct fdir )(uint32_t) hi       0x%08x         offset:0x%lx \n",s,m->fdir.hi,offsetof(struct rte_mbuf,fdir));
//    printf("%s   (struct rte_mbuf_sched ) sched                   offset:0x%lx \n",s,offsetof(struct rte_mbuf,rte_mbuf_sched));
//    printf("%s   (union ) tx_adapter                0x016x         offset:0x%lx \n",s,  offsetof(struct rte_mbuf,tx_adapter));
//    printf("%s   (uint16_t) txq         0x%04x              offset:0x%lx \n",s,m->nb_segs    ,offsetof(struct rte_mbuf,txq ));
    printf("%s   (union ) hash                      0x016x         offset:0x%lx \n",s,offsetof(struct rte_mbuf,hash));

    printf( "Txoffload is offset 0x%lx bytes intothe header \n",offsetof(struct rte_mbuf, tx_offload) );

 
    printf( "Payload address (rte_pktmbuf_mtod(m, struct rte_ether_hdr *): %p  \n",rte_pktmbuf_mtod(m, struct rte_ether_hdr *) ); 
    printf( " EVOL EVOL  \"m->buf_addr\" may not be the same address as \"m\"  \n        because of cache line alignement requirements  \n"); 
    i = 0;
    done = 0;
    while (done == 0)
    {

         sprintf(string," Data for mbuf segment %d",i);
         p = (((uint8_t*) m->buf_addr)  + m->data_off );
         PrintBuff(p,m->data_len,p,string);
         i++;
         if (m->next == NULL) done =1;
         m = m->next;
    } 

    printf("\n end of %s\n",__FUNCTION__);
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
     printf("%s-->pktmbuf:   %p payload address: %p  \n",s,m,eth_hdr); 
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

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
int  fs_string_init(fs_string_t *ptr,char * my_string)
{
     uint64_t i;
       
     if (ptr  == NULL)  return -1;
     ptr->index = 0;
     ptr->instance = 0xffffffff;
     memset(ptr->string,0,8);   // make sure first character looked at is a 0x00
     if( my_string == NULL)
     {
        strcpy((char *)ptr->string_base,FS_STRING);
        return 0 ;
     }
     else
     {
         for ( i = 0 ; ( *(my_string + i) != 0x00) && (i < (FS_STRING_MAX - sizeof(uint32_t) - 4)) ; i++) /* 0x, 0x00 & -1 */
         {
              ptr->string_base[i] = *(my_string + i);
         }
     }  
     return 0;
}



uint8_t  fs_string_getch( fs_string_t *ptr)
{
    // test if pointing to the end of the string 
    ptr->index++;
    if( *(ptr->string + ptr->index) == 0x00)
    {
        ptr->instance++;
        ptr->index = 0;
        sprintf( (char *)(ptr->string),"%s0x%08x",(char *)(ptr->string_base),ptr->instance);
    }
    return (*(ptr->string + ptr->index));

}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


int fs_mbuf_init( struct rte_mempool * pool, fs_mbuf_t * p )
{
     if ( pool == NULL)  return -1;
     if ( p    == NULL)  return -2;
     // save the pool we are to allocate mbuff-s from

     p->pool = pool;

     p->m_base =  rte_pktmbuf_alloc(p->pool);
     if (p->m_base == NULL )
        rte_exit(EXIT_FAILURE, "Not enough mbufs available A");

     p->m = p->m_base;   // copy the base to a working instance
     // get the starting point to write in the buffer
     p->my_fptr = (uint8_t*)rte_pktmbuf_mtod( p->m, struct rte_ether_hdr *);
     // set the size of the data we will write.
     p->mbuf_data_len_sz = p->m->buf_len - RTE_PKTMBUF_HEADROOM;
     // init the counting loop index
     p->mbuf_wr_cnt =  p->mbuf_data_len_sz;

     return 0;
     
}


int fs_mbuf_add_buf(fs_mbuf_t * p, uint8_t * buf, int64_t buf_sz )
{
   struct rte_mbuf * tmp_mbuf;

    while( buf_sz > 0 )
    {
        if(  p->mbuf_wr_cnt  <= 0)
        {
            // set the size of the data in the current mbuf
            p->m->data_len       =  p->mbuf_data_len_sz  ;        // set mbuf data_len
            p->m_base->pkt_len  +=  p->mbuf_data_len_sz  ;        // update pkt_total

            tmp_mbuf =  rte_pktmbuf_alloc(p->pool);
            if (tmp_mbuf == NULL )
                   rte_exit(EXIT_FAILURE, "Not enough mbufs available B");
 
            // increment the segs on the base mbuf.
            p->m_base->nb_segs ++ ;
            // sent the next pointer 
            p->m->next =  tmp_mbuf;

            p->m = tmp_mbuf;           
            p->m->next = NULL ;                         // set next mbuf to null
            p->my_fptr = (uint8_t*) rte_pktmbuf_mtod( p->m , struct rte_ether_hdr *); // Get the ptr
            p->mbuf_wr_cnt=  p->mbuf_data_len_sz;
        }

        *p->my_fptr++ = *buf++;
        p->mbuf_wr_cnt -- ;
        buf_sz -- ;
    }


return 0;
}

int fs_mbuf_close( fs_mbuf_t * p )
{
 int l;
    // fix data_len in the last mbuf
    l  = p->mbuf_data_len_sz - p->mbuf_wr_cnt ;
    p->m->data_len = l;
    p->m_base->pkt_len += l;

return 0;
}








