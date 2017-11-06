/**
 * AT45xx.c
 * Serial Data Flash Driver 
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
/****************************************************************
todo: .Document
      .Clean up and name normalize
*****************************************************************/
#include <stdio.h>
#include <atmel\at89x52.h>
#include <defs51.h>
#include <delays.h>
#include <spi.h>
#include <AT45xx.h>


// Hardware specific
#ifndef CPAD
 // DESA51?
	#define FCS1 P1_3         // First SDF
	// Not used in CPAD
	#define FRES          // RESET 
	#define FWP           // Write protect
#else  // CPAD board
	#define FCS1 P2_7         // First SDF
	// Not used in CPAD
	#define FRES          // RESET 
	#define FWP           // Write protect
#endif


#define  DF_CS_LO()  FCS1=0
#define  DF_CS_HI()  FCS1=1



BYTE DF_ReadStatus(void)
{
   BYTE Data;

   DF_CS_LO();

   //command
   SPI_wr(DF_STATUS);

   Data = SPI_rd();

   DF_CS_HI();
   SPI_init();
   return (Data);
}// DF_ReadStatus

void DF_WriteBuffer1(WORD Address, BYTE* Data, WORD size)
{
   DF_CS_LO();

   //command
   SPI_wr(BUFFER1_WRITE);
   //don't care 8 bits
   SPI_wr(0);
   //2 high address bits
   SPI_wr(HIBYTE(Address));
   //8 low address bits
   SPI_wr(LOBYTE(Address));
   //write data byte
  do { 
   SPI_wr(*Data);
   Data++;
  } while(--size);
  
   DF_CS_HI();
   SPI_init();
} // DF_WriteBuffer1

void DF_ReadBuffer1(WORD Address, BYTE* dst, WORD size)
{

   DF_CS_LO();

   //command
   SPI_wr(BUFFER1_READ);
   //don't care 8 bits
   SPI_wr(0);
   //2 high address bits
   SPI_wr(HIBYTE(Address));
   //8 low address bits
   SPI_wr(LOBYTE(Address));
   //don't care 8 bits
   SPI_wr(0);
   do {
      *dst = SPI_rd();
      dst++;
   }while(--size);
   DF_CS_HI();
   SPI_init();
}// DF_ReadBuffer1

/*
void DF_ErasePage(WORD PageNum)
{
   DF_CS_LO();

   PageNum <<= 1;

   //command
   SPI_wr(PAGE_ERASE);
   // high address bits
   SPI_wr(HIBYTE(PageNum));
   // low address bits
   SPI_wr(LOBYTE(PageNum));
   //don't care 8 bits
   SPI_wr(0);

   DF_CS_HI();
   SPI_init();
}// DF_ErasePage
*/


void DF_ReadMainMemory(WORD PageNum, WORD SubAddress, BYTE *buff, WORD size)
{
 BYTE Data;

   PageNum <<= 1;

   DF_CS_LO();
   //command
   SPI_wr(MAIN_MEMORY_PAGE_READ);
   //7 high page numder bits
   Data = HIBYTE(PageNum);
   SPI_wr(Data);
   Data = LOBYTE(PageNum)&0xFE; 
   Data |= (HIBYTE(SubAddress) & 0x01);
   SPI_wr(Data);
   //8 low address bits
   Data = LOBYTE(SubAddress);
   SPI_wr(Data);
   //don't care 32 bits
   SPI_wr(0);
   SPI_wr(0);
   SPI_wr(0);
   SPI_wr(0);

  do {
   *buff = SPI_rd();
   buff++;
  }while( --size );
  
   DF_CS_HI();
   SPI_init();
}// DF_ReadMainMemory

void DF_Buffer12MainMemory(WORD PageNum)
{
   PageNum <<= 1;

   DF_CS_LO();

   //command
   SPI_wr(BUFFER1_PROGRAM_NO_ERASE);
   // high address bits
   SPI_wr(HIBYTE(PageNum));
   // low address bits
   SPI_wr(LOBYTE(PageNum));
   //don't care 8 bits
   SPI_wr(0);

   DF_CS_HI();
   SPI_init();
}// DF_Buffer12MainMemory

void DF_Buffer12MainMemoryE(WORD PageNum)
{
   PageNum <<= 1;

   DF_CS_LO();

   //command
   SPI_wr(BUFFER1_PROGRAM_WITH_ERASE);
   // high address bits
   SPI_wr(HIBYTE(PageNum));
   // low address bits
   SPI_wr(LOBYTE(PageNum));
   //don't care 8 bits
   SPI_wr(0);

   DF_CS_HI();
   SPI_init();
} // DF_Buffer12MainMemoryE

void DF_Page2Buffer1(WORD Pagenum)
{
   Pagenum <<= 1;
   DF_CS_LO();
   
   //command
   SPI_wr(MAIN_MEMORY_PAGE_TO_BUFFER1_COPY);
   // high address bits
   SPI_wr(HIBYTE(Pagenum));
   // low address bits
   SPI_wr(LOBYTE(Pagenum));
   //don't care 8 bits
   SPI_wr(0);
   
   DF_CS_HI();
   SPI_init();
   
} // DF_Page2Buffer1


void DF_AutoPageRewrite(WORD PageNum)
{
   DF_CS_LO();

   PageNum <<= 1;

   //command
   SPI_wr(AUTO_PAGE_THROUGH_BUFFER1_REWRITE);
   // high address bits
   SPI_wr(HIBYTE(PageNum));
   // low address bits
   SPI_wr(LOBYTE(PageNum));
   //don't care 8 bits
   SPI_wr(0);

   DF_CS_HI();
   SPI_init();
}

//*****************************************************************************[ENDL]***************

