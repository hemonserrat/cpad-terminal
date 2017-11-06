#include <stdio.h>
#include <string.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <delays.h>
#include <1wire.h>
#include <rtc02.h>
#include <3wire.h>
#include <AT45xx.h>
#include <lcd.h>
#include <sound.h>
#include <keypad.h>

#ifdef MONITOR51                         /* Debugging with Monitor-51 needs   */
char code reserve [3] _at_ 0x23;         /* space for serial interrupt if     */
#endif                                   /* Stop Exection with Serial Intr.   */
                                         /* is enabled                        */

byte buff[10];
byte atr[4];

void main (void) 
{
RTC02_DATA t;
byte cmp;
unsigned int d1;
unsigned int d2;
unsigned int d3;


#ifndef MONITOR51
    SCON  = 0x50;		        /* SCON: mode 1, 8-bit UART, enable rcvr      */
    TMOD |= 0x20;               /* TMOD: timer 1, mode 2, 8-bit reload        */
    TH1   = 221;                /* TH1:  reload value for 1200 baud @ 16MHz   */
    TR1   = 1;                  /* TR1:  timer 1 run                          */
    TI    = 1;                  /* TI:   set TI to send first char of UART    */
#endif

/* 		 t.DIA_SEMANA=2; 
 		 t.DIA=0x08;     
 		 t.MES=0x06;     
 		 t.ANO=0x99;     
 		 t.HORA=0x21;    
 		 t.MIN=0x20;     
 		 t.SEG=0x00;     
 		 RTC02_FullLoad(&t);  
*/
  P1_7 = 1;
  cmp=0;
//  LCD_init();
//  KP_init();
//  SND_init();

  while (1) {
/* key board 
      if( KP_kbhit() )
	  {
	    cmp = KP_scan();
        printf("%c",cmp);
		while( KP_kbhit() );
	  }
*/

/* LCD 
   dodemo();
*/
/* Flash driver tests 

    cmp = DF_ReadStatus();  

     DF_Page2Buffer1(0);

	  buff[0]=0x65; buff[1]=0x65; buff[2]=0x65;
      DF_WriteBuffer1(0, buff, 3);
	for( d1=0; d1<264; d1++){
	 DF_ReadBuffer1(d1, buff, 1);
	 d3 = (WORD)buff[0]&0x00FF;
	 printf("%02X:%c ", d3, buff[0]);
	}



    for( d1=0; d1<4096; d1++)
	{
	  for( d2=0; d2<264; d2++)
	  {
	    DF_ReadMainMemory(d1, d2, buff, 1);
		d3 = (WORD)buff[0]&0x00FF;
		printf("%02X:%c ", d3, buff[0]);
	  }
	}
*/
/* 3-wire tests
        if( IsCardPresent() )
        {
           ReadATR(atr);
		   printf("CHECK PROT: %c\n\r", (CHECK_PROT(atr[0]))?'.':'*');
		   printf("CHECK APP: %c\n\r", (CHECK_APP(atr[0]))?'.':'*');
		   printf("CHECK SIZE: %u\n\r", CHECK_SIZE(atr[1]) );
		   
		   buff[0]=0xFF;
		   buff[1]=0xFF;
           cmp=Verify(buff);
           if( !cmp ){
//            WriteByte(100, 0x55);
		    ReadMemory(100, buff, 1);
//		    buff[0]=0xFF;
//		    buff[1]=0xFF;
//            WritePassword(buff);
		   }
          while( IsCardPresent() );
        }
 */
// ibutton TEST
        if( read_id(buff) ){
		  printf("ERROR DE CRC\n\r");
		} else {
        printf(">>");
        for(cmp=0; cmp<8; cmp++){
		        d1=buff[7-cmp];
	           printf("%02X", d1);
        }
        printf("<<\n\r");
		}

/*  RTC TEST
  
	RTC02_FullRead(&t);
	if( cmp!=t.SEG ){
      d1=t.DIA; d2=t.MES; d3=t.ANO;
	   printf("%02X/%02X/%02X",d1,d2,d3);      
      d1=t.HORA; d2=t.MIN; d3=t.SEG;
	   printf(" %02X:%02X:%02X\n\r",d1,d2,d3);
	   cmp=t.SEG;
	}
*/
  }


}

// ***************************************************************************************[ENDL]************* 

