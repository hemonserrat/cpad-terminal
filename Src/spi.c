/**
 * SPI.c
 * Serial peripheral Interface - software impl on x51
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
#include <defs51.h>
#include <delays.h>
#include <spi.h>

#ifndef CPAD
 // DESA51?
	#define SI    P1_1   // Serial Output to chip SI
	#define SO    P1_2   // Serial Input to chip SO
	#define SCK   P1_0
#else  // CPAD board
	#define SI    P0_5   // Serial Output to chip SI
	#define SO    P0_6   // Serial Input to chip SO
	#define SCK   P0_4
#endif

#define DELAY bwait(20)

void SPI_init( void )
{
 SO = 1; 
 SI = 0;
 SCK = 1;    
}// SPI_init

BYTE SPI_io(BYTE out)
{
byte idx;
byte in;
       SCK=0;  
	   in = 0;
	   for( idx=0; idx<8; idx++ )
	   {
        WDTHIT();
		SI=(out&0x80); 
		DELAY;     
		SCK=1;
		DELAY;   
        in=in<<1;
         if( SO )
             in |= 0x01;
         else
             in &= 0xFE;                
		SCK=0;  
		out=out<<1;
	   }
return in;
}// SPI_io

//*****************************************************************************[ENDL]***************




