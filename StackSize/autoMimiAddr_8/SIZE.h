

	  // task
#define  TASKSIZE           4 
//#define  STACKRAMBYTES   2500
#define  STACKRAMBYTES   0x2a4
    // event and service
#define  PENDSIZE           3
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0}; 
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES        sizeof(long double)

