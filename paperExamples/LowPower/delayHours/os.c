
#include "SIZE.h"
#include "OS.h"

#define  TABLELENGTH      ( ( TASKSIZE + 1 ) / 33   + 1 )
#define  QBYTES             BULKBYTES
#define  QLENGTH          ( (QBYTES + BULKBYTES) / sizeof(void*) )
#define  QMEMORY            QSIZE*QLENGTH*sizeof(void*)
#define  MEMORYPOOL       ( QMEMORY + MEMORYPOOLBYTES )
#define  GLOBALBULKLENGTH ( MEMORYPOOL % BULKBYTES ? MEMORYPOOL / BULKBYTES + 1 : MEMORYPOOL / BULKBYTES )	
#define  FREEBULKLENGTH   ( GLOBALBULKLENGTH ? GLOBALBULKLENGTH / 33   + 1 : 0 )

          // inline.c
extern int  findLeastBitOS(unsigned int);
extern int  interruptNumberOS(void);
extern void setPSPOS(unsigned int);
extern void setCONTROLOS(unsigned int);
extern unsigned int returnPSPOS(void);

typedef struct
{
   void**  q;    
	 int     inIndex;      // push entry(array index)	
   int     outIndex;
	 int     items;
}qbodyOS;	

typedef struct
{
	 void*  dataRx; 
	 int    itemsRx;
	 char   completeRx;
	
	 int    itemsTx;
	 void*  dataTx;  
   int    length;
	 char   power;
	 char   completeTx;
}qtxrxOS;

typedef struct
{
   int           priority;         // pending task
   int          *numberArray;	
	 int           readyNumber;   
   char          eventType;	
	 unsigned int  privateFlag;      // flag
} eventOS;

typedef struct
{
   char a[BULKBYTES];
}bulkTypeOS;

void          (*overflowHandlerOS)(int);
void          (*lowPowerTimerOS)(void);
unsigned int* CurrentTaskOS;      // address of sp(the entry of CPU registers stored in ram stack)
unsigned int* NextTaskOS;         // address of sp 
int           CurrentPriorityOS;   // 0 <= task priority value <= TASKSIZE - 1
int           TickPerSecondOS;
int           WaitTickOS[TASKSIZE];     // exclude idleTaskOS()
unsigned int  ReadyTableOS[TABLELENGTH];  // bit index is priority
unsigned int  CountTaskOS[TASKSIZE]; 
int           SystemTickOS;
char          ErrorPendSizeOS = 0;
char          PowerOnOS = 1;

unsigned int* TaskSpPointerOS[TASKSIZE+1];  // include idleTaskOS()
unsigned int  TaskStackOS[STACKRAMBYTES/4];  // include all tasks and idleTaskOS()
int           PackSizeOS[TASKSIZE+1];
unsigned int  DangerStackOS[TABLELENGTH];
int           ResidueOS[TASKSIZE];
int           DeltaResidueOS[TASKSIZE];
int           MaxDeltaOS[TASKSIZE];
char          SafeLevelOS[TASKSIZE];
int           AutoPackItemsOS;

unsigned int  PriorityOwnEventOS[TABLELENGTH];
eventOS       EventNumberTaskOS[PENDSIZE];

void*         ReceiveMessageOS[MAILSIZE];	 
int           MutexOwnerOS[MUTEXSIZE];   // priority value of mutex owner
unsigned int  PublicFlagOS[FLAGSIZE];
unsigned int  FlagAllOrAnyOS[TABLELENGTH];  // bit value 1 means private flag must match all the bits required, bit value 0 means private flag could match only one bit, priority value is bit index

bulkTypeOS    PoolOS[GLOBALBULKLENGTH];
unsigned int  FreeBulkNoOS[FREEBULKLENGTH];
int           StartBulkNoOS[GETMEMORYSIZE];  
char          LeakNoOS[GETMEMORYSIZE];
char          LackNoOS[GETMEMORYSIZE];
int           LeakNoAllOS[GETMEMORYSIZE];
int           LackNoAllOS[GETMEMORYSIZE];
void*         MemoryFreeAddressOS;

int*          DelayUntilOS[UNTILSIZE][2];       // conditional delay
int           DelayUntilPriorityOS[UNTILSIZE];  // priority value

qbodyOS       QBodyOS[QSIZE];
qtxrxOS       QTxRxOS[QSIZE];
void*         qRetrieveOS[QSIZE][QLENGTH];
char          FlagTxRxOS = 0;

int           TaskClockOS[TASKSIZE+1][2];   // [0]- before task executing  [1]- after task executing
unsigned int  TaskExecuteClockOS[TASKSIZE+1];  // clock elapsed
int           TaskLoadOS[TASKSIZE];       // %

unsigned int  MinDelayTickOS;

/*****************************************************************/
/*                        Kernel                                 */
/*****************************************************************/

void* mallocOS(int);                    // called by initializeEventOS()
void  assignPaddingSpOS(void);          // called by initializeTaskOS()
void  setTableOS(unsigned int*, int);   // called by initializeTaskOS()
void  idleTaskOS(void);                 // used   by initializeTaskOS()
void  minDelayTickOS(void);             // called by schedulerOS()
char  checkDelayUntilOS(void);          // called by SysTick_Handler()
void  nonBlockingValueTransferOS(void); // called by SysTick_Handler()
void  delayTickOS(int);                 // called by deleteSelfOS()
	
#if   defined  CM0 

void initializeSysTickOS(void)
{
	  SystickLoadRegisterOS = (unsigned int)TICK - 1;
	  SystickCurrentValueRegisterOS = 0x0;
	  SystickControlRegisterOS = (1<<0) | (1<<1) | (1<<2); // enable, interrupt, system cpu clock
}

void setHandlerPriorityOS(void)
{
	  unsigned int lowest = 0xff;
	  unsigned int highest = 0x0;
	
	  HandlerPriorityRegisterOS |= lowest  << 16;
	  HandlerPriorityRegisterOS |= highest << 24;
}


void initializeTaskOS( void (*taskName[])(void) )
{
	  int i;
	  int k;
	  unsigned int *spAddr;

	  assignPaddingSpOS();
	
		for ( i=0; i <= TASKSIZE; i++)  // idleTaskOS's priority value =  TASKSIZE
		{
			  setTableOS( ReadyTableOS, i );  
			
			 DISABLE_INTERRUPT;
			  TaskExecuteClockOS[i] = 0x0;			
				 
        spAddr =  TaskSpPointerOS[i];

			  *spAddr = (unsigned int) ((int)spAddr + (PaddingOS[i]+1)*WORDSIZE );  // the entry of CPU registers in task's stack
			
        *(spAddr + PaddingOS[i] + 16) = 0x01000000;                  // xPSR copy from registerStack[15]   

				if( i != (int)TASKSIZE )
				{
				    *(spAddr + PaddingOS[i] + 15) = (unsigned int)taskName[i];   // PC copy from registerStack[14] 
				    WaitTickOS[i] = 0; 
				}
				else
				{
				    *(spAddr + PaddingOS[i] + 15) = (unsigned int)idleTaskOS;						 
				}
					 
			  for( k=0; k<PaddingOS[i]; k++ )
        {
            *(spAddr + 1 + k) = IDLEITEM;        // pack[i]
        }	
				
				if ( i != (int)TASKSIZE )
			  {
				 	 DeltaResidueOS[i] = 0;
			     ResidueOS[i] = PaddingOS[i];
				   MaxDeltaOS[i] = 0;		
           CountTaskOS[i] = 0;	
           SafeLevelOS[i] = MAXLEVEL + 3;					
			  }
		  ENABLE_INTERRUPT;	
			 
	 } // for
} 

#endif



void setTableOS(unsigned int* Table, int priority)  
{
	  int  index;   // active priority value = 32 * IndexOS + BitOS
	  char bit;	
	
	  index = priority / 32;
	  bit = priority % 32;	
	 DISABLE_INTERRUPT;
	  Table[index] |= ( 1<< bit );
	 ENABLE_INTERRUPT; 
}

void clearTableOS(unsigned int* Table, int priority)  
{
	  int  index;  
	  char bit;	
	
	  index = priority / 32;
	  bit = priority % 32;	
	 DISABLE_INTERRUPT;
	  Table[index]  &=  ~( 1<< bit );	
	 ENABLE_INTERRUPT; 
}


      // return 1 or 0 only
char checkSetBitOS(unsigned int *Table, int priority)
{
    unsigned int priorityBit = 0x0;
	  int  index;	
	  char bit;	

	  index = priority / 32;	
	  bit = priority % 32;	
	  priorityBit |=  ( 1<< bit );
	  priorityBit &= Table[index];
	
	  bit =1;
	  if ( priorityBit == 0x0 )    
	  {
		    bit = 0;
	  }
	 
	  return bit;
}


void currentExecuteClockOS(void)
{
	  int  i;
    int  clockValue;     // ignore SysTick count
	  int  clockDifference;
		                         // SysTick Counter  is down counter
   	if ( TaskClockOS[CurrentPriorityOS][0] >= TaskClockOS[CurrentPriorityOS][1] )
		{
			  clockValue = TaskClockOS[CurrentPriorityOS][0] - TaskClockOS[CurrentPriorityOS][1];
		}
		else
		{                 
			  clockValue = (int)TICK - TaskClockOS[CurrentPriorityOS][1] + TaskClockOS[CurrentPriorityOS][0];
		}		

		clockDifference = clockValue;
		
		if (  SystemTickOS > 0 )
		{
		    clockDifference = (SystemTickOS - 1) * (int)TICK + clockValue;			
		}
		         // DISABLE_INTERRUPT
		else if ( (SystemTickOS == 0) && (TaskClockOS[CurrentPriorityOS][1] >= TaskClockOS[CurrentPriorityOS][0]) )
		{
			  clockDifference = (int)TICK  - TaskClockOS[CurrentPriorityOS][1] + TaskClockOS[CurrentPriorityOS][0];	
		}

	 DISABLE_INTERRUPT;
		
		TaskExecuteClockOS[CurrentPriorityOS] += clockDifference;

		if ( TaskExecuteClockOS[CurrentPriorityOS] > 0xefffffff )
		{  
			  for (i=0; i<=TASKSIZE; i++)
				{
		        TaskExecuteClockOS[i] = 0;			
				}
				
			  SystemTickOS = 0;
		}
	 ENABLE_INTERRUPT; 		
}


