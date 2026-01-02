
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

#define increment   200
#define LENGTH        75
int   data[LENGTH];
int   countGlobal = 1;

void delayTick(void)
{
    int  delay;
    int  N = 10;
	
		delay = countGlobal % N;	
		delayTickOS(delay+3);	
}

void display(char sign, int *executionCount, int countRx, int count)
{
	  int percent;

	  if ( countGlobal > *executionCount )
		{ 
			   sendByte(sign);
			   percent = (100*countRx / count );
			   print32bits( percent );	
			   *executionCount += increment;	
		}
}

  //   Application Handshake

int   TxData = -1;
char  RxOK = 1;

void tx_applicationHandshake(void)   
{
	  int   i = 0;
	
	  while(1)
    {
			  if ( RxOK )
				{
			      TxData = data[i];			
					  RxOK = 0;
			      i++;
            if ( i >= LENGTH )	 i = 0;
				}
	
				countGlobal++;	
		    delayTick();
    }
} 

void rx_applicationHandshake(void)  
{	 
	   int  execution[1];
     int  *executionCount = execution;
	   int  countRx = 1;
	   int  count = 1;
	
	   *executionCount = increment;
     while(1)
     {  
			   if ( TxData >= 0 )
				 { 
					    countRx++;
			        TxData = -1;	
					    RxOK = 1;
				 }

				 count++;
				 display('A', executionCount, countRx, count);	
		     countGlobal++;					 
         delayTick();			 
      }
}

  //  Self Priority

char  startTx = 1;

void tx_selfPriority(void)   
{	
	  int i;
	
	  while(1)
    {  
				i = countGlobal % LENGTH;	
			
			  if ( startTx )
				{
				    selfPriorityMail_TxOS( 0, &data[i] );	
            startTx = 0;			
				}
							
				countGlobal++;	
				delayTick();		
    }
} 

void rx_selfPriority(void)  
{	 
		 int  *pint;
	   int  execution[1];
     int  *executionCount = execution;
	   int  countRx = 1;
	   int  count = 1;
		 
	   *executionCount = increment;	 
     while(1)
     {  
			   pint = selfPriorityMail_RxOS(0);

			   if( pint != 0x0 )
				 {
             countRx++;
					   startTx = 1;
			   }
				 
				 count++;
				 display('B', executionCount, countRx, count);	
   		   countGlobal++;					 
         delayTick();						 
      }
}


  //   Kernel Handshake

int   RxBuff[LENGTH];

void tx_nonblockingTask(void)   
{
	  int  packetLength;
	  int  channel0 = 0;
	
	  while(1)
    {
			  packetLength = countGlobal % LENGTH;	
        nonblockTxOS(channel0, data, packetLength+1, 0);						
			
				countGlobal++;	
				delayTick();	
    }
}

void rx_nonblockingTask(void)  
{	
	   int  RecentLength;
	   int  execution[1];
     int  *executionCount = execution;
	   int  countRx = 1;
	   int  count = 1;
	   int  channel0 = 0;
	
	  *executionCount = increment;	
     while(1)
     {   
				 RecentLength = nonblockRxOS(channel0, RxBuff);	
					 
				 if ( RecentLength > 0 )
				 {
             countRx++;
				 }

	       count++;				 
				 display('C', executionCount, countRx, count);	
		     countGlobal++;					 
         delayTick();		 
      }
} 


  //  Queue

void tx_Q(void)   
{
	  int  channel1 = 1;

	  while(1)
    {
			 	postQOS(channel1, data);	
			
				countGlobal++;	
				delayTick();		
    }
}

void rx_Q(void)  
{	
	   int   channel1 = 1;	
		 int** pint;
	   int   number[] = {channel1, -1};
	   int   execution[1];
     int  *executionCount = execution;
	   int   countRx = 1;
	   int   count = 1;
			 
	   *executionCount = increment;	
		 
     while(1)
     {
		 	   pint = pendQOS(number, NULL, NULL, INFINITE);
				 if( pint != 0x0 )
				 {
             countRx++;
			   }

	       count++;				 
				 display('D', executionCount, countRx, count);
				 countGlobal++;					 
      }
}  


void task0(void)   
{
	  while(1)
    {
				countGlobal++;	
        delayTick();			
    }
}

void (*taskName[])(void)={rx_applicationHandshake, tx_applicationHandshake, rx_selfPriority, tx_selfPriority, rx_nonblockingTask, tx_nonblockingTask, rx_Q, tx_Q, task0};

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
	//	 sendByte('W');sendByte('A');sendByte('I');sendByte('T');sendByte('.');sendByte('.');sendByte('.');
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL); 
		 sendByte('0'+ errorCode);	// never execute if start successfully
	 
} 




