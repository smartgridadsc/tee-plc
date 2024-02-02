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

#include "iec_std_lib.h"

#include "accessor.h"

#include "POUS.h"

// CONFIGURATION CONFIG0

void RES0_init__(void);

void config_init__(void) {
  BOOL retain;
  retain = 0;
  
  RES0_init__();
}

void RES0_run__(unsigned long tick, int iter);

void config_run__(unsigned long tick, int iter) {
  RES0_run__(tick, iter);
}

// only used for optee to end context
void RES0_end__(void);

void config_end__(void) {
  RES0_end__();
}
unsigned long long common_ticktime__ = 20000000ULL; /*ns*/
unsigned long greatest_tick_count__ = 0UL; /*tick*/
