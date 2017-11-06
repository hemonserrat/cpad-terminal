/**
 * RTC.c
 * RTC DS1302 functions
 *
 * This file is part of CPAD Lightweght terminal for small payments
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
/* #define _DEBUG_ 1 */
#include <stdio.h>
//#include <reg52.h>  
#include <atmel\at89x52.h>
#define _OLD_ 1
#include <defs51.h>
#include <rtc02.h>

#if (__C51__ < 610 ) 
 #error This program requires KEIL C 6.10 or higher
#endif


#ifndef CPAD
 // DESA51?
	#define CS_RTC        P1_2
	#define SK            P1_0
	#define DIO           P1_1
#else  // CPAD board
	#define CS_RTC        P1_4
	#define SK            P0_4
	#define DIO           P0_5
#endif

static void soutbyte( byte out );
static byte sinbyte( void );


#ifdef _DEBUG_
void SendByte(unsigned char c)
{
  while (!TI);
  TI = 0;
  SBUF = c;
}

void SendString(char *s, unsigned char len)
{
    unsigned char i;
    for (i = 0; i<len; i++) SendByte(s[i]);
}

void main(void)
{
RTC02_DATA t;
int cmp;

 /* configurar Serial 9600,n,8,1 */
/*  IE = 0x93; 	 #10010011B */
  IE = 0x83;   /* sin serie */
  IP = 0x00; 	/* #00000000B */
  TMOD = 0x22; 	/* #00100010B     ;TIMER 1 = CONT. 8AUTO BAUD */
  TH1 = 0xFD;  	/* 253 BAUD RATE SERIE 9600 c/11.059 (244=2400) */
  PCON = 0x00; 	/*   #00000000B     ;SMOD=0 p/VEL SERIE */
/*  SCON = 0x60; 	 #01100000B SERIE MODO1, REN=0, SM2=0 */
  SCON = 0x7A;	/* con REN */
/* TCON */
  TF0=0;		/* clear overflow	*/
  TF1=0;		/* clear overflow	*/
  TR1=1; 		/* start			*/
/*  REN=1; 	enable char Rx ( ver SCON)	*/
  IT0=0;


/* PONE_LA_HORA: */
	 P3=0xCF;   /* CS_E2 = 0 / CS_RTC = 0 */
 		 t.DIA_SEMANA=2; 
 		 t.DIA=0x08;     
 		 t.MES=0x06;     
 		 t.ANO=0x99;     
 		 t.HORA=0x21;    
 		 t.MIN=0x20;     
 		 t.SEG=0x00;     
 		 RTC02_FullLoad(&t);  
do {
/* LEE_LA_HORA: */
	   RTC02_FullRead(&t);
 	   SendByte(0xFF);         
 	   SendByte(t.DIA_SEMANA); 
 	   SendByte(t.DIA);        
 	   SendByte(t.MES);        
 	   SendByte(t.ANO);        
 	   SendByte(t.HORA);       
 	   SendByte(t.MIN);        
 	   SendByte(t.SEG);        
 	   SendByte(0xFF);         
	 for( cmp=0 ;cmp<32000; cmp++);
   

}while(1);

}
#endif

void ssoutbyte( byte out )
{
byte count;
	count=8;
	B = out;
   do{
	SK=LPULSE;
   	   DIO=B0;  
 	   B = B>>1; 
	SK=HPULSE;                 
   }while(--count);
}

byte ssinbyte( void )
{
byte count;
	count=8;
   DIO=ENABLE;                 
   B = 0;
   do
   {
    SK=LPULSE;                  
	B = B>>1;
	B7 = DIO;
	SK=HPULSE;
   }while(--count);

return B;
}


void RTC02_FullRead(RTC02_DATA* pTime)
{
byte nbytes, out;
byte *pcData;
					pcData = (byte*)pTime;
/* LEO EN BURST */
						/* DPULSE(CS_RTC); */
 						CS_RTC=ENABLE;  
 						CS_RTC=DISABLE; 

						SK=OFF;                  /* CK FLANCO ASCENDENTE */
						DIO=ON;
						CS_RTC=ENABLE;             /* CS */
/* acceso al acumulador
	operaciones de bits
	flags como el carry
*/
						out = RTC02_DIR_CBU;
				  ssoutbyte( out|1 ); 
				  nbytes = 7;  /* CANTIDAD DE BYTES */
				  do
				  {
                    WDTHIT();
					DIO=ENABLE;  /*  DATO HIGH PARA LEER  */
 					DIO=DISABLE; 
					/* APULSE(DIO); */
					 *pcData=ssinbyte(); 
					pcData++;
				  }while( --nbytes );
						CS_RTC=DISABLE;
					SK=OFF;
						/* DPULSE(DIO); */
 						DIO=OFF; 
 						DIO=ON;  
}


void RTC02_FullLoad(RTC02_DATA* pTime)
{
			RTC02_Write( RTC02_DIR_CON, RTC02_WRITE_ENABLED );   /* WRITE ENABLED */
			RTC02_Write( RTC02_DIR_SEG, RTC02_HALT_CLOCK );	  /* HALT CLOCK    */

			RTC02_Write( RTC02_DIR_SEG, pTime->SEG );  /* Y SACA EL HALT */

			RTC02_Write( RTC02_DIR_MIN, pTime->MIN );
                        
			RTC02_Write( RTC02_DIR_HOR, pTime->HORA );
                        
			RTC02_Write( RTC02_DIR_DIS, pTime->DIA_SEMANA );

			RTC02_Write( RTC02_DIR_DIA, pTime->DIA );
                        
			RTC02_Write( RTC02_DIR_MES, pTime->MES );

                        
			RTC02_Write( RTC02_DIR_ANO, pTime->ANO );

			RTC02_Write( RTC02_DIR_TRI, RTC02_TRICKLE_CHARGE );   /* TRICKLE CHARGE  */
											  /*  1D/2K          */

			RTC02_Write( RTC02_DIR_CON , RTC02_WRITE_PROTECTED );   /* WRITE PROTECTED  */
}


void RTC02_Write(byte DirRtc, byte DatoRtc )
{
						/*  APULSE(CS_RTC);  */
 		              CS_RTC=ENABLE;                 
                       CS_RTC=DISABLE;               
        
                      SK=LPULSE;               /*  CK FLANCO ASCENDENTE  */
                      CS_RTC=ENABLE;           /*  CS */
                      WDTHIT();
					  ssoutbyte( DirRtc );
					  ssoutbyte( DatoRtc );
                       
                        CS_RTC=DISABLE;
                        SK=LPULSE;
						/*  DPULSE(DIO); */
                         DIO=HIGH;                 
                         DIO=LOW;                  
                         DIO=HIGH;               
}

byte RTC02_Read(byte DirRtc)
{
byte res;
					/* APULSE(CS_RTC); */
		                CS_RTC=ENABLE;             
                        CS_RTC=DISABLE;              
        
                        SK=LPULSE;                  /* CK FLANCO ASCENDENTE */
                        DIO=HIGH;
                        CS_RTC=ENABLE;             /* CS */
                        WDTHIT();
						ssoutbyte( DirRtc|1 );
                        res = ssinbyte();                                                

                        CS_RTC=DISABLE;      
                        SK=LPULSE;              

                        DIO=LOW;                 
                        DIO=HIGH;                
						/*  DPULSE(DIO); */
return res;
}

/*********************************************************************************************/
