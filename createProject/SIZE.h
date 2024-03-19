
	  // task
#define  TASKSIZE          3 

#define  STACKRAMBYTES     (CPUREGISTER + 40)*WORDSIZE*(TASKSIZE + 1)

    // event and service
#define  PENDSIZE           TASKSIZE
#define  GETMEMORYSIZE      2 * TASKSIZE
#define  QSIZE              TASKSIZE    
#define  MAILSIZE           TASKSIZE
#define  FLAGSIZE           TASKSIZE	
#define  MUTEXSIZE          TASKSIZE
#define  UNTILSIZE          TASKSIZE
#define  MEMORYPOOLBYTES    TASKSIZE * 40

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0};  
#define  PADDINGIDLE       0      // idleTaskOS() padding
