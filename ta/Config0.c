/*******************************************/
/*     FILE GENERATED BY iec2c             */
/* Editing this file is not recommended... */
/*******************************************/

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
