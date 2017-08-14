/**
 * 1wire.H
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

#ifndef _1WIRE_H_
#define _1WIRE_H_ 1

#define W1_OK      0x00
#define W1_CRC     0x01  
#define W1_FOUND   0x02


byte read_id(byte *buff);



// 1-wire prototypes
extern bit  _1w_init(void);
extern byte _1w_in_byte(void);
extern void _1w_out_byte(byte d);
extern byte _1w_CRC8(byte Data, byte Accum);

#endif
/* ***************************************************************[ENDL]**** */



