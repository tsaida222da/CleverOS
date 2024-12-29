
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


int  a[] = {0, 1, 2};
int  b[] = {7, 8, 9};
int  delay0 = 4;
int  delay1 = 8;

void transmitter1(void)   
{
	  int i = 0;
	
	  while(1)
    {
			 	postQOS(0, &a[i]);
				i++;
				if ( i > 2 )
				{
				   i = 0;
				}
				delayTickOS(delay0);
    }
} 

void transmitter2(void)   
{
	  int i = 0;
	
	  while(1)
    {
		    postQOS(1, &b[i]);
		  	i++;
				if ( i > 2 )
				{
				   i = 0;
				}
				delayTickOS(delay1);
    }
} 

void receiver(void)  
{	 
	   int    number[] = {0, 1, -1};
	   int**  pint2;
	
     while(1)
     {  
		 	   pint2 = pendQOS(number, NULL, NULL, INFINITE );
				 sendByte('0'+ **(pint2) );								 
      }
}

void (*taskName[])(void)={ transmitter1, transmitter2, receiver };


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