int highestEventPriorityOS(void)
{
	  int  i;
    int  highestPriority = -1;	
	
	  for (i=0; i<TASKSIZE; i++)
		{
			  if ( checkSetBitOS(PriorityOwnEventOS, i) )
				{
					  highestPriority = i;
					  break;
				}
		}
	
		return  highestPriority;
}


void schedulerOS(void)  
{
    int  highestPriority;	
	  int  index;
	  int  base;
	  int  i;	
	  char setBit;
	  char danger = 1;

	  setBit = checkSetBitOS(PriorityOwnEventOS, CurrentPriorityOS);
	
	  if( ( !setBit ) || ( CurrentPriorityOS ==  TASKSIZE ) )  // if CurrentPriorityOS acquires event, it executes continuously
 		{
			  while ( danger > 0 )
				{  
					  highestPriority = highestEventPriorityOS();
					
					  if ( highestPriority < 0 )
						{
				        index = 0;
		            while (  ReadyTableOS[index] == 0  )
		            {
				            index++;		
		            }

                 highestPriority =  findLeastBitOS(ReadyTableOS[index]);
				
					       if ( index >= 1 )
					       {
					           base = 0;
	                   for ( i = index; i > 0; i-- )
		                 {
			                  base += 32;
		                 }
	                   highestPriority += base;
					       }
						}
							
						danger = 0;
						if ( checkSetBitOS(DangerStackOS, highestPriority) > 0 )
						{ 
							  danger = 1;
							 DISABLE_INTERRUPT;	
	              WaitTickOS[highestPriority] = ISOLATE;
	             ENABLE_INTERRUPT;	
		            clearTableOS(PriorityOwnEventOS, highestPriority);	
	              clearTableOS(ReadyTableOS, highestPriority);
							
								if ( overflowHandlerOS != NULL)
								{
								   overflowHandlerOS(highestPriority );
								}
						}
			  } // while  
                        // counting execution times for task loading and stack safe level
				if( (CurrentPriorityOS != (int)TASKSIZE) && (highestPriority != CurrentPriorityOS) ) 
			  {					
						 CountTaskOS[CurrentPriorityOS]++;
										
						 if ( CountTaskOS[CurrentPriorityOS] > 0xefffffff )  
             {
	                CountTaskOS[CurrentPriorityOS] = COUNTSTART;
             }	
				}
				          // low power mode
				if( (CurrentPriorityOS != (int)TASKSIZE) && (highestPriority == (int)TASKSIZE) && (lowPowerTimerOS != NULL) && PowerOnOS )
			  {
					   minDelayTickOS();
					   PowerOnOS = 0;	 
				}
				else if ( (highestPriority != (int)TASKSIZE) && (!PowerOnOS) )
				{
					  PowerOnOS = 1;
				}
				    // task loading : executable condition
        if( highestPriority != CurrentPriorityOS )
        {  	
						  DISABLE_INTERRUPT;    // after executing current task 
                TaskClockOS[CurrentPriorityOS][1] = (int)SystickCurrentValueRegisterOS;
							ENABLE_INTERRUPT; 
						 
						    currentExecuteClockOS();

							 DISABLE_INTERRUPT;
								         // before executing next task
                 TaskClockOS[highestPriority][0] = (int)SystickCurrentValueRegisterOS;
							   SystemTickOS = 0;	
								
							              //  context Switch
							    CurrentTaskOS = TaskSpPointerOS[CurrentPriorityOS];
							    NextTaskOS = TaskSpPointerOS[highestPriority];
	                CurrentPriorityOS = highestPriority;
						      InterruptControlRegisterOS = 1 << 28;
						 	  ENABLE_INTERRUPT; 	 	
        }
 
		} // if( ( !setBit ) || ( CurrentPriorityOS ==  TASKSIZE ) ) 
		else if( setBit ) 
		{
			  CountTaskOS[CurrentPriorityOS]++;
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


void initializeEventOS(void)
{
		 int          i;
	   void*        address= 0x0;
	
		 for ( i=0; i<TABLELENGTH; i++)
		 {
			  DISABLE_INTERRUPT;		
			   PriorityOwnEventOS[i] = 0x0;
         FlagAllOrAnyOS[i] = 0x0;	
			 	 DangerStackOS[i] = 0x0;		
	      ENABLE_INTERRUPT; 				 
		 }	
		 
		 for ( i=0; i<PENDSIZE; i++)
		 {
			 DISABLE_INTERRUPT;	
			   EventNumberTaskOS[i].priority = -1;	
			   EventNumberTaskOS[i].numberArray = 0x0;				
			   EventNumberTaskOS[i].readyNumber = -1;	
			   EventNumberTaskOS[i].eventType = (char)-1;
			   EventNumberTaskOS[i].privateFlag = 0x0;	
       ENABLE_INTERRUPT; 			 
		 }		 

		 for ( i=0; i<MAILSIZE; i++)
		 {
			  DISABLE_INTERRUPT;	
			   ReceiveMessageOS[i] = 0x0; 
			  ENABLE_INTERRUPT; 
		 }
		 
		 for ( i=0; i<FLAGSIZE; i++)
		 { 
			  DISABLE_INTERRUPT;	
			   PublicFlagOS[i] = 0x0; 
			  ENABLE_INTERRUPT;
		 }
		 
		 for ( i=0; i<MUTEXSIZE; i++)
		 {
			  DISABLE_INTERRUPT;
		      MutexOwnerOS[i] = -1;
			  ENABLE_INTERRUPT;
		 }	
	
		 for ( i=0; i< GETMEMORYSIZE; i++) 
		 {
			  DISABLE_INTERRUPT;	
			   StartBulkNoOS[i] =  -1; 
			   LackNoOS[i] = 0;
			   LeakNoOS[i] = 0;		
         LackNoAllOS[i] = 0;
         LeakNoAllOS[i] = 0;			 			 
			  ENABLE_INTERRUPT; 
		 }
		 	 
                      // initialize memory pool
		 for ( i=0; i<FREEBULKLENGTH; i++ ) // must before executing mallocOS()
		 {
			   DISABLE_INTERRUPT;
				  FreeBulkNoOS[i] = 0xffffffff;    // 1 is available
			   ENABLE_INTERRUPT;
		 }	
 
     for(i=0; i<QSIZE; i++)	  // must after setting FreeBulkNoOS[]
		 {
			    address = mallocOS( QBYTES );
			 
			  DISABLE_INTERRUPT;
	        QBodyOS[i].q = (void**)address;
			    QBodyOS[i].inIndex =0;
				  QBodyOS[i].outIndex =0;
				  QBodyOS[i].items =0;
			 
			    QTxRxOS[i].completeTx = 1;
			 		QTxRxOS[i].completeRx = 1;
			    QTxRxOS[i].itemsRx = 0;
			    QTxRxOS[i].power = 0;
			  ENABLE_INTERRUPT;
		 } 	

		 for ( i=0; i<UNTILSIZE; i++ ) 
		 {
			   DISABLE_INTERRUPT;
				  DelayUntilOS[i][0] = 0x0;			 
				  DelayUntilOS[i][1] = 0x0;		 
				  DelayUntilPriorityOS[i] = -1;
			   ENABLE_INTERRUPT;
		 }

}



int paddingToRamOS(int totalPadding, int taskSize)
{
	  int ramBytes;
	
	  ramBytes = ( totalPadding + PADDINGIDLE + CPUREGISTER*(taskSize+1) ) * WORDSIZE;
	
	  return ramBytes;
}



int ramToPaddingOS(int ramBytes, int taskSize)
{
	  int totalPadding;
	
	  totalPadding = ( ramBytes - CPUREGISTER*(taskSize+1)*WORDSIZE ) / (int)WORDSIZE;
	
	  return totalPadding;
}


void assignPaddingSpOS(void)
{
	  int i;
	  int totalPadding;
	  int k;
		int eachTotal;
	  int amount = 0;
	
	  for(i=0; i<TASKSIZE; i++)
		{
			  amount += PaddingOS[i];
		}
		      // assign PaddingOS[]
		if ( amount == 0 )  // use STACKRAMBYTES and AutoPackItemsOS
		{
				totalPadding =  ramToPaddingOS(STACKRAMBYTES, TASKSIZE);
	      AutoPackItemsOS = (totalPadding - PADDINGIDLE) / (int)TASKSIZE;	
			
	      for(i=0; i<TASKSIZE; i++)
		    {
			     PaddingOS[i] = AutoPackItemsOS;	
		    }	
	 	}	
		
	  PaddingOS[TASKSIZE] = PADDINGIDLE;   // idleTaskOS()				

		for(i=0; i<=TASKSIZE; i++)
		{
	      eachTotal= 0;
			
		    for(k=0; k<i; k++)
		    {
	         eachTotal += PaddingOS[k];
		    }		
				
		    TaskSpPointerOS[i] = (unsigned int*)( (int)TaskStackOS + (i*CPUREGISTER + eachTotal)*WORDSIZE );
		}		
}



char checkStartErrorOS(int arraySize, int startPriority, void (*lowPowerTimer)(void))
{
	    int  i;
		  int  amount= 0;
	    char errorCode= 0;
	
	    for(i=0; i<TASKSIZE; i++)
		  {
			    amount += PaddingOS[i];
		  }	
			
      if ( paddingToRamOS(amount, TASKSIZE) > STACKRAMBYTES )
			{
			    errorCode = 1;  
			}
	
      if ( arraySize !=  TASKSIZE  )  
		  { 	
			    errorCode = 2;     // error will stop OS
		  }			
			
      if (  (startPriority >=  TASKSIZE) || (startPriority < 0)  )  
		  { 	
			    errorCode = 3;  				
		  }
			
			if ( (lowPowerTimer != NULL) && (PADDINGIDLE < 2) )
			{
			    errorCode = 4;  
			}

			if ( (QSIZE>0) && (QLENGTH < 1) )
			{
			    errorCode = 5;  
			}
			
	    return errorCode;
}


char startOS(void (*taskName[])(void), int arraySize, int startPriority, void (*lowPowerTimer)(void) )
{ 
	  char         errorCode;
	  unsigned int topStackPointer;
	  unsigned int OSclock = TICK;
	
		errorCode = checkStartErrorOS(arraySize, startPriority, lowPowerTimer);

	  if( errorCode )
		{
			  return errorCode;
		}
		
		lowPowerTimerOS = lowPowerTimer;
    initializeSysTickOS();
    setHandlerPriorityOS();		
		initializeEventOS();
		initializeTaskOS(taskName);

		TickPerSecondOS = (int)((unsigned int)CPUclockOS / OSclock);
    CurrentPriorityOS = startPriority;			 
		topStackPointer = (unsigned int)TaskSpPointerOS[CurrentPriorityOS+1];
		setPSPOS(topStackPointer);
    setCONTROLOS(0x02);         // use PSP
		
		taskName[startPriority]();
	 
		return 0;   // never reach if start OS successfully 
 }

 

void resumeTaskOS(int priority)
{
	 DISABLE_INTERRUPT;	
	   WaitTickOS[priority] = 0;
	 ENABLE_INTERRUPT;	
		 setTableOS(PriorityOwnEventOS, priority);
		 setTableOS(ReadyTableOS, priority);  // the pending task is ready, regardless waitTick		
}


void pauseTaskOS(int tick)
{
	 DISABLE_INTERRUPT;	
	   WaitTickOS[CurrentPriorityOS] = tick;
	 ENABLE_INTERRUPT;	
		 clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);	
	   clearTableOS(ReadyTableOS, CurrentPriorityOS);		
}


int currentPriorityMapEventIndexOS(char eventType)
{
	   int i;
	   int index= -1;  
 
	       // one task can simutaneously wait(pend function) various event types(Sem, Box, or Flag).    	 
	   for (i=0; i<PENDSIZE; i++)
		 {
         if (  ( (EventNumberTaskOS[i].priority == CurrentPriorityOS) && (EventNumberTaskOS[i].eventType == eventType) ) || (EventNumberTaskOS[i].priority == -1)   )
				 {
					    index = i;
					    break;
				 }
 		 }

		 return index;
}



char IsStartPendOS(char eventType)
{
	  int  i;  
    char pend = 0;

		for( i = 0; i < PENDSIZE; i++  )   
    {
		 	  if(  (EventNumberTaskOS[i].priority >= 0) && (EventNumberTaskOS[i].eventType == eventType)  )
				{
            pend = 1;
					  break;
				}				
		 }

     return pend;		 
}


char justifyNumberArrayOS(int *array)  // terminating sign in number array
{
	   int  previousNumber = -3;
	   int  i;
	   char error = 1;  //  no terminating sign
	
	   i = 0;  										
     while( ( array[i] >= 0) && ( array[i] != previousNumber )  )								 
     { 
			    previousNumber = array[i];
		      i++;
		 } 	

     if ( array[i] < 0 )
		 {
          error = 0;
		 }	

     return error;			
}



int readReadyNumberOS(char eventType)
{
		 char setBit;	
		 int  index;	
     int  readyNumber = -1;
	
	   setBit = checkSetBitOS(PriorityOwnEventOS, CurrentPriorityOS);
	
     if ( setBit > 0 )	
		 {
		  	 index = currentPriorityMapEventIndexOS(eventType);
			   readyNumber = EventNumberTaskOS[index].readyNumber;
		 }
		 
	   return  readyNumber;
}


           // -1: timeout    -2: No terminating sign 
int pendCodeOS(int *array,char eventType)
{
	   char  terminate;
	   int   readyNumber;
	
		 terminate = justifyNumberArrayOS(array);      // error  : terminate = 1
		 readyNumber = readReadyNumberOS(eventType);   // timeout : readyNumber = -1
		 if ( terminate == 1 )
		 {
		    readyNumber = -2;  //  No terminating sign 
		 }		
				
     return readyNumber;		
}



unsigned int* queryReadyTableOS(void)
{
    return  ReadyTableOS;
}


int nonBlockingCallbackOS(int (*callback)(void))
{
	  int status;
		
	   DISABLE_INTERRUPT;
		  status = callback();
	   ENABLE_INTERRUPT;	
		
	  return status;
}



void SysTick_Handler(void)
{
     int  i;
	   char schedule=0; 
		 char resumeUntil;
	
		DISABLE_INTERRUPT;
	   SystemTickOS++;
		ENABLE_INTERRUPT;

     nonBlockingValueTransferOS();
	
     for( i=0; i< TASKSIZE; i++ )   // i is task's priority value
     {
         if ( WaitTickOS[i] >= 1 )		// if WaitTickOS[] < 0 (infinite waiting), only acquiring event can let the infinite waiting task be ready.   
         {
					 DISABLE_INTERRUPT;
				    WaitTickOS[i]--;
					 ENABLE_INTERRUPT;
         } 

				 if ( WaitTickOS[i] == 0  )	
         {
					   setTableOS(ReadyTableOS, i);
					   schedule = 1;
         } 						 
     } // for
				
     resumeUntil = checkDelayUntilOS();

		 if ( resumeUntil || schedule )
		 {  		
  	     schedulerOS();		
		 }
}



           // 0- OK,  1- too much pend functions,  3- PENDSIZE is too large
char errorPendSizeOS(void)
{
	  return ErrorPendSizeOS;
}


void deleteSelfOS(void)
{
	  delayTickOS( ISOLATE );
}


/*****************************************************************/
/*                          Stack                                */
/*****************************************************************/

int* minimumStackOS(int* minimumRam) 
{
    int i;
    int k;
    int minimumPack= 0;
    unsigned int  *spAddr;

    for ( i=0; i<= TASKSIZE; i++ )  
    {
       spAddr = TaskSpPointerOS[i];

       k = 0;				
       while(  *(spAddr + 1 + k) == IDLEITEM  )
       {
            k++;
       }
				
	    DISABLE_INTERRUPT;
       PackSizeOS[i] = PaddingOS[i] - k; 
      ENABLE_INTERRUPT;				
	
	     minimumPack += PackSizeOS[i];
				
    } // for

		if ( minimumRam != NULL )
		{
        *minimumRam = ( minimumPack + CPUREGISTER*(TASKSIZE+1) ) * WORDSIZE;
		}

		return PackSizeOS;
}


int autoMinimumStackOS(void) 
{
   int i;
   int k;
   int optimalPack;
   unsigned int  *spAddr;
	 int optimalRam;
	 int min = 0x7FFFFFFF;

   for ( i=0; i< TASKSIZE; i++ )  
   {
       spAddr = TaskSpPointerOS[i];

       k = 0;				
       while(  *(spAddr + 1 + k) == IDLEITEM )
       {
            k++;
       }
		
			 if ( k < min )
			 {
				   min = k;
			 }
    } 

	  optimalPack = (AutoPackItemsOS - min) * TASKSIZE + PADDINGIDLE;
		
    optimalRam = ( optimalPack + CPUREGISTER*(TASKSIZE+1) ) * WORDSIZE;
			
    return  optimalRam;
}


int autoPackItemsOS(void)
{
	  return AutoPackItemsOS;
}


void currentResidueOS(void) 
{
    int k = 0;
	
    while(  *(TaskSpPointerOS[CurrentPriorityOS] + 1 + k) == IDLEITEM  )
    { 
         k++;
    }

		DISABLE_INTERRUPT;
		 DeltaResidueOS[CurrentPriorityOS] = ResidueOS[CurrentPriorityOS] - k;
		 ResidueOS[CurrentPriorityOS] = k;
										
		 if ( DeltaResidueOS[CurrentPriorityOS] > MaxDeltaOS[CurrentPriorityOS] )
		 {
		      MaxDeltaOS[CurrentPriorityOS] = DeltaResidueOS[CurrentPriorityOS];
		 }	
    ENABLE_INTERRUPT;	 
}


char dangerSafeOS(int level)
{
	 char danger = 0;

	 currentResidueOS();
	
	 switch( level )
	 {   
	   case 1:
		   if ( DeltaResidueOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS] )
		   {
					  danger = 1;	 
			 }
			 break;
				
		  case 2:
		    if ( (ResidueOS[CurrentPriorityOS] < 10) || (DeltaResidueOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) )
			 	{
				  	danger = 1;	  
			  }
			  break;

	    case 3:
		    if (  (MaxDeltaOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) || (ResidueOS[CurrentPriorityOS] < 15) || (DeltaResidueOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) )
			  {
					 	danger = 1;	 	 
			 }
			 break;

		  case 4:
		    if (  (ResidueOS[CurrentPriorityOS] < 2*PaddingOS[CurrentPriorityOS]/3) || (MaxDeltaOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) || (ResidueOS[CurrentPriorityOS] < 18) || (DeltaResidueOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) )
			  {
					  danger = 1;	  
				}
			  break;						
					
			 case 5:
	       if (  (ResidueOS[CurrentPriorityOS] < 3*PaddingOS[CurrentPriorityOS]/4) || (MaxDeltaOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) || (ResidueOS[CurrentPriorityOS] < 20) || (DeltaResidueOS[CurrentPriorityOS] > ResidueOS[CurrentPriorityOS]) )
				 {
					   danger = 1;	  
				 }
				 break;
	   }

		 return danger;
}


