/*
 * Copyright (C) 2008 Antoine Drouin
 * Copyright (C) 2009 John Stowers
 *
 * This file is part of wasp, some code taken from paparazzi (GPL)
 *
 * wasp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * wasp is distributed in the hope that it will be useful,
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
/******************************************************************************
 *
 * $RCSfile$
 * $Revision$
 *
 * Header file for Philips LPC ARM Processors.
 * Copyright 2004 R O SoftWare
 *
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *
 *****************************************************************************/
#ifndef INC_LPC_SPI_H
#define INC_LPC_SPI_H

// Serial Peripheral Interface Registers (SPI)
typedef struct
{
  REG_8 cr;                             // Control Register
  REG_8 _pad0[3];
  REG_8 sr;                             // Status Register
  REG_8 _pad1[3];
  REG_8 dr;                             // Data Register
  REG_8 _pad2[3];
  REG_8 ccr;                            // Clock Counter Register
  REG_8 _pad3[3];
  REG_8 tcr;                            // Test Control Register
  REG_8 _pad4[3];
  REG_8 tsr;                            // Test Status Register
  REG_8 _pad5[3];
  REG_8 tor;                            // Test Observe Register
  REG_8 _pad6[3];
  REG_8 flag;                           // Interrupt Flag Register
  REG_8 _pad7[3];
} spiRegs_t;

#endif
