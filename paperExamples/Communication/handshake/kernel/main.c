
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


#define LENGTH    190
int   data[LENGTH];
int   RxBuff[LENGTH];
int   countGlobal = 1;

void delayTick(void)
{
    int  delay;
    int  N = 10;
	
		countGlobal++;	
		delay = countGlobal % N;	
		delayTickOS(delay+1);	
}

void tx_nonblocking(void)   
{
    int   TxPacketLength;	
	  char  startTx;
	
	  while(1)
    {
				
				TxPacketLength = countGlobal % LENGTH;
			  sendByte('a');
			  startTx = nonblockTxOS(0, data, TxPacketLength, 0);			
			  sendByte('b');
			
			  if ( startTx )
				{
				   sendByte('T');				
				   print32bits( data[TxPacketLength-1] );
				}
				
			  delayTick();
    }
}

void rx_nonblocking(void)  
{	
	   int  i;
	   int  RecentLength;
	
     while(1)
     {   
				 RecentLength = nonblockRxOS(0, RxBuff);	
			 
			   if ( RecentLength > 0 )
				 {
					   sendByte('R');			
				     print32bits( RxBuff[RecentLength-1] ); 
							 
					 	 for ( i=0; i<RecentLength; i++ )
				     {
					       RxBuff[i] = -1;
				     }
				 }
         delayTick();				 
      }
} 


void task0(void)   
{
	  while(1)
    {
        delayTick();					
    }
}

void (*taskName[])(void)={tx_nonblocking,rx_nonblocking, task0};


int main(void)  
{
	   char errorCode;
	   int  startTaskIndex;
	   int  arraySize;
	   int  i;
	
	   for ( i=0; i<LENGTH; i++ )
		 {
			  data[i] = i;
		 }	
		 
	   initializeUART(9600);   
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} 




