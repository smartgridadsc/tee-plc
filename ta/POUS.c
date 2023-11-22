#define __lit(type,value,...) (type)value##__VA_ARGS__
// Keep this macro expention step to let sfx(__VA_ARGS__) change into L or LL
#define __literal(type,value,...) __lit(type,value,__VA_ARGS__)
#define __BOOL_LITERAL(value) __literal(BOOL,value)

void PROGRAM0_init__(PROGRAM0 *data__, BOOL retain) {
  __INIT_LOCATED(BOOL,__IX100_0,data__->M10,retain)
  __INIT_LOCATED_VALUE(data__->M10,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_1,data__->M11,retain)
  __INIT_LOCATED_VALUE(data__->M11,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_2,data__->M12,retain)
  __INIT_LOCATED_VALUE(data__->M12,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_3,data__->M13,retain)
  __INIT_LOCATED_VALUE(data__->M13,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_4,data__->M14,retain)
  __INIT_LOCATED_VALUE(data__->M14,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_5,data__->M15,retain)
  __INIT_LOCATED_VALUE(data__->M15,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_6,data__->M16,retain)
  __INIT_LOCATED_VALUE(data__->M16,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX100_7,data__->M17,retain)
  __INIT_LOCATED_VALUE(data__->M17,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_0,data__->MQ10,retain)
  __INIT_LOCATED_VALUE(data__->MQ10,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_1,data__->MQ11,retain)
  __INIT_LOCATED_VALUE(data__->MQ11,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_2,data__->MQ12,retain)
  __INIT_LOCATED_VALUE(data__->MQ12,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_3,data__->MQ13,retain)
  __INIT_LOCATED_VALUE(data__->MQ13,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_4,data__->MQ14,retain)
  __INIT_LOCATED_VALUE(data__->MQ14,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_5,data__->MQ15,retain)
  __INIT_LOCATED_VALUE(data__->MQ15,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_6,data__->MQ16,retain)
  __INIT_LOCATED_VALUE(data__->MQ16,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX100_7,data__->MQ17,retain)
  __INIT_LOCATED_VALUE(data__->MQ17,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_0,data__->M20,retain)
  __INIT_LOCATED_VALUE(data__->M20,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_1,data__->M21,retain)
  __INIT_LOCATED_VALUE(data__->M21,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_2,data__->M22,retain)
  __INIT_LOCATED_VALUE(data__->M22,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_3,data__->M23,retain)
  __INIT_LOCATED_VALUE(data__->M23,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_4,data__->M24,retain)
  __INIT_LOCATED_VALUE(data__->M24,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_5,data__->M25,retain)
  __INIT_LOCATED_VALUE(data__->M25,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_6,data__->M26,retain)
  __INIT_LOCATED_VALUE(data__->M26,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__IX101_7,data__->M27,retain)
  __INIT_LOCATED_VALUE(data__->M27,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_0,data__->MQ20,retain)
  __INIT_LOCATED_VALUE(data__->MQ20,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_1,data__->MQ21,retain)
  __INIT_LOCATED_VALUE(data__->MQ21,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_2,data__->MQ22,retain)
  __INIT_LOCATED_VALUE(data__->MQ22,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_3,data__->MQ23,retain)
  __INIT_LOCATED_VALUE(data__->MQ23,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_4,data__->MQ24,retain)
  __INIT_LOCATED_VALUE(data__->MQ24,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_5,data__->MQ25,retain)
  __INIT_LOCATED_VALUE(data__->MQ25,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_6,data__->MQ26,retain)
  __INIT_LOCATED_VALUE(data__->MQ26,__BOOL_LITERAL(FALSE))
  __INIT_LOCATED(BOOL,__QX101_7,data__->MQ27,retain)
  __INIT_LOCATED_VALUE(data__->MQ27,__BOOL_LITERAL(FALSE))
 
  // __INIT_LOCATED(WORD,__IW100,data__->IW0,retain)
  // __INIT_LOCATED_VALUE(data__->IW0,0)
  // __INIT_LOCATED(WORD,__IW101,data__->IW1,retain)
  // __INIT_LOCATED_VALUE(data__->IW1,0)
  // __INIT_LOCATED(WORD,__IW102,data__->IW2,retain)
  // __INIT_LOCATED_VALUE(data__->IW2,0)
  // __INIT_LOCATED(WORD,__IW103,data__->IW3,retain)
  // __INIT_LOCATED_VALUE(data__->IW3,0)
  // __INIT_LOCATED(WORD,__IW104,data__->IW4,retain)
  // __INIT_LOCATED_VALUE(data__->IW4,0)
  // __INIT_LOCATED(WORD,__IW105,data__->IW5,retain)
  // __INIT_LOCATED_VALUE(data__->IW5,0)
  // __INIT_LOCATED(WORD,__IW106,data__->IW6,retain)
  // __INIT_LOCATED_VALUE(data__->IW6,0)
  // __INIT_LOCATED(WORD,__IW107,data__->IW7,retain)
  // __INIT_LOCATED_VALUE(data__->IW7,0)

  // __INIT_LOCATED(WORD,__QW100,data__->QW0,retain)
  // __INIT_LOCATED_VALUE(data__->QW0,0)
  // __INIT_LOCATED(WORD,__QW101,data__->QW1,retain)
  // __INIT_LOCATED_VALUE(data__->QW1,0)
  // __INIT_LOCATED(WORD,__QW102,data__->QW2,retain)
  // __INIT_LOCATED_VALUE(data__->QW2,0)
  // __INIT_LOCATED(WORD,__QW103,data__->QW3,retain)
  // __INIT_LOCATED_VALUE(data__->QW3,0)
  // __INIT_LOCATED(WORD,__QW104,data__->QW4,retain)
  // __INIT_LOCATED_VALUE(data__->QW4,0)
  // __INIT_LOCATED(WORD,__QW105,data__->QW5,retain)
  // __INIT_LOCATED_VALUE(data__->QW5,0)
  // __INIT_LOCATED(WORD,__QW106,data__->QW6,retain)
  // __INIT_LOCATED_VALUE(data__->QW6,0)
  // __INIT_LOCATED(WORD,__QW107,data__->QW7,retain)
  // __INIT_LOCATED_VALUE(data__->QW7,0)
}

