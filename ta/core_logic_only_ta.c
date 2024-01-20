//-----------------------------------------------------------------------------
// Copyright 2024 Illinois Advanced Research Center at Singapore
//
// This file is part of TEE-PLC Project.
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


#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <core_logic_only_ta.h>

#include "iec_types_all.h"
#include "accessor.h"

typedef struct {
  // PROGRAM Interface - IN, OUT, IN_OUT variables

  // PROGRAM private variables - TEMP, private and located variables
  __DECLARE_LOCATED(WORD,M1)
  __DECLARE_LOCATED(WORD,M2)
  __DECLARE_LOCATED(WORD,M3)
  __DECLARE_LOCATED(WORD,M4)

} PROGRAM0;

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result core_logic(uint32_t param_types, TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	
	if(param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	
	
	PROGRAM0* data__ = (PROGRAM0*)params[0].memref.buffer;

	// IMSG("NW Address of IW100 is: %p", (void*)(data__->M1.value));
	// IMSG("NW Address of IW101 is: %p", (void*)(data__->M2.value));
	// IMSG("NW Address of QW100 is: %p", (void*)(data__->M3.value));
	// IMSG("NW Address of QW101 is: %p", (void*)(data__->M4.value));
	

	// first store the original value of the pointers
	IEC_WORD* m1p = data__->M1.value;
	IEC_WORD* m2p = data__->M2.value;
	IEC_WORD* m3p = data__->M3.value;
	IEC_WORD* m4p = data__->M4.value;

	// then replace all the pointers with the translated value
	// this buffer is the real space to restore IW100-QW101
	IEC_WORD* mapped_loc = (IEC_WORD*)params[1].memref.buffer;
	data__->M1.value = &mapped_loc[0];
	data__->M2.value = &mapped_loc[1];
	data__->M3.value = &mapped_loc[2];
	data__->M4.value = &mapped_loc[3];

	// IMSG("traslated address of IW100 is: %p", (void*)data__->M1.value);
	// IMSG("traslated address of IW101 is: %p", (void*)data__->M2.value);
	// IMSG("traslated address of QW100 is: %p", (void*)data__->M3.value);
	// IMSG("traslated address of QW101 is: %p", (void*)data__->M4.value);

	// IMSG("data__->M1.value is %d", *(data__->M1.value));
	// IMSG("data__->M2.value is %d", *(data__->M2.value));
	// IMSG("data__->M3.value is %d", *(data__->M3.value));
	// IMSG("data__->M4.value is %d", *(data__->M4.value));
	
	// core logic of PLC
  	__SET_LOCATED(data__->,M3,,__GET_LOCATED(data__->M1,));
  	__SET_LOCATED(data__->,M4,,__GET_LOCATED(data__->M2,));

	// and finally assign the pointers back
	data__->M1.value = m1p;
	data__->M2.value = m2p;
	data__->M3.value = m3p;
	data__->M4.value = m4p;

	// IMSG("R:NW Address of IW100 is: %p", (void*)(data__->M1.value));
	// IMSG("R:NW Address of IW101 is: %p", (void*)(data__->M2.value));
	// IMSG("R:NW Address of QW100 is: %p", (void*)(data__->M3.value));
	// IMSG("R:NW Address of QW101 is: %p", (void*)(data__->M4.value));

	return TEE_SUCCESS;
}


/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_CORE_LOGIC_CMD_CORE_LOGIC:
		return core_logic(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
