
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



void transfer(unsigned char  p )
{
	  unsigned char  x;
	
	  if (p < 10) // 0,1 .....9
	  {
		  x = p + 48 ;
	  }
	  else  // 10, 11,....,15
	  {
        x = p + 87;  // a,b,¡K¡K,f
	  }	
    sendByte(x);
}

void print32bits(unsigned  int  y)
{
    unsigned char  p;

	  sendByte( '0' );
	  sendByte( 'x' );	
	  p = (y & 0xF0000000) >>28;
    transfer(p);
	  p = (y & 0x0F000000) >>24;
    transfer(p);
	  p = (y & 0x00F00000) >>20;
    transfer(p);
	  p = (y & 0x000F0000) >>16;
    transfer(p);
    p = (y & 0x0000F000) >>12;
    transfer(p);
	  p = (y & 0x00000F00) >>8;
    transfer(p);
	  p = (y & 0x000000F0) >>4;
    transfer(p);
	  p = (y & 0x0000000F) >>0;
    transfer(p);
		sendByte(' ');
}

#define ARENASIZE   300

char  UsedBlock  = 0;
char  FreeBlock  = 1;
char  StartBlock = 2;
char  m_Bitmap[ARENASIZE];
char  m_Arena[ARENASIZE];
void* pointer1;
void* pointer2;
int   count = 0;
int   ten = 0;
char  displayB = 0;
int   x = 8;
int   y = 8;

void initiateBitmap(void)
{
   int  i;
  

   for (i=0; i<ARENASIZE; i++)
   {
      m_Bitmap[i] = FreeBlock;
   }

}


void* bitmapAllocate(int requiredblocks)
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


void display1(void)
{
	 int     i = 0;
	 char    found = 0;
	
	 while ( !found )
	 {
		   if ( m_Bitmap[ARENASIZE-1-i] == UsedBlock )
			 {
		       found = 1;
			 }			 
			 i++;
	 }
   print32bits( i );	 
}


void display2(void)
{
	 int     i = 0;
	 char    found = 0;
	 char    table[ARENASIZE];
	
   queryFreeBulkNoOS(table, ARENASIZE);
	 while ( !found )
	 {
		   if ( table[ARENASIZE-1-i] < 1 )
			 {
		       found = 1;
			 }			 
			 i++;
	 }	
   print32bits( i );	 
}


void task0(void)  
{	
    while(1)
    { 
			 pointer1 = bitmapAllocate(x);
			 ten++;
			 if ( ten % 10 == 0 )
			 {
				  displayB = 1;
			    sendByte('@');			
			    print32bits( ten );
			    sendByte('A');
			    (pointer1 == 0x0) ? sendByte('a') : display1();
       }
			 count++;
		   delayTickOS(x+5);
    }
}


void task1(void)  
{	 
    while(1)
    {
       bitmapFree(pointer1);
			 count++;
			 delayTickOS(y+10);		 
     }
}


void task2(void)  
{	
    while(1)
    {
	     pointer2 = getMemoryOS(0, x, NULL);
			 if ( displayB )
			 {
				  displayB = 0;
			    sendByte('B');				 
			    (pointer2 == 0x0) ? sendByte('b') : display2();
			 }
			 count++;			
		   delayTickOS(x+5);
    }
}

void task3(void)  
{	 
    while(1)
    {     	
			 putMemoryOS(0);	
			 count++;		
			 delayTickOS(y+10);		 
    }
}

void delayTask(void)  
{	 
    while(1)
    {
			 count++;	
       x = count % 8 + 1;	
			 count++;	
       y = count % 8 + 2;			
			 delayTickOS(x);		 
     }
}

void (*taskName[])(void)={task0, task1,task2, task3, delayTask};


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




