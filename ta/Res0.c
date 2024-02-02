//-----------------------------------------------------------------------------
// Copyright 2024 Illinois Advanced Research Center at Singapore Ltd.
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

// #include "iec_std_lib.h"
#include "iec_types_all.h"

// RESOURCE RES0

extern unsigned long long common_ticktime__;

#include "accessor.h"
#include "POUS.h"

#include "Config0.h"

#include "POUS.c"

// we only need the data, not the whole struct
BOOL TASK0;
PROGRAM0 RES0__INSTANCE0;
#define INSTANCE0 RES0__INSTANCE0

void RES0_init__(void) {
  BOOL retain;
  retain = 0;
  
  TASK0 = 0; //__BOOL_LITERAL(FALSE);
  PROGRAM0_init__(&INSTANCE0,retain);
}

void RES0_run__(unsigned long tick) {
  // TASK0 = !(tick % 1); // TODO: the timer to control the updating rate
  TASK0 = 1;
  if (TASK0) {
    PROGRAM0_body__(&INSTANCE0);
  }
}

