/**
 * GNV_MAIN.c
 * CPAD - NGV Pump POS APP
 *
 * This file is part of CPAD Lightweght terminal for small payments
 * Demo Application to show close-payment system in NGV stations. 
 * 
 *
 * Copyright (C) 2002,  Hernan Monserrat hemonserrat<at>gmail<dot>com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/**************************************************************************
todo:
         .RTC garbage detector
         .fs: save and append >> all [mix] or sectorized?
         .MIGRATE TO 55 > WDT, TMR2 (SOUND), TMR0 x SFT TIMER.
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <delays.h>
#include <rtc02.h>
#include <3wire.h>
#include <1wire.h>
#include <lcd.h>
#include <sound.h>
#include <keypad.h>
#include <AT45xx.h>
#include <jbus.h>

//#define  _BULK 1  // TEST FLASH

#ifdef MONITOR51                         /* Debugging with Monitor-51 needs   */
char code reserve [3] _at_ 0x23;         /* space for serial interrupt if     */
#endif                                   /* Stop Exection with Serial Intr.   */
                                         /* is enabled                        */

#define APP_VERH   1
#define APP_VERL   0
// ****************************************************************
// Hardware MACROS
#define RTC_OFF()    P1_4 = 0             // RTC TURN OFF
#define FLASH_OFF()  P2_7 = 1             // FLASH TURN OFF
#define LCD_OFF()    P1_5 = 0; P3_6=0     // LCD OFF
#define IS_TOUCH()   (!P1_0)              // TOUCH PIN

// IO MACROS ******************************************************
// ITALIAN GNC HEAD
#define OPEN1()      T1=0       // START
#define OPEN2()      P2_6=0     // PLAY
#define OPEN3()      P1_7=0     // STOP
#define CLOSE1()     T1=1
#define CLOSE2()     P2_6=1
#define CLOSE3()     P1_7=1

// FLASH DISK *****************************************************
#define FD_PSIZE   264     // Page size in bytes
#define FD_PNUM    4096
#define FD_MARK    0xA3    // AT 0
#define FD_BOOT    0x0000  // Page offset
#define FD_DB      0x0001  // Start Page for records

#define FD_ID      0x0001  // 1
#define FD_RELAY1  0x0002  // 1
#define FD_RELAY2  0x0003  // 1
#define FD_SPACE   0x0004  // 1
#define FD_PRGTM   0x0005  // 1
#define FD_PA      0x0006  // 2
#define FD_PC      0x0008  // 2

// Flash Record ***************************************************
typedef unsigned long AMOUNT;
typedef unsigned long BNUM;

typedef struct {
 byte   type;      //  1b   Record type (see below)
 byte   card[4];   //  4BCD Card Number
 byte   date[3];   //  3BCD AAMMDD
 byte   time[2];   //  2BCD HHMM
 BNUM   meter;     //  4b
 AMOUNT amount;    //  4b
 word   charge;    //  2b
} FREG; // 20

// TYPES **********************************************************
#define FT_BLOCKED    0x01
#define FT_CHARGE     0x02
#define FT_TRANSAC    0x03
#define FT_DELETED    0x04
#define FT_END        0xFF  // END OF RECORDS

/*--------------------------**
**                          **
**  Card equotes and defs   **
**--------------------------*/
// BIT MASKS ***************************************************
#define APP_VER      0x01
#define SC_BLOCKED   0x01  // BIT MASK
#define SC_SUPER     0x02  
// ERROR CODES *************************************************
#define SC_SUCCESS    0x00
#define SC_ER_UNKNOW  0x01
#define SC_ER_VALID   0x02
#define SC_ER_BLOCKED 0x03
// FIELD OFFS **************************************************
#define SC_INITOFF   8
#define SC_FMT       SC_INITOFF+0   //  3b   CARD format
#define SC_VER       SC_INITOFF+3   //  1b   CARD Version
#define SC_DATAOFF   23
#define SC_NAME      SC_DATAOFF+4   // 50b   User Name
#define SC_VALID     SC_DATAOFF+60  //  5BCD DDMMAAHHMM Validity time
#define SC_FLAGS     SC_DATAOFF+773 //  1b   0: Blocked 1: User/Super
#define SC_NUM       SC_DATAOFF+329 //  4BCD CARD Number
#define SC_TOUCH     SC_DATAOFF+65  //  8b   Ibutton 
#define SC_CHARGE    SC_DATAOFF+709 //  2b   Extra Charge
#define SC_PATENT    SC_DATAOFF+319 //  Xb   Car ID
#define SC_CONV      SC_DATAOFF+151 //  Xb   Equipment ID
#define SC_SUM       SC_DATAOFF+777 //  4b   Fidelity points
#define SC_PASSW     SC_DATAOFF+774 //  3BCD User/Super PIN
#define SC_XOR       SC_DATAOFF+388 //  1b   XOR

/*--------------------------**
**                          **
**  Local global variables  **
**--------------------------*/
// SFT TIMER  *************************************************
word                gTCount=0;      // Periodic sft counter
byte                SEC_TIMER=0;    // Secs counter
bit                 gIsTimeout;     // Timeout flag
// APP ********************************************************
FREG        idata   gReg;           // DB Record
RTC02_DATA  idata   gRTC;           // Real time clock
byte        idata   gBuff[20];      // General pourpouse var
AMOUNT      idata   gAmount;        // calcs
byte                gbTmp;          // GPV
byte        idata   gMASK[4];       // Encrypt mask  
bit                 gIsOnProcess;   // Process flag
// FSM ********************************************************
byte                gEvent;         // current event
byte                gNextEvent;     // next event
byte                gEventData;     // data event
#define EV0         gBuff[19]       // Extended data event 0
#define EV1         gBuff[18]       // Extended data event 1
#define EV2         gBuff[17]       // Extended data event 2
// ERROR CONTROL **********************************************
byte        idata   gERROR;         // Error control Flags
// FILE DESCRIPTOR ********************************************
word                gCP=0;          // current page
word                gCB=0;          // current byte
word        idata   gPA=0;          // Page Address
word        idata   gPC=0;          // Page write counter
// RTC RAM STORAGE ********************************************
#define RR_ID       RTC02_DIR_RM0         // 0
#define RR_PAH     (RTC02_DIR_RM0|0x02)   // 1  PAGE ADDRESS
#define RR_PAL     (RTC02_DIR_RM0|0x04)   // 2
#define RR_PCH     (RTC02_DIR_RM0|0x06)   // 3  PAGE COUNTER
#define RR_PCL     (RTC02_DIR_RM0|0x08)   // 4
#define RR_CPH     (RTC02_DIR_RM0|0x0A)   // 5  LAST PAGE
#define RR_CPL     (RTC02_DIR_RM0|0x0C)   // 6
#define RR_CBH     (RTC02_DIR_RM0|0x0E)   // 7  LAST BYTE 
#define RR_CBL     (RTC02_DIR_RM0|0x10)   // 8
#define RR_RGH     (RTC02_DIR_RM0|0x12)   // 9  RECORDS IN DB
#define RR_RGL     (RTC02_DIR_RM0|0x14)   // 10
#define RR_DIS     (RTC02_DIR_RM0|0x16)   // 11 DISPLAY 
#define RR_RCH     (RTC02_DIR_RM0|0x18)   // 12 CHECKSUM OR CONTROL

#define RR_MAGIC   0xA5

