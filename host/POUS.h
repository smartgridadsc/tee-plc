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
