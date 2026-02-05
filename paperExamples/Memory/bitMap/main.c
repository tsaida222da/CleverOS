
#include "OS.h"

#define  RDR    (1<<0)
#define  THRE   (1<<5)
#define  TEMT   (1<<6)

void  initializeUART(int baudrate)
{
   int Fdiv;
   int regVal;

   NVIC_DisableIRQ(UART_IRQn);
   LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);	// IOCON clock enable 	
   LPC_IOCON->PIO1_6 |= 0x01;    // RX   PIN-15
   LPC_IOCON->PIO1_7 |= 0x01;    // TX   PIN-16
   LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);	// UART clock enable   
   LPC_SYSCON->UARTCLKDIV = 0x1;    
   LPC_UART->LCR = 0x83;           
   regVal = LPC_SYSCON->UARTCLKDIV;								
   Fdiv = (((SystemCoreClock*LPC_SYSCON->SYSAHBCLKDIV)/regVal)/16)/baudrate ;
   LPC_UART->DLM = Fdiv / 256;							
   LPC_UART->DLL = Fdiv % 256;
   LPC_UART->LCR = 0x03;		
   LPC_UART->FCR = 0x07;		
   regVal = LPC_UART->LSR; 
   while (( LPC_UART->LSR & (THRE|TEMT)) != (THRE|TEMT) );
   while ( LPC_UART->LSR & RDR )
   {
	    regVal = LPC_UART->RBR;	
   }
   NVIC_EnableIRQ(UART_IRQn);
   LPC_UART->IER= (1<<0);     
} //  initUART(int baudrate)


void sendByte(char data)
{
  	while ( !(LPC_UART->LSR & THRE) ); // wait until Tx FIFO is empty
  	LPC_UART->THR = data;
	
  	while ( !(LPC_UART->LSR & THRE) ); // wait until Tx FIFO is empty
  	LPC_UART->THR = ' ';	
}


#define ARENASIZE   6

char  FreeBlock  = 0;
char  UsedBlock  = 1;
char  StartBlock = 2;
char  m_Bitmap[ARENASIZE];
char  m_Arena[ARENASIZE];
char  *pchar;


void initiateBitmap(void)
{
   int  i;
  

   for (i=0; i<ARENASIZE; i++)
   {
      m_Bitmap[i] = FreeBlock;
   }

}


void* bitmapAllocate(int requiredblocks)  // the block size is 1 byte
{
   int   i;
   int   location = 0;
   int   available;
   void* pointer;
	
   if (requiredblocks == 0)
   { 
       return  0x0;
   }

   while ( location <= ARENASIZE - requiredblocks )
   {
       available = 0;

       for ( i=0; i<requiredblocks; i++ )
       {
          if ( m_Bitmap[i + location] != FreeBlock )
          {
             break;
          }

          available++;
       }

       if ( available == requiredblocks )
       {
           pointer = (void*)(m_Arena + location);
           m_Bitmap[location++] = StartBlock;

           for ( i=1; i<requiredblocks; i++ )
           {
              m_Bitmap[location++] = UsedBlock;
           }

           return  pointer;
       }
       else
       {
           location += available +1;
       }
    } //while

    return  0x0;
}


void bitmapFree(void* pointer)
{
    int   location;

    if ( pointer == 0x0 )  return;


    location = (int)( (char*)pointer - m_Arena );

    if ( m_Bitmap[location] == StartBlock )
    {
        m_Bitmap[location++] = FreeBlock;

        while (  ( location < ARENASIZE) && (m_Bitmap[location] == UsedBlock)  )
        {
	         m_Bitmap[location++] = FreeBlock;
        }
    }
}


void display(void)
{
	 int     i = 0;

	 for (i=0; i<ARENASIZE; i++)
	 {
	     sendByte('0'+ m_Bitmap[i] );
	 }	
   sendByte( ' ' );	 
}



void allocateTask(void)  
{	
    while(1)
    {
			 sendByte( 'A' );	
	     pchar = bitmapAllocate(2);
			
			 if ( pchar == 0x0 ) 
			 {
				  sendByte( '#' );	
			 }
			 else
			 {
          display();
			 }
		 
		   delayTickOS(10);
    }
}

void freeTask(void)  
{	 
    while(1)
    {
			 sendByte( 'B' );	
       bitmapFree(pchar);
       display();
			 delayTickOS(20);		 
     }
}

void (*taskName[])(void)={allocateTask, freeTask};


int main(void)  
{
	   char errorCode;
	   int startTaskIndex;
	   int arraySize;
	
	   initiateBitmap();
	   initializeUART(9600);   // 600 bytes
	
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} // main




