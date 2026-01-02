
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


int  T1[] = {1, 2};
int  T2[] = {8, 9};

int**  pint;

void receiver1(void)  
{	 
	  int    number[] = {0, 1, -1};
	
     while(1)
     {  
		 	   pint = pendQOS(number, NULL, NULL, INFINITE );
         sendByte('A');
				 sendByte('0'+ **(pint) );			
			 	 sendByte('0'+ *(*pint + 1) );	
      }
}

void receiver2(void)  
{	
    int    number[] = {2, 3, -1};

     while(1)
     {  
		 	   pint = pendQOS(number, NULL, NULL, INFINITE );
         sendByte('B');
				 sendByte('0'+ **(pint) );			
			 	 sendByte('0'+ *(*pint + 1) );	
      }
}

void transmitter1(void)   
{
	  while(1)
    {
			  sendByte('#');
			  sendByte('x');
			 	postQOS(0, T1);
			  sendByte('y');
			 	postQOS(2, T1);			
				delayTickOS(10);
    }
} 

void transmitter2(void)   
{
	  while(1)
    {
			  sendByte('@');
			 	postQOS(1, T2);
			 	postQOS(3, T2);			
				delayTickOS(10);
    }
}  

void (*taskName[])(void)={receiver1,receiver2,transmitter1,transmitter2};


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