unsigned int* irregularIdleDataOS(int *number) 
{
		unsigned int*  highAddress = 0x0;
    int  k;
	  unsigned int  *spAddr;
	  unsigned int* idleAddress[2] = {0, 0};
	
    spAddr = TaskSpPointerOS[CurrentPriorityOS];
 
	  if(  (*(spAddr + 1) == (unsigned int)IDLEITEM) || (*((unsigned int*)*spAddr -1) == (unsigned int)IDLEITEM)  )
		{
		   k = 1;				
       while(  *(spAddr + 1 + k) == (unsigned int)IDLEITEM  )
       {
          k++;
       }
		   idleAddress[0] = (unsigned int*)(spAddr + k);
		
		   k = 0;
		   while (  ( ( (unsigned int)*((unsigned int*)*spAddr -1 - k) == (unsigned int)IDLEITEM) ) && ( (unsigned int)((unsigned int*)*spAddr -1 - k) > (unsigned int)idleAddress[0] ) )
		   {
			    k++;
		   }
		   idleAddress[1] = (unsigned int*)( (unsigned int*)*spAddr -1 - k );

		   if ( ( number != NULL) && ((unsigned int)idleAddress[0] != (unsigned int)idleAddress[1] ) )
		   {
			    highAddress = idleAddress[1];
				  *number = ( ( (unsigned int)idleAddress[1] - (unsigned int)idleAddress[0] ) / sizeof(unsigned int) );
		   }
	  }
		
		return highAddress;		
}


