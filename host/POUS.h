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


#ifndef __POUS_H
#define __POUS_H

#include "accessor.h"
#include "iec_std_lib.h"
#include <tee_client_api.h>

// PROGRAM PROGRAM0
// Data part
typedef struct {
  // PROGRAM Interface - IN, OUT, IN_OUT variables

  // PROGRAM private variables - TEMP, private and located variables
  __DECLARE_LOCATED(WORD,M1)
  __DECLARE_LOCATED(WORD,M2)
  __DECLARE_LOCATED(WORD,M3)
  __DECLARE_LOCATED(WORD,M4)

} PROGRAM0;

void PROGRAM0_init__(PROGRAM0 *data__, BOOL retain);
// Code part
void PROGRAM0_body__(PROGRAM0 *data__, int iter);

#endif //__POUS_H
