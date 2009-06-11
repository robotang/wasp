/*
 * $Id$
 *  
 * Copyright (C) 2003-2005  Antoine Drouin
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 *
 */

/** \file led.h
 *  \brief arch independant LED (Light Emitting Diodes) API
 *
 *  Should ignore LEDs with IDs <= 0
 */

#ifndef LED_H
#define LED_H

#include "std.h"

void
led_init ( void );

void
led_on ( uint8_t id);

void
led_off ( uint8_t id);

void
led_toggle ( uint8_t id);

void
led_check ( uint8_t id);

#endif /* LED_H */