void checkSafetyLevelOS(int level, void (*handler)(int))
{	
	  if ( level < 1 )
		{
			  level = 1;
		}
		else if ( level > MAXLEVEL )
		{
	      level = MAXLEVEL;
		}
		
		if ( (CountTaskOS[CurrentPriorityOS] >= COUNTSTART) && dangerSafeOS(level)  )
		{
        setTableOS(DangerStackOS, CurrentPriorityOS);
			  
			 DISABLE_INTERRUPT;
	      overflowHandlerOS = handler;
		   ENABLE_INTERRUPT;
			
        schedulerOS();
		}
}


int queryResidueStackOS(void)
{
	  currentResidueOS();

	  return  ResidueOS[CurrentPriorityOS];
}


char querySafetyLevelOS(void)
{
	  char level;
	  int  i;
	
	  if ( CountTaskOS[CurrentPriorityOS] >= COUNTSTART )
		{
	     for ( i=SafeLevelOS[CurrentPriorityOS]; i > 0 ; i-- )
		   {
		       if ( (i < SafeLevelOS[CurrentPriorityOS]) && dangerSafeOS(i) )
		       {
					    DISABLE_INTERRUPT;
				       SafeLevelOS[CurrentPriorityOS] = i;
					    ENABLE_INTERRUPT;	
			     }
		   }

       level = SafeLevelOS[CurrentPriorityOS];			 
		}
		else
		{
			 level = MAXLEVEL + 4;
		}
		 
		return  level;
}



int cpuRegisterRegionOS(unsigned int *context, int maxLength)
{
	   int count;
	   unsigned int sp;
	   unsigned int PSP;	
	
		 sp = *TaskSpPointerOS[CurrentPriorityOS];
	   PSP = returnPSPOS();
	
		 count = 0;
		 while ( ( (unsigned int)((unsigned int *)PSP - 1 - count) >= sp) && (count < maxLength) )
		 {
		 	    *(context + count) = *( (unsigned int *)PSP - 1 - count );
				  count++;
		 }

		 return  count;
}


int localVariableRegionOS(unsigned int *context, int maxLength)
{
	   int i;
	   int present;
	   int count = 0;
	   unsigned int *high;
	   unsigned int *low;
	   unsigned int PSP;	
	
	   high = TaskSpPointerOS[CurrentPriorityOS + 1];
		 low = (unsigned int *)*TaskSpPointerOS[CurrentPriorityOS];
	   PSP = returnPSPOS();

		 i = 0;
		 while ( (*(high -1 -i) != (unsigned int)IDLEITEM) && (count < maxLength) && ((unsigned int)(high -1 -i)>PSP) )
		 {
		 	    *(context + i) = *(high -1 -i);
			    count++;
				  i++;
		 }
	   present = i;
	
		 i = 0;
		 while ( (*(low -1 -i) != (unsigned int)IDLEITEM) && (count < maxLength) )
		 {
		 	    *(context + present + i) = *(low -1 -i);
			    count++;
				  i++;
		 }
			
		 return  count;
}


/*****************************************************************/
/*                           Delay                               */
/*****************************************************************/

void delayTickOS(int tick)
{
		if ( interruptNumberOS() == 0 )
		{	
        pauseTaskOS(tick);		 
        schedulerOS();	
		}
} 


