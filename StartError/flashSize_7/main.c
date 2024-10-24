
#include "OS.h"

int    *pint;
char    carray[3];
int a = 0;
int b = 5;
float   *pfloat;

void task0(void)  
{	
    int n[]={0, -1}; 
		
	  while(1)
    { 
				 queryReadyTableOS();
			   nonBlockingCallbackOS(NULL);
			   errorPendSizeOS();
			   deleteSelfOS();
			   schedulerOS();

			   ramToPaddingOS(2000, 2);
			   paddingToRamOS(20, 2);
         autoPackItemsOS();
				 minimumStackOS(NULL);
			   autoMinimumStackOS();
				 queryResidueStackOS();
         queryDangerTaskOS();
         checkStackSafetyOS(1, 1);
				 querySafeLevelOS(8);

				 delayTickOS( 8 );
			   delayTimeOS(0, 0, 0, 10);				
				 delayUntilEqualOS(&a, &b);
			   delayUntilTrueOS(&a);
				 
				 postSemOS(0);
				 pendSemOS(n, 5);
								 
			   postMailOS(0, 0x0);	
			   pendMailOS(n, 0x0, 0, 5);
			   readMailOS(0, 0);
				 
			   postFlagOS(0, 0x0 , FLAG_SET);
				 pendFlagOS(n, 0x0, FLAG_MATCH_ANY, 5 );
			   queryPublicFlagOS(0);
			   checkPublicFlagBitOS(0, 0);
	
			   pendMutexOS(n, 5);
			   postMutexOS();
			
		     getMemoryOS(11, 0);
			   getMemoryWithPutOS(1, 1);
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
         qRxValueOS(0, 0x0);
	       qTxValueOS(0, 0x0, 4, 2);
				 qTxRealtimeOS(0, 0x0);
         qRxRealtimePendOS(0);
				 packetLengthOS(0);
	
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
     startOS(taskName, arraySize, startTaskIndex, NULL, NULL); 
} 


