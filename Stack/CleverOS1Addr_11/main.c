#include <LPC11xx.h>
#include "OS.h"

#define  RDR    (1<<0)
#define  THRE   (1<<5)
#define  TEMT   (1<<6)


typedef struct 
{
   unsigned int  sp;  // sp must be the first element
	 int           pack[9];
	 unsigned int  registerStack[16];
} stackOS;

extern stackOS  TaskOS[];

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
	
  	while ( !(LPC_UART->LSR & THRE) ); 
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


void task0(void)  
{	 while(1)   { pendSemOS(0, INFINITEOS  ); sendByte('0'); sendByte(' '); print32bits( (unsigned int) __get_PSP());	print32bits((unsigned int)TaskOS[0].sp);	print32bits((unsigned int)&TaskOS[0].sp); print32bits((unsigned int)&TaskOS[0].registerStack[15]); postSemOS(1);  }   } 
void task1(void)  
{	 while(1)   { pendSemOS(1, INFINITEOS  );	sendByte('1'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[1].sp);	print32bits((unsigned int)&TaskOS[1].sp); print32bits((unsigned int)&TaskOS[1].registerStack[15]); postSemOS(2);   }   } 
void task2(void)  
{	 while(1)   { pendSemOS(2, INFINITEOS  );	sendByte('2'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[2].sp);	print32bits((unsigned int)&TaskOS[2].sp); print32bits((unsigned int)&TaskOS[2].registerStack[15]); postSemOS(3);   }   } 
void task3(void)  
{	 while(1)   { pendSemOS(3, INFINITEOS  );	sendByte('3'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[3].sp);	print32bits((unsigned int)&TaskOS[3].sp); print32bits((unsigned int)&TaskOS[3].registerStack[15]); postSemOS(4); 	  }   } 
void task4(void)  
{	 while(1)   { pendSemOS(4, INFINITEOS  );	sendByte('4'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[4].sp);	print32bits((unsigned int)&TaskOS[4].sp); print32bits((unsigned int)&TaskOS[4].registerStack[15]); postSemOS(5);   }   } 
void task5(void)  
{	 while(1)   { pendSemOS(5, INFINITEOS  );	sendByte('5'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[5].sp);	print32bits((unsigned int)&TaskOS[5].sp); print32bits((unsigned int)&TaskOS[5].registerStack[15]); postSemOS(6);   }   } 
void task6(void)  
{	 while(1)   { pendSemOS(6, INFINITEOS  );	sendByte('6'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[6].sp);	print32bits((unsigned int)&TaskOS[6].sp); print32bits((unsigned int)&TaskOS[6].registerStack[15]); postSemOS(7); 	  }   } 
void task7(void)  
{	 while(1)   { pendSemOS(7, INFINITEOS  );	sendByte('7'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[7].sp);	print32bits((unsigned int)&TaskOS[7].sp); print32bits((unsigned int)&TaskOS[7].registerStack[15]); postSemOS(8);   }   } 
void task8(void)  
{	 while(1)   { pendSemOS(8, INFINITEOS  );	sendByte('8'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[8].sp);	print32bits((unsigned int)&TaskOS[8].sp); print32bits((unsigned int)&TaskOS[8].registerStack[15]); postSemOS(9);   }   } 
void task9(void)  
{	 while(1)   { pendSemOS(9, INFINITEOS  );	sendByte('9'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[9].sp);	print32bits((unsigned int)&TaskOS[9].sp); print32bits((unsigned int)&TaskOS[9].registerStack[15]); postSemOS(10); 	  }   } 
void task10(void)  
{	 while(1)   { pendSemOS(10, INFINITEOS  );	sendByte('A'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[10].sp);	print32bits((unsigned int)&TaskOS[10].sp); print32bits((unsigned int)&TaskOS[10].registerStack[15]); postSemOS(11);    }   } 
void task11(void)  
{	 while(1)   { pendSemOS(11, INFINITEOS  );	sendByte('B'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[11].sp);	print32bits((unsigned int)&TaskOS[11].sp); print32bits((unsigned int)&TaskOS[11].registerStack[15]); postSemOS(12); 	  }   } 
void task12(void)  
{	 while(1)   { pendSemOS(12, INFINITEOS  );	sendByte('C'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[12].sp);	print32bits((unsigned int)&TaskOS[12].sp); print32bits((unsigned int)&TaskOS[12].registerStack[15]); postSemOS(13); 	  }   } 
void task13(void)  
{	 while(1)   { pendSemOS(13, INFINITEOS  );	sendByte('D'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[13].sp);	print32bits((unsigned int)&TaskOS[13].sp); print32bits((unsigned int)&TaskOS[13].registerStack[15]); postSemOS(14); 	   }   } 
void task14(void)  
{	 while(1)   { pendSemOS(14, INFINITEOS  );	sendByte('E'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[14].sp);	print32bits((unsigned int)&TaskOS[14].sp); print32bits((unsigned int)&TaskOS[14].registerStack[15]); postSemOS(15); 	  }   } 
void task15(void)  
{	 while(1)   { pendSemOS(15, INFINITEOS  );	sendByte('F'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[15].sp);	print32bits((unsigned int)&TaskOS[15].sp); print32bits((unsigned int)&TaskOS[15].registerStack[15]); postSemOS(16);   }   } 
void task16(void)  
{	 while(1)   { pendSemOS(16, INFINITEOS  );	sendByte('G'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[16].sp);	print32bits((unsigned int)&TaskOS[16].sp); print32bits((unsigned int)&TaskOS[16].registerStack[15]); postSemOS(17); 	  }   } 
void task17(void)  
{	 while(1)   { pendSemOS(17, INFINITEOS  );	sendByte('H'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[17].sp);	print32bits((unsigned int)&TaskOS[17].sp); print32bits((unsigned int)&TaskOS[17].registerStack[15]); postSemOS(18); 	  }   } 
void task18(void)  
{	 while(1)   { pendSemOS(18, INFINITEOS  );	sendByte('I'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[18].sp);	print32bits((unsigned int)&TaskOS[18].sp); print32bits((unsigned int)&TaskOS[18].registerStack[15]); postSemOS(19);    }   } 
void task19(void)  
{	 while(1)   { pendSemOS(19, INFINITEOS  );	sendByte('J'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[19].sp);	print32bits((unsigned int)&TaskOS[19].sp); print32bits((unsigned int)&TaskOS[19].registerStack[15]); postSemOS(20); 	   }   } 
void task20(void)  
{	 while(1)   { pendSemOS(20, INFINITEOS  );	sendByte('K'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[20].sp);	print32bits((unsigned int)&TaskOS[20].sp); print32bits((unsigned int)&TaskOS[20].registerStack[15]); postSemOS(21);   }   } 
void task21(void)  
{	 while(1)   { pendSemOS(21, INFINITEOS  );	sendByte('L'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[21].sp);	print32bits((unsigned int)&TaskOS[21].sp); print32bits((unsigned int)&TaskOS[21].registerStack[15]); postSemOS(22); 	  }   } 
void task22(void)  
{	 while(1)   { pendSemOS(22, INFINITEOS  );	sendByte('M'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[22].sp);	print32bits((unsigned int)&TaskOS[22].sp); print32bits((unsigned int)&TaskOS[22].registerStack[15]); postSemOS(23);   }   } 
void task23(void)  
{	 while(1)   { pendSemOS(23, INFINITEOS  );	sendByte('N'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[23].sp);	print32bits((unsigned int)&TaskOS[23].sp); print32bits((unsigned int)&TaskOS[23].registerStack[15]); postSemOS(24);   }   } 
void task24(void)  
{	 while(1)   { pendSemOS(24, INFINITEOS  );	sendByte('O'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[24].sp);	print32bits((unsigned int)&TaskOS[24].sp); print32bits((unsigned int)&TaskOS[24].registerStack[15]); postSemOS(25);    }   } 
void task25(void)  
{	 while(1)   { pendSemOS(25, INFINITEOS  );	sendByte('P'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[25].sp);	print32bits((unsigned int)&TaskOS[25].sp); print32bits((unsigned int)&TaskOS[25].registerStack[15]); postSemOS(26);    }   } 
void task26(void)  
{	 while(1)   { pendSemOS(26, INFINITEOS  );	sendByte('Q'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[26].sp);	print32bits((unsigned int)&TaskOS[26].sp); print32bits((unsigned int)&TaskOS[26].registerStack[15]); postSemOS(27); 	   }   } 
void task27(void)  
{	 while(1)   { pendSemOS(27, INFINITEOS  );	sendByte('R'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[27].sp);	print32bits((unsigned int)&TaskOS[27].sp); print32bits((unsigned int)&TaskOS[27].registerStack[15]); postSemOS(28);    }   } 
void task28(void)  
{	 while(1)   { pendSemOS(28, INFINITEOS  );	sendByte('S'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[28].sp);	print32bits((unsigned int)&TaskOS[28].sp); print32bits((unsigned int)&TaskOS[28].registerStack[15]); postSemOS(29);    }   } 
void task29(void)
{	 while(1)   { pendSemOS(29, INFINITEOS  );	sendByte('T'); sendByte(' '); print32bits( (unsigned int)__get_PSP());  print32bits((unsigned int)TaskOS[29].sp);	print32bits((unsigned int)&TaskOS[29].sp); print32bits((unsigned int)&TaskOS[29].registerStack[15]); postSemOS(30); 	   }   } 
void task30(void)  
{	 
  while(1) 
  {  
      delayTimeOS(0,0,0, 800); sendByte('U'); sendByte(' '); print32bits( (unsigned int)__get_PSP());print32bits((unsigned int)TaskOS[30].sp);	print32bits((unsigned int)&TaskOS[30].sp);  print32bits((unsigned int)&TaskOS[30].registerStack[15]); postSemOS(0);
  }
} 


void (*taskName[])(void)={task0,task1,task2,task3,task4,task5,task6,task7,task8,task9,task10,task11,task12,task13,task14,task15,task16,task17,task18,task19,
      task20,task21,task22,task23,task24,task25,task26,task27,task28,task29,task30};

			

      // ErrorCode : 1- TaskCountOS != TASKSIZE+1
int main(void)  
{
	   char errorCode;
	   int startTaskIndex;
	   int arraySize;
	
	   initializeUART(9600);   // 600 bytes
	   arraySize = sizeof(taskName) / sizeof(taskName[0]);
     startTaskIndex = 0;
     errorCode = startOS(taskName, arraySize, startTaskIndex, NULL, NULL);  // create idleTaskOS()
		 sendByte('0'+ errorCode);	// never execute if start successfully 
	
} // main