void delayTimeOS( int hour, int minute, int second, int  mS)
{
    int tick;
	  int remainder;	

		if ( interruptNumberOS() == 0 )
		{	
	     if ( ( hour >= 0 ) && (  minute >= 0 ) && ( second >= 0 ) && ( mS >= 0 ) )
		   {
	         tick      = TickPerSecondOS * ( hour * 3600 + minute * 60 + second );
	         tick     += TickPerSecondOS * mS / 1000; 
		       remainder = TickPerSecondOS * mS % 1000; 
			 
			     if ( remainder > 500 )
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


void delayUntilEqualOS(int *a, int *b)
{
	  int  k= 0;
	  char find= 0;

		if ( interruptNumberOS() == 0 )
		{	
       if ( (a != NULL) && (b != NULL) )
       {
          while( (k<UNTILSIZE) && (!find) )
          {
       	      if( (DelayUntilPriorityOS[k] == CurrentPriorityOS) || (DelayUntilPriorityOS[k] == -1) )
       	      {
       	         DISABLE_INTERRUPT;
       	          DelayUntilOS[k][0] = a;
     	            DelayUntilOS[k][1] = b;      	   
       	          DelayUntilPriorityOS[k] = CurrentPriorityOS; 
		             ENABLE_INTERRUPT; 
				  
			            find = 1;
			            break;  	
       	      }
       	      k++;
		      }
	     	
          pauseTaskOS(INFINITE);		 
       }

       schedulerOS();	
	  }
}


void delayUntilTrueOS(int *a)
{
	  int  k= 0;
	  char find= 0;

		if ( interruptNumberOS() == 0 )
		{	
       if ( a != NULL )
       {
           while( (k<UNTILSIZE) && (!find) )
           {
       	       if( (DelayUntilPriorityOS[k] == CurrentPriorityOS) || (DelayUntilPriorityOS[k] == -1) )
       	       {
       	          DISABLE_INTERRUPT;
       	           DelayUntilOS[k][0] = a;
     	             DelayUntilOS[k][1] = 0x0;      	   
       	           DelayUntilPriorityOS[k] = CurrentPriorityOS; 
		              ENABLE_INTERRUPT; 
				  
			             find = 1;
			             break;  	
       	       }
       	       k++;
		       }
	     	
           pauseTaskOS(INFINITE);		 
       }

       schedulerOS();	
	  }
}


char checkDelayUntilOS(void)
{
	 int  k;
	 char resume= 0;
	
	 for(k=0; k<UNTILSIZE; k++)	
	 {                                 // until true
		 	 if(  (*DelayUntilOS[k][0] != 0) && (DelayUntilOS[k][1] == 0x0) &&(DelayUntilPriorityOS[k] >= 0)  )
	     {	
	    	   resumeTaskOS( DelayUntilPriorityOS[k] );
			     resume = 1;
		   } 
			                              // until equal
	     else if(  (*DelayUntilOS[k][0] == *DelayUntilOS[k][1]) && (DelayUntilPriorityOS[k] >= 0)  )
	     {	
	    	   resumeTaskOS( DelayUntilPriorityOS[k] );
			     resume = 1;
		   } 
	 } 
	 
	 return resume;
}


/*****************************************************************/
/*                          Semaphore                            */
/*****************************************************************/

void postSemOS(int number)
{
	  int i;     
	  int priority;
	  int *array;	
    int previousValue;	
	  int k;
	  int index;

	  if (  number >= 0 )
		{	
				 clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);
			   index = currentPriorityMapEventIndexOS(SEM);
			
		     if( index >= 0 )
		     {
					  DISABLE_INTERRUPT;	
				      EventNumberTaskOS[index].readyNumber = -1;
            ENABLE_INTERRUPT;					 
				 }						
				 
			   if(  IsStartPendOS(SEM) )
				 {			
		        for( i = 0; i < PENDSIZE; i++  )   
            {
							 if ( EventNumberTaskOS[i].eventType == SEM )
							 {
				           priority =  EventNumberTaskOS[i].priority;	

								   if ( (priority != CurrentPriorityOS) && (WaitTickOS[priority] >= (int)INFINITE) )
							     {  
								       array = EventNumberTaskOS[i].numberArray;
 								       previousValue = -999;
                       k = 0;   
						 
                       while( (array != NULL) &&( array[k] >= 0) && ( array[k] != previousValue ) )								 
                       { 
										      if( array[k] == number )
										   	  {
											   	  DISABLE_INTERRUPT;			
			                       EventNumberTaskOS[i].readyNumber = number;	
								            ENABLE_INTERRUPT;
												
												     resumeTaskOS(priority);	
                             break;														
											     }
										       else
											     {
										          previousValue =  array[k];
											     }
											
									         k++;
													
								       } // while
										
								    } // if ( priority != CurrentPriorityOS )
														
								 } // if ( EventNumberTaskOS[i].eventType == SEM )
	
            } // for	

				 } //	if(  IsPending(SEM) )
		
	   } // if (  number >= 0 )
	 
		 if ( interruptNumberOS() == 0 )
		 {	
         schedulerOS();
			   CountTaskOS[CurrentPriorityOS]--;	
		 }		 
}


int pendSemOS(int *array, int timeout)
{
    int  index;
	  int  readyNumber= -1;
	
	  if ( interruptNumberOS() == 0 )
		{
		    index = currentPriorityMapEventIndexOS(SEM);				 

		    if( index >= 0 )
		    {
		       DISABLE_INTERRUPT;
			 	     EventNumberTaskOS[index].priority = CurrentPriorityOS;					 
				     EventNumberTaskOS[index].numberArray = array; 
				     EventNumberTaskOS[index].eventType = SEM;					
		       ENABLE_INTERRUPT;	
	 
            pauseTaskOS(timeout);		
	
	          schedulerOS();
		    }
				else
				{
					DISABLE_INTERRUPT;
					  ErrorPendSizeOS = 1;
					ENABLE_INTERRUPT;
				}
				
			  readyNumber = pendCodeOS(array, SEM);
				
	  } // if ( interruptNo == 0 )
		 
    return readyNumber;	
}


/*****************************************************************/
/*                         Mail Message                          */
/*****************************************************************/

                 // message must be the address of global variable
void postMailOS(int number, void *messageAddr)
{
	  int i;     
	  int priority;
	  int *array;	
    int previousValue;	
	  int k;
	  int index;

	  if (  (number >= 0) && (number < MAILSIZE) )
		{	
			   ReceiveMessageOS[number] = messageAddr;	
				 clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);
			   index = currentPriorityMapEventIndexOS(MAIL);	
			
		     if( index >= 0 )
		     {
					  DISABLE_INTERRUPT;
				      EventNumberTaskOS[index].readyNumber = -1;	
            ENABLE_INTERRUPT;					 
				 }		
				 
			   if(  IsStartPendOS(MAIL) )
				 {			
		        for( i = 0; i < PENDSIZE; i++  )   
            {
							 if ( EventNumberTaskOS[i].eventType == MAIL )
							 {
				           priority =  EventNumberTaskOS[i].priority;	

						       if ( (priority != CurrentPriorityOS) && (WaitTickOS[priority] >= (int)INFINITE) )
							     {
								       array = EventNumberTaskOS[i].numberArray;
 								       previousValue = -999;
                       k = 0;   
										 
                       while( (array != NULL) && ( array[k] >= 0) && ( array[k] != previousValue ) )								 
                       { 
										      if( array[k] == number )
										   	  {
											   	  DISABLE_INTERRUPT;		
			                        EventNumberTaskOS[i].readyNumber = number;	
								             ENABLE_INTERRUPT;
												
												      resumeTaskOS(priority);		
                              break;															
											     }
										       else
											     {
										           previousValue =  array[k];
											     }
											
									         k++;
													
								       } // while
										
								    } // if ( priority != CurrentPriorityOS )
									
								 } // if ( EventNumberTaskOS[i].eventType == MAIL )
	
             } // for	

				 } //	if(  IsPending(MAIL) )
		
	   } // if (  number >= 0 )

		 if ( interruptNumberOS() == 0 )
		 {	
         schedulerOS();
			   CountTaskOS[CurrentPriorityOS]--;	
		 }		 
}


void* readMailOS(int number, char clear)
{
	  void* address = 0x0;

		if ( (number >= 0) && (number < MAILSIZE) )
		{
	     address = ReceiveMessageOS[number];
	
	     if( clear )
		   {
			   DISABLE_INTERRUPT;
			     ReceiveMessageOS[number] = 0x0;
			   ENABLE_INTERRUPT;	
		   }
	 }

		 return  address;
}


void* pendMailOS(int *array, int *readyNumberAddr, char clear, int timeout)
{
    int   index;
	  int   readyNo;
	  void* address= 0x0;
	
	  if ( interruptNumberOS() == 0 )
		{
		    index = currentPriorityMapEventIndexOS(MAIL);				 

		    if( index >= 0 )
		    {
		       DISABLE_INTERRUPT;
			 	     EventNumberTaskOS[index].priority = CurrentPriorityOS;					 
				     EventNumberTaskOS[index].numberArray = array; 
				     EventNumberTaskOS[index].eventType = MAIL;					
		       ENABLE_INTERRUPT;	
	 
            pauseTaskOS(timeout);		
	
	          schedulerOS();
		    }
				else
				{
					DISABLE_INTERRUPT;
					  ErrorPendSizeOS = 1;
					ENABLE_INTERRUPT;
				}
				
				readyNo = pendCodeOS(array, MAIL);	
				
				if ( readyNumberAddr != NULL )
				{
					  *readyNumberAddr = readyNo;
				}
				
				if( (readyNo >= 0) && (readyNo < MAILSIZE) )
				{
				    address = readMailOS(readyNo, clear);
				}
				
	  } // if 
		 
		return address;
}



/*****************************************************************/
/*                           Mutex                               */
/*****************************************************************/

void postMutexOS(void)
{
	  int i;
	  int number;
	  int index;
	  int priority;
	  int minPriority = 0x7FFFFFFF ;
	  int minIndex = -1;
	  int *array;	
			 
		if( interruptNumberOS() == 0 )  // ISR can not call postMutexOS()
		{		
	     index = currentPriorityMapEventIndexOS(MUTEX);	
	     if( index >= 0 )
	     {
		   	  number = EventNumberTaskOS[index].readyNumber;  // own the mutex number
			   	if( (number >= 0) && (number < MUTEXSIZE) && (MutexOwnerOS[number] == CurrentPriorityOS) )
			   	{
					   clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);
					  DISABLE_INTERRUPT;	
				     EventNumberTaskOS[index].readyNumber = -1;
						 MutexOwnerOS[number] = -1;		 // mutex is free	
					  ENABLE_INTERRUPT;		
			
						 if( IsStartPendOS(MUTEX) )
						 {
		            for( i = 0; i < PENDSIZE; i++  )   // set the highest priority task ready
                {
							     if ( EventNumberTaskOS[i].eventType == MUTEX )
							     {
				               priority =  EventNumberTaskOS[i].priority;	

						           if ( (priority != CurrentPriorityOS) && (WaitTickOS[priority] >= (int)INFINITE) )
							         {
							             array = EventNumberTaskOS[i].numberArray;   
 
									         if(  (array[0] == number) && (priority < minPriority) )
									         {
												      minPriority = priority;
												      minIndex = i;													
								           }
							         } // if ( priority != CurrentPriorityOS )
														
							     } // if ( EventNumberTaskOS[i].eventType == SEM )
	
                } // for	
					   } // if( IsStartPendOS(MUTEX) )
						
						 if( minIndex >= 0 )
						 {
							  DISABLE_INTERRUPT;	
							   MutexOwnerOS[number] = minPriority;						
			           EventNumberTaskOS[minIndex].readyNumber = number;	
							 	 EventNumberTaskOS[minIndex].numberArray = 0x0;	
							  ENABLE_INTERRUPT;
												
							   resumeTaskOS(minPriority);							
						 }					
						
	        } // if(  (number >= 0) && (number < MUTEXSIZE)  )
				
	     } // if( index >= 0 )					 
				 
		   schedulerOS();	 
			 CountTaskOS[CurrentPriorityOS]--;			 
		} // if( interruptNumberOS
}


int pendMutexOS(int *array, int timeout)
{
     int  index;
	   int  number;
	   int  readyNumber= -1;

	   if ( interruptNumberOS() == 0 )
		 {
	      index = currentPriorityMapEventIndexOS(MUTEX);				 

		    if( index >= 0 )
	      {
					   number = array[0];  // only one mutex
					   if( MutexOwnerOS[number] == CurrentPriorityOS )
						 {
							    return number;
						 }
						 
		       DISABLE_INTERRUPT;
			 	     EventNumberTaskOS[index].priority = CurrentPriorityOS;					 
				     EventNumberTaskOS[index].numberArray = array; 
				     EventNumberTaskOS[index].eventType = MUTEX;					
		       ENABLE_INTERRUPT;	
			 
			       if( (number >= 0) && (number < MUTEXSIZE) && (MutexOwnerOS[number] < 0) )
				     {
							   setTableOS(PriorityOwnEventOS, CurrentPriorityOS);
		           DISABLE_INTERRUPT;											
                 MutexOwnerOS[number] = CurrentPriorityOS;
							   EventNumberTaskOS[index].numberArray = 0x0;		
			           EventNumberTaskOS[index].readyNumber = number;
		           ENABLE_INTERRUPT;												
			       } 
	           else
			   	   {		
                 pauseTaskOS(timeout);		
	
	               schedulerOS();
			   	   }
				 
		     }	// if( index >= 0 )
				 else
				 {
					 DISABLE_INTERRUPT;
					   ErrorPendSizeOS = 1;
					 ENABLE_INTERRUPT;
				 }
				
				 readyNumber = readReadyNumberOS(MUTEX);
		 
		 } // if ( interruptNumberOS() == 0 )
		 
     return readyNumber;		// the only one (array) element
}


