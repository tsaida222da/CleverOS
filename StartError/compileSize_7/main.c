
#include "OS.h"

int    *pint;
char    carray[3];
int a = 0;
int b = 5;
float   *pfloat;
void (*taskName[1])(void);
void (*lowpower)(void);
void (*display)(unsigned int);

void task0(void)  
{	
    int n[]={0, -1}; 
		
	  while(1)
    { 
			   startOS(taskName,1,0, lowpower);
				 queryReadyTableOS();
			   nonBlockingCallbackOS(NULL);
			   errorPendSizeOS();
			   deleteSelfOS();
			   schedulerOS();

			   ramToPaddingOS(2000, 2);
			   paddingToRamOS(20, 2);
         autoPackItemsOS();
				 minimumStackOS(NULL);
			   minimumPaddingOS(1, display, 1);
			   autoMinimumStackOS();
				 queryResidueStackOS();
			   cpuRegisterRegionOS(NULL, 1);
         checkSafetyLevelOS(1, NULL);
         querySafetyLevelOS();
			   irregularIdleDataOS();
			   localVariableRegionOS(0x0, 2);

				 delayTickOS( 8 );
			   delayTimeOS(0, 0, 0, 10);				
				 delayUntilEqualOS(&a, &b);
			   delayUntilTrueOS(&a);
				 
				 postSemOS(0);
				 pendSemOS(n, 5);
				 
				 pendMutexOS(n, 5);
			   postMutexOS();
								 
			   postMailOS(0, 0x0);	
			   pendMailOS(n, 0x0, 0, 5);
			   readMailOS(0, 0);
				 selfPriorityMail_TxOS(0, NULL);
         selfPriorityMail_RxOS(0);

			   postFlagOS(0, 0x0 , FLAG_SET);
				 pendFlagOS(n, 0x0, FLAG_MATCH_ANY, 5 );
			   queryPublicFlagOS(0);
			   checkPublicFlagBitOS(0, 0);
			
		     getMemoryOS(11, 0, NULL);
			   getMemoryWithPutOS(1, 1, NULL);
				 putMemoryOS(0);
			   mallocOS(8);
			   freeOS(pfloat);
				 queryFreeBulkNoOS(carray, 1);
			   leakMemoryNoOS();
			   lackMemoryNoOS();
         leakAllOS();
         lackAllOS();

         postQOS(0, 0x0);  
         pendQOS(n, NULL, NULL, 5);
			   qReadyNumberOS(pint);
         queryRemainItemsOS(0);
         nonblockRxOS(0, 0x0);
	       nonblockTxOS(0, 0x0, 4, 2);
				 realtimeTxOS(0, 0x0);
         realtimeRxPendOS(0);
	       selfPriorityQ_TxOS(0,NULL);
	       selfPriorityQ_RxOS(0, NULL);
	
			   relativeTaskLoadOS();
         idleTaskLoadOS();

				 matchRegisterOS();
				 
         ASSERT(0);	
    } 
} 

void (*taskName[])(void)={task0};

int main(void)  
{
	   int startTaskIndex;
	   int arraySize;
	
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     startOS(taskName, arraySize, startTaskIndex, NULL); 
} 


