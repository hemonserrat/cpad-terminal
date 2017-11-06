/**
 * sle4428.H
 * 3-wire SC Protocol for SLE4428 Logical cards
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
#ifndef __SLE4428_H_

/*  Setup for SLE4442 cards */
#define CARDLENGTH  256

#define SC_OK         0x00
#define SC_BLOCKED    0x01
#define SC_ER_MASK    0x80

/*   Reader State defs  */
#define  RS_NOREADER 0x60
#define  RS_NOCARD   0x00
#define  RS_CARD     0x20

/*  timer defs // 100*/
#define DELAY  bwait(20)

/* TEST MACROS */
#define CHECK_PROT(H1)  ((H1&0xF0)==0x90)
#define CHECK_APP(H1)   ((H1&0x0F)==0x02)
//#define CHECK_SIZE(H2)  ((H2&0x78)>>3)
word CHECK_SIZE(byte h2);


byte GetReaderState(void);
#define IsCardPresent() (GetReaderState()==RS_CARD)

void ReadATR(byte *atr);
void ReadMemory(word addr, byte *buff, word length);
void ReadMemoryPr(word addr, byte *buff, word length, byte *pr);

/* only for write operations */
byte Verify(byte *psc);
void WriteByte(word addr, byte src);
void WriteMemory(word addr, byte *buff, word len);
void WritePassword(byte *psc);

#endif

/********************************************************************[ENDL]**/