/*****************************************************************/
/*                            Flag                               */
/*****************************************************************/

void postFlagOS(int number, unsigned int modifyPublicFlag, char setOrClear )
{
	  int i;     
	  int priority;
	  unsigned int interestBits;
	  char AllOrAny;
	  char match;	
	  int *array;	
    int previousValue;	
	  int k;	
	  int index;

		if (  (number >= 0) && (number < FLAGSIZE)  )
		{
		   DISABLE_INTERRUPT;	
         switch (setOrClear)  // set/clear  PublicFlagOS[]
			   {
					  case FLAG_CLEAR:  // 0
							 PublicFlagOS[number] &= ~modifyPublicFlag;	// clear the interest bits						 
					     break;
									 
					  case FLAG_SET:  
							 PublicFlagOS[number] |= modifyPublicFlag;   // set the interest bits
							 break;
				 }
		   ENABLE_INTERRUPT;	
				 
				 clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);
			   index = currentPriorityMapEventIndexOS(FLAG);	
				 
		     if( index >= 0 )
		     {
		        DISABLE_INTERRUPT;
				      EventNumberTaskOS[index].readyNumber = -1;
				      EventNumberTaskOS[index].privateFlag = 0;		// reset private flag		
					  ENABLE_INTERRUPT;					
				 }					

			   if(  IsStartPendOS(FLAG) )
				 {				
		        for( i = 0; i < PENDSIZE; i++  )   
            {
							 if ( EventNumberTaskOS[i].eventType == FLAG )
							 {	 
								  priority = EventNumberTaskOS[i].priority;

								  if ( (priority != CurrentPriorityOS) && (WaitTickOS[priority] >= (int)INFINITE) )
							    {
								     array = EventNumberTaskOS[i].numberArray;
 								     previousValue = -999;
                     k = 0;   
										
                     while( (array != NULL) && (array[k] >= 0) && (array[k] != previousValue) )								 
                     {									
								         if( array[k] == number )
	         		           {
									           interestBits	= PublicFlagOS[number] & EventNumberTaskOS[i].privateFlag; // compare bit value 1

										         match = 0;
										         AllOrAny = checkSetBitOS(FlagAllOrAnyOS, priority);
											
								 		         switch(AllOrAny)
										         {
												        case FLAG_MATCH_ALL:   // 1,  public flag bits match with ALL the task's interest bits( value 1 )
                                    if ( (interestBits != 0) && (interestBits == EventNumberTaskOS[i].privateFlag) )  // every bit is match
														        {
                       		               match = 1;	
														        }
														        break;
												
														    case FLAG_MATCH_ANY:  // 0,  public flag bits match with ANY ONE( value 1 ) of the task's interest bits
														        if ( interestBits != (unsigned int) 0 )  // at least one bit match with(value 1)
											  		        {
                        		             match = 1;	
											 		          }													 
											  		        break;
							  		          } // switch
										
										          if ( match )
								 		          {
												       	 DISABLE_INTERRUPT;	
					           		           EventNumberTaskOS[i].readyNumber = number; 
																   EventNumberTaskOS[i].privateFlag = 0;		// reset private flag							 
													       ENABLE_INTERRUPT;		
														
                     		           resumeTaskOS(priority);	
                                   break;																	
										          }
									 
								          } // if( array[arrayIndex] == number )
										      else
											    {
										           previousValue =  array[k];
											    }
											
									        k++;
													
								      } // while												 
													
							     } // if(   priority != CurrentPriorityOS   )									 
													
							 } // if ( EventNumberTaskOS[i].eventType == FLAG )
									 
					} // for
				
		   } // if(  IsPending(FLAG) )
			   
 		 }	//  if ( number < FLAGSIZE )	
	 
		 if ( interruptNumberOS() == 0 )
		 {	
         schedulerOS();
			   CountTaskOS[CurrentPriorityOS]--;	
		 }			 
}


int pendFlagOS(int *array, unsigned int privateFlag, char allOrAny, int timeout)
{
	   int  index;
	   int  readyNumber= -1;

	   if ( interruptNumberOS() == 0 )
		 {	
	   		 index = currentPriorityMapEventIndexOS(FLAG);				 

		    if( index >= 0 )
		    {
		         DISABLE_INTERRUPT;
			 	       EventNumberTaskOS[index].priority = CurrentPriorityOS;					 
				       EventNumberTaskOS[index].numberArray = array; 
				       EventNumberTaskOS[index].privateFlag = privateFlag;						 
				       EventNumberTaskOS[index].eventType = FLAG;					
			       ENABLE_INTERRUPT;	
					
               if ( allOrAny == FLAG_MATCH_ALL )	// 1- match with All of bits    0- match with ANY one bit
			         {
 			             setTableOS(FlagAllOrAnyOS, CurrentPriorityOS);  // match with all of bits(1) or any one bit(0)
			         }
	 			       else
				       {
				           clearTableOS(FlagAllOrAnyOS, CurrentPriorityOS); 
				       }	
				 
               pauseTaskOS(timeout);		
	
	             schedulerOS();
			   }
				 else
				 {
						 DISABLE_INTERRUPT;
					     ErrorPendSizeOS = 1;
						 ENABLE_INTERRUPT;	
				 }		
					
		     readyNumber = pendCodeOS(array, FLAG);						
			
	   } // if 
		 
     return readyNumber;	
}



unsigned int queryPublicFlagOS(int flagNumber)
{
	  return PublicFlagOS[flagNumber];
}


                         // return value is either 1 or 0
char checkPublicFlagBitOS(int flagNumber, char bitNumber)
{
	  return  checkSetBitOS( &PublicFlagOS[flagNumber], bitNumber);
}


/*****************************************************************************/
/*                             Memory Management                             */
/*****************************************************************************/



void* memoryAddressOS(int bulkNo)
{
    void* bulkAddress;		
	
		bulkAddress = (void*)((unsigned int)PoolOS + (unsigned int)(bulkNo * BULKBYTES) );

	  return bulkAddress;
}



int findFreeMemoryOS(int desiredBulkLength)
{
	  int  k;
		int  i = 0;
		int  length1;    // continue free bulk
    int  margeNo = -1;
		char setBit;
	
		if ( (GLOBALBULKLENGTH > 1) && (desiredBulkLength < GLOBALBULKLENGTH) )
		{
			 k =0;
			 while ( (FreeBulkNoOS[k] == 0) || (FreeBulkNoOS[k] == 0x80000000) )
			 {
				 	 i = (FreeBulkNoOS[k] == 0x80000000) ? 8 * sizeof(unsigned int) * (k + 1) - 1 : 8 * sizeof(unsigned int) * (k + 1);
				   k++;
			 }

       while( i < GLOBALBULKLENGTH-1 )
		   {
		       if( checkSetBitOS(FreeBulkNoOS, i) )
		       { 
						   length1 = 1;
					     setBit = checkSetBitOS(FreeBulkNoOS, i+1); // find continuous free bulk
							 
	             while( setBit && (i+length1 < GLOBALBULKLENGTH) && (length1 <= desiredBulkLength + 1) )
		           {
			             length1++;
						       if( i+length1 < GLOBALBULKLENGTH )
							     {
					             setBit = checkSetBitOS(FreeBulkNoOS, i+length1);	// check continuous free bulk	
                   }										
		           } 
 
               if( (length1 > desiredBulkLength+1) || ( (i==0) && (length1 == desiredBulkLength+1) ) )
			         { 
							     margeNo = i;
								   break;
			         }

							 i += length1+1;				 
				   } 
				   else
				   {
					     i++;
				   } // if( checkSetBitOS
						
		    }	// while
		 } // if ( GLOBALBULKLENGTH > 1 )
		 else if ( (desiredBulkLength < 2) && checkSetBitOS(FreeBulkNoOS, 0) )
		 {
			   margeNo = 0;
		 }

	   return  margeNo;
}


int memoryOS(int getNo, int bytes)
{
	  int  i;           // bulk number of available memory
 	  int  margeNo;     // marge bulk number
	  int  desiredBulkLength;
	  int  frontBulkNo;
	  int  rearBulkNo;
	  int  realBytes = 0;

		if( (bytes > 0) && (getNo < GETMEMORYSIZE) && (getNo >= (int)MALLOC) )
		{
			 desiredBulkLength = bytes % BULKBYTES ? bytes / BULKBYTES+1 : bytes / BULKBYTES ;	
	     if( desiredBulkLength == 0 )
		   {
			    desiredBulkLength = 1;
		   }
       margeNo = findFreeMemoryOS(desiredBulkLength);
	
			 if ( margeNo >= 0 )
		   {	 
                     //  using   free   using    
                     // 0 0 0 1 1 1 1 1 0 0  FreeBulkNoOS[margeNo]=1
                     //       |     |   |         
						         //       |     |   margeNo+length1    length1=5, desiredBulkLength=3
                     //  margeNo rearBulkNo

					  frontBulkNo = margeNo<1 ? 0 : margeNo+1;  // if margeNo=0, margeNo is available bulk

						rearBulkNo = margeNo<1 ? desiredBulkLength-1 : margeNo + desiredBulkLength;				

				 	  for(i= frontBulkNo; i<= rearBulkNo; i++ ) //  FreeBulkNoOS[margeNo] = 1
				    {
			         clearTableOS(FreeBulkNoOS, i);			// using
				    }
						
	 	      DISABLE_INTERRUPT;						
				    if( getNo >= 0 )
				  	{  
							 StartBulkNoOS[getNo] = frontBulkNo;
				       MemoryFreeAddressOS = memoryAddressOS(frontBulkNo); 
				    }
				    else if( getNo == (int)MALLOC )
				    {
					     MemoryFreeAddressOS = memoryAddressOS(frontBulkNo);
				    }
				 ENABLE_INTERRUPT; 
					 
					  realBytes = ( rearBulkNo - frontBulkNo + 2 ) * (int)BULKBYTES;
		   } 
			 else  if ( getNo >= 0 )
			 {
				   DISABLE_INTERRUPT;	
				    LackNoOS[getNo] = 1;
				    LackNoAllOS[getNo]++;
				   ENABLE_INTERRUPT;
			 } // if ( margeNo >= 0 )
			 
	  } // if( (bytes > 0) 
	
		return realBytes;
}


