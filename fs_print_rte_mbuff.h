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




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define FS_STRING       "EAT_AT_JOES"
#define FS_STRING_MAX   256
// A pair of functions which generate an 
//    character string of of your choice 
//    appended with an incrementing uint32_t indx
typedef struct fs_string_struct { 
      char   string_base[FS_STRING_MAX];  // string
      char   string[FS_STRING_MAX];       // string + instance
      int32_t  index;                       // which character in the string
      uint32_t instance;                    // index counter in the string
} fs_string_t;



int    fs_string_init(fs_string_t *p, char *my_string  );
uint8_t  fs_string_getch(fs_string_t *p);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
typedef struct fs_mbuf_struct {
    struct   rte_mempool * pool ;    // pool to allocate  mbufs from
    struct   rte_mbuf *m_base ;      // pointer to m_buff Base;
    struct   rte_mbuf *m ;           // pointer to current m_buff;
    uint8_t * my_fptr ;              // base address of where to write data in mbuf
    uint16_t  mbuf_data_len_sz; 
    uint16_t  mbuf_wr_cnt; 

} fs_mbuf_t;

int fs_mbuf_init( struct rte_mempool * pool, fs_mbuf_t * p );
int fs_mbuf_add_buf(fs_mbuf_t * p, uint8_t *buf, int64_t buf_sz );
int fs_mbuf_close( fs_mbuf_t * p );




#endif
