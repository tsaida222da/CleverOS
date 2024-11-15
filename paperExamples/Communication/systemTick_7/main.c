
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
 
int Tx[7] = {1, 2, 3, 4, 5, 6, 7};
int Rx[7];
int delay = 1;

void receiver(void)   
{
	  int i; 

	  while(1)
    {
         qRxValueOS(0, Rx);
				 delayTickOS(delay);

				 if ( Rx[0] > 0 )
				 {
			      for (i=0; i < packetLengthOS(0); i++)
				    {
			          sendByte('0' + Rx[i]);
						    Rx[i] = -1;
				    }
			   }
				 sendByte('#');
    }
} 

void transmitter(void)   
{
	  int x = 1;

	  while(1)
    { 
			  sendByte('A'); 
        qTxValueOS(0, Tx, x, 0);
			  sendByte('B'); 
				delayTickOS(delay);

				x++;
				if ( x > 7 )
				{
				   x = 1;
				}
    }
} 

void (*taskName[])(void)={receiver, transmitter};

      // ErrorCode : 1- TaskCountOS != TASKSIZE+1
int main(void)  
{
	   char errorCode;
	   int startTaskIndex;
	   int arraySize;
	
	   initializeUART(9600);   // 600 bytes
	
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} // main




