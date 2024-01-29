//-----------------------------------------------------------------------------
// Copyright 2024 Illinois Advanced Research Center at Singapore
//
// This file is part of TEE-PLC.
//
// TEE-PLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TEE-PLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TEE-PLC.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------


#ifndef POUS_H_
#define POUS_H_

#include "iec_types_all.h"
#include "accessor.h"

// PROGRAM PROGRAM0
// Data part
typedef struct {
  // PROGRAM Interface - IN, OUT, IN_OUT variables

  // PROGRAM private variables - TEMP, private and located variables
  __DECLARE_LOCATED(BOOL,M10)
  __DECLARE_LOCATED(BOOL,M11)
  __DECLARE_LOCATED(BOOL,M12)
  __DECLARE_LOCATED(BOOL,M13)
  __DECLARE_LOCATED(BOOL,M14)
  __DECLARE_LOCATED(BOOL,M15)
  __DECLARE_LOCATED(BOOL,M16)
  __DECLARE_LOCATED(BOOL,M17)
  __DECLARE_LOCATED(BOOL,MQ10)
  __DECLARE_LOCATED(BOOL,MQ11)
  __DECLARE_LOCATED(BOOL,MQ12)
  __DECLARE_LOCATED(BOOL,MQ13)
  __DECLARE_LOCATED(BOOL,MQ14)
  __DECLARE_LOCATED(BOOL,MQ15)
  __DECLARE_LOCATED(BOOL,MQ16)
  __DECLARE_LOCATED(BOOL,MQ17)
  __DECLARE_LOCATED(BOOL,M20)
  __DECLARE_LOCATED(BOOL,M21)
  __DECLARE_LOCATED(BOOL,M22)
  __DECLARE_LOCATED(BOOL,M23)
  __DECLARE_LOCATED(BOOL,M24)
  __DECLARE_LOCATED(BOOL,M25)
  __DECLARE_LOCATED(BOOL,M26)
  __DECLARE_LOCATED(BOOL,M27)
  __DECLARE_LOCATED(BOOL,MQ20)
  __DECLARE_LOCATED(BOOL,MQ21)
  __DECLARE_LOCATED(BOOL,MQ22)
  __DECLARE_LOCATED(BOOL,MQ23)
  __DECLARE_LOCATED(BOOL,MQ24)
  __DECLARE_LOCATED(BOOL,MQ25)
  __DECLARE_LOCATED(BOOL,MQ26)
  __DECLARE_LOCATED(BOOL,MQ27)
  // __DECLARE_LOCATED(WORD,IW0)
  // __DECLARE_LOCATED(WORD,IW1)
  // __DECLARE_LOCATED(WORD,IW2)
  // __DECLARE_LOCATED(WORD,IW3)
  // __DECLARE_LOCATED(WORD,IW4)
  // __DECLARE_LOCATED(WORD,IW5)
  // __DECLARE_LOCATED(WORD,IW6)
  // __DECLARE_LOCATED(WORD,IW7)
  // __DECLARE_LOCATED(WORD,QW0)
  // __DECLARE_LOCATED(WORD,QW1)
  // __DECLARE_LOCATED(WORD,QW2)
  // __DECLARE_LOCATED(WORD,QW3)
  // __DECLARE_LOCATED(WORD,QW4)
  // __DECLARE_LOCATED(WORD,QW5)
  // __DECLARE_LOCATED(WORD,QW6)
  // __DECLARE_LOCATED(WORD,QW7)
  
} PROGRAM0;

// BOOL TASK0;
// PROGRAM0 RES0__INSTANCE0;
// #define INSTANCE0 RES0__INSTANCE0

void PROGRAM0_init__(PROGRAM0 *data__, BOOL retain);
// Code part
void PROGRAM0_body__(PROGRAM0 *data__);

#endif /* POUS_H_ */