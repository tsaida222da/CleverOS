
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

int task2Wait = 1;
int task3Wait = 1;
int task4Wait = 1;

void overflowHandler(int priority)
{
	   sendByte('@');
		 sendByte('0' + priority);

	   switch( priority )
		 {
			 case 1:
		      postSemOS(0); 
			    task2Wait = 0;
			    break;
			 
			 case 2:
		      postSemOS(1); 
			    task3Wait = 0;
			    break;	

			 case 3:
		      postSemOS(2); 
			    task4Wait = 0;
			    break;		 
		 }
}


int fib(int n)    // Fibonacci numbers
{
	  return  n < 3 ? 1 : fib(n-1) + fib(n-2);	
}


void task0(void)  
{ 
   while(1) 
   { 
		   sendByte('A');	
		   delayTickOS(10);
    } 
} 

void task1(void)  
{ 
   int x = 1;

   while(1) 
   { 
		 	 sendByte('B');
		   fib(x++);
		   checkStackSafetyOS(2, 5);
		   delayTickOS(10);
    } 
} 

void task2(void)  
{ 
   int x = 1;
	 int n1[]={0, -1};
	 
   while(1) 
   { 
		   if ( task2Wait )
		   {
		 	    pendSemOS(n1, INFINITEOS); 
		   }
		   sendByte('C');
		   fib(x++);
		   checkStackSafetyOS(2, 4);
		   delayTickOS(10);
    } 
}

void task3(void)  
{ 
   int x = 1;
	 int n1[]={1, -1};
	 
   while(1) 
   { 
		   if ( task3Wait )
		   {
		 	    pendSemOS(n1, INFINITEOS); 
		   }
		   sendByte('D');
		   fib(x++);
		   checkStackSafetyOS(2, 3);
		   delayTickOS(10);
    } 
}

void task4(void)  
{ 
   int x = 1;
	 int n1[]={2, -1};
	 
   while(1) 
   { 
		   if ( task4Wait )
		   {
		 	    pendSemOS(n1, INFINITEOS); 
		   }
		   sendByte('E');
		   fib(x++);
		   checkStackSafetyOS(2, 2);
		   delayTickOS(10);
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
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL, overflowHandler);  

		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} // main






