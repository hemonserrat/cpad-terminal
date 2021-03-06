/**
 * BUZZER.H
 * CPAD Board Buzzer Interface Definitions
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
#ifndef _BUZZER_H_
#define _BUZZER_H_ 1


#ifndef _DELAYS_H
 #include <delays.h>
#endif

#define BUZZER_PORT P1_6

#define BEEP(A)  BUZZER_PORT=1; delay(A); BUZZER_PORT=0


#endif
/* ***************************************************************[ENDL]**** */



