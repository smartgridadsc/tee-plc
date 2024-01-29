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


// optee libutee
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "string.h"
#include "string_ext.h"
#include <arm_user_sysreg.h>

// modbus related headers
#include "my_modbus.h"

// ta headers
#include "scan_cycle.h"
#include "scan_cycle_param.h"

// iec61131-3
#include "POUS.h"
#include "Config0.h"

uint64_t tic_logic, toc_logic;

uint32_t pctcnt2us(uint64_t cnt)
{
	return (cnt * 1000000ULL) / read_cntfrq();
}

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	return TEE_SUCCESS;
}


void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}


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

	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}


void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */

	IMSG("Goodbye!\n");
}

static TEE_Result initialize(uint32_t param_types, TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	int slave_num = params[0].value.a;
	DMSG("slave_num is: %d", slave_num);

	// ==============================================
	//            iec61131-3 program init
	// ==============================================
	config_init__();

	TEE_Result res;
	res = modbus_init(slave_num);


	// ==============================================
	//              benchmark 
	// ==============================================
	uint64_t pct_1, pct_2;
	uint32_t frq;
	pct_1 = barrier_read_counter_timer();
	for(int i=0; i<1000; i++)
		frq = read_cntfrq();
	pct_2 = barrier_read_counter_timer();
	MSG("frq is %lu, frq_measure_time(*1000)=%lu", frq, pctcnt2us(pct_2-pct_1));

	
	pct_1 = barrier_read_counter_timer();
	for(int i=0; i<1000; i++)
		read_cntpct();
	pct_2 = barrier_read_counter_timer();
	MSG("pct_measure_time(*1000)=%lu", pctcnt2us(pct_2-pct_1));


	pct_1 = barrier_read_counter_timer();
	for(int i= 0; i<100; i++){
		MSG("test uart. this line has no meaning. %lu, %lu\n", pctcnt2us(pct_2-pct_1), pctcnt2us(pct_2-pct_1));
	}
	pct_2 = barrier_read_counter_timer();
	MSG("uart_time=%lu", pctcnt2us((pct_2-pct_1)/100));

	if(res != TEE_SUCCESS){
		DMSG("TA: initialize: modbus_init faled. Error Code: %x", res);
		return res;
	}
	res = modbus_connect_all();
	return res;
}

static TEE_Result finalize(uint32_t param_types, TEE_Param params[4]){
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	
	TEE_Result res;
	res = modbus_close_all();
	if(res != TEE_SUCCESS) DMSG("Close Modbus Failed: %x", res);
	// TODO: should try to close again
	modbus_clear();
	return TEE_SUCCESS;
}


static uint32_t update_modbus_input(void){
	//modbus_poll();
	
	uint32_t res = querySlaveRead();
	updateBuffersIn_MB();
	// need to update the internal buffer then pass to NW
	return res;
}

static uint32_t update_modbus_output(void){
	updateBuffersOut_MB();
	uint32_t res = querySlaveWrite();
	// need to update the internal buffer then pass to NW
	updateTimeStampInSHM(pctcnt2us(toc_logic-tic_logic));
	return res;
}

static TEE_Result execute_core_logic(void){
	//=========================================
	//         iec 61131-3 control logic
	//=========================================
	// TODO: update tick
	uint32_t tick = 0;
	config_run__(tick);
	// TODO: update tick
	return TEE_SUCCESS;
}

// This is the scan cycle of a PLC
static TEE_Result scan_cycle(uint32_t param_types,TEE_Param params[4]){
	
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	uint32_t res_bit = 0;

#if __DEBUG__
	MSG(" ==================== SCAN CYCLE %d BEGIN ====================", params[0].value.a);
#endif

	extern void glueVars(void);
	glueVars();

	res_bit = update_modbus_input();


	tic_logic = barrier_read_counter_timer();
	execute_core_logic();
	toc_logic = barrier_read_counter_timer();

	IMSG("control logic execution time: %lu us", pctcnt2us(toc_logic-tic_logic));

	res_bit |= update_modbus_output(); // already in higher 16 bit

	// reflect value to shared memory
	extern uint8_t* __IX100_0;
	uint8_t* shm = params[2].memref.buffer;

	// manually set the shared memory size
	// and also the print function
	TEE_MemMove(shm, __IX100_0, sizeof(uint8_t)*32+sizeof(uint16_t)*32); // bool is 1byte, pointer is 8byte

	// { // just curly braces the part to avoid the leakage of pof
	// 	char shm_msg[100] = {0};
	// 	char* pof = shm_msg;
	// 	pof += snprintf(pof, 13, "SHM to NW: ");
	// 	for(int i=0; i<32; i++){
	// 		pof += snprintf(pof,3, "%u ",shm[i]);
	// 	}
	// 	for(int i=32; i<32+16; i+=2){ // each register takes 2 bytes
	// 		pof += snprintf(pof,8, "0x%x ",shm[i+1]<<8 | shm[i]);
	// 	}
	// 	IMSG("%s", shm_msg);
	// }


	params[0].value.b = res_bit;

#if __DEBUG__
	MSG(" ==================== SCAN CYCLE %d  END  ====================", params[0].value.a);
#endif

	return TEE_SUCCESS;
}


TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	// we need the sess_ctx to record time of each part
	// (void)&sess_ctx; /* Unused parameter */
	uint64_t tic_scancyc, toc_scancyc;
	TEE_Result res;

	switch (cmd_id) {
	case TA_SCAN_CYCLE_INIT: // should be called during openplc initialization
		return initialize(param_types, params);
	case TA_SCAN_CYCLE_EXEC:
		// tic_scancyc = barrier_read_counter_timer();
		res = scan_cycle(param_types, params);
		// toc_scancyc = barrier_read_counter_timer();
		// MSG("Time measurement of scan cycle in TA is %u us", pctcnt2us(toc_scancyc-tic_scancyc));
		return res;
	case TA_SCAN_CYCLE_EXIT: // should be called during openplc finalization
		return finalize(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
