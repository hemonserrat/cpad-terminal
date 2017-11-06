/**
 * SPI.H
 * Serial peripheral Interface  
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
#ifndef _SPI_H
#define _SPI_H

#ifndef _BYTE_
 typedef unsigned char BYTE;
 typedef unsigned int WORD;
#define _BYTE_ 1
#endif

// Function prototypes
extern void SPI_init( void );
extern BYTE SPI_io(BYTE out);
#define SPI_wr(A)    SPI_io(A)
#define SPI_rd()     SPI_io(0x00)

#endif // _SPI_H

//*****************************************************************************[ENDL]***************