// ************************************************************
// TIMER MACROS
#define CLEARTIMEOUT()  SEC_TIMER=0; gIsTimeout=0
#define TIMEOUT(A)      (SEC_TIMER > A)
#define CHECKTIMEOUT()  gIsTimeout
#define SETTIMEOUT()    gIsTimeout=1
// ************************************************************
// FSM MACROS
#define POST(A,B)    gEvent=A; gNextEvent=B
#define EVENT(A)     gEvent=A
#define POSTD(A,B,C) gEvent=A; gNextEvent=B; gEventData=C
#define EVENTD(A,B)  gEvent=A; gEventData=B
#define DISPATCH()   gEvent=gNextEvent
// ************************************************************
// FSM STATES
#define FS_IDLE         1
#define FS_INITCARD     2
#define FS_RELEASEKEY   3
#define FS_KEYHIT       4
#define FS_DOMENU       5
#define FS_WAITKEY      6
#define FS_HEAD         7
#define FS_ID           8
#define FS_DATE         9
#define FS_TIME        10
#define FS_INPUT       11
#define FS_PROCESS     12
#define FS_MENUHIT     13
#define FS_DEMO        14
#define FS_DEMO1       15
#define FS_DEMO2       16
#define FS_REMOVECARD  21
#define FS_INITGNV     22
#define FS_CHECKID     23
#define FS_RELAY1      24
#define FS_RELAY2      25
#define FS_DISPATCH    26
#define FS_METERS      27
#define FS_AMOUNT      28
#define FS_TOTAL       29
#define FS_CONFIRM     30
#define FS_MSUM        31
#define FS_THANKS      32
#define FS_INNUM       33
#define FS_PNUM        34
#define FS_INSERTCARD  35
#define FS_TKEY        36
#define FS_STEP0       37
#define FS_STEP1       38
// ************************************************************
// STATES
#define ST_OK          0x00  // All ok i'm idle
#define ST_BUSY        0x01  // On process pumping with card inserted
#define ST_FAIL        0x02  // last process failure

#define SETSTATE(A)    (gERROR=A)
#define SETBIT(A)      (gERROR|=A)
#define CLEARBIT(A)    (gERROR&=~(A))

/*--------------------------**
**                          **
**     Messages             **
**--------------------------*/
                        //  1234567890123456
const byte code *msg1     =" VETRANO GNV  . ";
const byte code *msg2     ="(c)CMA 2002     ";
const byte code *s_card   ="    TARJETA     ";
const byte code *s_inval  ="    INVALIDA    ";
const byte code *s_vence  ="    VENCIDA     ";
const byte code *s_bloq   ="    BLOQUEADA   ";
const byte code *s_veh0   ="    VEHICULO    ";
const byte code *s_veh1   ="   NO COINCIDE  ";
const byte code *s_total  ="TOTAL A ABONAR: ";
const byte code *s_enter  =" PRESIONE ENTER ";
const byte code *s_metros ="TOTAL DE METROS:";
const byte code *s_IM     ="INGRESE METROS: ";
const byte code *s_CA     ="INGRESE VALOR:  ";
const byte code *s_gracias="    GRACIAS...  ";
const byte code *s_puntos ="PUNTOS:";
const byte code *s_insert ="INSERTE TARJETA ";
const byte code *s_menu0  ="1.RELOJ   2.ID  ";
const byte code *s_menu1  ="3.PROG    4.TEST";
const byte code *s_remove =" RETIRE TARJETA ";		              
const byte code *s_date   ="FECHA: DDMMAA";
const byte code *s_time   ="HORA: HHMMSS";
const byte code *s_id     ="ID:";
const byte code *s_prg0   =" MODO PROGRAMA  ";
const byte code *s_prg1   ="    CABEZAL     ";
const byte code *s_keys   ="TECLAS...";
const byte code *s_lcd    ="LCD...";
const byte code *s_snd    ="SONIDO...";
const byte code *s_clave  ="CLAVE:";
                          //1234567890123456789012345678901234567890123
const byte code *s_welcome="** BIENVENIDO: INICIAR CARGA == VERIFIQUE AUTO *** ";


void SendStatus(void);

// **************************************************************************
#if 0   // uncomment this function to verify serial communication
/*
 * Test Function: verify serial communication with HyperTerminal
 */
void TestSerial (void)  {
  char c = 'A';

  TI = 1;
  while (1)  {
    if (RI)  {
      c = SBUF;
      RI = 0;
    }
    while (!TI);
    TI = 0;
    SBUF = c;
  }
}
#endif
// **************************************************************************


void wrshft( byte l1, byte l2)
{
			 gbTmp++;
			 if( gbTmp > 40 )
		                gbTmp=1;
            wr_ctrl(DD_SD|SD_SC); // shift to the left
			gotoXY( gbTmp,1 );
            wr_data( l1 );    
			gotoXY( gbTmp,2 );
            wr_data( l2 );    
	        wwait(MAKEWORD(0x00, gBuff[10]));         
} // wrshft

void BCDtoASCII(byte bcd, byte *asc)
{
byte res;   
     res = bcd&0xF0;
	 res = res>>4;
	 *asc++= res + 48;
	 res = bcd&0x0F;
     *asc= res + 48;
} // BCDtoASCII


byte ASCIItoBCD(byte *asc)
{
  byte res;
     res = *asc - 48;
	 res = res<<4;
	 asc++;
	 if( *asc==0 )
      return (res&0xF0);
	 res = res|((*asc - 48)&0x0F);
	return res;
} // ASCIItoBCD


/****************************** xreverse **************************
PURPOSE: reverse the contents of string s
=============================================================================
PARAMETERS:	  s - NULL terminated string
-----------------------------------------------------------------------------
RETURNS:	 none
-----------------------------------------------------------------------------
GLOBALS:     none
******************************************************************************/
void xreverse(byte *s)
{
  byte *j;
  byte c;
  j = s + strlen(s) - 1;
  while(s < j) {
    c = *s;
    *s++ = *j;
    *j-- = c;
  }
} // xreverse

/****************************** xitoa **************************
PURPOSE:  converts an integer to an ASCII string representation
=============================================================================
PARAMETERS:	 n  - number to be converted
			 s  - Buffer to store result
		    pad	- right alignment padding
-----------------------------------------------------------------------------
RETURNS:	 none
-----------------------------------------------------------------------------
GLOBALS:     none
******************************************************************************/
void ltoa(unsigned long n, byte *s, byte pad)
{
  byte *ptr;
  ptr = s;
  do {
    *ptr++ = n % 10 + '0';
	 pad--;
    } while ((n = n / 10) > 0);
  while(pad--) *ptr++=' ';
  *ptr = 0x00;
  xreverse(s);
} // ltoa


void dodemo(void)
{
byte x;
byte *p1;
byte *p2;

   switch(gEvent)
   {
     case FS_DEMO:
	    	wr_ctrl(DD_CLS);
            wr_ctrl(DD_EM|EM_ID);
    		wr_ctrl(DD_HOME);
            wr_ctrl(DD_OF|OF_D);
			EVENT(FS_DEMO1);
     break;
	 case FS_DEMO1:
	     p1 = msg1;
		 p2 = msg2;
         for( x=1; x<17; x++ ){   
		    // WDHIT()
			gotoXY( x,1 );
            wr_data( *p1++ );    
			gotoXY( x,2 );
            wr_data( *p2++ );    
	       wwait(0x1A00);
         }   
		   gotoXY(14,1);
		   wr_data('0'+APP_VERH);
		   gotoXY(16,1);
		   wr_data('0'+APP_VERL);
           wr_ctrl(DD_OF|OF_D);
		   EVENT(FS_DEMO2);
	 break;
	 case FS_DEMO2:
         SND_play(INTRO);
         for( x=0; x<16; x++){
		    // WDHIT()
           wr_ctrl(DD_SD|SD_SC|SD_RL); 
	       wwait(0x1900);
         }
        wr_ctrl(DD_OF|OF_D);
		wr_ctrl(DD_CLS);
		EVENT(FS_IDLE);
	 break;
	}
} // dodemo


void fs_WaitReady(void)
{
  while( !IS_DF_READY(DF_ReadStatus()) )
  {
	    WDTHIT();
        continue; 
  }

} // fs_WaitReady


void fs_Store(word off, byte *d, byte size)
{
  DF_WriteBuffer1(off, d, size);
  fs_WaitReady();
}

/*
void fs_SetBoot(word Offset, byte* d, byte size)
{
  DF_Page2Buffer1(FD_BOOT);
  fs_WaitReady();
  fs_Store(Offset, d, size);
  DF_Buffer12MainMemoryE(FD_BOOT);      
  gPC++; 
  fs_WaitReady();
}// SetBoot
*/         
void fs_GetBoot(word Offset, byte* d, byte size)
{
  DF_Page2Buffer1(FD_BOOT);
  fs_WaitReady();
  DF_ReadBuffer1(Offset, d, size);
}// GetBoot

