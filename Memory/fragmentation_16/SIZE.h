


	  // task
#define  TASKSIZE           5 
#define  STACKRAMBYTES   1000

    // event and service
#define  PENDSIZE           0
#define  MEMORYPOOLBYTES  1000
#define  GETMEMORYSIZE      4
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0}; 
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES         1



