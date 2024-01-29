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

// #include "iec_std_lib.h"
#include "iec_types_all.h"
#include "accessor.h"
#include "Config0.h"


// CONFIGURATION CONFIG0

void RES0_init__(void);

void config_init__(void) {
  BOOL retain;
  retain = 0;
  
  RES0_init__();
}

void RES0_run__(unsigned long tick);

void config_run__(unsigned long tick) {
  RES0_run__(tick);
}
unsigned long long common_ticktime__ = 2000000ULL; /*ns*/
unsigned long greatest_tick_count__ = 0UL; /*tick*/