void fs_Init(void)
{
 byte n;
        fs_WaitReady();         
#ifdef  _BULK
/*    	 n=0xFF;
	     for( gPC=0; gPC<FD_PSIZE; gPC++ ) {
	       DF_WriteBuffer1(gPC, (byte*)&n, sizeof(byte));     
           fs_WaitReady();         
		 }
         n = FD_MARK;
         DF_WriteBuffer1(FD_BOOT, (BYTE*)&n, sizeof(byte));     
         fs_WaitReady();         
         // Write page
         DF_Buffer12MainMemoryE(FD_BOOT);      
         fs_WaitReady();
*/
        wr_ctrl(DD_CLS);
		gotoXY(1,2);
		gbTmp='.';
//         for(gPA=0; gPA<MAX_PAGE_8MBIT_DENSITY; gPA++)
//		 {
		  wr_data(gbTmp); 
	      DF_Page2Buffer1(0);
		  fs_WaitReady();
	      for( gPC=0; gPC<FD_PSIZE; gPC+=44 )
		  {
             DF_ReadBuffer1(gPC, gIO, 44);
			 gIOx=0;
			 gDataLen=44;
			 gComEvent = JB_TX;
			 DRX();
			 TI=1;
			 while( gComEvent==JB_TX );
          }
//		  if( (gPA&0x000F)==0x000F )
//		  {
//		    gotoXY(1,2);
//			gbTmp=(gbTmp=='.')?'*':'.';
//		  }
//		 }
#else
        fs_GetBoot(FD_BOOT, (BYTE*)&n, sizeof(byte));
        if( n!=FD_MARK ) // not formatted
        {
	    	wr_ctrl(DD_CLS);
			outstr("FORMAT...");
			gotoXY(1,2);
	     // clear Buffer
    	 n=0xFF;
		 gbTmp='.';
	     for( gPC=0; gPC<FD_PSIZE; gPC++ )
	       fs_Store(gPC, (byte*)&n, sizeof(byte));     
         for(gPA=1; gPA<MAX_PAGE_8MBIT_DENSITY; gPA++)
		 {
		  wr_data(gbTmp); 
          DF_Buffer12MainMemoryE(gPA);      
		  fs_WaitReady();
		  if( (gPA&0x000F)==0x000F )
		  {
		    gotoXY(1,2);
			gbTmp=(gbTmp=='.')?'*':'.';
		  }
		 }
       	  wr_ctrl(DD_CLS);
		  outstr("BOOTRECORD...");
		 // DF_Page2Buffer1(FD_BOOT);
         // Format mark
         n = FD_MARK;
         fs_Store(FD_BOOT, (BYTE*)&n, sizeof(byte));     
         n=0x01;
         fs_Store(FD_ID, (BYTE*)&n, sizeof(byte));
         n=0x01;
         fs_Store(FD_RELAY1, (BYTE*)&n, sizeof(byte));
         n=0x01;
         fs_Store(FD_RELAY2, (BYTE*)&n, sizeof(byte));
         n=0x02;
         fs_Store(FD_SPACE, (BYTE*)&n, sizeof(byte));
         n=0x01;
         fs_Store(FD_PRGTM, (BYTE*)&n, sizeof(byte));
         gPA=0x0000;
         fs_Store(FD_PA, (byte*)&gPA, sizeof(word));
         // Write page
         DF_Buffer12MainMemoryE(FD_BOOT);      
         fs_WaitReady();
        }  
        if(RTC02_Read(RR_RCH)!=RR_MAGIC)
		{
       	  wr_ctrl(DD_CLS);
		  outstr("RTC...");
			RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			 RTC02_Write( RR_ID, 0x01 );
			 RTC02_Write( RR_PAH, 0x00 );
			 RTC02_Write( RR_PAL, 0x00 );
			 RTC02_Write( RR_PCH, 0x00 );
			 RTC02_Write( RR_PCL, 0x00 );
			 RTC02_Write( RR_CPH, 0x00 );
			 RTC02_Write( RR_CPL, 0x00 );
			 RTC02_Write( RR_CBH, 0x00 );
 			 RTC02_Write( RR_CBL, 0x00 );
			 RTC02_Write( RR_RGH, 0x00 );
 			 RTC02_Write( RR_RGL, 0x00 );
 			 RTC02_Write( RR_DIS, 0x2A );
 			 RTC02_Write( RR_RCH, RR_MAGIC );
			RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */      
		}
	 gBuff[0] = RTC02_Read(RR_PAH);
     gBuff[1] = RTC02_Read(RR_PAL);
     gPA = MAKEWORD(gBuff[1], gBuff[0]);
#endif
} // fs_Init

void fs_Check(void)
{
  if( gPC==0 )
        return; // nothing to do
  do { 
      DF_AutoPageRewrite(gPA);
      fs_WaitReady();
      gPA++;
      if( gPA > (MAX_PAGE_8MBIT_DENSITY-1) )
                          gPA = 0; // set to the first
  } while (--gPC );
  // Save actual PA
	RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			 RTC02_Write( RR_PAH, HIBYTE(gPA) );
			 RTC02_Write( RR_PAL, LOBYTE(gPA) );
	RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */
} // fs_Check

byte fs_Find(byte type, byte *card)
{
  byte t[6];
         gCP = FD_DB; // RECORD DATABASE
   do {
     DF_Page2Buffer1(gCP);
     fs_WaitReady();		  
	 gCB = 0;
	 do {
      DF_ReadBuffer1(gCB, t, 1);
	  if( t[0] == FT_END ) return 0; // EOF not found
	  if( t[0] == type )
	  {
        DF_ReadBuffer1(gCB+1, t, 4);
		if( !memcmp( card, t, 4) ) 
	   	                   return 1;
      }
	  gCB+=sizeof(FREG);
     // WDHIT()
	 } while(gCB<FD_PSIZE);
	 gCP++;
   } while(gCP < FD_PNUM );

 return 0; 
} // fs_Find

byte fs_GetChunk(void)
{
byte t;
         gCP = FD_DB; // RECORD DATABASE
   do {
     DF_Page2Buffer1(gCP);
	 fs_WaitReady();
	 gCB = 0;
	 do {
      DF_ReadBuffer1(gCB, &t, 1);
	  if( (t == FT_END) || (t==FT_DELETED) )
	  {
          if( (gCB+sizeof(FREG)) < FD_PSIZE )
    		                            return 1;
//       return 0; // no space
	  }
	  gCB+=sizeof(FREG);
     // WDHIT()
	 } while(gCB<FD_PSIZE);
	 gCP++;
   } while(gCP < FD_PNUM );

 return 0; 
} // fs_GetChunk

void fs_Save(byte type, FREG *r)
{
 word x;

    if( fs_GetChunk()==0 ){
		 SETBIT(ST_FAIL);
     return;
	}

  fs_WaitReady();

 if( r == NULL )
 {
  gReg.type = type;
  gReg.date[0] = gRTC.ANO;   
  gReg.date[1] = gRTC.MES;   
  gReg.date[2] = gRTC.DIA;
  gReg.time[0] = gRTC.HORA;   
  gReg.time[1] = gRTC.MIN;   
  fs_Store(gCB,(byte *)&gReg, sizeof(FREG) );
 } else {
  fs_Store(gCB,(byte *)r, sizeof(FREG) );  
 }
  DF_Buffer12MainMemoryE(gCP);            
  fs_WaitReady();
  gPC++;
  gBuff[0] = RTC02_Read(RR_CPH);
  gBuff[1] = RTC02_Read(RR_CPL);
  x = MAKEWORD(gBuff[1], gBuff[0]);
   if( gCP > x )
   {
    RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
     RTC02_Write( RR_CPH, HIBYTE(gCP) );
     RTC02_Write( RR_CPL, LOBYTE(gCP) );
	RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */
   }

  gBuff[0] = RTC02_Read(RR_CBH);
  gBuff[1] = RTC02_Read(RR_CBL);
  x = MAKEWORD(gBuff[1], gBuff[0]);
   if( gCB > x )
   {
    RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			 RTC02_Write( RR_CBH, HIBYTE(gCB) );
			 RTC02_Write( RR_CBL, LOBYTE(gCB) );
	RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */
   }

  if( type == FT_TRANSAC )
  {
	  gBuff[0] = RTC02_Read(RR_RGH);
	  gBuff[1] = RTC02_Read(RR_RGL);
	  x = MAKEWORD(gBuff[1], gBuff[0]);
	  x++;
    RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			 RTC02_Write( RR_RGH, HIBYTE(x) );
			 RTC02_Write( RR_RGL, LOBYTE(x) );
    RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */  
  }
} // fs_Save

