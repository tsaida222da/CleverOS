


	  // task
#define  TASKSIZE           5 
#define  STACKRAMBYTES   0x440
    // event and service
#define  PENDSIZE           0
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0x50, 0x50, 0x50, 0x50, 0x0a};     
#define  PADDINGIDLE       0      // idleTaskOS() padding



