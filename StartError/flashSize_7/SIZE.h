

	  // task
#define  TASKSIZE           1 
#define  STACKRAMBYTES   1000

    // event and service
#define  PENDSIZE           5
#define  MEMORYPOOLBYTES   48
#define  GETMEMORYSIZE      2
#define  QSIZE              2    
#define  MAILSIZE           1
#define  FLAGSIZE           1	
#define  MUTEXSIZE          1
#define  UNTILSIZE          2

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0}; 
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES        sizeof(long double)