void fs_Delete(byte ft)
{
byte t;
bit findf;
bit eof;
   gCP = FD_DB; // RECORD DATABASE
   eof=1;
   do {
     DF_Page2Buffer1(gCP);
	 fs_WaitReady();
	 gCB = 0;
	 findf=0;
	 do {
      DF_ReadBuffer1(gCB, &t, 1);
	  if( t == FT_END) eof=0;
	  if( t==ft )
	  {
	   t = FT_DELETED;
       fs_Store(gCB, &t, 1);	   
	   findf=1;
	  }
	  gCB+=sizeof(FREG);
     // WDHIT()
	 } while( (gCB<FD_PSIZE) && eof);

	 if( findf )
	 {
       DF_Buffer12MainMemoryE(gCP);            
       fs_WaitReady();
       gPC++;
	 }
	 gCP++;
   } while( (gCP < FD_PNUM) && eof );

        if( ft == FT_TRANSAC )
        {
          RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			 RTC02_Write( RR_RGH, 0x00 );
			 RTC02_Write( RR_RGL, 0x00 );
          RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */  
        } 

} // fs_Delete


void fs_GetRecord(byte numh, byte numl, byte ft, byte *d)
{
 word num;
 word x;
 byte t;

   num = MAKEWORD(numl, numh);
   memset((byte *)d,0,sizeof(FREG));
   x = 1;
   gCP = FD_DB; // RECORD DATABASE
   do {
     DF_Page2Buffer1(gCP);
     fs_WaitReady();		  
	 gCB = 0;
	 do {
      DF_ReadBuffer1(gCB, &t, 1);
	  if( t == FT_END) return;
	  if( t == ft )
	  {
	    if( x == num )
		{
          DF_ReadBuffer1(gCB, d, sizeof(FREG) );
	   	  return;
		}
	    x++;
		if( x > num ){
		 SETBIT(ST_FAIL);
	     return;
		}
      }
	  gCB+=sizeof(FREG);	  
     // WDHIT()
	 } while(gCB<FD_PSIZE);
	 gCP++;
   } while(gCP < FD_PNUM );
		 SETBIT(ST_FAIL);
}


void EncDec(byte enc, byte *buff, byte length)
{
  do { 
   if( enc ) {
    *buff ^= gMASK[0];
    *buff ^= gMASK[1];
    *buff ^= gMASK[2];
    *buff ^= gMASK[3];
   } else {
    *buff ^= gMASK[3];
    *buff ^= gMASK[2];
    *buff ^= gMASK[1];
    *buff ^= gMASK[0];
   }
   buff++;
  }while( --length );
}

/****************************** CardValid *********************************
PURPOSE: Validates card
=============================================================================
PARAMETERS:	  none
-----------------------------------------------------------------------------
RETURNS:	 SC_SUCCESS - OK
             SC_XXXXXXX - ERROR CODE TO DISPLAY
******************************************************************************/
byte CardValid(void)
{
  ReadATR(gBuff);
  if(!(CHECK_PROT(gBuff[0])) ) return SC_ER_UNKNOW;
  if(!(CHECK_APP(gBuff[0])) ) return SC_ER_UNKNOW;
  if(CHECK_SIZE(gBuff[1])<1024 ) return SC_ER_UNKNOW;
//  wr_data('1');

  ReadMemory(SC_FMT, gBuff, 3);
  ReadMemory(SC_XOR, gMASK, 4);  
  EncDec(0, gBuff, 3);
  if( gBuff[0]!=0x0C ) return SC_ER_UNKNOW;
  if( gBuff[1]!=0x0D ) return SC_ER_UNKNOW;
  if( gBuff[2]!=0x02 ) return SC_ER_UNKNOW;
//  wr_data('2');
  
  ReadMemory(SC_VER, gBuff, 1);
  EncDec(0, gBuff, 1);
  if( gBuff[0]!=APP_VER) return SC_ER_UNKNOW;
//  wr_data('3');

  ReadMemory(SC_VALID, gBuff, 5);
  EncDec(0, gBuff, 5);

  if( gRTC.ANO  > gBuff[0] ) return SC_ER_VALID;  // AA  
  if( gRTC.ANO == gBuff[0] )
  {
    if( gRTC.MES  > gBuff[1] ) return SC_ER_VALID;  // MM
	if( gRTC.MES == gBuff[1] )
	{
      if( gRTC.DIA  > gBuff[2] ) return SC_ER_VALID;  // DD	
	  if( gRTC.DIA == gBuff[2] )
	  {
        if( gRTC.HORA > gBuff[3] ) return SC_ER_VALID; // HH
		if( gRTC.HORA == gBuff[3] )
		{
           if( gRTC.MIN  > gBuff[4] ) return SC_ER_VALID;  // MM
		} // HH
	  } // DD
	} // MM
  }  // AA
  
  ReadMemory(SC_FLAGS, gBuff, 1);
  EncDec(0, gBuff, 1);
  if( gBuff[0]&SC_BLOCKED ) return SC_ER_BLOCKED;
  gbTmp = gBuff[0];
//  wr_data('4');

  ReadMemory(SC_NUM, gBuff, 4);
  EncDec(0, gBuff, 4);
  gBuff[0] = gBuff[2]^0xA2;
  gBuff[1] = gBuff[3]^0xA1;
  gBuff[2] = 0;
  if( Verify(gBuff)!=SC_OK )
                  return SC_ER_BLOCKED;
//  wr_data('5');
  ReadMemory(SC_NUM, gBuff, 4);
  EncDec(0, gBuff, 4);
  if( gIsOnProcess )
  {
     // CARD IS A SUPERVISOR???
    if( gbTmp&SC_SUPER )
	{
       memcpy( gReg.card, gBuff, 4);
	   return SC_SUCCESS;
	}
    if( memcmp(gBuff, gReg.card, 4) )
	                    return SC_ER_UNKNOW;
  }
//  wr_data('6');
  memcpy( gReg.card, gBuff, 4);
  if( fs_Find(FT_BLOCKED, gBuff) )
  {
    gbTmp|=SC_BLOCKED;
	EncDec(1,&gbTmp, 1);
    WriteByte(SC_FLAGS, gbTmp);
	// save transac on post
	gReg.charge = 0x9000;
    fs_Save(FT_TRANSAC, NULL); // Add REGISTER    
   return SC_ER_BLOCKED;
  }
  
//  wr_data('7');
  if( fs_Find(FT_CHARGE, gBuff) )
  {
    // card charge to 0
	gbTmp = 0;
	EncDec(1, &gbTmp, 1);
	WriteByte(SC_CHARGE, gbTmp);
	// save transac on post
	gReg.charge = 0x8000;
    fs_Save(FT_TRANSAC, NULL); // Add REGISTER
  }

return SC_SUCCESS; // success
} // CardValid

/****************************** ShowTime *********************************
PURPOSE:  Every 5 secs show current date time or card insert msg.
=============================================================================
PARAMETERS:	  none
-----------------------------------------------------------------------------
RETURNS:	 none
******************************************************************************/
void ShowTime(void)
{
byte n;
	 // update time
    n = RTC02_Read(RTC02_DIR_SEG);
	if( n==gbTmp ) return;
	gbTmp = n;
	n = n&0x0F;
	switch( n )
	{
	 case 0x05:
       wr_ctrl(DD_OF|OF_D);
       wr_ctrl(DD_CLS);
       outstr(s_insert);
	 break;
	 case 0x00:
       wr_ctrl(DD_OF|OF_D);
       wr_ctrl(DD_CLS);
       RTC02_FullRead(&gRTC);
	   BCDtoASCII(gRTC.DIA, gBuff);
	   gBuff[2] = '/';
	   BCDtoASCII(gRTC.MES, &gBuff[3]);
	   gBuff[5] = '/';
	   gBuff[6] = '2';
	   gBuff[7] = '0';
	   BCDtoASCII(gRTC.ANO, &gBuff[8]);
       gBuff[10] = 0;
	   outstr(gBuff);
	   gotoXY(1,2);
	   BCDtoASCII(gRTC.HORA, gBuff);
	   gBuff[2] = ':';
	   BCDtoASCII(gRTC.MIN, &gBuff[3]);
	   gBuff[5] = ':';
	   BCDtoASCII(gRTC.SEG, &gBuff[6]);
       gBuff[8] = 0; 
	   outstr(gBuff);
	 break;
	} // switch
} // ShowTime


