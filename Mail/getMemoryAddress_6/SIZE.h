	

	
	  // task
#define  TASKSIZE          3 
#define  STACKRAMBYTES     2500

    // event and service
#define  PENDSIZE           2
#define  MEMORYPOOLBYTES    100
#define  GETMEMORYSIZE      3
#define  QSIZE              0    
#define  MAILSIZE           3
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

    // padding part of task stack(selective)
int PaddingOS[TASKSIZE+1] = {0};  
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES        sizeof(long double)


