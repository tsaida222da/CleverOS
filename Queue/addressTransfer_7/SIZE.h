


	  // task
#define  TASKSIZE           2
#define  STACKRAMBYTES   2000

    // event and service
#define  PENDSIZE           1
#define  MEMORYPOOLBYTES   48
#define  GETMEMORYSIZE      3
#define  QSIZE              3    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0}; 
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES         2