/****************************** poll_events *********************************
PURPOSE:  Manage FSM states
=============================================================================
PARAMETERS:	  none
-----------------------------------------------------------------------------
RETURNS:	 none
******************************************************************************/
void poll_events(void)
{
  switch(gEvent)
  {
    case FS_IDLE:
         /* Show */
		 ShowTime();

         /* check card insert */
         if( IsCardPresent() )
  		 {
	        memset((byte idata *)&gReg,0,sizeof(FREG));
		    EV0=FS_REMOVECARD;
			gBuff[10]=RTC02_Read(RR_DIS);  // Display Time
     		POST(FS_INITCARD, FS_INITGNV);
		 }

         /* key board */
        if( KP_kbhit() )
	    {
 	      gbTmp = KP_scan();
		  CLEARTIMEOUT();
		  POSTD(FS_RELEASEKEY,FS_KEYHIT,gbTmp);
		}
    break;
	case FS_DEMO:
	case FS_DEMO1:
	case FS_DEMO2:
	    dodemo();
	break;
    case FS_RELEASEKEY:
		 if( !KP_kbhit() ) {
		           DISPATCH();
		 } 
		 if( TIMEOUT(10) || CHECKTIMEOUT() )
		 {
		   SND_play(ERROR);
           SETTIMEOUT();
 		   DISPATCH();
		 }
	break;
	case FS_KEYHIT:
	    	 if( gEventData=='S' )
		     {
               SND_play(OK);
//			   EVENT(FS_DOMENU);
			   EVENT(FS_STEP0);
			   return;
			 }
	    SND_play(ERROR);		
		EVENT(FS_IDLE);
	break;
	case FS_STEP0:
		        if( KP_kbhit() )
			    {
		 	      gbTmp = KP_scan();
				  CLEARTIMEOUT();
				  POSTD(FS_RELEASEKEY,FS_KEYHIT,gbTmp);
				}
	         if( !IsCardPresent() )
			 {
			   SND_play(ERROR); 
			   SND_play(ERROR); 
	           wr_ctrl(DD_CLS);
			   outstr(s_insert);
			   EV0 = FS_REMOVECARD;
			   POST(FS_INSERTCARD, FS_STEP0);
			 } else {
				  ReadMemory(SC_FLAGS, gBuff, 1);
                  EncDec(0, gBuff, 1);
				  if( gBuff[0]&SC_SUPER )
				  {
                     wr_ctrl(DD_CLS);
			         outstr(s_clave);
			         EV0 = FS_STEP1;
			         EV1 = 6;
				     EV2 = 'P';
			         EVENT(FS_INPUT);
				  } else {
                     wr_ctrl(DD_CLS);
		             outstr(s_card);		              		                
   		             gotoXY(1,2);
		             outstr(s_inval);		              		                
	                 SND_play(ERROR);
			         gotoXY(1,1);
		             outstr(s_remove);		              
			         EVENT(FS_REMOVECARD);				    
				  }			   
			 }
	break;
	case FS_STEP1:
	      if( gBuff[0]==0 )
		  {
                 wr_ctrl(DD_CLS);
	             outstr(s_remove);		              
		         EVENT(FS_REMOVECARD);				    
            return;
		  }
		  ReadMemory(SC_PASSW, &gBuff[7], 3);        
		  EncDec(0, &gBuff[7], 3);
		    if( ASCIItoBCD(gBuff)==gBuff[7] )
			  if( ASCIItoBCD(&gBuff[2])==gBuff[8] )
			     if( ASCIItoBCD(&gBuff[4])==gBuff[9] )
				 {
			            EVENT(FS_DOMENU);
                   return;
				 }
	                 SND_play(ERROR);
	                 SND_play(ERROR);
			         EVENT(FS_STEP0);				            
	break;
	case FS_DOMENU:
	      wr_ctrl(DD_CLS);
		  outstr(s_menu0);
		  gotoXY(1,2);
		  outstr(s_menu1);
		  CLEARTIMEOUT();
		  POST(FS_WAITKEY, FS_MENUHIT);
	break;
	case FS_WAITKEY:
          if( KP_kbhit() )
		  {
		    gEventData = KP_scan();		    
			CLEARTIMEOUT();
			EVENT(FS_RELEASEKEY);
		 }
		 if( TIMEOUT(10) )
		 {
		   SND_play(ERROR);
		   SETTIMEOUT();
		   EVENT(FS_RELEASEKEY);
		 }
	break;
	case FS_MENUHIT:
	    if( CHECKTIMEOUT() )
		{
           EVENT(FS_STEP0);				    
		  return;
		}
	    SND_play(BEEP);
        switch( gEventData )
	    {
	     case '1':
              wr_ctrl(DD_CLS);
              RTC02_FullRead(&gRTC);
			  outstr(s_date);
			   BCDtoASCII(gRTC.DIA, gBuff);
			   BCDtoASCII(gRTC.MES, &gBuff[2]);
			   BCDtoASCII(gRTC.ANO, &gBuff[4]);
		       gBuff[6] = 0;
	           gotoXY(1,2);
			   outstr(gBuff);
			   EV0 = FS_DATE;
			   EV1 = 6;
			   EV2 = 0;
			   EVENT(FS_INPUT);
	   break;
	   case '2':
              wr_ctrl(DD_CLS);
			  outstr(s_id);
			  gbTmp = RTC02_Read(RR_ID);
			  BCDtoASCII(gbTmp, gBuff);
			  gBuff[2]=0;
              gotoXY(1,2);
			  outstr(gBuff);
			   EV0 = FS_ID;
			   EV1 = 2;
			   EV2 = 0;
			   EVENT(FS_INPUT);
	   break;
	   case '3':
              wr_ctrl(DD_CLS);
			  outstr(s_prg0);
			  gotoXY(1,2);
			  outstr(s_prg1);
              gbTmp = 0; // CLOSE
 		      CLEARTIMEOUT();
		      POST(FS_TKEY, FS_HEAD);
	   break; 
	   case '4':
              wr_ctrl(DD_CLS);
			  outstr(s_keys);
			  do {
				  while( !KP_kbhit() ) ; // WDHIT();
				  gbTmp = KP_scan();
				  while( KP_kbhit() ); // WDHIT()
				  SND_play(BEEP);
				  gotoXY(8,2);
				  wr_data(gbTmp);
			    
                  SendStatus();

              }while( gbTmp!='A' ); 	           
              wr_ctrl(DD_CLS);
			  outstr(s_lcd);
			  wwait(0xB400); /* 500 ms */
              wr_ctrl(DD_CLS);
			  for(gbTmp = 0; gbTmp <16; gbTmp++) wr_data(0xFF);
			  gotoXY(1,2);
			  for(gbTmp = 0; gbTmp <16; gbTmp++) wr_data(0xFF);
			  wwait(0xB400); /* 500 ms */
			  wwait(0xB400); /* 500 ms */
              wr_ctrl(DD_CLS);
			  outstr(s_snd); SND_set(ELIZA);
 	          EVENT(FS_DOMENU);
	   break;
	   case 'A':  
            wr_ctrl(DD_CLS);
            outstr(s_remove);		              
            EVENT(FS_REMOVECARD);				    
	   break;
	   default:
	        SND_play(ERROR);
			EVENT(FS_DOMENU);
	  }
	break;
	case FS_INPUT:
	           gotoXY(1,2);
               wr_ctrl(DD_OF|OF_D|OF_B);
	           gbTmp=1;          
		       CLEARTIMEOUT();
	   		   POST(FS_WAITKEY, FS_PROCESS);
	break;
	case FS_PROCESS:
	            if(CHECKTIMEOUT())
				{
                 wr_ctrl(DD_CLS);
	             outstr(s_remove);		              
		         EVENT(FS_REMOVECARD);				    
				 return;
				}
    	        SND_play(BEEP);
                switch(gEventData)
				{
				  case 'C':
				  case 'P':
				  case 'S':
				        SND_play(ERROR);
				  break;
				  case 'D':				       
				        gbTmp=1;
				        gotoXY(gbTmp,2);
				  break;
				  case 'E':
				      if( (gbTmp-1) == EV1 )
					  {
		                wr_ctrl(DD_OF|OF_D);
                        gBuff[gbTmp]=0;
					    EVENT(EV0);
				        return;
                      } else SND_play(ERROR); 
				  case 'A': 
		                wr_ctrl(DD_OF|OF_D);
				        gBuff[0]=0;
				        EVENT(EV0);
				      return;
				  default:
				   if( gbTmp < EV1+1 ) 
                   {
				    gotoXY(gbTmp,2);
				    gBuff[gbTmp-1] = gEventData;
                    if( EV2 != 'P' )
				      wr_data(gEventData);
					else
				      wr_data('*');
					gbTmp++;
				   }					
				}
		   CLEARTIMEOUT();
   		   POST(FS_WAITKEY, FS_PROCESS);
	break;
	case FS_DATE:
	      if( gBuff[0]==0 )
		  {
		         EVENT(FS_DOMENU);				    
            return;
		  }
			     //change date
			    gRTC.DIA=ASCIItoBCD(gBuff);
			    gRTC.MES=ASCIItoBCD(&gBuff[2]);
			    gRTC.ANO=ASCIItoBCD(&gBuff[4]);                                

              wr_ctrl(DD_CLS);
			   outstr(s_time);
			   BCDtoASCII(gRTC.HORA, gBuff);
			   BCDtoASCII(gRTC.MIN, &gBuff[2]);
			   BCDtoASCII(gRTC.SEG, &gBuff[4]);
		       gBuff[6] = 0;
	           gotoXY(1,2);
			   outstr(gBuff);
			   EV0 = FS_TIME;
			   EV1 = 6;
			   EV2 = 0;
			   EVENT(FS_INPUT);
	break;
	case FS_TIME:
	      if( gBuff[0]==0 )
		  {
		         EVENT(FS_DOMENU);				    
            return;
		  }
			     //change TIME
			    gRTC.HORA=ASCIItoBCD(gBuff);
			    gRTC.MIN=ASCIItoBCD(&gBuff[2]);
			    gRTC.SEG=ASCIItoBCD(&gBuff[4]);                                                  
 		        RTC02_FullLoad(&gRTC);  
			   EVENT(FS_DOMENU);
	break;
	case FS_ID:
	      if( gBuff[0]==0 )
		  {
		         EVENT(FS_DOMENU);				    
            return;
		  }
			   gbTmp=ASCIItoBCD(gBuff);
 			    RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
                RTC02_Write( RR_ID, gbTmp);
		    	RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */                
			   EVENT(FS_DOMENU);
	break;
	case FS_TKEY:
          if( KP_kbhit() )
		  {
		    gEventData = KP_scan();		    
			CLEARTIMEOUT();
			DISPATCH();
		 }
		 if( TIMEOUT(10) )
		 {
		   SND_play(ERROR);
		   SETTIMEOUT();
		   DISPATCH();
		 }
	break;
	case FS_HEAD:
	              if(CHECKTIMEOUT())
				  {
				    if( gbTmp==0 ){
			         EVENT(FS_DOMENU);				    
					 return;
				    }
				  }
                  switch(gEventData)
				  {
				   case 'C':  // START
                         if( gbTmp ) // ON KEY
						 {
							CLOSE1();
						    SND_play(BEEP);
							gbTmp=0;
		                    POST(FS_TKEY, FS_HEAD);
						 } else {
				            OPEN1();
 							gbTmp=1;
		                    POST(FS_RELEASEKEY, FS_HEAD);
						 }						  
				   break;
				   case 'P':  // PLAY
                         if( gbTmp ) // ON KEY
						 {
							CLOSE2();
						    SND_play(BEEP);
							gbTmp=0;
		                    POST(FS_TKEY, FS_HEAD);
						 } else {
				            OPEN2();
 							gbTmp=1;
		                    POST(FS_RELEASEKEY, FS_HEAD);
						 }						  
				   break;
				   case 'D':  // STOP				        
                         if( gbTmp ) // ON KEY
						 {
							CLOSE3();
						    SND_play(BEEP);
							gbTmp=0;
		                    POST(FS_TKEY, FS_HEAD);
						 } else {
				            OPEN3();
 							gbTmp=1;
		                    POST(FS_RELEASEKEY, FS_HEAD);
						 }						  
				   break;
				   case 'A':  
				         SND_play(BEEP);
				         EVENT(FS_DOMENU);
						 return;
				   default:
				    SND_play(ERROR);
		            POST(FS_TKEY, FS_HEAD);
				  }
		      CLEARTIMEOUT();
	break;
	//******************************************************
	case FS_INITCARD:
//	     gotoXY(1,1);
         wr_ctrl(DD_OF|OF_D);
		 wr_ctrl(DD_CLS);
		 gbTmp = CardValid();
		 if( gbTmp == SC_SUCCESS )		 
		 {
		    CLEARTIMEOUT();
			gbTmp=16;
			EV1 = 0;            
		    DISPATCH();
		 } else {
		    outstr(s_card);		              		                
   		    gotoXY(1,2);
		    switch(gbTmp)
			{
	         case SC_ER_UNKNOW:
		         outstr(s_inval);		              		                
			 break;
    	     case SC_ER_VALID:
		         outstr(s_vence);		              
			 break;
        	 case SC_ER_BLOCKED:
		         outstr(s_bloq);		              
		 	 break;
			}
	         SND_play(ERROR);
			 gotoXY(1,1);
		     outstr(s_remove);		              
			 EVENT(EV0);
        }   	   
	break;
	case FS_REMOVECARD:
        if( !IsCardPresent() )
		{
		   EVENT(FS_IDLE);
		}
	break;
	case FS_INSERTCARD:
        if( IsCardPresent() )
		{
		   EVENT(FS_INITCARD);
		}
	break;
	case FS_INITGNV: 
             if( !IsCardPresent() )
		     {
			   SND_play(ERROR); 
			   SND_play(ERROR); 
			   EVENT(FS_IDLE);
			   return;
		     }
              switch( read_id(gBuff) )
			  {
                case W1_OK:
			          if( gBuff[0]==0x01 ){ // family code
			                  EVENT(FS_CHECKID);
				             return; //!!!
				      }
                case W1_CRC:
                       SND_play(ERROR);
                break;
			  }
			 if( TIMEOUT(30) )
   		     {
			  SND_play(ERROR); 
			  EVENT(FS_REMOVECARD);
 			 }
 		      gBuff[0]=*(s_welcome+EV1);
			  if( gBuff[0] == 0 ) { 
				EV1=0;
			    SND_play(BEEP);
				return; 
			  }
             // APELLIDO-PATENTE-NRO CONVERSOR		 
			 if(EV1 < 20 ){
			   ReadMemory(SC_NAME+EV1, &gBuff[1],1);
               EncDec(0, &gBuff[1], 1);
			 } else if( (EV1 >= 20) && (EV1<30 ) )
			 {
			   ReadMemory(SC_PATENT+(EV1-20), &gBuff[1],1);
               EncDec(0, &gBuff[1], 1);
			 } else if( (EV1 >= 30) && (EV1<40) )
			 {
			   ReadMemory(SC_CONV+(EV1-30), &gBuff[1],1);
               EncDec(0, &gBuff[1], 1);
			 } else gBuff[1]=' ';
			 wrshft(gBuff[0],gBuff[1]);
			 EV1=EV1+1;
    break;   
	case FS_CHECKID:
			   ReadMemory(SC_TOUCH, &gBuff[10], 8);
               EncDec(0, &gBuff[10], 8);
			   if( !memcmp( gBuff, &gBuff[10], 8))
			   {
			     gIsOnProcess=1;
				 SETBIT(ST_BUSY);
  		         wr_ctrl(DD_CLS);
  		         wr_data('.');
				 fs_GetBoot(FS_RELAY1, &gEventData, 1);
				 if(gEventData > 5 ) gEventData=1;
				 CLEARTIMEOUT();
				 OPEN1();				 
				 EVENT(FS_RELAY1);				 
			   } else {
                 SND_play(ERROR);
  		         wr_ctrl(DD_CLS);
			     outstr(s_veh0);		              		                
   			     gotoXY(1,2);
		 	     outstr(s_veh1);		              		                

/*
                     SEND(JB_ERROR, 0); // CMD
					 SEND(gBuff[0], 1); // DATA
					 SEND(gBuff[1], 2); // DATA
					 SEND(gBuff[2], 3); // DATA
					 SEND(gBuff[3], 4); // DATA
					 SEND(gBuff[4], 5); // DATA
					 SEND(gBuff[5], 6); // DATA
					 SEND(gBuff[6], 7); // DATA
					 SEND(gBuff[7], 8); // DATA
					 SETLEN(9); // NUMBER OF BYTES
					 JB_send();
*/
				 EVENT(FS_REMOVECARD);
			   }

	break;
	case FS_RELAY1:
	        if( TIMEOUT(gEventData) )
			{
	          wr_data('.');
			  CLOSE1();
			  fs_GetBoot(FD_SPACE, &gEventData, 1);
			  if(gEventData > 5 ) gEventData=1;
			  CLEARTIMEOUT();
			  EVENT(FS_RELAY2);
			}
	break;
	case FS_RELAY2:
		    if( TIMEOUT(gEventData) )
			{
	         wr_data('.');
			 fs_GetBoot(FS_RELAY2, &gEventData, 1);
			 if(gEventData > 5 ) gEventData=1;
			 CLEARTIMEOUT();
			 OPEN2();
		  	 EVENT(FS_DISPATCH);			
			 gbTmp=0;
			}
	break;
	case FS_DISPATCH:
	       if( gbTmp== 0)
		   {
	         if( TIMEOUT(gEventData) )
			 {
 		       wr_data('.');
			   CLOSE2();
	           wr_ctrl(DD_CLS);
			   outstr("EN DESPACHO...");
               gbTmp=1;
			 }
		   } else {
	         // WAIT KEY PRESS AND CHECK CARD
             /* key board */
             if( KP_kbhit() )
	         {
 	            //gbTmp = KP_scan();
                wr_ctrl(DD_CLS);
			    outstr(s_IM);
			    EV0 = FS_METERS;
			    EV1 = 5;
			    EVENT(FS_INNUM);
		     }
             if( !IsCardPresent() )
		     {
			   SND_play(ERROR); 
			   SND_play(ERROR); 
	           wr_ctrl(DD_CLS);
			   outstr(s_insert);
			   EV0 = FS_INSERTCARD;
			   POST(FS_INSERTCARD, FS_DISPATCH);
			   return;
		     }
			 if( gbTmp == 16 ) // re-init
			 {
	           wr_ctrl(DD_CLS);
			   outstr("EN DESPACHO...");
               gbTmp=1;
			 }
		   }
	break;
    case FS_METERS:	               
	            // store meters
                gReg.meter = atol(gBuff);
                if( gReg.meter == 0 )
				{
				  SND_play(ERROR);
                  wr_ctrl(DD_CLS);
			      outstr(s_IM);
			      EV0 = FS_METERS;
			      EV1 = 5;
			      EVENT(FS_INNUM);
				  return;
				}
                wr_ctrl(DD_CLS);
			    outstr(s_CA);
			    EV0 = FS_AMOUNT;
			    EV1 = 12;
			    EVENT(FS_INNUM);
	break;
	case FS_AMOUNT:
			// store AMOUNT
			gReg.amount = atol(gBuff);
            if( gReg.amount == 0 )
			{
			  SND_play(ERROR);
                wr_ctrl(DD_CLS);
			    outstr(s_CA);
			    EV0 = FS_AMOUNT;
			    EV1 = 12;
			    EVENT(FS_INNUM);
			  return;
			}
			// calculo
            ReadMemory(SC_CHARGE, (byte*)&gbTmp, 1);                     
            EncDec(0, (byte*)&gbTmp, 1);
			gReg.charge = (word)gbTmp&0x00FF;
			if( gbTmp > 0 )
               gAmount = gReg.amount+((gReg.amount*gbTmp)/100);
			else
			  gAmount = gReg.amount;
			EVENT(FS_TOTAL);
	break;
	case FS_TOTAL:
	        // mostrar total a abonar
            ltoa( gAmount, gBuff, 15);
			gBuff[15]=0;
            wr_ctrl(DD_CLS);
    	    outstr(s_total);
            gotoXY(15,2);
			outstr(&gBuff[13]);
            gotoXY(14,2);
			wr_data(',');
			gBuff[13]=0;
			gotoXY(1,2);
			outstr(gBuff);
			// guardar transac
			fs_Save(FT_TRANSAC, NULL);
			CLEARTIMEOUT();
			EVENT(FS_CONFIRM);
	break;
	case FS_CONFIRM:
             if( KP_kbhit() )
	         {
			    EVENT(FS_MSUM);
		     }
			 if( TIMEOUT(10) )
			 {
			   CLEARTIMEOUT();
			   SND_play(ERROR);
               EVENT(FS_CONFIRM);
			 }
	break;
	case FS_MSUM:
             if( !IsCardPresent() )
		     {
			   SND_play(ERROR); 
			   SND_play(ERROR); 
			   EV0 = FS_INSERTCARD;
			   POST(FS_INSERTCARD, FS_MSUM);
			   return;
		     }
             // Add sum to card
			 gAmount=0;
             ReadMemory(SC_SUM, (byte *)&gAmount, sizeof(AMOUNT));
             EncDec(0, (byte *)&gAmount, sizeof(AMOUNT));
			 gAmount+=gReg.meter;
             EncDec(1, (byte *)&gAmount, sizeof(AMOUNT));
             WriteMemory(SC_SUM, (byte *)&gAmount, sizeof(AMOUNT));
             EncDec(0, (byte *)&gAmount, sizeof(AMOUNT));
               wr_ctrl(DD_CLS);
			   outstr(s_gracias);
			   gotoXY(1,2);
			   outstr(s_puntos);
			 if( gAmount < 99999L )
			 {
			   ltoa(gAmount, gBuff, 5);
			   gBuff[5]=0;
			   gotoXY(15, 2);
			   outstr(&gBuff[3]);
			   gotoXY(14,2);
			   wr_data(',');
			   gBuff[3]=0;
			   gotoXY(11,2);
			   outstr(gBuff);
			 } else {
			   gotoXY(9,2);
			   outstr("CONSULTE");
			 }

			 SND_set(INTRO);
             CLEARTIMEOUT();
   			 EVENT(FS_THANKS);
	break;
	case FS_THANKS:               
	      if( TIMEOUT(30) || KP_kbhit() )
		  {
            SND_play(BEEP);
		    CLEARTIMEOUT();
		    if( !IsCardPresent() )
			{
			  gIsOnProcess=0;
			  CLEARBIT(ST_BUSY);
			  EVENT(FS_IDLE);
			}
		  }
	break;
	case FS_INNUM:
	           gotoXY(1,2);
			   //1234567890123456
			   outstr("                ");
		       gotoXY(14,2);
	           wr_data(',');				     
	           gotoXY(16,2);
               wr_ctrl(DD_OF|OF_D|OF_B);
	           gbTmp=1;          
		       CLEARTIMEOUT();
	   		   POST(FS_WAITKEY, FS_PNUM);
	break;
	case FS_PNUM:
             if( !IsCardPresent() )
		     {
			   SND_play(ERROR); 
			   SND_play(ERROR); 
	           wr_ctrl(DD_CLS);
			   outstr(s_insert);
			   EV0 = FS_INSERTCARD;
			   POST(FS_INSERTCARD, FS_DISPATCH);
			   return;
		     }
	           if( CHECKTIMEOUT() )
			   {
			     EVENT(FS_INNUM);
				 return;
			   }
    	        SND_play(BEEP);
                switch(gEventData)
				{
				  case 'C':
				  case 'P':
				  case 'S':
				        SND_play(ERROR);
				  break;
				  case 'A': 
				  case 'D':				       
				      EVENT(FS_INNUM);
				  return;
				  case 'E':
				      if( gbTmp > 2 )
					  { 
				       gBuff[gbTmp-1]=0;
		               wr_ctrl(DD_OF|OF_D);
					   EVENT(EV0);
				       return;
					  } else  
					     SND_play(ERROR);						
				  break;
				  default:
				   if( gbTmp < EV1+1 ) 
                   {
		            gBuff[gbTmp-1] = gEventData;					
				      if( gbTmp > 2){
				          gotoXY(16-gbTmp,2);
						  outstrn(gBuff, gbTmp-2);
				          gotoXY(15,2);
						  outstrn(&gBuff[gbTmp-2], 2);
					  } else {
				          gotoXY(17-gbTmp,2);
						  outstrn(gBuff, gbTmp);
					  } 
					gbTmp++;
				    gotoXY(16,2);
				   }					
				}
		   CLEARTIMEOUT();
   		   POST(FS_WAITKEY, FS_PNUM);
	break;
  } // switch
} // poll_events



