
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>     // for offsetof
#include <arpa/inet.h>  // for ntohl, htonl, ...

#include "fs_extras.h"
#include "fs_net_common.h"
//#include "arp.h"
//#include "sprintbuff.h"

int g_fnet_verbose = 0;
#define VERBOSE(x)  if(g_fnet_verbose >= x)

int16_t  InitalTimeToLive = 0x40;




// assume the string is at least 32 chars
static char g_ip_format_string[32];
char * formatIpAddr(uint32_t ip)
{
sprintf(g_ip_format_string,"%d.%d.%d.%d (0x%08x)",((ip>>24) & 0xff),
                                      ((ip>>16) & 0xff),
                                      ((ip>>8)&0xff),
                                      ((ip>>0)&0xff),ip);
return g_ip_format_string;
}




////////////////////////////////////////////////////
// this could be more bullit proof :-(
// string must be a.b.c.d
uint32_t StringToIp(char * string)
{
int i;
uint32_t ip = 0;
int index = 0;
char * ptr[4] = {0,0,0,0};;

  for( i = 0 ; i < 4 ; i++)
  {
    //sift off any leading spaces
    while(*(string+index) == ' ') index++ ;
    // Should be pointing to a digit, if so, save starting point.
    if( ( *(string+index) >= '0') && (*(string+index) <='9'))
    {
         ptr[i] = string+index;
    }
    else
    {
         return 0; // bad input, should be a digit here.
    }
    // move through digits.
    while ((*(string+index) >= '0') && (*(string+index) <= '9'))
    {
        index ++;
    }

    // we should be at a '.' or NULL.  terminate anyway
    *(string+index)= 0x00; // terminate the string
    index++;  // move to next Char
  }

  // we should now have pointers to four deciaml Numbers.
  for( i = 0 ; i < 4 ; i++)
  {
      ip = ip << 8;
      ip += (atoi(ptr[i]) & 0xff);
  }

  return ip;
}


void Print_ip_endpt(ip_endpt_t * var,char *var_name)
{
    printf("%s:\n",var_name);
    printf("  IP Address : %s\n",formatIpAddr(var->ip_addr));
    printf("  Port       : %5d     0x%08x \n",var->port ,var->port );
    printf("  HW MAC     : %s\n",formatMACAddr(&(var->MAC)));
}
#if 1

#ifdef fcrc16

/** Reverses bit order. MSB -> LSB and LSB -> MSB. */
uint32_t reverse(uint32_t x)
{
    uint32_t ret = 0;
    int32_t  i;
    for ( i=0; i<32; ++i)
    {
        if ((x & 0x1) != 0)
        {
            ret |= (0x80000000 >> i);
        }
        else {}
        x = (x >> 1);
    }
    return ret;
}

//   for starting new crc calc, make sure crc=0xffffffff
uint16_t crc16(uint8_t * message, uint32_t msgsize, uint16_t crc)
{
    uint32_t i, j; // byte counter, bit counter
    uint8_t byte;
    uint16_t poly = 0x1021;
    uint32_t ByteData;
    uint32_t CRCData;

    i = 0;
    for (i=0; i < msgsize; ++i)
    {
        byte = message[i];       // Get next byte.
        for (j=0; j<= 7; ++j)    // Do eight times.
        {
            // Take Bits MSB to LSBs
            ByteData = ( byte & (0x80 >> j)) == 0 ? 0 : 1 ;
            // Look at MSB of CRCs
            CRCData  = ( crc  & 0x8000     ) == 0 ? 0 : 1 ;
            if ((CRCData ^ ByteData) > 0)
                crc = (crc << 1) ^ poly;
            else
                crc = crc << 1;
        }
    }
    return crc;
}

//   for starting new crc calc, make sure crc=0xffffffff
uint32_t crc32(uint8_t * message, uint32_t msgsize, uint32_t crc)
{
    uint32_t i, j; // byte counter, bit counter
    uint32_t byte;
    uint32_t poly = 0x04C11DB7;
    uint32_t ByteData;
    uint32_t CRCData;

    i = 0;
    for (i=0; i<msgsize; ++i)
    {
        byte = message[i];       // Get next byte.
        byte = reverse(byte);    // 32-bit reversal.        ***********

        for (j=0; j<= 7; ++j)   // Do eight times.
        {
            // Take Bits MSB to LSBs  ***  Reverse made this 32 bits
            ByteData = ( byte & (0x80000000 >> j)) == 0 ? 0 : 1 ;
            // Look at MSB of CRCs
            CRCData  = ( crc  &  0x80000000      ) == 0 ? 0 : 1 ;
            if ((CRCData ^ ByteData) > 0)
                crc = (crc << 1) ^ poly;
            else
                crc = crc << 1;
        }
    }
    return reverse(~crc);
}

#endif



/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////MAC///////////////////////////////////////////


static char g_MAC_format[40];
// returns the ":" separated string value
char *  formatMACAddr(MacAddr_t* mac)
{
         sprintf(g_MAC_format ,"%02x:%02x:%02x:%02x:%02x:%02x",
                                     mac->addr[0],
                                     mac->addr[1],
                                     mac->addr[2],
                                     mac->addr[3],
                                     mac->addr[4],
                                     mac->addr[5]
                                      );
//    printf("i[%d]%s:%s  %s\n ",__LINE__,__FILE__,__FUNCTION__,g_MAC_format);
    return g_MAC_format;
}

// if the format is correct, will return 0,
//   if Error, conversion Results Unknown


static int64_t fatox( char * string)
{
    uint64_t result = 0;
    int64_t i = 0;
    int64_t done = 0;
    uint8_t in;
    int64_t length = strlen(string);

    while( (done == 0) && (i < length))
    {
        in = *(string+i);
        if( in == 0x00)
        {
            done = 1;
        }
        else
        {
            if (( in >= '0') &&( in <= '9'))
            {
                result <<= 4;
                result += in & 0x0f;
            }
            else if( (( in >= 'a') &&( in <= 'f')) ||
                     (( in >= 'A') &&( in <= 'F')) )
            {
                result <<= 4;
                result += ((in & 0x0f) + 9);
            }
            else if( ( in == 'x') || ( in == 'X'))
            {
            }
            else
            {
                done = 1; // unrecognized character
            }
            i++;
        }
    }
    return result;
}

static inline int IsHex(char c);
// returns a 0 if the character is a valid Hex value
static inline int IsHex(char c)
{
     if( ( ( c >= '0') &&  ( c <= '9' )) ||
         ( ( c >= 'a') &&  ( c <= 'f' )) ||
         ( ( c >= 'A') &&  ( c <= 'F' ))  )
     return 0;
     else
     return 1;
}

 // on success (0), update the *mac,
 // on failure (-1),  leaves *mac unchanged.
