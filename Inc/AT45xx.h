/**
 * AT45xx.H
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
#ifndef _AT45xx_H
#define _AT45xx_H 1

#ifndef _BYTE_
 typedef unsigned char BYTE;
 typedef unsigned int WORD;
#define _BYTE_ 1
#endif

//ATMEL SerialDataFlash Commands
#define MAIN_MEMORY_PAGE_READ                    0x52
#define BUFFER1_READ                             0x54
#define BUFFER2_READ                             0x56
#define MAIN_MEMORY_PAGE_TO_BUFFER1_COPY         0x53
#define MAIN_MEMORY_PAGE_TO_BUFFER2_COPY         0x55
#define MAIN_MEMORY_PAGE_TO_BUFFER1_COMPARE      0x60
#define MAIN_MEMORY_PAGE_TO_BUFFER2_COMPARE      0x61
#define BUFFER1_WRITE                            0x84
#define BUFFER2_WRITE                            0x87
#define BUFFER1_PROGRAM_WITH_ERASE               0x83
#define BUFFER2_PROGRAM_WITH_ERASE               0x86
#define BUFFER1_PROGRAM_NO_ERASE                 0x88
#define BUFFER2_PROGRAM_NO_ERASE                 0x89
#define PAGE_ERASE                               0x81
#define BLOCK_ERASE                              0x50
#define MAIN_MEMORY_THROUGH_BUFFER1_PROGRAM      0x82
#define MAIN_MEMORY_THROUGH_BUFFER2_PROGRAM      0x85
#define AUTO_PAGE_THROUGH_BUFFER1_REWRITE        0x58
#define AUTO_PAGE_THROUGH_BUFFER2_REWRITE        0x59
#define DF_STATUS                                0x57

#define DF_BUFFER1 0x00
#define DF_BUFFER2 0x01

//polarity of ready/busy signaling
#define IS_DF_BUSY() IS_DF_READY_LO()

//DataFlash status register bits

#define DF_STATUS_READY_MASK   0x80
#define IS_DF_READY(_status_)      (_status_ &                      \
                                            DF_STATUS_READY_MASK)
#define DF_STATUS_COMP_MASK 0x40
#define IS_DF_COMP_OK(_status_) ((_status_ &                     \
                                            DF_STATUS_COMP_MASK) \
                                            == 0)
                                            
#define DF_STATUS_DENSITY_MASK 0x38
#define DF_1MBIT_DENSITY_CODE  0x08
#define DF_2MBIT_DENSITY_CODE  0x10
#define DF_4MBIT_DENSITY_CODE  0x18
#define DF_8MBIT_DENSITY_CODE  0x20
#define DF_16MBIT_DENSITY_CODE 0x28
#define DF_32MBIT_DENSITY_CODE 0x30
#define DF_64MBIT_DENSITY_CODE 0x38

//Density related parameters
#define MAX_PAGE_32MBIT_DENSITY    8192
#define MAX_PAGE_8MBIT_DENSITY     4096
#define MAX_PAGE_1MBIT_DENSITY      512
#define PAGESIZE                    264

extern BYTE DF_ReadStatus(void);
extern void DF_WriteBuffer1(WORD Address, BYTE* Data, WORD size);
extern void DF_ReadBuffer1(WORD Address, BYTE* src, WORD size);
extern void DF_ErasePage(WORD PageNum);
extern void DF_ReadMainMemory(WORD PageNum, WORD SubAddress, BYTE *buff, WORD size);
extern void DF_Buffer12MainMemory(WORD PageNum);
extern void DF_Buffer12MainMemoryE(WORD PageNum);
extern void DF_Page2Buffer1(WORD PageNum);
extern void DF_AutoPageRewrite(WORD PageNum);


#endif
//*****************************************************************************[ENDL]***************