void SendStatus(void)
{
 SEND(JB_ERROR, 0); // CMD
 SEND(gERROR, 1); // DATA
 SETLEN(2); // NUMBER OF BYTES
 JB_send();
} // SendStatus

void poll_com(void)
{
      switch(gComEvent)
      {
        case JB_PROCESS: // this means that we must process and RX CMD
		// gIOx: Packet len
		// gIO[0] Command
		      CLEARBIT(ST_FAIL);
		      switch( gIO[0] )
			  {
			    case JB_ERROR:  // RETURN STATUS
				     SendStatus();
				break;
				case JB_FDWR:  // WRITE FLASH: [PH][PL][CHUNK:0-10][CHUNK:24]
				     gCP = MAKEWORD(gIO[2],gIO[1]);
					 gCB = gIO[3]*24;
                     fs_WaitReady();
                     DF_Page2Buffer1(gCP);
                     fs_WaitReady();		  
                     fs_Store(gCB,(byte *)&gIO[4], 24 );
                     DF_Buffer12MainMemoryE(gCP);            
                     fs_WaitReady();
                     gPC++;
					 SendStatus();
				break;
                case JB_FDRD: // READ FLASH: [PH][PL][chunk:0-10] return [CHUNK:24]
				     gCP = MAKEWORD(gIO[2],gIO[1]);
					 gCB = gIO[3]*24;
                     fs_WaitReady();
                     DF_Page2Buffer1(gCP);
                     fs_WaitReady();		  
				     SEND(gIO[0], 0); // CMD
                     DF_ReadBuffer1(gCB, GET(1), 24);
					 SETLEN(25); // NUMBER OF BYTES
					 JB_send();					 
				break;
		        case JB_SREC: // SET RECORD "ADD" [FREG]
				  // charge FREG				  
				  fs_Save(gIO[1], (FREG*)&gIO[1]);
				  SendStatus();
				break;
				case JB_EALL: // ERASE ALL DATABASE: [FT]
				  fs_Delete(gIO[1]);
				  SendStatus();
                break;
				case JB_GREC: // GET RECORD	[NUMH][NUML][FT]			
				     fs_GetRecord(gIO[1], gIO[2], gIO[3], GET(1) );  
                     SEND(gIO[0],0);
					 SETLEN(21); // NUMBER OF BYTES
					 JB_send();					 
				break;
				case JB_GNUM: // GET NUMBER OF RECORDS FT=TRANSAC
				     SEND(gIO[0], 0); // CMD
					 SEND(RTC02_Read(RR_RGL),1);
					 SEND(RTC02_Read(RR_RGH),2);
					 SETLEN(3); // NUMBER OF BYTES
					 JB_send();
				break;				
				case JB_GFE:  // GET FREE SPACE?
				     SendStatus(); //?
				break;   
				case JB_STM:  // SET TIME
					 gRTC.ANO = gIO[1];
   				     gRTC.MES = gIO[2];
   				     gRTC.DIA = gIO[3];
					 gRTC.HORA = gIO[4];   
                     gRTC.MIN = gIO[5];   
				     RTC02_FullLoad(&gRTC);  
				     SendStatus();
				break;
				case JB_GTM:  // GET TIME: RETURN: AAMMDDHHMM
				     RTC02_FullRead(&gRTC);
				     SEND(gIO[0], 0); // CMD
					 SEND(gRTC.ANO,1);
   				     SEND(gRTC.MES,2);
   				     SEND(gRTC.DIA,3);
					 SEND(gRTC.HORA,4);   
                     SEND(gRTC.MIN,5);   
					 SETLEN(6); // NUMBER OF BYTES
					 JB_send();                
				break;
				case JB_SRL:  // SET RELAY1|RELAY2|SPACE TIME|DISPLAY TIME:  [RLY1][RLY2][ST][DT]
	                 DF_Page2Buffer1(FD_BOOT);
		             fs_WaitReady();
                     if( gIO[1]!=0 ) 
                      fs_Store(FD_RELAY1, (BYTE*)&gIO[1], sizeof(byte));
                     if( gIO[2]!=0 ) 
                      fs_Store(FD_RELAY2, (BYTE*)&gIO[2], sizeof(byte));
                     if( gIO[3]!=0 ) 
                      fs_Store(FD_SPACE, (BYTE*)&gIO[3], sizeof(byte));
                     // Write page
                     DF_Buffer12MainMemoryE(FD_BOOT);      
                     fs_WaitReady();
                     if( gIO[4]!=0 ) 
					 {
					   RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
 					    RTC02_Write( RR_DIS, gIO[4] );
					   RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */      
					 }
				   SendStatus();
				break;
				case JB_VER:  // FIRMWARE VERSION: RETURN [VERH][VERL]
				    SEND(gIO[0], 0); // CMD
                    SEND(APP_VERH,1);
					SEND(APP_VERL,2);
					SETLEN(3);
					JB_send();
				break;
				default:
				  gComEvent = JB_IDLE; // RESET STATE
         		  ERX(); // ENABLE RX
			  }
		break;
      }

} // poll_com


