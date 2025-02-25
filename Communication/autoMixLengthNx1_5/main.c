
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
        x = p + 87;  // a,b,�K�K,f
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

int    Itx[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
int    Irx[9]; 
float  Ftx[4] = {0.16, 0.17, 0.18, 0.19};
float  Frx[4]; 
int    delay = 5;

void receiver(void)   
{
	  int i;
	
	  while(1)
    {
         qRxValueOS(0, Irx);
			   qRxValueOS(1, Frx);
				 
				 delayTickOS(delay);

			   for (i=0; i < packetLengthOS(0); i++)
				 {
			       sendByte('0' + Irx[i]);
				 }
				 for (i=0; i < packetLengthOS(0); i++)
				 {
			       Irx[i] = 0;
				 }
				 
				 sendByte('#');
				 
				 for (i=0; i < packetLengthOS(1); i++)
				 {
			       print32bits( (int)(Frx[i] * (float)100) );
				 }
				 for (i=0; i < packetLengthOS(1); i++)
				 {
			       Frx[i] = 0.0;
				 }
    }
} 

void transmitter1(void)   
{
	  int count = 0;
	  int x;
	
	  while(1)
    {
			   sendByte('A' );
			   x = (count % 10);
         qTxValueOS(0, Itx, x, 0);
			   count++;
				 delayTickOS(delay);
    }
} 

void transmitter2(void)   
{
		int count = 0;
	  int x;
	
	  while(1)
    {
			   sendByte('B' );
			   x = (count % 5);
         qTxValueOS(1, Ftx, x, 2);
			   count++;
				 delayTickOS(delay);
    }
} 

void (*taskName[])(void)={receiver, transmitter1, transmitter2};

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




