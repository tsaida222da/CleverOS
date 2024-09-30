
#include "PORT.h"

#if   defined ( CM0 )
	 #define  ASSERTOS(x) if( !(x) ){ __asm( "cpsid i		\n" ); for(;;); }
	 #define  DISABLE_INTERRUPT  __asm( "cpsid i		\n" );
   #define  ENABLE_INTERRUPT   __asm( "cpsie i		\n" );
	 #define  CPUREGISTER         17
		 
   #if   defined ( NXP )
      #include <LPC11xx.h>
	    #define  CPUclockOS   SystemCoreClock
   #endif
#endif
	 
#define  OSCLOCK_1S       CPUclockOS 
#define  OSCLOCK_500mS    CPUclockOS/2
#define  OSCLOCK_200mS    CPUclockOS/5
#define  OSCLOCK_100mS    CPUclockOS/10     // 10 tick per second
#define  OSCLOCK_50mS     CPUclockOS/20
#define  OSCLOCK_20mS     CPUclockOS/50
#define  OSCLOCK_10mS     CPUclockOS/100
#define  OSCLOCK_5mS      CPUclockOS/200
#define  OSCLOCK_2mS      CPUclockOS/500
#define  OSCLOCK_1mS      CPUclockOS/1000
#define  OSCLOCK_500uS    CPUclockOS/2000
#define  OSCLOCK_200uS    CPUclockOS/5000
#define  OSCLOCK_100uS    CPUclockOS/10000
#define  OSCLOCK_1M       1000000

           // select OS tick
#define  CLOCKOS          OSCLOCK_100mS
#define  BULKBYTES        80

#if   defined ( ARM )     
  #define  WORDSIZE    4
       // core register address
  #define SystickControlRegisterOS       ( *( ( volatile unsigned int* ) 0xE000E010 ) )
  #define SystickLoadRegisterOS          ( *( ( volatile unsigned int* ) 0xE000E014 ) )
  #define SystickCurrentValueRegisterOS  ( *( ( volatile unsigned int* ) 0xE000E018 ) )
  #define InterruptControlRegisterOS     ( *( ( volatile unsigned int* ) 0xE000ED04 ) )
  #define HandlerPriorityRegisterOS      ( *( ( volatile unsigned int* ) 0xE000ED20 ) )  //SHPR3
#endif

           // check safety
#define  COUNTSTARTOS      5
#define  MAXLEVEL          5
#define  NULL              0x0

           // timeout infinite
#define  INFINITEOS       -1
#define  ISOLATEOS        -2

           // event
#define  SEM               0
#define  MAIL              1	
#define  FLAG              2
#define  QUEUE             3
#define  MUTEX             4
            // used in pendMailOS()
#define  NOTCLEARMAIL      0
#define  CLEARMAIL         1
            // used in postFlagOS()
#define  FLAG_CLEAR        0
#define  FLAG_SET          1
	          // used in pendFlagOS()
#define  FLAG_MATCH_ANY    0
#define  FLAG_MATCH_ALL    1
            // used in memoryOS()
#define  MALLOC           -1

/*****************************************************************/
/*                    API                                        */
/*****************************************************************/

         // Kernel
char          startOS(void (*taskName[])(void), int arraySize, int startPriority, void (*lowPowerTimer)(void), void (*danger)(int));
unsigned int* queryReadyTableOS(void);
int           nonBlockingCallbackOS(int (*)(void));
void          deleteSelfOS(void);
char          errorPendSizeOS(void);
void          schedulerOS(void);
         // Stack
int           ramToPaddingOS(int ramBytes, int taskSize);
int           paddingToRamOS(int totalPadding, int taskSize);						
int           autoPackItemsOS(void);
int*          minimumStackOS(int* minimumRam);
int           autoMinimumStackOS(void);
int           queryResidueStackOS(void);
char          queryDangerTaskOS(void);
void          checkStackSafetyOS(int startCount, int level);
char          querySafeLevelOS(int count);
         // Delay
void          delayTickOS(int tick);
void          delayTimeOS( int hour, int minute, int second, int mS);
void          delayUntilEqualOS(int *a, int *b);
void          delayUntilTrueOS(int *a);
         // Semaphore
void          postSemOS(int number);
int           pendSemOS(int *array, int timeout);
         // Mutex
void          postMutexOS(void);
int           pendMutexOS(int *array, int timeout);
         // Mail Message
void          postMailOS(int number, void *messageAddr);
void*         readMailOS(int number, char clear);
void*         pendMailOS(int *array, int *readyNumberAddr, char clear, int timeout);
         // Flag
void          postFlagOS(int number, unsigned int modifyPublicFlag, char setOrClear );
int           pendFlagOS(int *array, unsigned int privateFlag, char allOrAny, int timeout);
char          checkPublicFlagBitOS(int flagNumber, char bitNumber);
unsigned int  queryPublicFlagOS(int flagNumber);
         // Memory Management
void*         getMemoryOS(int getNo, int bytes);
void*         getMemoryWithPutOS(int getNo, int bytes);
void*         margeAddressOS(void* relyAddress);
void*         dangerAddressOS(void* relyAddress, int *dangerBytes);
void          putMemoryOS(int getNo); 
void*         mallocOS(int bytes);
void          freeOS(void* ptr);
int           maxFreeMemoryOS(void);
int           minFreeMemoryOS(void);
int           queryFreeBulkNoOS(char* result, int length);
int           leakMemoryNoOS(void);
int           lackMemoryNoOS(void);
int*          leakMemoryNoAllOS(void);
int*          lackMemoryNoAllOS(void);
         // Queue
int           postQOS(int number, void *messageAddr);
void*         pendQOS(int *array, int* readyNo, int* items, int timeout);
int           qReadyNumberOS(void* retrieveAddress);
int           queryRemainItemsOS(int number);
void          qRxIntFloatOS(int qNo, void* pData);
void          qTxIntFloatOS(int qNo, void* pData, int length, char power);
int           packetLengthOS(int qNo);
void          qTxRealtimeOS(int number, void *messageAddr);
void*         qRxRealtimePendOS(int number);
         // Task Loading
int*          relativeTaskLoadOS(void);
int           idleTaskLoadOS(void);
         // Low Power Mode
unsigned int  matchRegisterOS(void);	
unsigned int  divisorOS(void);

	
	


