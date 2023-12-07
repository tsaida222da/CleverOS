


	  // task
#define  TASKSIZE          6
#define  STACKRAMBYTES     (CPUREGISTER + 40)*WORDSIZE*(TASKSIZE + 1)

    // event and service
#define  PENDSIZE           2 * TASKSIZE
#define  GETMEMORYSIZE      2 * TASKSIZE
#define  QSIZE              2 * TASKSIZE    
#define  MAILSIZE           2 * TASKSIZE
#define  FLAGSIZE           TASKSIZE	
#define  MUTEXSIZE          TASKSIZE
#define  UNTILSIZE          TASKSIZE
#define  MEMORYPOOLBYTES    TASKSIZE * 48

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0};  
#define  PADDINGIDLE       0      // idleTaskOS() padding

