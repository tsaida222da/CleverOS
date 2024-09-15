
#include "SIZE.h"
#include "OS.h"

typedef struct 
{
    unsigned int  sp;  // sp must be the first element
	  unsigned int  pack[PADDING];
	  unsigned int  registerStack[16];
} stackOS;     

void         (*lowPowerTimerOS)(void);
stackOS       *CurrentTaskOS; 
stackOS       *NextTaskOS;  
stackOS       TaskOS[TASKSIZE+1];  // including idleTaskOS()
int           WaitTickOS[TASKSIZE];
unsigned int  ReadyTableOS;  // bit value 1 is ready state, bit index is task priority value
unsigned int  MinDelayTickOS;
int           CurrentPriorityOS;   // priority value <= TASKSIZE - 1, priority value begin from 0
int           TickPerSecondOS;
char          PowerOnOS = 1;
char          SemNumberTaskOS[TASKSIZE];  // initialized in startOS(), used in pendSemOS(), array index is priority value, array value is task's pending SemNumber(0 ~ 255).
char          PriorityOwnEventOS[TASKSIZE];     // array index is priority value, 0 <= priority <=  TASKSIZE - 1


void initializeSysTickOS(void)
{
	  SystickLoadRegisterOS = (int)CLOCKOS -1;
	  SystickCurrentValueRegisterOS = 0x0;
	  SystickControlRegisterOS = (1<<0) | (1<<1) | (1<<2); // enable, interrupt, system(CPU) clock
}


void setHandlerPriorityOS(void)
{
	  unsigned int lowest = 0xff;
	  unsigned int highest = 0x0;
	
	  HandlerPriorityRegisterOS |= lowest  << 16;
	  HandlerPriorityRegisterOS |= highest << 24;
}


void setReadyTableOS(int priority)  
{
	DISABLE_INTERRUPT;
	  ReadyTableOS |= ( 1<< priority );
	ENABLE_INTERRUPT;
}


void clearReadyTableOS(int priority)
{
	ENABLE_INTERRUPT;
	  ReadyTableOS &=  ~( 1<< priority );	
	ENABLE_INTERRUPT;
}


                  // Table must not be zero
__asm  int findLeastBitOS(unsigned int Table)
{
	MOVS R3, #0
	
loop
	LSRS R0,R0,#1
	BCS  return
	ADDS  R3, R3, #1
	CMP  R3, #32
	BLT  loop

return
	MOV  R0, R3  
	BX   LR
}


__asm  int interruptNumberOS()
{
  MRS  R0, IPSR
	BX   LR	
}


__asm void PendSV_Handler(void)
{	
//PUSH 
	MRS	R0, PSP  
	SUBS R0, #32
	STMIA R0!,{R4-R7} 
	MOV	R4, R8
	MOV	R5, R9
	MOV	R6, R10
	MOV	R7, R11
	STMIA R0!,{R4-R7}   
	SUBS R0, #32       
	IMPORT  CurrentTaskOS  
	LDR	R2, =CurrentTaskOS
	LDR	R1, [R2]         
	STR	R0, [R1]        
	
//POP	
	IMPORT  NextTaskOS   
	LDR	R2, =NextTaskOS
	LDR	R1, [R2]
	LDR	R0, [R1]        
	LDMIA R0!,{R4-R7}  
	LDMIA R0!,{R1-R3}  
	MOV	R8, R1
	MOV	R9, R2
	MOV	R10, R3
	LDMIA R0!,{R1}	
	MOV	R11, R1        

	MSR	PSP, R0      

	LDR R3, =0xFFFFFFFD  

	BX	R3       
}

__asm void setPSPOS(unsigned int topPointer)
{
	  PRESERVE8
	  THUMB
	
    MSR  PSP, R0
	  BX   LR	
}


__asm void setCONTROLOS(unsigned int usePSP)
{
		PRESERVE8
	  THUMB
	
    MSR  CONTROL, R0
	  ISB
	  BX   LR	
}


          // low power mode
void minDelayTickOS(void)
{
	  int  i;
		unsigned int  min = 0xffffffff;

    for (i=0; i<TASKSIZE; i++)
		{
        if ( (WaitTickOS[i] > 0) && (WaitTickOS[i] < min) )
				{
					  min = WaitTickOS[i];
				}
		}			
		
    for (i=0; i<TASKSIZE; i++)
		{
			 DISABLE_INTERRUPT;
			  WaitTickOS[i] -= min;
			 ENABLE_INTERRUPT;
			
			  if ( WaitTickOS[i] == 0 )
				{
					  setReadyTableOS(i);
				}
		}

	 DISABLE_INTERRUPT;		
		MinDelayTickOS = min;
	 ENABLE_INTERRUPT;
}



unsigned int matchRegisterOS(void)
{
	  return  MinDelayTickOS;
}



