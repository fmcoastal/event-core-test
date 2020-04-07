#ifndef _fs_tstamp_h
#define _fs_tstamp_h



//#define  TSTAMP_OUT

#ifdef TSTAMP_OUT
typedef int fs_time_stamp;
#define tstamp_print( p,m )
#define tstamp_init( p,m )
#define tstamp_start( p)
#define tstamp_capture( p)
#define tstamp_process(p)
#define tstamp_end(p)
#define tstamp_get_avg( p)
#define tstamp_results_string(X,Y) ("NOT IMPLEMENTED")

#else

// functions to measure time intervals 

// two ways to use the stop.
//  1) call tstamp_capture() -> just grabs the time
//         then you have to callthe tstamp_process
//  2) call tstamp_end() it will grab the clock and do the process.
// use the init function to clear.
// to speed up,  keep RUNNING_SUM a binary power of 2 and adjust the code below.
//               Also, do not worry about first RUNNING_SUM avg to be correct?


//#define FS_TSTAMP_DEBUG 
#undef FS_TSTAMP_DEBUG 


#ifdef FS_TSTAMP_DEBUG
static char FS_TS__NORMAL[]   ={0x1b,'[','0','m',0x00};  // NORMAL=$'\e[0m'
static char FS_TS__GREEN[]   ={0x1b,'[','3','2','m',0x00};  // GREEN=$'\e[32m'

#define FS_TS_DEBUG() \
    printf("%s%s%s\n",FS_TS__GREEN,__FUNCTION__,FS_TS__NORMAL);\
    tstamp_print(pi,0);
#else
#define FS_TS_DEBUG()
#endif

#define HIGH_PRECISION

#ifdef HIGH_PRECISION 
// if you use this function, you must have the appropriate driver loaded 
// on your Arm Machine
//  sudo insmod pmu_el0_cycle_counter.ko
#define GET_TICKS() rte_rdtsc()
#else
#define GET_TICKS() rte_get_timer_cycles()
#endif





#define RUNNING_SUM  128
#define TIME_STAMP_NAME_LEN  64
typedef struct time_stamp_struct {
uint64_t   start;
uint64_t   stop;
uint64_t   interval;
uint64_t   max;
uint64_t   min;
uint64_t   samples;
uint64_t   rsum_index;
uint64_t   rsum;
uint64_t   rsum_samp[RUNNING_SUM];
int64_t    id;                           // Used in you need an ID in the structure 
char       name[TIME_STAMP_NAME_LEN];
char       string[256];
} fs_time_stamp;




static inline void tstamp_print(fs_time_stamp * p,int more)
{
    printf("%s           %p\n",p->name,p);
    printf(" start:       %lu\n",p->start);
    printf(" stop:        %lu\n",p->stop);
    printf(" interval:    %lu\n",p->interval);
    printf(" max:         %lu\n",p->max);
    printf(" min:         %lu\n",p->min);
    printf(" samples:     %lu\n",p->samples);
    printf(" rsum:        %lu\n",p->rsum);
    printf(" rsum_index:  %lu\n",p->rsum_index);
#ifdef HIGH_PRECISION
    printf(" clkFreq      %lu\n",rte_get_tsc_hz());
#else
    printf(" clkFreq      %lu\n",rte_get_timer_hz());
#endif


    if ( more != 0)
    {
       int i;
       for( i = 0 ; i < RUNNING_SUM ; i++)
       { 
            if( (i % 4) == 0) printf("\n  "); 
             printf("%3d) %lu ",i,p->rsum_samp[i]);
       }
       printf("\n");
    }
}



static inline void tstamp_init(fs_time_stamp * p,char * name)
{
   int len;
    memset( p, 0x0, sizeof(fs_time_stamp));
    p-> min = 0xfffffffffffffff;
    len = strlen(name) + 1 ;
    len = ( len < TIME_STAMP_NAME_LEN ) ? len : TIME_STAMP_NAME_LEN ;     
    memcpy(&(p->name[0]),name,len);
    FS_TS_DEBUG();
    return;
}

static inline void tstamp_start(fs_time_stamp * p)
{
    p-> start = GET_TICKS();
//    FS_TS_DEBUG();   // if not working, uncomment and look, else the print adds into your measurement
 
    return;

}

static inline void tstamp_capture(fs_time_stamp * p)
{
    p-> stop = GET_TICKS();
    FS_TS_DEBUG();
 
    return;
}


static inline void tstamp_process(fs_time_stamp * p)
{
    p->interval = p->stop - p->start;
    if ( p->start > p->stop) p->interval =  (~ p->start)  + p->stop + 1 ;

    // Max Min calcs
    p->min = p->interval < p->min ? p->interval : p->min ;
    p->max = p->interval > p->max ? p->interval : p->max ;

    // avg calc
    p->rsum  -= p->rsum_samp[ p->rsum_index ];
    p->rsum_samp[ p->rsum_index ] = p->interval;
    p->rsum  += p->interval;

    // make the index go from 0 to (RUNNING_SUM -1)
    p-> rsum_index =   (p->rsum_index  <  (RUNNING_SUM -1))  ? ( (p->rsum_index)+1) : 0 ;

    // flag we have another
    p->samples++;
    FS_TS_DEBUG();
 
    return;

}

static inline void tstamp_end(fs_time_stamp * p)
{
    p-> stop = rte_get_timer_cycles();
    tstamp_process(p);
    FS_TS_DEBUG();
}


static inline uint64_t tstamp_get_avg(fs_time_stamp *p)
{
    if (p->samples > RUNNING_SUM )
            return  p->rsum/RUNNING_SUM;
    else if (p->rsum_index != 0)
            return p->rsum/p->rsum_index;

    return 0;
}


static inline char* tstamp_results_string (fs_time_stamp * p, const char * label )
{
#ifdef HIGH_PRECISION
    sprintf(&(p->string[0])," \"%s\"   max: %lu    min:%lu    avg:%lu   <-cpu clks  samples:%lu\n ",label,p->max,p->min,tstamp_get_avg(p),p->samples);
#else
    sprintf(&(p->string[0])," \"%s\"   max: %lu    min:%lu    avg:%lu    hz: %lu  samples:%lu\n ",label,p->max,p->min,tstamp_get_avg(p),rte_get_timer_hz(),p->samples);
#endif
    return &(p->string[0]);
}


#endif   // TSTAMP_OUT

#endif