void main (void) 
{

//  AUXR=0x08;
#ifndef MONITOR51
// ************************************************* 
// INIT TABLE                                        
  IE   = 0x12;  // EA|-|ET2|ES|ET1|EX1|ET0|EX0 
  IP   = 0x00; 	// -|-|PT2|PS|PT1|PX1|PT0|PX0 
                //    TIMER 1    |   TIMER 0     
  TMOD = 0x21; 	// GATE|C/T|M1|M0|GATE|C/T|M1|M0 0x22
  PCON = 0x00; 	//  SMOD|-|-|-|GF1|GF0|PD|IDL 
  TH1  = 0xFD;  // 253 BAUD RATE SERIE 9600 c/11.059 (244=2400) 
  SCON = 0x78;	// SM0|SM1|SM2|REN|TB8|RB8|TI|RI 
  TCON = 0x40;  // TF1|TR1|TF0|TR0|IE1|IT1|IE0|IT0 
  // 0100 0000 start T1 (baud rate) 
// ************************************************* 
  EA=1;/*  rock and roll */  
#endif
// TestSerial();
//  ISDinit ();        // initialize uVision2 Debugger and continue program run
//  ISDwait ();        // wait for connection to uVision2 Debugger
  SETSTATE(ST_OK);
  gIsOnProcess=0;
  LCD_init();         // Display
  	wr_ctrl(DD_CLS);
    wr_ctrl(DD_EM|EM_ID);
    wr_ctrl(DD_HOME);
    wr_ctrl(DD_OF|OF_D);
  wr_data('.');
//  while(1); // crack
  EVENT(FS_DEMO);     // Initial event
  wr_data('.');
  SND_init();         // Sound
  wr_data('.');
  KP_init();          // Key pad
  wr_data('.');
  RTC_OFF();          // Real time clock
  wr_data('.');
  fs_Init();          // Flash File system
  wr_data('.');
  gbTmp = RTC02_Read(RR_ID);
  JB_init(gbTmp);          // Init serial port protocol
  wr_data('.');
  while (1) {

    // APP events
    poll_events();

   	/* check comm activity */
    poll_com();

    /* dispatch Sound events */
	SND_task(); 

	// hit watchdog 
    WDTHIT();

	if( gTCount++==2800 )
	{
	 SEC_TIMER++;
	 gTCount=0;
	}

	// FLASH HELTH
	fs_Check();

  } // while end


} // main

// ******************************************************[ENDL]************* 