int highestEventPriorityOS(void)
{
	  int  i;
    int  highPriority = -1;	
	
	  for (i=0; i<TASKSIZE; i++)
		{
			  if ( PriorityOwnEventOS[i] == 1 )
				{
					  highPriority = i;
					  break;
				}
		}
	
		return  highPriority;
}



void schedulerOS(void)
{	
	  int   highestPriority;  
	
	          // task that owns event does not need to be scheduled. it means the task owns the highest priority to occupy CPU
	  if ( ( PriorityOwnEventOS[CurrentPriorityOS] < 1 ) || ( CurrentPriorityOS == (int)TASKSIZE ) )  // CurrentPriorityOS  that own events does not switch context( highest priority )
		{
			  highestPriority = highestEventPriorityOS();
				if ( highestPriority < 0 )
				{
            highestPriority =  findLeastBitOS(ReadyTableOS);
				}
							
				if( (CurrentPriorityOS != (int)TASKSIZE) && (highestPriority == (int)TASKSIZE) && (lowPowerTimerOS != NULL) && PowerOnOS ) 
			  {
             minDelayTickOS();
             PowerOnOS = 0;							 
				}
				else if( (highestPriority != (int)TASKSIZE) && (!PowerOnOS)) 
			  {
            PowerOnOS = 1;					
				}
				 
        if(  highestPriority != CurrentPriorityOS   )
        {  
					  DISABLE_INTERRUPT;
	      		        //  context Switch
	             CurrentTaskOS = &TaskOS[CurrentPriorityOS];
               NextTaskOS = &TaskOS[highestPriority];
	             CurrentPriorityOS = highestPriority;	    // change CurrentPriorityOS							
               InterruptControlRegisterOS = 1 << 28;
					  ENABLE_INTERRUPT;
        }
		} 
}


void idleTaskOS(void)
{
    while(1) 
		{ 					 
				    // low power mode
		     if ( ( lowPowerTimerOS != NULL ) && !( PowerOnOS ) )
				 {
					   lowPowerTimerOS();
				 }					
		}
} 



void initializeTaskOS( void (*handler)(void), int priority)
{
		int i;
	
    DISABLE_INTERRUPT;				 
     setReadyTableOS(priority );    

	   TaskOS[priority].sp = (unsigned int)( &TaskOS[priority].registerStack[0] );  
     TaskOS[priority].registerStack[15] = 0x01000000;                  
		 TaskOS[priority].registerStack[14] = (unsigned int)handler; 
    ENABLE_INTERRUPT;	

		for( i=0; i<(int)PADDING; i++ )
    {
			 DISABLE_INTERRUPT;	
        TaskOS[priority].pack[i] = 0xf000000f;
			 ENABLE_INTERRUPT;	
    }				 
} 


char checkStartErrorOS(int arraySize, int startPriority, void (*lowPowerTimer)(void))
{
	   char errorCode= 0;

     if ( arraySize >= 32  )   // maximum TASKSIZE is 31
		 { 	
		     errorCode = 1; 
		 }
		 
     if ( arraySize !=  (int)TASKSIZE  )  
		 { 	
		     errorCode = 2;     // error will stop OS
		 }			
			
     if (  (startPriority >=  (int)TASKSIZE) || (startPriority < 0)  )  
		 { 	
		     errorCode = 3;  				
		 }
			
		 if ( (lowPowerTimer != NULL) && ((int)PADDING < 10) )
		 {
		     errorCode = 4;  
		 }

		 if ( ((unsigned int)CPUclockOS % (unsigned int)CLOCKOS) != 0 )
		 {
				 errorCode = 5;	 
		 }
			
	   return  errorCode;
}


          // ReadyTable is initialized
char startOS(void (*taskName[])(void), int arraySize, int startPriority, void (*lowPowerTimer)(void))
{ 
	   int          i;
	   char         errorCode;
	   unsigned int OSclock = CLOCKOS;
	
		 errorCode = checkStartErrorOS(arraySize, startPriority, lowPowerTimer);

	   if( errorCode )
		 {
			  return errorCode;
		 }	

		 lowPowerTimerOS = lowPowerTimer;
	   initializeSysTickOS();
		 setHandlerPriorityOS();	
		 
		       // initialize  tasks
		 for ( i=0; i<TASKSIZE; i++) // idleTaskOS's priority value is TASKSIZE
		 {
			 DISABLE_INTERRUPT;
			   SemNumberTaskOS[i] = (char)-1;				 
			 	 PriorityOwnEventOS[i] = 0;
	       WaitTickOS[i] = 0;		
			 ENABLE_INTERRUPT;
			   initializeTaskOS(taskName[i], i);  
		 }
		
		 initializeTaskOS(idleTaskOS, TASKSIZE); 

		 TickPerSecondOS = (int)((unsigned int)CPUclockOS / OSclock);
     CurrentPriorityOS = startPriority;		 
     CurrentTaskOS = &TaskOS[CurrentPriorityOS]; 

		 setPSPOS((unsigned int)&CurrentTaskOS->registerStack[15] + 4);
     setCONTROLOS(0x02);         // use PSP
		 
		 taskName[CurrentPriorityOS]();
	 
		 return 0;   // never reach if successful
	 
 }


