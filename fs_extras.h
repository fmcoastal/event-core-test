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
#define FONT_CALL_ARGUMENTS_COLOR() FONT_CYAN()
/*
Template for PRINT_DATA_STRUCTUERS


#ifdef PRINT_CALL_ARGUMENTS
    FONT_CALL_ARGUMENTS_COLOR();
    printf(  " Call Args: i :id %d  rte_event_port_conf: \n", );
    FONT_NORMAL();
#endif
*/


/*******************************/
/*    PRINT_DATA_STRUCTUERS    */
/*******************************/
#define FONT_DATA_STRUCTURES_COLOR() FONT_YELLOW()
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
    FONT_DATA_STRUCTURES_COLOR();

    printf("%sstruct xxx %s %d\n",s,string,id);
    printf("%s uint32_t var:  %d\n",s,p->  );
    printf("%s struct   var:  -TBD-\n",s   );
    FONT_NORMAL();
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
   int iiiii;\
   for ( iiiii = 0 ; iiiii < (INDENT_SIZE * x) ; iiiii++) s[iiiii] = ' ';


/******************i*****/
/*    Colors &  macros  */
/************************/

static const char C_RED[]     ={0x1b,'[','3','1','m',0x00}; /* RED    =$'\e[31m' */
static const char C_GREEN[]   ={0x1b,'[','3','2','m',0x00}; /* GREEN  =$'\e[32m' */
static const char C_YELLOW[]  ={0x1b,'[','3','3','m',0x00}; /* YELLOW =$'\e[33m' */
static const char C_BLUE[]    ={0x1b,'[','3','4','m',0x00}; /* BLUE   =$'\e[34m' */
static const char C_MAGENTA[] ={0x1b,'[','3','5','m',0x00}; /* MAGENTA=$'\e[34m' */
static const char C_CYAN[]    ={0x1b,'[','3','6','m',0x00}; /* CYAN   =$'\e[36m' */
static const char C_WHITE[]   ={0x1b,'[','3','7','m',0x00}; /* WHITE  =$'\e[37m' */
static const char C_NORMAL[]  ={0x1b,'[','0','m',0x00};     /* NORMAL =$'\e[0m'  */


#define FONT_GREEN()  printf("%s", C_GREEN);
#define FONT_CYAN()   printf("%s", C_CYAN  );
#define FONT_YELLOW() printf("%s", C_YELLOW);
#define FONT_NORMAL() printf("%s", C_NORMAL);


#endif
