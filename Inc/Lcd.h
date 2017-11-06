/**
 * LCD.H
 * LCD Driver rutines definitions   
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
#ifndef _LCD_H
#define _LCD_H 1


#define DD_FS      0x20 /* Function Set */
#define FS_DL      0x10 /* 0: 4 bit     */
#define FS_N       0x08 /* 1: 2 Lines   */
#define FS_F       0x04 /* 1: 5x10 dots */

#define DD_EM      0x04 /* Entry Mode Set */
#define EM_ID      0x02 /* 1: Inc.        */
#define EM_S       0x01 /* 1: w/display shift */

#define DD_OF      0x08 /* Display ON/OFF */
#define OF_D       0x04 /* Display ON/OFF */
#define OF_C       0x02 /* Cursor ON/OFF */
#define OF_B       0x01 /* Cursor Blink */

#define DD_SD      0x10 /* Cursor/display Shift */
#define SD_SC      0x18 /* 1: display shift */
#define SD_RL      0x14 /* 1: right         */

#define DD_CLS     0x01 /* clear display */
#define DD_HOME    0x02 /* home cursor */  

#ifdef _DEMO
  extern void dodemo(void);
#endif

extern void LCD_init(void);
extern byte rd_busy(void);
extern void wr_ctrl( byte ctrl );
extern void wr_data(byte  ddata);
extern void wr_common(byte ddata);
extern void gotoXY(byte x, byte y);
extern void outstr( byte *pstr );
extern void outstrn( byte *pstr, byte n );


#endif

/*  ***************************************************** */