void* getMemoryOS(int getNo, int bytes, int *realBytes)
{
	   int getBytes = 0;
	
	   MemoryFreeAddressOS = 0x0;
	
		 if( (getNo >= 0) && (getNo < GETMEMORYSIZE) )
		 { 
	      if( StartBulkNoOS[getNo] < 0 )
		    {	
            getBytes = memoryOS(getNo, bytes);
	      } 
		    else   // memory leak
		    {
					DISABLE_INTERRUPT;
           LeakNoOS[getNo] = 1;
					 LeakNoAllOS[getNo]++;
          ENABLE_INTERRUPT;					
		    }
	   }

		 if ( realBytes != NULL )
		 {
		     *realBytes = getBytes;
		 }
	 
		 return  MemoryFreeAddressOS;
}


void* mallocOS(int bytes)
{		
	  MemoryFreeAddressOS = 0x0;
	  memoryOS(MALLOC, bytes);

		return  MemoryFreeAddressOS;
}



void putMemoryOS(int getNo)
{
	  int   k;
	  int   frontBulkNo;
	  char  setBit;	

	  if( (getNo >= 0) && (getNo < GETMEMORYSIZE) )
		{
			  frontBulkNo = StartBulkNoOS[getNo];
				StartBulkNoOS[getNo] = -1;
			
        if( (frontBulkNo >= 0) && (frontBulkNo < GLOBALBULKLENGTH) )
			  { 
			      if( !checkSetBitOS(FreeBulkNoOS, frontBulkNo) )
				    {  
			          for( k = frontBulkNo; k<GLOBALBULKLENGTH; k++)
			          {
					          setBit = checkSetBitOS(FreeBulkNoOS, k);
			 	            if ( setBit  ) // k is margeBulkNo
					          {							
           	           break;									
					          }
											
			  	          setTableOS(FreeBulkNoOS, k); 			
			          }
				
			      } // if( !checkSetBitOS(
		
				} // if( (bulkNo >= 0) &&
	  } 
}


void freeOS(void* ptr)
{
	  int   k;
	  int   frontBulkNo= -1;
	  char  setBit;	

	  for( k=0; k<GLOBALBULKLENGTH; k++)
	  {
				if( (int)ptr == (int)memoryAddressOS(k) )
				{
            frontBulkNo = k;
					  break;
				}									
		}								

    if( frontBulkNo >= 0 )
		{		
			  if( !checkSetBitOS(FreeBulkNoOS, frontBulkNo) )
				{
			      for( k= frontBulkNo; k<GLOBALBULKLENGTH; k++)
			      {
					      setBit = checkSetBitOS(FreeBulkNoOS, k);
			 	        if ( setBit  ) // k is margeBulkNo
					      {
           	           break;									
					      }
							
			  	       setTableOS(FreeBulkNoOS, k); 			
			      }
				
			  } // if( !checkSetBitOS(
		}						
}


void* getMemoryWithPutOS(int getNo, int bytes, int *realBytes)
{		
	  putMemoryOS(getNo);	

		return  getMemoryOS(getNo, bytes, realBytes);
}



int queryFreeBulkNoOS(char* result, int length)
{
	 int i;
	
	 if ( length <= GLOBALBULKLENGTH )
	 {
	     for (i=0; i<length; i++)
	     {
		       result[i] = checkSetBitOS(FreeBulkNoOS, i);
	     }		 
	 }
	 else
	 {
			 for (i=0; i<GLOBALBULKLENGTH; i++)
	     {
		       result[i] = checkSetBitOS(FreeBulkNoOS, i);
	     }
			 
			 for (i=GLOBALBULKLENGTH; i<length; i++)
	     {
		       result[i] = 9;
	     }			 
	 }
	 
	 return GLOBALBULKLENGTH;
}


int leakMemoryNoOS(void)
{
	  int i;
	  int leakNo = -1;    // OK
	
	  for (i=0; i<GETMEMORYSIZE; i++)
	  {
			  if ( LeakNoOS[i] > 0 )
				{
					  leakNo = i;
					  LeakNoOS[i] = 0;
					  break;
				}
		}
	  return leakNo;
}


int lackMemoryNoOS(void)
{
	  int i;
	  int lackNo = -1;    // OK
	
	  for (i=0; i<GETMEMORYSIZE; i++)
	  {
			  if ( LackNoOS[i] > 0 )
				{
					  lackNo = i;
					  LackNoOS[i] = 0;
					  break;
				}
		}
	  return lackNo;
}


int* leakAllOS(void)
{
	  return LeakNoAllOS;
}


int* lackAllOS(void)
{
	  return LackNoAllOS;
}

/*****************************************************************/
/*                            Queue                              */
/*****************************************************************/


int postQOS(int number, void *messageAddr)
{
	  int i;     
	  int priority;
	  int inIndex;
	  int *array;	
	  int k;	
    int previousValue;
	  int remainItems = 0;

		if ( (number >= 0) && (number < QSIZE) && ( QBodyOS[number].q != 0x0) && (QBodyOS[number].items < QLENGTH) )
		{		
				 clearTableOS(PriorityOwnEventOS, CurrentPriorityOS);	
	     DISABLE_INTERRUPT;
		   	 inIndex = QBodyOS[number].inIndex++;		
			   QBodyOS[number].q[inIndex] = messageAddr; // user must handle the data type.	
			   QBodyOS[number].items++;

				 if(  QBodyOS[number].inIndex >= QLENGTH )
			   {
				      QBodyOS[number].inIndex = 0;				
			   }
		   ENABLE_INTERRUPT;
				 
				 remainItems = QLENGTH - QBodyOS[number].items;
		
				 if(  IsStartPendOS(QUEUE) )
				 { 
		        for( i = 0; i < PENDSIZE; i++  )   
            {
							 if ( EventNumberTaskOS[i].eventType == QUEUE )
							 {
				          priority = EventNumberTaskOS[i].priority;	
								 
						      if ( (priority != CurrentPriorityOS) && (WaitTickOS[priority] >= (int)INFINITE) )
					        {
							       array = EventNumberTaskOS[i].numberArray;
	 							     previousValue = -999;
                     k = 0; 

                     while( (array != NULL) && (array[k] >= 0) && (array[k] != previousValue) )								 
                     {
 									       if( array[k] == number )
							           {
							                resumeTaskOS(priority);	
                              break;														 
						  	          } 
											    else
											    {
										           previousValue =  array[k];
											    }	

										      k++;	
													
							       } // while
							
						      } // if ( priority != CurrentPriorityOS )
								
							  } // if ( EventNumberTaskOS[i].eventType == Q )

             } // for	
				 
				  }	// if(  IsPending(Q) ) 

	   } // if ( number >= 0 )
		 
		 if ( interruptNumberOS() == 0 )
		 {	
         schedulerOS();
			   CountTaskOS[CurrentPriorityOS]--;	
		 }		 

		 return remainItems;
}



void* readQOS(int number, int* items)
{
	  int   i;
	  int   outIndex;
	  char  setBit;
	  void* retrieveAddress = 0x0;
	
	  if ( items != NULL )
		{
	      *items = 0;
		}
		
		setBit = checkSetBitOS(PriorityOwnEventOS, CurrentPriorityOS);
	
    if((setBit > 0) && (number >= 0) && (number < QSIZE) && (QBodyOS[number].items > 0))
		{ 
		     for(i=0; i<QBodyOS[number].items; i++)
		     {
	            outIndex = QBodyOS[number].outIndex++;
					  DISABLE_INTERRUPT;	
              qRetrieveOS[number][i] = QBodyOS[number].q[outIndex];						 
					 
		          if( QBodyOS[number].outIndex >= QLENGTH )
			        {
					        QBodyOS[number].outIndex = 0;
			        }
						ENABLE_INTERRUPT;						
		     } // for	
				
				 retrieveAddress = qRetrieveOS[number];
				 if ( items != NULL )
		     {
				     *items = QBodyOS[number].items;	
		     }
				DISABLE_INTERRUPT;	
				 QBodyOS[number].items = 0;   // clear message
				ENABLE_INTERRUPT;		
					
		} // if(  (setBit > 0)	&& (number >= 0)  )
		 
	  return retrieveAddress;
}


int findItemNumberOS(int *array)
{
	   int  k = 0;	
     int  previousValue = -999;		
     int  number;	
  
		 number = array[0];
	
     while( ( number >= 0) && (number < QSIZE) && ( number != previousValue ) && (QBodyOS[number].items == 0) )								 
     { 
          previousValue = number;
          k++;
			    if ( (array[k] >= 0) && (array[k] < QSIZE) )
					{
		           number = array[k];	
					}										
		 }	
	             // number == previousValue
		 if ( (number >= 0) && (number < QSIZE) )
		 {
		    if ( QBodyOS[number].items == 0 )
		    {
			      number = -1;   // no item
		    }
	   }
		 else
		 {
			    number = -1;
		 }
		 
		 return number;
}


