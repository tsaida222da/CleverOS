#include <LPC11xx.h>

#define  ASSERTOS(x) if( !(x) ){ __asm{cpsid i}; for(;;); }
#define  DISABLE_INTERRUPT  __asm( "cpsid i		\n" );
#define  ENABLE_INTERRUPT   __asm( "cpsie i		\n" );

#define  OSCLOCK_1S      SystemCoreClock  // defined in system_LPC11xx.c
#define  OSCLOCK_500mS   SystemCoreClock/2
#define  OSCLOCK_200mS   SystemCoreClock/5
#define  OSCLOCK_100mS   SystemCoreClock/10   // OS frequency is 10Hz. 10 ticks per second
#define  OSCLOCK_50mS    SystemCoreClock/20
#define  OSCLOCK_10mS    SystemCoreClock/100
#define  OSCLOCK_1mS     SystemCoreClock/1000
#define  OSCLOCK_100uS   SystemCoreClock/10000

#define  OSCLOCK_10K     10000
#define  OSCLOCK_100K    100000
#define  OSCLOCK_1M      1000000

#define  CLOCKOS         OSCLOCK_100mS  // number of CPU clock per OS tick

#define  DIVISOROS        100000

#define  NULL             0x0
#define  INFINITEOS      -1
	
#define SystickControlRegisterOS       ( *( ( volatile unsigned int* ) 0xE000E010 ) )
#define SystickLoadRegisterOS          ( *( ( volatile unsigned int* ) 0xE000E014 ) )
#define SystickCurrentValueRegisterOS  ( *( ( volatile unsigned int* ) 0xE000E018 ) )
#define InterruptControlRegisterOS     ( *( ( volatile unsigned int* ) 0xE000ED04 ) )
#define HandlerPriorityRegisterOS      ( *( ( volatile unsigned int* ) 0xE000ED20 ) )  //SHPR3

            // application interface function
char startOS(void (*[])(void), int, int, void (*)(void));
unsigned int queryReadyTableOS(void);
int  findOptimalPaddingOS(void);
void deleteSelfOS(void);
void delayTickOS(int);
void delayTimeOS(int, int, int, int);
void postSemOS(char);
void pendSemOS(char, int);
void schedulerOS(void);
int  tickMatchOS(int);
