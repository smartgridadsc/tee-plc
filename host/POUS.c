//-----------------------------------------------------------------------------
// Copyright 2023 ARCS
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



#include <err.h>
#include <stdio.h>
#include <string.h>
#include <accessor.h>
#include "iec_types_all.h"
#include <time.h>
#include <stdlib.h>
#include "ladder.h"

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <scan_cycle.h>

/* the OPTEE related variables are defined here */
TEEC_Result res;
TEEC_Context ctx;
TEEC_Session sess;
TEEC_Operation op;
TEEC_UUID uuid = TA_SCAN_CYCLE_UUID;
uint32_t err_origin;

TEEC_SharedMemory shm_struct;
TEEC_SharedMemory shm_located;

long diff(timespec start, timespec end);
void write_log();

long timing_init = 0;
long timing_core[ITER] = {0};
long timing_final = 0;
timespec tic, toc;

void PROGRAM0_init__(PROGRAM0 *data__, BOOL retain) {
  __INIT_LOCATED(WORD,__IW100,data__->M1,retain)
  __INIT_LOCATED_VALUE(data__->M1,0)
  __INIT_LOCATED(WORD,__IW101,data__->M2,retain)
  __INIT_LOCATED_VALUE(data__->M2,0)
  __INIT_LOCATED(WORD,__QW100,data__->M3,retain)
  __INIT_LOCATED_VALUE(data__->M3,6)
  __INIT_LOCATED(WORD,__QW101,data__->M4,retain)
  __INIT_LOCATED_VALUE(data__->M4,7)

  // ---------------- timing prob initialization tic begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &tic);
  // ----------------- timing prob initialization tic end -------------------

  TEEC_InitializeContext(NULL, &ctx);

  // set shm params for PROGRAM0 struct
  shm_struct = {0};
  shm_struct.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
  shm_struct.buffer = data__;
  shm_struct.size = sizeof(PROGRAM0);

  res = TEEC_RegisterSharedMemory(&ctx, &shm_struct);
  if(res != TEEC_SUCCESS){
    TEEC_ReleaseSharedMemory(&shm_struct);
    errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x",res);
    exit(1);
  }

  // set shm params for located mems
  shm_located = {0};
  shm_located.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
  // notes: we are passing block mem to secure world
  // __IW100 is actually the start loc of the block mem
  shm_located.buffer = data__->M1.value; // this location is currently set manually
  shm_located.size = sizeof(WORD)*4; // this size is currently set manually: M1,M2,M3,M4

  res = TEEC_RegisterSharedMemory(&ctx, &shm_located);
  if(res != TEEC_SUCCESS){
    TEEC_ReleaseSharedMemory(&shm_located);
    errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x", res);
	exit(1);
  }

  /* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

  op.paramTypes = TEEC_PARAM_TYPES(
				TEEC_MEMREF_WHOLE,
				TEEC_MEMREF_WHOLE,
				TEEC_NONE, TEEC_NONE);

  op.params[0].memref.parent = &shm_struct;
  op.params[1].memref.parent = &shm_located;

  res = TEEC_OpenSession(&ctx, &sess, &uuid,TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
	errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",res, err_origin);

  // ---------------- timing prob initialization toc begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &toc);
  timing_init = diff(tic, toc);
  // ----------------- timing prob initialization toc end -------------------
}

// Code part
void PROGRAM0_body__(PROGRAM0 *data__, int iter) {
  // ---------------- timing prob core logic tic begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &tic);
  // ----------------- timing prob core logic tic end -------------------

  res = TEEC_InvokeCommand(&sess, TA_SCAN_CYCLE_CMD_CORE_LOGIC, &op, &err_origin);
  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",res, err_origin);

  // ---------------- timing prob finalization toc begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &toc);
  timing_core[iter] = diff(tic, toc);
  // ----------------- timing prob finalization toc end -------------------
  goto __end;

__end:
  return;
} // PROGRAM0_body__() 

// add this function here to finalize the tee context
void PROGRAM0_end__(PROGRAM0 *data__){
  // ---------------- timing prob finalization tic begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &tic);
  // ----------------- timing prob finalization tic end -------------------

  TEEC_CloseSession(&sess);
  TEEC_FinalizeContext(&ctx);

  // ---------------- timing prob finalization toc begin ------------------
  clock_gettime(CLOCK_MONOTONIC, &toc);
  timing_final = diff(tic, toc);
  // ----------------- timing prob finalization toc end -------------------

  write_log();
}

long diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp.tv_sec*1000000000+temp.tv_nsec;
}

void write_log(){
  FILE *f;
  f = fopen("timing.txt", "w+");
  if(f == NULL){
    printf("Failed to open file");
  }
  fprintf(f, "%ld\n", timing_init);
  for(int i=0; i<ITER; i++){
    fprintf(f, "%ld\n", timing_core[i]);
  }
  fprintf(f, "%ld\n", timing_final);
  fclose(f);
}