void* pendQOS(int *array, int* readyNo, int* items, int timeout)
{
   int   index;		
   int   number = -1;
	 void* retrieveAddress = 0x0;

	 if ( interruptNumberOS() == 0 )
	 {	
	   	index = currentPriorityMapEventIndexOS(QUEUE);				 

		  if( index >= 0 )
		  { 
		     DISABLE_INTERRUPT;
			 	  EventNumberTaskOS[index].priority = CurrentPriorityOS;					 
				  EventNumberTaskOS[index].numberArray = array; 
				  EventNumberTaskOS[index].eventType = QUEUE;					
		     ENABLE_INTERRUPT;	

				  number = findItemNumberOS(array);
					                
			    if( (number >= 0) && (number < QSIZE) )  // find items
				  { 										
							setTableOS(PriorityOwnEventOS, CurrentPriorityOS);	
			    } 			
          else
			   	{ 
              pauseTaskOS(timeout);		
	            schedulerOS();
								                 // wake up from pending
					    number = findItemNumberOS(array);										
			   	} // else
							
					if( number >= 0 )  // find items
					{ 
					  DISABLE_INTERRUPT;
		//				  SleepPendQOS[number] = pend;         
							retrieveAddress = readQOS(number, items);         									
						ENABLE_INTERRUPT;	
					} // if( number >= 0 )
							
		   } // if( index >= 0 )
			 else
			 { 
				 DISABLE_INTERRUPT;
				   ErrorPendSizeOS = 1;
				 ENABLE_INTERRUPT;
			 }

	 } // if	 

   if ( readyNo != NULL )		
	 {			 
		  *readyNo = number;
	 }
		 
   return retrieveAddress; 	
}


int qReadyNumberOS(void* retrieveAddress)
{
	  int i;
	  int number= -1;
	
	  for(i=0; i<QSIZE; i++)
		{
			  if( (int)retrieveAddress == (int)&qRetrieveOS[i][0] )
				{
					   number = i;
				}
		}
		
		return number;
}


void qTxRealtimeOS(int number, void *messageAddr)
{
	  postQOS(number, messageAddr);
}


void* qRxRealtimePendOS(int number)
{
	  pendQOS(&number, NULL, NULL, INFINITE);;
	
	  return  qRetrieveOS[number][0];
}



int queryRemainItemsOS(int number)
{
   return QLENGTH - QBodyOS[number].items;
}

       // value communication

void qTxValueOS(int qNo, void* pData, int length, char power)
{
	DISABLE_INTERRUPT;
	  if ( (qNo < QSIZE) && (qNo >= 0) && (QTxRxOS[qNo].completeTx) && (length > 0) )
	  {
		   QTxRxOS[qNo].itemsTx = 0;
	     QTxRxOS[qNo].dataTx = pData;
	     QTxRxOS[qNo].length = length;
	     QTxRxOS[qNo].power = power;
		   QBodyOS[qNo].inIndex = 0;
			
			 QTxRxOS[qNo].completeTx = 0;	 
	  }
		else if ( length == 0 )
		{
			  QTxRxOS[qNo].length = 0;
		}
	ENABLE_INTERRUPT;
}


int writeQOS(int qNo, void *messageAddr, int* remainTxItems)
{
	  int inIndex;
	  int remainQItems = 0;

		if ( ( QBodyOS[qNo].q != 0x0) && (QBodyOS[qNo].items <= QLENGTH) )
		{	
	     DISABLE_INTERRUPT;
		   	 inIndex = QBodyOS[qNo].inIndex++;		
			   QBodyOS[qNo].q[inIndex] = messageAddr; 
			   QBodyOS[qNo].items++;
         QTxRxOS[qNo].itemsTx++;
			
				 if(  QBodyOS[qNo].inIndex >= QLENGTH )
			   {
				      QBodyOS[qNo].inIndex = 0;				
			   }
		   ENABLE_INTERRUPT;
				 
				 remainQItems = QLENGTH - QBodyOS[qNo].items;
				 *remainTxItems = QTxRxOS[qNo].length - QTxRxOS[qNo].itemsTx;
	   } 
 
		 return remainQItems;
}


void qTxOS(void)
{
	 int  i;
	 char m;
	 int  multiplier;
	 int  intData;
	 int  remainTxItems;
	 int  remainQItems;
	
	 for (i=0; i<QSIZE; i++)
	 {
		   multiplier = 1;
		 	 for (m=0; m < QTxRxOS[i].power; m++)
	     {
		       multiplier *= 10;		
	     }
			 
		   if ( !QTxRxOS[i].completeTx )
		   { 
				  remainTxItems = QTxRxOS[i].length - QTxRxOS[i].itemsTx;

          if ( queryRemainItemsOS(i) && remainTxItems )
	        {		 
		         intData = QTxRxOS[i].power ? (int)(( *((float*)QTxRxOS[i].dataTx + QTxRxOS[i].itemsTx) ) * (float)multiplier ) : *((int*)(QTxRxOS[i].dataTx) + QTxRxOS[i].itemsTx);
			       remainQItems = writeQOS(i, (void*)intData, &remainTxItems);
						 
	           while ( remainQItems && remainTxItems )
		         {
                intData = QTxRxOS[i].power ? (int)(( *((float*)QTxRxOS[i].dataTx + QTxRxOS[i].itemsTx) ) * (float)multiplier ) : *((int*)(QTxRxOS[i].dataTx) + QTxRxOS[i].itemsTx);			  
				        remainQItems = writeQOS(i, (void*)intData, &remainTxItems);
	           }
          } 
		   } 
	 } 
}

	 
void qRxValueOS(int qNo, void* pData)
{
	DISABLE_INTERRUPT;	
	  if ( (qNo < QSIZE) && (qNo >= 0) && (QTxRxOS[qNo].completeRx) )
	  { 
		 	 QTxRxOS[qNo].itemsRx = 0;
			 QTxRxOS[qNo].dataRx = pData;	
		   QBodyOS[qNo].outIndex = 0;
			
			 QTxRxOS[qNo].completeRx = 0;	
		   FlagTxRxOS = 1;
	  }
	ENABLE_INTERRUPT;	
}
				 
				 
void qRxOS(void)
{
	 int  i;
	 char m;
	 int  k;
	 int  outIndex;
	 int  multiplier;

	 for (i=0; i<QSIZE; i++)
	 {
	     multiplier = 1;
	     for (m=0; m < QTxRxOS[i].power; m++)
	     {
		      multiplier *= 10;		
	     }
	 		
			 if ( !QTxRxOS[i].completeRx )
		   {   
	        for(k=0; k < QBodyOS[i].items; k++)
	        {
			      DISABLE_INTERRUPT;	
	           outIndex = QBodyOS[i].outIndex++;

             if ( multiplier == 1 )    // int
			       {
				        *((int*)QTxRxOS[i].dataRx + QTxRxOS[i].itemsRx) = (int)(QBodyOS[i].q[outIndex]);	
			       }
			       else
			       {
				        *((float*)QTxRxOS[i].dataRx + QTxRxOS[i].itemsRx) = (float)((int)(QBodyOS[i].q[outIndex])) / (float)multiplier;			
			       }

		         QTxRxOS[i].itemsRx++;
				
             if( QBodyOS[i].outIndex >= QLENGTH )
             {
                QBodyOS[i].outIndex = 0;
             }

				     if ( QTxRxOS[i].itemsRx ==  QTxRxOS[i].length )
		         {
		            QTxRxOS[i].completeRx = 1;
							  QTxRxOS[i].completeTx = 1;
		         }
		        ENABLE_INTERRUPT;						
	        } 	// for
					
				 DISABLE_INTERRUPT;	
					QBodyOS[i].items = 0;  
				 ENABLE_INTERRUPT;	
		   }
	 } // for
}


int packetLengthOS(int qNo)
{
	  return  QTxRxOS[qNo].length;
}


void nonBlockingValueTransferOS(void)
{
		 if ( FlagTxRxOS )
		 {
	       qTxOS();
	       qRxOS();
		 }
}

/*****************************************************************/
/*                        Task Load                              */
/*****************************************************************/

int* relativeTaskLoadOS(void)
{
    int   i;	
	  int   rounding;
    long double   totalCount = 0.0;
	
	  for (i=0; i< TASKSIZE; i++)  // exclude idleTask()
		{
				totalCount += (long double)CountTaskOS[i];
		}

	  for (i=0; i< TASKSIZE; i++)
		{
			 TaskLoadOS[i] = (int)((long double)CountTaskOS[i] / totalCount * 100);
			 rounding = (int)((long double)CountTaskOS[i] / totalCount * 1000) % 10;		
			 if ( rounding >= 5)
			 {
				   TaskLoadOS[i]++;
			 }
		}
		
		return  TaskLoadOS;	
}


int idleTaskLoadOS(void)
{
    int i;	
		int rounding;
	  int percentage;
    long double   totalClock = 0.0;

	  for (i=0; i<= TASKSIZE; i++)  // include idleTask()
		{
			  totalClock +=  (long double)TaskExecuteClockOS[i];
		}
		
		percentage = (int)((long double)TaskExecuteClockOS[TASKSIZE] / totalClock * 100); 
		
		rounding = (int)((long double)TaskExecuteClockOS[TASKSIZE] / totalClock * 1000) % 10;
		if ( rounding >= 5)
		{
				  percentage++;
		}
				
		return percentage;
	}


/*****************************************************************/
/*                     Low Power Mode                            */
/*****************************************************************/

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
					  setTableOS(ReadyTableOS, i);
				}
		}
     	
	 DISABLE_INTERRUPT;		
		SystemTickOS += min;
		MinDelayTickOS = min;
	 ENABLE_INTERRUPT;		
}


unsigned int matchRegisterOS(void)
{
	  return  MinDelayTickOS;
}


/*************************** end **********************************/



