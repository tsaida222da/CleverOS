


	  // task
#define  TASKSIZE            3 
#define  STACKRAMBYTES    0x01C4

    // event and service
#define  PENDSIZE           0
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              1  
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0x0e, 0x0f, 0x10};  
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES       380


