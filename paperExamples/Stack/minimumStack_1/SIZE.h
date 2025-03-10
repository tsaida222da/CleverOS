
#define  TASKSIZE           2    
//#define  STACKRAMBYTES   1000
#define  STACKRAMBYTES   0x144

#define  PENDSIZE           0
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0

//int PaddingOS[TASKSIZE+1] = {0};
int PaddingOS[TASKSIZE+1] = {0x11, 0x0d}; 
#define  PADDINGIDLE         0      // idleTaskOS() padding
#define  BULKBYTES           1

