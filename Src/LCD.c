/**
 * LCD.c
 * Liquid Crystal display Driver 
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
// TESTED ON: DATAVISION 16236 4BIT & 8BIT MODES
#include <stdio.h>
#include <atmel\at89x51.h>
#include <defs51.h>
#include <delays.h>
#include <lcd.h>

#ifndef CPAD
 // DESA51?
 #define DISP_ENABLE    T1
 #define READ_WRI       T0
 #define REG_SEL        INT1
 #define LCD_PORT       P1
 #define LCD_MODE4      4
 #define READ_WRI_LO    READ_WRI=0
 #define READ_WRI_HI    READ_WRI=1
#else  // CPAD board
 #define DISP_ENABLE    P3_6
 #define REG_SEL        P1_5
 #define LCD_PORT       P0
 #define LCD_MODE4      4
 #define READ_WRI_LO  
 #define READ_WRI_HI
#endif

#ifdef _DV16236
  #define TIME0 0xB400
  #define TIME4 0xB400
  #define TIME1 50
  #define TIME2 500
#else
  #define TIME0 0xB400
  #define TIME4 0xB400
  #define TIME1 20  //50
  #define TIME2 500 //500
#endif

void wr_ctrl4( byte ctrl );

#ifdef _DEMO
byte code *msg1="*** SDP v1.0 ***";
byte code *msg2="(c)JANUS 2002   ";
byte code *msg3="The embedded    ";
byte code *msg4="systems experts!";

void dodemo(void)
{
byte x;
byte *p1;
byte *p2;
				wr_ctrl(DD_CLS);
                wr_ctrl(DD_EM|EM_ID);
				wr_ctrl(DD_HOME);
                wr_ctrl(DD_OF|OF_D);
         p1=msg1;      
         p2=msg2; 
         for( x=1; x<17; x++ ){   
			gotoXY( x,1 );
            wr_data( *p1++ );    
			gotoXY( x,2 );
            wr_data( *p2++ );    
	       wwait(0x5A00); // 500 ms 
         }   
           wr_ctrl(DD_OF);
			gotoXY( 1,1 );
           outstr(msg3);
			gotoXY( 1,2 );
           outstr(msg4);
         for( x=0; x<3;x++ ){
	       wwait(0xB400); // 500 ms 
           wr_ctrl(DD_OF|OF_D);
    	   wwait(0xB400); // 500 ms 
           wr_ctrl(DD_OF);
         }                
           wr_ctrl(DD_OF|OF_D);
         for( x=0; x<16; x++){
           wr_ctrl(DD_SD|SD_SC|SD_RL); 
	       wwait(0x5A00); // 500 ms 
         }
        wr_ctrl(DD_OF|OF_D|OF_B);
		wr_ctrl(DD_CLS);
}
#endif

void LCD_init(void)
{
#ifndef LCD_MODE4
                wwait(500);
                wr_ctrl(DD_FS|FS_DL|FS_N|FS_F);
                wwait(500);
                wr_ctrl(DD_FS|FS_DL|FS_N|FS_F);
                wwait(500);
                wr_ctrl(DD_FS|FS_DL|FS_N|FS_F);
                                
                wr_ctrl(DD_FS|FS_DL|FS_N|FS_F);
                wr_ctrl(DD_OF);
#else
 DISP_ENABLE=0;
 READ_WRI_LO;
 REG_SEL=0;

	             wwait(TIME0); /* 500 ms */
                wr_ctrl4(DD_FS);
	             wwait(TIME0); /* 500 ms */
	             wwait(TIME0); /* 500 ms */
#ifdef INIT_LARGE
               wr_ctrl4(DD_FS);
               wwait(TIME0); /* 500 ms */
               wr_ctrl4(DD_FS);                               
               wr_ctrl4(DD_FS);
               wwait(TIME0); /* 500 ms */
#endif
                wr_ctrl(DD_FS|FS_N|FS_F);
	            wwait(TIME4); /* 500 ms */
                wr_ctrl(DD_OF);
		        wr_ctrl(DD_CLS);
                wr_ctrl(DD_EM|EM_ID); 
#endif
}

byte rd_busy(void)
{
byte idata x;
		REG_SEL = DISABLE;      /* select control register */
		READ_WRI_HI;	        /* strobe read line  */
        DISP_ENABLE=ENABLE;		/* strobe enable high */
	  	bwait(TIME1);
        x=LCD_PORT;	            /* read data out */
        DISP_ENABLE=DISABLE;	/* ... and then bring it low */
return x;        
}

void wr_ctrl( byte ctrl )
{
#ifdef RD_ENABLE
        while( rd_busy()&0x80 );
#endif
		REG_SEL = DISABLE;      /* select control register */
		wr_common( ctrl );		/* write setup info to register */
		REG_SEL = ENABLE;      /* select control register */
}

void wr_ctrl4( byte ctrl )
{
		REG_SEL = DISABLE;      /* select control register */
		READ_WRI_LO;	/* strobe write line low */
        LCD_PORT=ctrl;		/* write data out */
        DISP_ENABLE=ENABLE;		/* strobe enable high */
        bwait(TIME1);
        DISP_ENABLE=DISABLE;		/* ... and then bring it low */
        wwait(TIME2);
        READ_WRI_HI;	/* pull write line high */
		REG_SEL = ENABLE;      /* select control register */
}

void wr_data(byte ddata)
{
#ifdef RD_ENABLE
    while( rd_busy()&0x80 );
#endif
  	REG_SEL=ENABLE; /* select data register */
	wr_common(ddata);		/* write data to LCD */
  	REG_SEL=DISABLE; 
}

void wr_common(byte ddata)
{
#ifndef LCD_MODE4
				READ_WRI_LO;	/* strobe write line low */
                LCD_PORT=ddata;		/* write data out */
                DISP_ENABLE=ENABLE;		/* strobe enable high */
	  	        bwait(TIME1);
                DISP_ENABLE=DISABLE;		/* ... and then bring it low */
                READ_WRI_HI;	/* pull write line high */
#else
				READ_WRI_LO;	/* strobe write line low */
                LCD_PORT=ddata;		/* write data out */
                DISP_ENABLE=ENABLE;		/* strobe enable high */
	  	        bwait(TIME1);
                DISP_ENABLE=DISABLE;		/* ... and then bring it low */
                READ_WRI_HI;	/* pull write line high */
	  	        bwait(TIME2);
				ddata=ddata<<4;
				READ_WRI_LO;	/* strobe write line low */
                LCD_PORT=ddata;		/* write data out */
                DISP_ENABLE=ENABLE;		/* strobe enable high */
	  	        bwait(TIME1);
                DISP_ENABLE=DISABLE;		/* ... and then bring it low */
                READ_WRI_HI;	/* pull write line high */
                wwait(TIME2);
#endif
}

void gotoXY(byte x, byte y)
{
	if( y== 1)
		wr_ctrl( 0x00 + x - 1 + 128);
	else
		wr_ctrl( 0x40 + x - 1 + 128);
}

void outstr( byte *pstr )
{
	while( *pstr )
		wr_data( *pstr++ );
}

void outstrn( byte *pstr, byte n )
{
	while( n-- )
		wr_data( *pstr++ );
}

/* **************************************************************** */