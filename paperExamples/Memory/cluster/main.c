
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
} 


void sendByte(char data)
{
  	while ( !(LPC_UART->LSR & THRE) ); // wait until Tx FIFO is empty
  	LPC_UART->THR = data;
	
  	while ( !(LPC_UART->LSR & THRE) ); // wait until Tx FIFO is empty
  	LPC_UART->THR = ' ';	
}


void display(void)
{
	 int     i;
	 char    table[20];
	
   queryFreeBulkNoOS(table, 20);
	
	 for (i=0; i<20; i++)
	 {
	     sendByte('0'+ table[i] );
	 }
   sendByte( '#' );	 
}

int d0 = 7 ;
int d1 = 5 ;
int d2 = 4 ;
int d3 = 3 ;
int d4 = 6 ;

void task0(void)  
{	
    while(1)
    {
	     getMemoryOS(0, 1);
       display();
		   delayTickOS(d0);
    }
}

void task1(void)  
{	
    while(1)
    {
	     getMemoryOS(1, 2);
       display();
		   delayTickOS(d1);
    }
}

void task2(void)  
{	
    while(1)
    {
	     getMemoryOS(2, 3);
       display();
		   delayTickOS(d2);
    }
}

void task3(void)  
{	
    while(1)
    {
	     getMemoryOS(3, 4);
       display();
		   delayTickOS(d3);
    }
}

void task4(void)  
{	 
    while(1)
    {
       putMemoryOS(0);
       putMemoryOS(1);	
       putMemoryOS(2);
			 putMemoryOS(3);
       display();
			 delayTickOS(d4);		 
     }
}

void (*taskName[])(void)={task0, task1, task2, task3, task4};


      // ErrorCode : 1- TaskCountOS != TASKSIZE+1
int main(void)  
{
	   char errorCode;
	   int startTaskIndex;
	   int arraySize;
	
	   initializeUART(9600);   // 600 bytes
	
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} // main




