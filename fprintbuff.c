
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fprintbuff.h"






#define fsprint printf




void PrintBuff(uint8_t * buffer, int32_t bufferSize,uint8_t * Address,const char * title)
{
      PrintBuffc(buffer, bufferSize, Address,title,NULL);
}


void PrintBuffc(uint8_t * buffer, int32_t bufferSize,uint8_t * Address,const char * title,const char * color)
{
    uint8_t * tmpptr0  = buffer; 
    uint8_t * tmpptr1  = tmpptr0; 
    int64_t  i          = 0 ;
    int64_t  m          = 0 ;
    int64_t  n          = 0 ;
    int64_t  j          = 0 ;
    int64_t  PrintCount = 0 ;   // used as counter to denote when to print \nadderss
    int64_t  BlankCnt   = 0 ;
    const char C_NORMAL[]  ={0x1b,'[','0','m',0x00};     /* NORMAL =$'\e[0m'  */
   
    if(color != NULL)    fsprint("%s",color);
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

    if(color != NULL)    fsprint("%s", C_NORMAL);
    fsprint("\n");
}



#define FS_ALIGNED 0
#define FS_UNNALIGNED_START 1

void DiffBuff(uint8_t * b1, uint8_t * b2, int32_t cmp_len,uint8_t * Address,const char * title,const char * color)
{
   
    int64_t  i                 = 0 ;
    int64_t  cnt_not_aligned   = 0 ;
    int64_t  cnt_aligned       = 0 ;
    int64_t  state             = FS_ALIGNED ;
    uint8_t  *u_ptr; 
    uint64_t  u_ptr_index; 
    const char C_NORMAL[]  ={0x1b,'[','0','m',0x00};     /* NORMAL =$'\e[0m'  */

    if(color != NULL)    fsprint("%s",color);
    if( title != NULL)   fsprint("\n%s\n",title);
#if 1

    fsprint("  b1:%p   b2:%p   cmp_len:0x%016llx  add:%p\n",b1,b2,(LLU)cmp_len,Address);

#endif
    if( (b1 == NULL)  || (b2 == NULL))
    {
        fsprint("One of the arguments passed is NULL.  \n");
        fsprint("  b1:%p   b2:%p   cmp_len:0x%016llx  add:%p\n",b1,b2,(LLU)cmp_len,Address);
        return;
    }
    
   for ( i = 0 ; i < cmp_len ; i++)
   {
      switch (state) {
      case FS_ALIGNED:
          if( *(b1+i)  != *(b2+i))
          {
              cnt_not_aligned = 1; // reset the un-aligned count
              u_ptr = (b1+i);
              u_ptr_index = i;
              state = FS_UNNALIGNED_START;
          }
          else
          {
              cnt_aligned++;
          }  
          break;
      case FS_UNNALIGNED_START:
          if( *(b1+i)  == *(b2+i))
          {
              cnt_aligned = 1; // reset the un-aligned count
              state = FS_ALIGNED;
              PrintBuff( u_ptr, cnt_not_aligned ,(uint8_t *)u_ptr_index,"Miss Compare");
          }
          else
          {
              cnt_not_aligned++;
          }  
          break;
 
      default:
         fsprint("I am lots  %d-%s:%s \n",__LINE__,__FILE__,__FUNCTION__);
         i = cmp_len;
         break;
      } // end Switch
   } //end for loop

    if(color != NULL)    fsprint("%s", C_NORMAL);

}