int StringToMac(char * string ,MacAddr_t * mac)
{
int i;
int j;
char tmp[3];
MacAddr_t x;

#ifdef DEBUG_SRTING_TO_MAC
printf("[%d]%s:%s  string %s\n",__LINE__,__FILE__,__FUNCTION__,string);
#endif

 for( i = 0 ; i < 6; i++)
 {
     tmp[1]= 0;
     tmp[2]= 0;
     j = 0;

#ifdef DEBUG_SRTING_TO_MAC
      printf("[%d]%s:%s  char %c\n",__LINE__,__FILE__,__FUNCTION__,*string);
#endif
     // first digit of the Octet
     if( IsHex(*string) == 0 )
         tmp[j++] = *string++;
     else
         goto MacErr;

#ifdef DEBUG_SRTING_TO_MAC
      printf("[%d]%s:%s  char %c\n",__LINE__,__FILE__,__FUNCTION__,*string);
#endif

     // Second digit of the Octet
     if( IsHex(*string) == 0 )
     {
         tmp[j++] = *string++;

         if( ( i > 5) && (*string != ':'))  goto MacErr;
         string ++;   // this will put us at the next octet
#ifdef DEBUG_SRTING_TO_MAC
         printf("[%d]%s:%s a) string %s\n",__LINE__,__FILE__,__FUNCTION__,tmp);
#endif
         x.addr[i] = fatox(tmp);
     }
     else if (*string == ':') //
     {
         string ++;   // this will put us at the next octet
#ifdef DEBUG_SRTING_TO_MAC
         printf("[%d]%s:%s b) string %s\n",__LINE__,__FILE__,__FUNCTION__,tmp);
#endif
         x.addr[i] = fatox(tmp);
     }
     else
        goto MacErr;
  }

  SetMAC( mac, &x);
#ifdef DEBUG_SRTING_TO_MAC
   printf("[%d]%s:%s MAC String \"%02x:%02x:%02x:%02x:%02x:%02x\"\n"
                               ,__LINE__,__FILE__,__FUNCTION__
                               ,mac->addr[0],mac->addr[1],mac->addr[2]
                               ,mac->addr[3],mac->addr[4],mac->addr[5]);
#endif
   return 0;

MacErr:
   printf("[%d]%s:%s  MacErr %d\n",__LINE__,__FILE__,__FUNCTION__,i);

   return -1;

}


void printMAC(MacAddr_t* mac,const char * string)
{
    printf(" %s  %s\n ",formatMACAddr(mac),string);
}

void SetMAC(MacAddr_t* Dst, MacAddr_t* Src)
{
int i = 0;
    for( i = 0 ; i < (int)(sizeof(MacAddr_t)) ; i++) Dst->addr[i] = Src->addr[i];
}

// returns 0 if the mac addresses are Identical
int  CompareMAC(MacAddr_t* mac0, MacAddr_t* mac1)
{
   int i;
   int flag = 0;
   for( i = 0 ; ((i < 6) && (flag == 0)) ; i++)
   {
      if( mac0->addr[i] != mac1->addr[i] ) flag = 1;
   }
   return flag;
}


void printMAC_Hdr_t(MAC_Hdr_t *hdr)
{
    uint16_t eth_type;
    printf("\n");
    printf("MAC Header:\n ");
    printMAC( &(hdr->DstMac)   ,"DesMacAddr");
    printMAC( &(hdr->SrcMac)   ,"SrcMacAddr");
    if(hdr->Q802_3 == 0)
    {
       printf(" %d        Q802.3 Number of VLANs \n ",hdr->Q802_3);
    }
    else
    {
       printf(" %d:%s - %s  VLAN DETECTED AND I DO NOT HANDLE\n ",__LINE__,__FILE__,__FUNCTION__);
    }
     eth_type=SWAP_16(hdr->EtherType);
     printf("0x%04x  EtherType:",eth_type);
     if     (eth_type == IPV4_PKT) printf(" IPV4\n ");            /* 0x0800 */
     else if(eth_type == IPV6_PKT) printf(" IPV6\n ");            /* 0x86dd */
     else if(eth_type == Q802_PKT) printf(" VLAN Tag Header\n "); /* 0x0801 */
     else if(eth_type == ARP_PKT ) printf(" ARP\n");              /* 0x0806 */
     else if(eth_type == SPN_TREE) printf(" Spanning Tree\n");    /* 0x0027 */
     else if(eth_type == LLDP_PKT) printf(" Link Layer Discovery Protocol\n");    /* 0x88cc */
     else                              printf(" Unrecognized MAC Frame\n");
//     printf("0x%08x  CRC32 -- not determined\n",hdr->CRC32);
     printf("\n");
}

// copies the MAC header data to the structure pointed to by MAC_Hdr_t
int GetMacData(uint8_t * pMACStart, MAC_Hdr_t *ExtractedData)
{
    unsigned int i = 0;
    char * ptr;
    uint16_t r;

    // First 12 bytes are destination and source MAC address
    // not clear if compiler will keep the MAC_Hdr_t data packed
    r = 12;
    ptr = (char *) &(ExtractedData->DstMac.addr[0]);
    for( i = 0 ; i < sizeof(MacAddr_t) ; i++)
    {
        *ptr++ =  *pMACStart++;
    }
    ptr = (char *) &(ExtractedData->SrcMac.addr[0]);
    for( i = 0 ; i < sizeof(MacAddr_t) ; i++)
    {
        *ptr++ =  *pMACStart++;
    }
    ExtractedData->Q802_3 = 0;
    // looking at interframe gap or Q802.3 depending on value
    // unsigned short must be 16 bits for this to work
    if( *((unsigned short*)pMACStart) == 0x0801)
    {
        r += 4;
        ExtractedData->Q802_3 += 1;
        pMACStart+=2;
        ExtractedData->vlan_0 = *((unsigned short *)pMACStart);
        pMACStart+=2;
    }
    if( *((unsigned short*)pMACStart) == 0x0801)
    {
        r += 4;
        ExtractedData->Q802_3 += 1;
        pMACStart+=2;
        ExtractedData->vlan_1 = *((unsigned short *)pMACStart);
        pMACStart+=2;
    }
    ExtractedData->EtherType = *((unsigned short *)pMACStart);
    ExtractedData->hdr_sz =  r;
    return 0;
}

