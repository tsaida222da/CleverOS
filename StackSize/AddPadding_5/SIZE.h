

#define  TASKSIZE           5 
#define  STACKRAMBYTES   2500
//#define  STACKRAMBYTES   0x4d8

#define  PENDSIZE           0
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0


int PaddingOS[TASKSIZE+1] = {0}; 
//int PaddingOS[TASKSIZE+1] = {0x50, 0x30, 0x20, 0x20, 0x10}; 
#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES        sizeof(long double)
