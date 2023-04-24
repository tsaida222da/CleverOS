
#include "OS.h"

#define  RDR    (1<<0)
#define  THRE   (1<<5)
#define  TEMT   (1<<6)
#define LSR_OE		   		(0x1<<1)  		// Overrun Error.
#define LSR_PE		   		(0x1<<2)  		// Parity Error. 
#define LSR_FE		   		(0x1<<3)  		// Framing Error.
#define LSR_BI		   		(0x1<<4)  		// Break Interrupt. 
#define LSR_RXFE	   		(0x1<<7)		// Error in RX FIFO.

/*  UART Interrupt Identification Register (U0IIR - address 0x4004 8008, Read Only) */
#define  IIR_PEND	    	(0x1<<0)     	// Interrupt status. 
// UART Interrupt identification. bit[3:1]
#define  IIR_RLS	    	(0x03)  		// 0x3 1   - Receive Line Status (RLS).
#define  IIR_RDA	    	(0x02)  		// 0x2 2a  - Receive Data Available (RDA).
#define  IIR_CTI			(0x06)  		// 0x6 2b  - Character Time-out Indicator (CTI).
#define  IIR_THRE	    	(0x01)     		// 0x1 3   - THRE Interrupt.
#define  IIR_MODI	    	(0x00)     		// 0x0 4   - Modem interrupt.


#define   BUFSIZE	30
volatile  int  UARTStatus;
volatile  int  UARTBuffer[BUFSIZE];
volatile  int  UARTCount = 0;

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

void setEINT0(void)
{
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16); // enable IOCON clock
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);  // enable GPIO clock	
	
	          // P0_7
	  LPC_IOCON->PIO0_7 =  0x0;
	  LPC_GPIO0->DIR   &= ~(1<<7);       // input
    LPC_GPIO0->IS    &= ~(1<<7);       // edge trigger
    LPC_GPIO0->IBE   &= ~(1<<7);	     // NOT both edge sense
	  LPC_GPIO0->IEV   &= ~(1<<7);       // negative edge
    LPC_GPIO0->IE    |=  (1<<7);	     // interrupt enable
	  NVIC_EnableIRQ(EINT0_IRQn); 
}


void PIOINT0_IRQHandler(void)
{
	   sendByte('@');	sendByte('@');	
	   postMailOS( 0, NULL );	
		 LPC_GPIO0->IC  |=  (1<<7);	
	   sendByte('#');	sendByte('#');
}


int number0[] = {0, -1};

void task0(void)  
{
     while(1) 
     { 
         sendByte('A');	
			   pendMailOS(number0, NULL, CLEARMAIL, INFINITEOS); 
      } 
}


void task1(void)  
{
     while(1) 
     { 
         sendByte('B');	
         delayTimeOS(0,0,0, 500);	
      } 
} 

void (*taskName[])(void)={task0,task1};

	
      // ErrorCode : 1- TaskCountOS != TASKSIZE+1
int main(void)  
{
	   char errorCode;
	   int startTaskIndex;
	   int arraySize;
	
	   initializeUART(9600);   
	   setEINT0();
     NVIC_SetPriority(EINT0_IRQn, 0xff );	
	
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL, NULL, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} // main