void SysTick_Handler(void)
{
     int  i;
     char  schedule = 0; 

		            // one Tick Passed and set ready task
     for( i=0; i<TASKSIZE; i++ )   // i is task's priority value
     {
				  if ( WaitTickOS[i] >= 1 )		// task's waitTick < 0 can not be ready(timeout is infinite)   
          {
						 DISABLE_INTERRUPT;
				       WaitTickOS[i]--;	
						 ENABLE_INTERRUPT;
          } 
						 
				  if ( WaitTickOS[i] == 0  )	
          {	
					     setReadyTableOS(i);							 					 
						   schedule = 1;
          } 						 
								
     } 
				
		 if ( schedule )
		 {
  	      schedulerOS();		
		 }
}


void pauseTaskOS(int timeout)
{	
     DISABLE_INTERRUPT;				
	    PriorityOwnEventOS[CurrentPriorityOS] = 0;	   // current task does not own event
	    WaitTickOS[CurrentPriorityOS] = timeout;
	   ENABLE_INTERRUPT;		
	 	  clearReadyTableOS(CurrentPriorityOS );
}


void delayTickOS(int tick)
{
	 if ( interruptNumberOS() == 0 )
	 {
       pauseTaskOS(tick);		 
       schedulerOS();	
	 }
} 


void deleteSelfOS(void)
{
	  delayTickOS( -1 );
}


void delayTimeOS( int hour, int minute, int second, int  mS)
{
	 int   tick;	 
	
	 if ( interruptNumberOS() == 0 )
	 {
	     if ( ( hour >= 0 ) && (  minute >= 0 ) && ( second >= 0 ) && ( mS >= 0 ) )
		   {
	         tick = TickPerSecondOS * ( hour * 3600 + minute * 60 + second );
	         tick += TickPerSecondOS * mS / 1000; 
			 
			     if ( (TickPerSecondOS * mS % 1000) > 500 )
				   {
					    tick++; 
				   }	
				 
			     if ( tick < 1 )
				   {
					    tick = 1; 
				   }
					 
	         pauseTaskOS(tick);
		   }
		 
       schedulerOS();	
	 }			 
} 



             // It is permissible that multiTask pend the same SemNumber 
void postSemOS(char semNumber)
{
	  int i;     // priority

	  if (  semNumber != (char)-1 )
		{
        for ( i=0; i < TASKSIZE; i++ )
        {
				    if(  i == CurrentPriorityOS )
				    {
							 DISABLE_INTERRUPT;	
				        PriorityOwnEventOS[CurrentPriorityOS] = 0;  // used in schedulerOS()		
	              SemNumberTaskOS[CurrentPriorityOS] = (char)-1;   // does not pend Sem
	             ENABLE_INTERRUPT;		 
						}
						else if( SemNumberTaskOS[i] == semNumber )  // resume all tasks pendding this number
						{	
	                  DISABLE_INTERRUPT;									 
	                    SemNumberTaskOS[i] = (char)-1; 		// delete sem number
			                PriorityOwnEventOS[i] = 1;
								      WaitTickOS[i] = 0;
		                  setReadyTableOS(i);  //  the pending task is ready now, regardless the waitTick value
								 	  ENABLE_INTERRUPT;	
						}	
        }
	  } // if (  semNumber >= 0 )

	  if ( interruptNumberOS() == 0 )
		{
	      schedulerOS();	
		}
}

                    //  timeout is the number of tick 
void pendSemOS(char semNumber, int timeout)
{ 
	  if ( interruptNumberOS() == 0 )
		{		
       DISABLE_INTERRUPT;			 
			   SemNumberTaskOS[CurrentPriorityOS] = semNumber; 
		   ENABLE_INTERRUPT;	
		 
		     pauseTaskOS(timeout);
		 
         schedulerOS();
	  }		 
}


int findOptimalPaddingOS(void) 
{
	  int i;
    int k;
    int pack;     
    int maximum = -1; 
	
	  for ( i=0; i< TASKSIZE; i++ )
	  {
        k = 0;
        while( TaskOS[i].pack[k] == 0xf000000f )
        {
              k++;
        }

        pack = (int)PADDING - k;                                

		    if (  pack > maximum )
		    {
			      maximum = pack;       
		    }		

	    } // for
				
      return maximum;
}


unsigned int queryReadyTableOS(void)
{
    return  ReadyTableOS;
}




