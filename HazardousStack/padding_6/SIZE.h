


	  // task
#define  TASKSIZE          1 
#define  STACKRAMBYTES     3000

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
//int PaddingOS[TASKSIZE+1] = {10};        //1
// int PaddingOS[TASKSIZE+1] = {15};       //2
// int PaddingOS[TASKSIZE+1] = {24};       //3
 int PaddingOS[TASKSIZE+1] = {27};       //4
// int PaddingOS[TASKSIZE+1] = {36};       //5

#define  PADDINGIDLE       0      // idleTaskOS() padding
#define  BULKBYTES        sizeof(long double)

