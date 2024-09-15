#include <LPC11xx.h>

#define  ASSERTOS(x) if( !(x) ){ __asm{cpsid i}; for(;;); }
#define  DISABLE_INTERRUPT  __asm( "cpsid i		\n" );
#define  ENABLE_INTERRUPT   __asm( "cpsie i		\n" );

#define  CPUclockOS   SystemCoreClock
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

#define  CLOCKOS         OSCLOCK_100mS  // number of CPU clock per OS tick

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
unsigned int  matchRegisterOS(void);

