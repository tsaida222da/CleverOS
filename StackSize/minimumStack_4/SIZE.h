
#define  TASKSIZE           5 
#define  STACKRAMBYTES   2500
//#define  STACKRAMBYTES   0x450
//#define  STACKRAMBYTES   0x44C
//#define  STACKRAMBYTES   0x448

#define  PENDSIZE           0
#define  MEMORYPOOLBYTES    0
#define  GETMEMORYSIZE      0
#define  QSIZE              0    
#define  MAILSIZE           0
#define  FLAGSIZE           0	
#define  MUTEXSIZE          0
#define  UNTILSIZE          0


int PaddingOS[TASKSIZE+1] = {0};    // 2500
//int PaddingOS[TASKSIZE+1] = {0x4a, 0x2b, 0x1a, 0x13, 0x0c};      // 0x450
//int PaddingOS[TASKSIZE+1] = {0x4a, 0x2a, 0x1a, 0x13, 0x0c};    // 0x44C
//int PaddingOS[TASKSIZE+1] = {0x4a, 0x2a, 0x1a, 0x12, 0x0c};      // 0x448
#define  PADDINGIDLE       0      // idleTaskOS() padding