// Returns the number of bytes to the "Next Packet Type"
//   does not count the 4bytes of crc at the end.
int GetMacHeaderSize(uint8_t * pMACStart)
{
    int r;

    // First 12 bytes are destination and source MAC address
    // not clear if compiler will keep the MAC_Hdr_t data packed
    r = 12;
    pMACStart += 12;
    // looking at interframe gap or Q802.3 depending on value
    // unsigned short must be 16 bits for this to work
    if( *((unsigned short*)pMACStart) == 0x0801)
    {
       r+=4;
       pMACStart += 4;
    }
    if( *((unsigned short*)pMACStart) == 0x0801)
    {
       r+=4;
       pMACStart += 4;
    }
    r += 2;  // InterFrame Gap
    return r;
}


/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  arp  //////////////////////////////////////////
//Arp Header
void printArpPktData_t(ArpPktData_t *p)
{   
    printf("\n");
    printf("ARP Header:   %p { \n",p );

    printf("%s    uint16_t   HwType           0x%04x  offset: 0x%02lx \n" ,"",ntohs(p->HwType)   ,offsetof(ArpPktData_t,HwType) );
    printf("%s    uint16_t   Protocol;        0x%04x  offset: 0x%02lx \n" ,"",htons(p->Protocol) ,offsetof(ArpPktData_t,Protocol) );
    printf("%s    uint16_t   HwAddLen;        0x%04x  offset: 0x%02lx \n" ,"",ntohs(p->HwAddLen) ,offsetof(ArpPktData_t,HwAddLen) );
    printf("%s    uint16_t   ProtocolAddLen;  0x%04x  offset: 0x%02lx \n" ,"",htons(p->ProtocolAddLen)  ,offsetof(ArpPktData_t,ProtocolAddLen));
    printf("%s    uint16_t   OpCode;          0x%04x  offset: 0x%02lx \n" ,"",htons(p->OpCode)   ,offsetof(ArpPktData_t,OpCode) );
    printf("%s    MacAddr_t  SrcHwAddr;        %s  \n"    ,"", formatMACAddr( &p->SrcHwAddr)  );
    printf("%s    uint32_t   SrcProtocolAddr   %s  \n"    ,"", formatIpAddr(ntohl(p->SrcProtocolAddr)));
    printf("%s    MacAddr_t  TarHwAddr         %s  \n"    ,"", formatMACAddr( &p->TarHwAddr    ) );
    printf("%s    uint32_t   TarProtocolAddr   %s  \n"    ,"", formatIpAddr(ntohl(p->TarProtocolAddr)));
    printf("} \n" );
}


/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////IPv4//////////////////////////////////////////
//IP Header

char g_FormattedIpAddr[23];

char * formatIPAddr(uint32_t IpAddr)
{
   sprintf(g_FormattedIpAddr, "%d.%d.%d.%d",((IpAddr >>24) & 0xff),
                                          ((IpAddr >>16) & 0xff),
                                          ((IpAddr >>8 ) & 0xff),
                                          ((IpAddr     ) & 0xff));

   return &(g_FormattedIpAddr[0]);
}


void printIPv4_Hdr_t(IPv4_Hdr_t *hdr)
{
    printf("\n");
    printf("IPv4 Header:\n");

    printf("  0x%02x        Version\n"     , hdr->Version);
    printf("  0x%02x        HeaderLength\n", hdr->HeaderLength);
    printf("  0x%02x        DiffSrv\n"     , hdr->DiffSrv);
    printf("  0x%02x        Congestion\n"  , hdr->Congestion);
    printf("  0x%04x      TotalLength (IPV4+Payload)\n"    , SWAP_16(hdr->TotalLength));
    printf("  0x%04x      Identification\n"                , SWAP_16(hdr->Identification));
    printf("  0x%02x        Flags\n"                       , hdr->Flags);
    printf("  0x%04x      FragOffset\n"                    , SWAP_16(hdr->FragOffset));
    printf("  0x%02x        TimeToLive\n"                  , hdr->TimeToLive);
    printf("  0x%02x        protocol (0x06 TCP 0x11 UDP 0x01 ICMP)\n" , hdr->Protocol);
    printf("  0x%04x      Checksum\n"                      , SWAP_16(hdr->CheckSum));
    printf("  0x%08x  SrcIPAddr Raw  \n"            , hdr->SrcIPAddr);
    printf("  0x%08x  SrcIPAddr  %s\n"            , SWAP_32(hdr->SrcIPAddr), formatIpAddr(SWAP_32(hdr->SrcIPAddr)) );
    printf("  0x%08x  DstIPAddr  %s\n"            , SWAP_32(hdr->DstIPAddr), formatIpAddr(SWAP_32(hdr->DstIPAddr)) ),
    printf("  0x%04x      Options\n"                      , hdr->Options);
}

// copies the MAC header data to the structure pointed to by MAC_Hdr_t
int GetIPv4Data(uint8_t * pIPv4Start, IPv4_Hdr_t *ExtractedData)
{
    unsigned short int i = 0;
    char c;

    c =  *pIPv4Start++;
    ExtractedData->Version      = (c>>4) & 0x0f;
    ExtractedData->HeaderLength =  c & 0x0f;
    c =  *pIPv4Start++;
    ExtractedData->DiffSrv      = (c>>2) & 0x3f;
    ExtractedData->Congestion   =  c & 0x03;

    ExtractedData->TotalLength  = *((unsigned short *)pIPv4Start);
    pIPv4Start+=2;
    ExtractedData->Identification  = *((unsigned short *)pIPv4Start);
    pIPv4Start+=2;

    i  = *((unsigned short *)pIPv4Start);
    pIPv4Start+=2;
    ExtractedData->Flags        = (i>>13) & 0x7; // Check this
    // should add no Fragment and Last Fragment
    ExtractedData->FragOffset   =  i & 0x1FFF;

    ExtractedData->TimeToLive  = *pIPv4Start;
    pIPv4Start+=1;
    ExtractedData->Protocol  = *pIPv4Start;
    pIPv4Start+=1;
    ExtractedData->CheckSum  = *((unsigned short *)pIPv4Start);
    pIPv4Start+=2;
    ExtractedData->SrcIPAddr  = *((unsigned int *)pIPv4Start);
    pIPv4Start+=4;
    ExtractedData->DstIPAddr  = *((unsigned int *)pIPv4Start);
    pIPv4Start+=4;

    ExtractedData->Options = 0;
    if(ExtractedData->HeaderLength != 5)
          ExtractedData->Options = *((unsigned short *)pIPv4Start);

    return 0;
}

