#ifndef _fs_extras_h
#define _fs_extras_h

//#define LOOP_CALLS
#undef LOOP_CALLS

/********************/
/*    WAI()         */
/********************/
#if 1
#define WAI() printf("%d-%s:%s\n",__LINE__,__FILE__,__FUNCTION__)
#else
#define WAI()
#endif


/********************/
/*  CALL_RTE()   */
/********************/
#if 1
#define CALL_RTE(x) printf("%s Call:  %s %s\n",C_GREEN,x,C_NORMAL);
#else
#define  CALL_RTE(x)
#endif


/*******************************/
/*    PRINT_CALL_ARGUMENTS     */
/*******************************/
#define PRINT_CALL_ARGUMENTS
/*
Template for PRINT_DATA_STRUCTUERS


#ifdef PRINT_CALL_ARGUMENTS
    FONT_CYAN();
    printf(  " Call Args: i :id %d  rte_event_port_conf: \n", );
    FONT_NORMAL();
#endif
*/


/*******************************/
/*    PRINT_DATA_STRUCTUERS    */
/*******************************/
#if 1
#define PRINT_DATA_STRUCTURES
#else
#undef PRINT_DATA_STRUCTURES
#endif
/*
Template for PRINT_DATA_STRUCTUERS

#ifdef PRINT_DATA_STRUCTURES
 void print_xxx  (int indent,const char* string,int id,struct xxx *p);
 void print_xxx  (int indent,const char* string,int id,struct xxx *p)
 {
     INDENT(indent);
     printf("%sstruct xxx %s %d\n",s,string,id);
     printf("%s uint32_t var:  %d\n",s,p->  );
     printf("%s struct   var:  -TBD-\n",s   );
 }
#else
#define print_xxx  (w,x,y,z)
#endif
*/


/********************/
/*    Indent macro  */
/********************/
#define INDENT_SIZE 3
#define INDENT(x) \
   char s[64]= {0};\
   int i;\
   for ( i = 0 ; i < (INDENT_SIZE * x) ; i++) s[i] = ' ';


/******************i*****/
/*    Colors &  macros  */
/************************/

static const char C_GREEN[] ={0x1b,'[','3','2','m',0x00}; /* GREEN =$'\e[32m' */
// yellow
// blue
static const char C_BLUE[]  ={0x1b,'[','3','4','m',0x00}; /* CYAN  =$'\e[32m' */
static const char C_CYAN[]  ={0x1b,'[','3','5','m',0x00}; /* CYAN  =$'\e[32m' */
static const char C_NORMAL[] ={0x1b,'[','0','m',0x00};    /* NORMAL=$'\e[0m'  */


#define FONT_GREEN()  printf("%s",C_GREEN);
#define FONT_CYAN()   printf("%s",C_CYAN);
#define FONT_NORMAL() printf("%s",C_NORMAL);


#endif
