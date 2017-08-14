/**
 * 1wire.c
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
#include <stdio.h>
#include <atmel\at89x52.h>
#include <intrins.h>
#include <defs51.h>
#include <delays.h>
#include <1wire.h>


#if (__C51__ < 610 ) 
 #error This program requires KEIL C 6.10 or higher
#endif


#ifndef CPAD
 // DESA51?
	#define PWR_ON_PIN 	P1_6 // - power to DS2401
	#define PWR_ON_DIR  P1_6
	
	#define ID_DQ_PIN P1_7		//  DQ for DS2401
	#define ID_DQ_DIR P1_7
#else  // CPAD board
	#define PWR_ON_PIN 	P1_1 // - power to DS2401
	#define PWR_ON_DIR  P1_1

	#define ID_DQ_PIN P1_0		//  DQ for DS2401
	#define ID_DQ_DIR P1_0

#endif

#define _1w_pin_hi()   ID_DQ_PIN=1
#define _1w_pin_low()  ID_DQ_PIN=0

/*
byte buff[8];
void main(void)
{
   read_id(0, buff);	// fetch the silicon ID
   while(1) continue;
}
*/

#pragma disable  // disable any interrupt
byte read_id(byte *buff)
{
   byte n, CRC;
// turn on power, set ID_DQ to high impedance
//   PWR_ON_DIR = 0;
   PWR_ON_PIN = 1;
   ID_DQ_DIR = 1;
//   delay(600);     // wait for 500 ms to be sure the device
    		       // has stabilized.
   if(_1w_init()==0) 
       return W1_FOUND;
   _1w_out_byte(0x33);	// read ROM

   for (n=0; n<8; n++)
   {
      buff[n]=_1w_in_byte();
   }

// turn off power and be sure ID_DQ is in high impedance state
    PWR_ON_PIN = 0;
//    PWR_ON_DIR = 1;
    ID_DQ_DIR = 1;
    
   CRC=0; 
   for (n = 0; n < 8; n++) CRC = _1w_CRC8(buff[n], CRC);  
   if( CRC ) return W1_CRC;
   return W1_OK;
}  // note that 8-byte ID is returned in array buff


// The following are standard 1-Wire routines.
bit _1w_init(void)
{
 word w; 
 bit is;
   _1w_pin_hi();
   _1w_pin_low();
   wwait(50);
   _1w_pin_hi();
   w = 500;
   is=0;
   while(w) 
   {
     --w;
     if( !ID_DQ_PIN ) is=1;
   }
 return is;
}

byte _1w_in_byte(void)
{
   byte i_byte, n;
   for (n=0; n<8; n++)
   {
       ID_DQ_PIN=0;
	   ID_DQ_PIN=1;
       WDTHIT();
      _nop_();
      _nop_();
      if(  ID_DQ_PIN )
      {
        i_byte=(i_byte>>1) | 0x80;	// least sig bit first
      }
      else
      {
        i_byte=i_byte >> 1;
      }
      wwait(3);
   }
   return(i_byte);
}

void _1w_out_byte(byte d)
{
   byte n;
   for(n=0; n<8; n++)
   {
      if (d&0x01)
      {
         ID_DQ_PIN=0;
		 ID_DQ_PIN=1;
         wwait(3);
      }
      else
      {
          ID_DQ_PIN=0;
	       wwait(3);
		  ID_DQ_PIN=1;
      }
      d=d>>1;
   }
}


// Calcula el CRC8 acumulado
byte _1w_CRC8(byte Data, byte Accum)
{
byte i, f;

   for (i = 0; i < 8; i++)
   {
      f    = 1 & (Data ^ Accum);
      Accum       >>= 1;
      Data        >>= 1;
      if (f) Accum ^= 0x8c;  // b10001100 es la palabra del CRC (x8 + x5 + x4 + 1)
                                  //  7..43..0  junto con el 1 aplicado a f.
   }
   return Accum;
}

/*
REVISIONS:    
           $Log$
*/
//*****************************************************************************[ENDL]***************