// Returns the number of bytes to the "Next Packet Type"
int GetIPv4HeaderSize(uint8_t * pIPv4Start)
{
    return (((*pIPv4Start) & 0x0f) * 4);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////UDP///////////////////////////////////////////



void printUDP_Hdr(UDP_Hdr *hdr)
{
    printf("\n");
    printf("UDP Header:\n");

    printf(" 0x%04x      Source Port      %d  \n",SWAP_16(hdr->SrcPort), SWAP_16(hdr->SrcPort));
    printf(" 0x%04x      Destination Port %d \n",SWAP_16(hdr->DstPort), SWAP_16(hdr->DstPort));
    printf(" 0x%04x      Length (header + Data)\n",SWAP_16(hdr->Length));
    printf(" 0x%04x      Checksum (Can be 0)\n",SWAP_16(hdr->CheckSum));
    printf("\n");
}

// copies the MAC header data to the structure pointed to by MAC_Hdr_t
int GetUDPData(uint8_t * pUDPStart, UDP_Hdr *ExtractedData)
{
    uint16_t * ptr = (unsigned short *)pUDPStart;

    ExtractedData->SrcPort = *ptr++;
    ExtractedData->DstPort = *ptr++;
    ExtractedData->Length = *ptr++;
    ExtractedData->CheckSum = *ptr++;

    return 0;
}

// Returns the number of bytes to the "Next Packet Type"
int GetUDPHeaderSize(uint8_t * pUDPStart)
{
     VERBOSE(1) printf("%s : %p \n",__FUNCTION__,pUDPStart);
     return sizeof(UDP_Hdr);
}


////////////////////////////////////////////////////
// assumes the pointer has a UDP header at the beginning.
//  We clear the CheckSum
//  Calculate a new checksum
//  place the checksum in the header
//  Per https://tools.ietf.org/html/rfc768 the checksum can be zero.  If an actual checksum i    s wanted
//     call this function after the IP header is attached.
//     remember if the checksum calculation needs to be all ones if it calculates to zero,
//  And the Checksum is done on uint16 boundries.
//static uint16_t CalculateUDPCkSum(uint8_t * ip_packet);

// Builds a TCP packet,
//   Calculates Length and Checksum
//static uint64_t build_tcp_packet(TCP_Hdr *TcpHdr , uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, u    int16_t BufSz );

// Builds a UDP packet,
//   Calculates Length and Checksum
//static uint64_t build_udp_packet(UDP_Hdr *UdpHdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, ui    nt16_t BufSz );



/* This Calculates the IPv4 checksum. It does not put the results in header field,
it simply returns the value

IP HEADER CHECKSUM:: This means that if we divide the IP header is 16 bit words and
sum each of them up and then finally do a ones compliment of the sum then the value
generated out of this operation would be the checksum.

Now, the above is done at the source side which is sending the data packet.  At the
destination side which receives the data packet replaces the checksum value in the
header with all zeros and then calculates the checksum based on the same algorithm
as mentioned above. After a checksum value is obtained then this value is compared
with the value that came in the header. This comparison decides whether the IP
header is fine or corrupted. */

// static uint16_t CalculateIPv4CkSum(uint8_t * buf );

//   Calculates Length and Checksum
// static uint64_t build_IPv4_packet(IPv4_Hdr_t *IPv4Hdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf,     uint16_t BufSz );

// build_mac_packet
// static uint64_t build_MAC_packet(MAC_Hdr_t *MACHdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, ui    nt16_t BufSz );



/*
static  uint8_t  g_PacketPayload[]={'D' ,'E' ,'A' ,'D' ,'B' ,'E' ,'E' ,'F' ,   'F' ,'E' ,'E' ,'D' ,'C' ,'A' ,'F' ,'E' ,
 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,   0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
 'c' ,'a' ,'1' ,'1' ,'f' ,'0' ,'0' ,'d' ,   'b' ,'a' ,'b' ,'e' ,'-' ,'E' ,'0' ,'B' };

static uint64_t  g_PayloadSz = 16*3;
*/

//#define BYTE_SWAP(x) ( ((x << 8) & 0xff00) | (( x >> 8)  & 0x00ff))
#define BYTE_SWAP(x) x

/* This Calculates the IPv4 checksum. It does not put the results in header field, it simply returns the value
IP HEADER CHECKSUM:: This means that if we divide the IP header is 16 bit words and
sum each of them up and then finally do a one~Rs compliment of the sum then the value
generated out of this operation would be the checksum.

Now, the above is done at the source side which is sending the data packet.  At the
destination side which receives the data packet replaces the checksum value in the
header with all zeros and then calculates the checksum based on the same algorithm
as mentioned above. After a checksum value is obtained then this value is compared
with the value that came in the header. This comparison decides whether the IP
header is fine or corrupted. */

uint16_t CalculateIPv4CkSum(uint8_t * buf )
{
    uint16_t  OriginalCheckSum;
    int32_t   j ;
    uint32_t  CheckSum = 0;
    uint32_t  scratch = 0;
    uint16_t *pCkSum = 0;
    int32_t   length = 0;

    //  PrintBuff(buf, 20 ,buf);

    length   = (*buf & 0x0f) * 2;
    pCkSum = (uint16_t *)buf;
    OriginalCheckSum = *(uint16_t*)(buf + 0x0A)  ;
    *(uint16_t*)(buf + IPv4_CHECKSUM_OFFSET) = 0  ;
    // set to zero for the calculation
    // copy Version/Length
    for (j = 0 ; j < length ; j++)
    {
        scratch = (uint32_t) BYTE_SWAP(*pCkSum);
        CheckSum +=  scratch ;   // EVOL EVOL watch networ byte order here.
        pCkSum+=1;                            // stupid microsoft
    }
    CheckSum = ((CheckSum & 0xffff0000) >> 16) + (CheckSum & 0x0000ffff) ;
    CheckSum = ((CheckSum & 0xffff0000) >> 16) + (CheckSum & 0x0000ffff) ;
    CheckSum = ~ CheckSum;   // 1's complement of 1's complement sum

    *(uint16_t*)(buf + IPv4_CHECKSUM_OFFSET) = OriginalCheckSum  ;

    return CheckSum;
}


//   Calculates Length and Checksum
uint64_t build_IPv4_packet(IPv4_Hdr_t *IPv4Hdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, uint16_t BufSz )
{
    int     i,index;
    uint8_t  *ptr ;
    uint8_t  *DestBuf = pBuf;

    if(BufSz < (DataSz + (IPv4Hdr->HeaderLength * sizeof(uint32_t) )))
    {
        return -1;
    }
    index = 0;
    // copy Version/Length
    *pBuf++ = (uint8_t)( ((IPv4Hdr->Version << 4) & 0xf0) | (IPv4Hdr->HeaderLength & 0x0f));
    index++;
    // copy  DiffServ/Congestion
    *pBuf++ = (uint8_t)( ((IPv4Hdr->DiffSrv << 2) & 0xfc) | (IPv4Hdr->Congestion & 0x03));
    index++;
    // TotalLength
    ptr = pBuf;  // for now save the location
    *pBuf++ = 0;
    *pBuf++ = 0;
    index+=2;
    // ID
    *pBuf++ = (uint8_t)( IPv4Hdr->Identification >> 8   );
    *pBuf++ = (uint8_t)( IPv4Hdr->Identification & 0xff );
    index+=2;
    // Flags & FragOffset
    *pBuf++ = (uint8_t)( ((IPv4Hdr->Flags << 5) & 0xe0) | ((IPv4Hdr->FragOffset >> 8) & 0x1f)  ) ;
    *pBuf++ = (uint8_t) (IPv4Hdr->FragOffset & 0xff)   ;
    index+=2;
    // copy TimeToLive
    *pBuf++ = (uint8_t)IPv4Hdr->TimeToLive;
    index++;
    // copy Protocol
    *pBuf++ = (uint8_t)IPv4Hdr->Protocol;
    index++;
    // CheckSum
    *pBuf++ = (uint8_t)( IPv4Hdr->CheckSum >> 8  ) ;
    *pBuf++ = (uint8_t)( IPv4Hdr->CheckSum & 0xff) ;
    index+=2;
    memcpy(pBuf,(void *) &(IPv4Hdr->SrcIPAddr),sizeof(uint32_t));
    pBuf += sizeof(uint32_t);
    index+= sizeof(uint32_t);
    memcpy(pBuf,(void *) &(IPv4Hdr->DstIPAddr),sizeof(uint32_t));
    pBuf += sizeof(uint32_t);
    index+= sizeof(uint32_t);
    if( IPv4Hdr->Options != 0 )
    {
        printf("%sdo somthing for IPv4Hdr->options \n",__FILE__);
    }
    // copy the data
    for( i = 0 ; i < DataSz ; i++,index++)
    {
        *pBuf++ = *pData++;
    }
    IPv4Hdr->TotalLength =index;
    *ptr++ = (uint8_t) (index >> 8);
    *ptr++ = (uint8_t) (index & 0xff);

    // note checksum was filled in with 0 above.
    IPv4Hdr->CheckSum =  CalculateIPv4CkSum(DestBuf);
    ptr = DestBuf + IPv4_CHECKSUM_OFFSET ;
    *ptr++ =  (uint8_t) (IPv4Hdr->CheckSum >> 8);
    *ptr++ =  (uint8_t) (IPv4Hdr->CheckSum & 0xff);

    return 0;
}


//#define PKO_HW_NOT_DOING_CRC
// build_mac_packet
//    in MACHdr  ->  MAC header data you want to start packet with
//    in pData   ->  IP data packet
//    in DataSz  ->  IP data Packet Size
//    in pbuf    ->  location/buffer to where the packet is written
//    in BufSz   ->  in size of starting buffer out - final packet size
uint64_t build_MAC_packet(MAC_Hdr_t *MACHdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, uint16_t BufSz )
{
    uint64_t  j;
    uint8_t  *ptr ;
    MAC_Hdr_t  *macptr = (MAC_Hdr_t *)pBuf;

    if(BufSz < (DataSz + MAC_HEADER_SIZE + (MACHdr->Q802_3 * 4)))  // Size of Final Packet
    {
        return -1;
    }
//    PrintBuff(pData, DataSz, pData);
    // copy the MacAddresses
    memcpy(pBuf,&(MACHdr->DstMac.addr[0]),sizeof(MacAddr_t));
    pBuf += sizeof(MacAddr_t);
    memcpy(pBuf,&(MACHdr->SrcMac.addr[0]),sizeof(MacAddr_t));
    pBuf+= sizeof(MacAddr_t);
    if(MACHdr->Q802_3 != 0)
    {
        if(MACHdr->Q802_3 >= 1 )// add vlan 0
        {
            *pBuf++ = (uint8_t) (Q802_PKT >> 8);       // watch the endian swap??
            *pBuf++ = (uint8_t) (Q802_PKT & 0xff);
            *pBuf++ = (uint8_t) (MACHdr->vlan_0 >> 8);
            *pBuf++ = (uint8_t) (MACHdr->vlan_0 &0xff);
        }
        if(MACHdr->Q802_3 >= 2 )// add vlan 0
        {
            *pBuf++ = (uint8_t) (Q802_PKT >> 8);       // watch the endian swap??
            *pBuf++ = (uint8_t) (Q802_PKT & 0xff);
            *pBuf++ = (uint8_t) (MACHdr->vlan_1 >> 8);
            *pBuf++ = (uint8_t) (MACHdr->vlan_1 &0xff);
        }
    }
    // copy the EtherType
    ptr  = (uint8_t *) (&(MACHdr->EtherType));
    *pBuf++ =*(ptr+0);                           // watch the endian swap??
    *pBuf++ =*(ptr+1);

//    PrintBuff(macptr , (uint32_t)((uint64_t)pBuf - (uint64_t)macptr) , macptr);
    // move the packet data
    for( j = 0 ; j < DataSz ; j++)
    {
        //  printf(" d:%02x ",*pData); // peek at the data being written
        *pBuf++ = *pData++;
    }

//    PrintBuff(macptr , (uint32_t)((uint64_t)pBuf - (uint64_t)macptr) , macptr);
#ifdef PKO_HW_NOT_DOING_CRC
    //calculate the CRC32 and store as necessary
    macptr->CRC32 = crc32((uint8_t *) macptr, (uint32_t)((uint64_t)pBuf - (uint64_t)macptr) , NEW_CRC32_SEED );
    MACHdr->CRC32 = macptr->CRC32;

    // add the crc32 to the end of the stream
    ptr  = (uint8_t *) (&(macptr->CRC32));
    for( j = 0 ; j < sizeof(macptr->CRC32) ; j++)
    {
        *pBuf++ = *ptr++;
    }
#endif
    // update the total packet length as necessary
    MACHdr->TotPacketLenght = ((uint64_t)pBuf - (uint64_t)macptr) ;

    return 0;
}

// Builds a UDP packet,
//   Calculates Length and Checksum
//  i Pointer to the UDP header Data
//  i pData udp packet data
//  i DataSz : size of UDP payload Data
//  i/o pBuf : input buffer, and resulting UDP Header+Data,
//  i BufSz : input udp buffer size resulting size is returned int the packet data for UDP
uint64_t build_udp_packet(UDP_Hdr *UdpHdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, uint16_t BufSz )
{
    uint64_t i;
    int64_t j;
    uint8_t  *ptr  = (uint8_t *)UdpHdr;
    UDP_Hdr  *uptr = (UDP_Hdr *)pBuf;

    if(BufSz < (DataSz + sizeof(UDP_Hdr)))
    {
        return -1;
    }
    // copy the header
    for(i = 0 ; i < sizeof(UDP_Hdr) ; i++)  // this work because the header and the struct are the same
    {
        *pBuf++ = *ptr++;
    }
    // copy the data
    for( j=0 ; j < DataSz ; i++,j++)
    {
        *pBuf++ = *pData++;
    }

    UdpHdr->Length = DataSz + 8;  //   UPD header plus Payload
    uptr->Length = DataSz + 8;

    UdpHdr->CheckSum = 0 ;    // set to 0 as allowed by UDP speck
    uptr->CheckSum = 0 ;

    return 0;
}

#define G_BUFF_SIZE  2048
static uint8_t  g_TCPBuff[G_BUFF_SIZE];
static uint8_t  g_UDPBuff[G_BUFF_SIZE];
static uint8_t  g_IPBuff[G_BUFF_SIZE];
static uint8_t  g_MACBuff[G_BUFF_SIZE];


//#define PRINT_WRAPPERS

//////////////////////////////////////////////////////////
//  UDP PACKET/
//  i dst  -> destination MAC addres, IP address, and UDP Port number
//  i src  -> source MAC addres, IP address, and UDP Port number
//  i data -> pointer to udp data
//  i datasz -> size of UDP Data
//  i/o PktPtr -> address of buffer used to contain resulting UDP header + udp Data
//  i/o PktSz -> size of starting buffer, and resulting size of udp header+ upd Data
int  FBuildUdpPacket( ip_endpt_t *dst,ip_endpt_t *src ,uint8_t * data, int64_t datasz ,uint8_t **PktPtr,int64_t * PktSz )
{
    UDP_Hdr  UDPHdr  ;
    IPv4_Hdr_t IPv4Hdr ;
    MAC_Hdr_t  MACHdr  ;

    int i;
#if 0
    uint8_t  * bptr;
    uint16_t HashResult=0;
    uint32_t * SIPOffset=0;
    uint32_t * DIPOffset=0;
    uint16_t * SPOffset=0;
    uint16_t * DPOffset=0;
    uint8_t  * ProtoOffset=0;
    uint32_t MacSize=0;
#endif
    // check if buffer is larger than 1 buffer because send raw will need to be fixed.
    if(datasz > 1500)
    {
        printf("%d:%s %s  -- UDP backet data too large\n",__LINE__,__FILE__,__FUNCTION__);
        return -2;
    }
    // clear intermediate
    for( i = 0 ; i < G_BUFF_SIZE ; i++)
    {
        g_TCPBuff[i] = 0xaa;
        g_UDPBuff[i] = 0xaa;
        g_MACBuff[i] = 0xaa;
        g_IPBuff[i] = 0xaa;
    }

/**** UDP ***/
        UDPHdr.SrcPort = src->port;
        UDPHdr.DstPort = dst->port;
// uint64_t build_udp_packet(UDP_Hdr *UdpHdr ,uint8_t * pData, uint16_t DataSz,uint8_t *pBuf, uint16_t BufSz )
        build_udp_packet(&UDPHdr , data , datasz ,g_UDPBuff, G_BUFF_SIZE);
//        VERBOSE(3) PrintBuff(g_UDPBuff, UDPHdr.Length ,NULL,"UDP Packet");
//        PrintBuff(g_UDPBuff, UDPHdr.Length ,NULL,"UDP Packet");

 /**** IPV4 ***/
        IPv4Hdr.Version        = 0x04;
        IPv4Hdr.HeaderLength   = 0x05;
        IPv4Hdr.DiffSrv        = 0x00;
        IPv4Hdr.Congestion     = 0x00;
        IPv4Hdr.TotalLength    = 20+sizeof(UDP_Hdr)+datasz;
        IPv4Hdr.Identification = 0 ;
        IPv4Hdr.Flags          = 0x2;
        IPv4Hdr.FragOffset     = 0;
        IPv4Hdr.TimeToLive     = InitalTimeToLive;
        IPv4Hdr.Protocol       = UDP_PACKET;
        IPv4Hdr.CheckSum       = 0;
        IPv4Hdr.SrcIPAddr      = src->ip_addr ;
        IPv4Hdr.DstIPAddr      = dst->ip_addr ;
        IPv4Hdr.Options        = 0 ;  // Optional field set to 0 so we are well behaved

        // Call the add function here then start he checksum work
        build_IPv4_packet(&IPv4Hdr ,g_UDPBuff, UDPHdr.Length,g_IPBuff, G_BUFF_SIZE);
//        VERBOSE(3) PrintBuff(g_IPBuff, IPv4Hdr.TotalLength ,NULL,"IPv4 Packet");
//        PrintBuff(g_IPBuff, IPv4Hdr.TotalLength ,NULL,"IPv4 Packet");

/**** IPV4 ***/
        memcpy(&(MACHdr.DstMac.addr[0]),&(dst->MAC.addr[0]),sizeof(MacAddr_t));
        memcpy(&(MACHdr.SrcMac.addr[0]),&(src->MAC.addr[0]),sizeof(MacAddr_t));
        MACHdr.Q802_3 = 0 ;       // vlans  0,1,or 2
        MACHdr.vlan_0 = 0x2065 ;  // Vlan 0
        MACHdr.vlan_1 = 0x0000 ;  // Vlan 0
        MACHdr.EtherType = IPV4_PKT ;

        // Call the add function here then start he checksum work
        build_MAC_packet(&MACHdr ,g_IPBuff, IPv4Hdr.TotalLength, *PktPtr, *PktSz);

//        VERBOSE(3)PrintBuff(*PktPtr, MACHdr.TotPacketLenght, NULL, "MAC Packet");
//        PrintBuff(*PktPtr, MACHdr.TotPacketLenght, NULL, "MAC Packet");

      *PktSz =  MACHdr.TotPacketLenght;
      return 0;

}


#endif

//#define DBG_GPD

/////////////////////////////////////////////////////////////////////////
//   Get Packet Data
//
// FIX ME>> THIS DOES NOT CHECK IF YOU OVERRUN THE BUF BUFFER
// It Also assumes data fits in 1 buffer
// it does not to TCP Protocol
// o DstMac  - if not null, returns packet's Destination MAC
// o SrcMac  - if not null, returns packet's Source MAC
// o DstIP   - if not null, returns packet's Destination IPV4 IP Address
// o SrcIP   - if not null, returns packet's Source IPV4 IP Address
// o DstPort - if not null, & TCP or UDP,  returns packet's Dst TCP/UDP Port
// o SrcPort - if not null, & TCP or UDP,  returns packet's Src TCP/UDP Port
// o L3 Type - if not null,  returns packet's L3 Data Type (IPV4 or ICMP)
// o L4 Type - if not null,  returns packet's L4 Data Type (UPD,TCP,ARP)
// o pktbuf  - if not null,  returns pointer to start of L2 Packet
// o pktsz   - if not null,  returns size of packet - final CRC4 Chk sum
// o payloadbuf- if not null,  returns pointer to start of UDP/ICMP/ARP/... Packet
//                             if TCP, should return pointer to TCP Hdr for
//                             further processing
// o payloadsz - if not null,  returns size of Payload Data
//
int ParsePacketData( uint8_t *buf, int64_t bufsz,
                     MacAddr_t *DstMAC, MacAddr_t *SrcMAC,
                     uint32_t *DstIpAddr  ,uint32_t *SrcIpAddr,
                     uint16_t *DstPort    ,uint16_t *SrcPort,
                     uint16_t *L3Type     ,uint16_t *L4Type,
                     uint8_t* *pktbuf     ,int64_t *pktsz,
                     uint8_t* *payloadbuf ,int64_t *payloadsz,
                     uint64_t *FPABuff )
{
//    cvmx_buf_ptr_t  buffer_ptr;
#ifdef DBG_GPD
//    uint64_t        start_of_buffer;
#endif
    uint8_t *       pMac;

    unsigned char * pPayload;
    int64_t         PayloadSz;

    MAC_Hdr_t         lMAChdr;
    unsigned char * pIPv4;
    IPv4_Hdr_t         lIPv4hdr;
    unsigned char * pUDP;
    UDP_Hdr         lUDPhdr;

    WAI();
    if(L3Type    != NULL) *L3Type    = 0;
    if(L4Type    != NULL) *L4Type    = 0;
    if(pktsz     != NULL) *pktsz     = 0;
    if(payloadsz != NULL) *payloadsz = 0;
    if(FPABuff   != NULL) *FPABuff   = 0; // on OII, fpa buff is always 0

     WAI();
//// L2 - MAC  Data

    // fish out the MAC info
    pMac = (uint8_t *)buf;
    if(pktbuf !=  NULL)  *pktbuf = pMac;             // the start of the Packet
    GetMacData(((uint8_t *) pMac),&lMAChdr);
#ifdef DBG_GPD
// fs check this
    printf("Size of MAC Hdr  0x%02x    %d\n", GetMacHeaderSize((uint8_t *) pMac), GetMacHeaderSize((uint8_t *) pMac));
    printMAC_Hdr_t(&lMAChdr);
    printf("\n");
#endif
    if( DstMAC != NULL)
    {
       memcpy(DstMAC,&(lMAChdr.DstMac.addr[0]),sizeof(MacAddr_t));
    }
    if( SrcMAC != NULL)
    {
       memcpy(SrcMAC,&(lMAChdr.SrcMac.addr[0]),sizeof(MacAddr_t));
    }
    // do not necessarily need bufsz unless you need want to check as you go throught the parse project
     if(bufsz == 0) return -1;

    // figure out the size of the entire packet
    //if(pktsz   != NULL) *pktsz =  GetMacHeaderSize((uint8_t *) pMac) ;   // the size of the packet
    if(pktsz   != NULL) *pktsz = lMAChdr.hdr_sz ;   // the size of the packet
    // make the assumption here we are an ARP or IPV4 packet
    //  return if ARP
    //  error if not IPV4

    if(SWAP_16(lMAChdr.EtherType) == ARP_PKT)
    {
         if(L3Type != NULL) *L3Type = ARP_PKT;
         // Packet size
         // need to DEBUG THIS  !!!.
         if(pktsz   != NULL) *pktsz += sizeof(ArpPktData_t);   // need to add chksum??? the size of the packet
         // Payload... ARP Packet location and size
         if(payloadbuf != NULL) *payloadbuf = pMac + GetMacHeaderSize((uint8_t *) pMac) ;
         if(payloadsz != NULL) *payloadsz  = sizeof(ArpPktData_t) ;

         return(0);
    }
    else if(SWAP_16(lMAChdr.EtherType) == IPV4_PKT )
    {
         if(L3Type != NULL) *L3Type = IPV4_PKT ;
   // FUNKY CODE.  THE CODE AFTER ELSE WILL HANDLE THE DEFAULT ????

    }

    else
    {
         if(L3Type != NULL) *L3Type = SWAP_16(lMAChdr.EtherType) ;
         return -3;
    }

//// IPV4 Data  becasue we returned above it it was not.
    WAI();
    // fish out the IP info
    pIPv4 =  pMac + GetMacHeaderSize((uint8_t *) pMac) ;
    GetIPv4Data(((uint8_t *) pIPv4),&lIPv4hdr);
#ifdef DBG_GPD
    printf("Size of IPv4 Hdr 0x%02x    %d\n", GetIPv4HeaderSize((uint8_t *) pIPv4), GetIPv4HeaderSize((uint8_t *) pIPv4));
    printIPv4_Hdr_t(&lIPv4hdr);
    printf("\n");
#endif
    // add the IP header size field because it inclueds the IP header and Payload
    if( pktsz   != NULL) *pktsz +=  (lIPv4hdr.TotalLength ) ;
    if( DstIpAddr != 0) *DstIpAddr = SWAP_32(lIPv4hdr.DstIPAddr);
    if( SrcIpAddr != 0) *SrcIpAddr = SWAP_32(lIPv4hdr.SrcIPAddr);
    if( L4Type != NULL) *L4Type = lIPv4hdr.Protocol ;

    // if it is not UDP return.
    if(lIPv4hdr.Protocol == UDP_PACKET)
    {
//// UDP Packet Data
        WAI();
    // fishout the UDP Info
        pUDP  =  pIPv4 + GetIPv4HeaderSize((uint8_t *) pIPv4) ;
        GetUDPData(((uint8_t *) pUDP), &lUDPhdr);
#ifdef DBG_GPD
        printf("Size of UDP Hdr  0x%02x     %d\n", GetUDPHeaderSize((uint8_t *) pUDP), GetUDPHeaderSize((uint8_t *) pUDP));
        printUDP_Hdr(&lUDPhdr);
        printf("\n");
#endif

        if(SrcPort != NULL) *SrcPort = SWAP_16(lUDPhdr.SrcPort);
        if(DstPort != NULL) *DstPort = SWAP_16(lUDPhdr.DstPort);

    // fish out the Payload
        pPayload  =  pUDP  +  GetUDPHeaderSize((uint8_t *) pUDP) ;
        PayloadSz =  (SWAP_16(lUDPhdr.Length)) - GetUDPHeaderSize((uint8_t *) pUDP);

        WAI();
// fs 5/23/2015
//  need to figure out what I was doing here.
//    buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);

// in the future, allocate a buffer for the calling function to free!
//   also, we do not work past 1 block  of PFA pool
       if( payloadbuf != NULL) *payloadbuf = pPayload;
       if( payloadsz != NULL)  *payloadsz  = PayloadSz;
#ifdef DBG_GPD
       printf("ppayload          : 0x%p  \n"     ,pPayload );
       printf("payload size      : %d   0x%02x", (int)PayloadSz ,(int)PayloadSz );
       PrintBuff( (uint8_t *)pPayload,(int64_t)PayloadSz,(uint8_t*)pPayload ,"address (Buffer Data)");
#endif
// fs end
       return 0;
    }
    // Are we an IPV4 / ICMP Packet?
    else if ( lIPv4hdr.Protocol == ICMP_PACKET  )
    {
//      ICMP Packet Data
        WAI();

        // fish out the Payload
        pPayload  =  pIPv4 + GetIPv4HeaderSize((uint8_t *) pIPv4) ;
        PayloadSz =  SWAP_16(lIPv4hdr.TotalLength)  -  GetIPv4HeaderSize((uint8_t *) pIPv4) ;

// in the future, allocate a buffer for the calling function to free!
//   also, we do not work past 1 block  of PFA pool
        if( payloadbuf != NULL)   *payloadbuf = pPayload;
        if( payloadsz != NULL)    *payloadsz  = PayloadSz;
#ifdef DBG_GPD
        printf("ppayload          : 0x%p  \n"     ,pPayload );
        printf("payload size      : %d   0x%02x", (int)PayloadSz ,(int)PayloadSz );
        PrintBuff( (uint8_t *)pPayload,(int64_t)PayloadSz,(uint8_t*)pPayload ,"ICMP address (Buffer Data)");
#endif
// fs end
        return 0;
    }
    else if ( lIPv4hdr.Protocol == TCP_PACKET  )
    {
      return 0;
    }
    //
    // other IPV4 packet types go here.
    //
    return -1;  // for now, we did not recognize packet type.
}


#ifdef USE_THIS_PRINTBUFF
#define LLU  long long unsigned
#define fsprint printf

void PrintBuff(uint8_t * buffer, int64_t bufferSize,uint8_t * Address,const char * title)
{
    uint8_t * tmpptr0  = buffer;
    uint8_t * tmpptr1  = tmpptr0;
    int64_t  i          = 0 ;
    int64_t  m          = 0 ;
    int64_t  n          = 0 ;
    int64_t  j          = 0 ;
    int64_t  PrintCount = 0 ;   // used as counter to denote when to print \nadderss
    int64_t  BlankCnt   = 0 ;

    if( title != NULL)   fsprint("\n%s\n",title);
#if 1

    fsprint("  buf:%p  sz:0x%016llx  add:%p\n",buffer,(LLU)bufferSize,Address);

#endif
    if( buffer == NULL)
    {
        printf("Cannot access address %p\n",buffer);
        return;
    }


    // align the lead
    BlankCnt = (unsigned long)Address & 0x000000000f;

    // print the lead
    if( BlankCnt != 0)  // if 0 jump to main body
    {
        for ( PrintCount = 0 ; PrintCount < BlankCnt ; PrintCount++ )
        {
            if( PrintCount == 0) // space between fields
            {
                fsprint("\n%016x",(unsigned)((unsigned long)Address & ~0x000000000f));
                tmpptr1 = tmpptr0;
            }
            if( (PrintCount % 8) == 0)
            {
                fsprint(" ");
            }
            fsprint("   ");
        }
        PrintCount--;  // remove last increment of printcount
        // print PrintCount data
        for ( m = 0  ; (PrintCount < 0xf) && (i < bufferSize); i++, m++,PrintCount++)
        {
            if(PrintCount % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" %02x",(unsigned char)(*tmpptr0++));
            Address++;
        }

        // special case here when count is less than one line and not starting at zero
        if ( i == bufferSize)
        {
            // print out the last space
            for (      ; (PrintCount < 0x0f) ; PrintCount++ )
            {
                if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint("   ");
            }
            // print PrintCount text space
            for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
            {
                if( PrintCount == 0)   // space between fields
                {
                    fsprint(" ");
                }
                else if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint(" ");
            }
            // print PrintCount characters
            for( n = 0 ; (n < m) ; n++)
            {
                if( n == 0 ) printf(" ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
            printf("\n");
            return;
        } // end i == bufferSize

        // print PrintCount text space
        for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
        {
            if( PrintCount == 0)   // space between fields
            {
                fsprint(" ");
            }
            else if( PrintCount  % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" ");
        }
        // print PrintCount characters
        n = 0;
        for( n = 0 ; (PrintCount <= 0xf) && (n < m) ; n++,PrintCount++)
        {
            if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                fsprint("%c",*tmpptr1);
            else
                fsprint(".");
            tmpptr1++;
        }
    }

    // print the body
    PrintCount = 0;
    for (   ; i < bufferSize ; i++)
    {
        if( PrintCount == 0 )
        {
            fsprint("\n%016llx",((LLU)Address & ~0x0f));
            tmpptr1 = tmpptr0;
        }
        if(PrintCount % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint(" %02x",(unsigned char)(*tmpptr0++));
        Address++;
        PrintCount ++;
        if( PrintCount  > 0x0f)
        {
            PrintCount = 0;
            for( j = 0 ; j < 16 ; j++)
            {
                if( j == 0 ) printf("  ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
        }
    }

    // print out the last space
    m = PrintCount;
    for (      ; (PrintCount <= 0x0f) ; PrintCount++ )
    {
        if( PrintCount  % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint("   ");
    }

    // print PrintCount characters
    for( n = 0 ; (n < m) ; n++)
    {
        if( n == 0 ) printf("  ");
        if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
            fsprint("%c",*tmpptr1);
        else
            fsprint(".");
        tmpptr1++;
    }
    fsprint("\n");
}
#endif