// Code part
void PROGRAM0_body__(PROGRAM0 *data__) {
  // Initialise TEMP variables

  // The core logic is moved to TA
  // we still need the program0_init_ to work as a block of shared memory

  // __SET_LOCATED(data__->,MQ10,,__GET_LOCATED(data__->M10,));
  // __SET_LOCATED(data__->,MQ11,,__GET_LOCATED(data__->M11,));
  // __SET_LOCATED(data__->,MQ12,,__GET_LOCATED(data__->M12,));
  // __SET_LOCATED(data__->,MQ13,,__GET_LOCATED(data__->M13,));
  // __SET_LOCATED(data__->,MQ14,,__GET_LOCATED(data__->M14,));
  // __SET_LOCATED(data__->,MQ15,,__GET_LOCATED(data__->M15,));
  // __SET_LOCATED(data__->,MQ16,,__GET_LOCATED(data__->M16,));
  // __SET_LOCATED(data__->,MQ17,,__GET_LOCATED(data__->M17,));

  // __SET_LOCATED(data__->,MQ20,,__GET_LOCATED(data__->M20,));
  // __SET_LOCATED(data__->,MQ21,,__GET_LOCATED(data__->M21,));
  // __SET_LOCATED(data__->,MQ22,,__GET_LOCATED(data__->M22,));
  // __SET_LOCATED(data__->,MQ23,,__GET_LOCATED(data__->M23,));
  // __SET_LOCATED(data__->,MQ24,,__GET_LOCATED(data__->M24,));
  // __SET_LOCATED(data__->,MQ25,,__GET_LOCATED(data__->M25,));
  // __SET_LOCATED(data__->,MQ26,,__GET_LOCATED(data__->M26,));
  // __SET_LOCATED(data__->,MQ27,,__GET_LOCATED(data__->M27,));

  // ===============================================================
  // here we donot use int for core logic, we use it for measurement
  // ===============================================================
  // __SET_LOCATED(data__->,WQ11,,__GET_LOCATED(data__->W11,));
	// __SET_LOCATED(data__->,WQ12,,__GET_LOCATED(data__->W12,));
  
  // __SET_LOCATED(data__->,WQ21,,__GET_LOCATED(data__->W21,));
	// __SET_LOCATED(data__->,WQ22,,__GET_LOCATED(data__->W22,));

  goto __end;

__end:
  return;

} // PROGRAM0_body__() 