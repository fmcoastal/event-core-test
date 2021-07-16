#ifndef _fprintbuff_h
#define _fprintbuff_h


#ifndef LLU
#define LLU long long unsigned
#endif


#ifndef LLU
#define LLU long long unsigned
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int32_t
#define int32_t int
#endif

#ifndef int64_t
#define int64_t long int
#endif






/*//////////////////////////
  Print a character byte array
    color -  will change the text color something other than white
               see fs_extras.h for color definitions
               eg:
                   static const char C_GREEN[]   ={0x1b,'[','3','2','m',0x00};   
*/



void PrintBuff(uint8_t * buffer, int32_t bufferSize,uint8_t * Address,const char * title);
void PrintBuffc(uint8_t * buffer, int32_t bufferSize,uint8_t * Address,const char * title,const char * color);


/*//////////////////////////
   Diff buffer
    color -  will change the text color something other than white
               see fs_extras.h for color definitions
               eg:
                   static const char C_GREEN[]   ={0x1b,'[','3','2','m',0x00};   
*/
void DiffBuff(uint8_t * b1, uint8_t * b2, int32_t cmp_len,uint8_t * Address,const char * title,const char * color);




#endif

