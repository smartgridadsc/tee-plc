//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
// This file is part of the OpenPLC Software Stack.
//
// OpenPLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenPLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenPLC.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// This file is responsible for gluing the variables from the IEC program to
// the OpenPLC memory pointers. It is automatically generated by the
// glue_generator program. PLEASE DON'T EDIT THIS FILE!
// Thiago Alves, May 2016
//-----------------------------------------------------------------------------

// #include "iec_std_lib.h"
#include "iec_types_all.h"

// TIME __CURRENT_TIME;
extern unsigned long long common_ticktime__;

//Internal buffers for I/O and memory. These buffers are defined in the
//auto-generated glueVars.cpp file
#define BUFFER_SIZE		1024

//Booleans
IEC_BOOL *bool_input[BUFFER_SIZE][8];
IEC_BOOL *bool_output[BUFFER_SIZE][8];

//Bytes
IEC_BYTE *byte_input[BUFFER_SIZE];
IEC_BYTE *byte_output[BUFFER_SIZE];

//Analog I/O
IEC_UINT *int_input[BUFFER_SIZE];
IEC_UINT *int_output[BUFFER_SIZE];

//Memory
IEC_UINT *int_memory[BUFFER_SIZE];
IEC_DINT *dint_memory[BUFFER_SIZE];
IEC_LINT *lint_memory[BUFFER_SIZE];

//Special Functions
IEC_LINT *special_functions[BUFFER_SIZE];


#define __LOCATED_VAR(type, name, ...) type __##name;
#include "LOCATED_VARIABLES.h"
#undef __LOCATED_VAR
#define __LOCATED_VAR(type, name, ...) type* name = &__##name;
#include "LOCATED_VARIABLES.h"
#undef __LOCATED_VAR

void glueVars(void);

void glueVars(void)
{
	bool_input[100][0] = __IX100_0;
	bool_input[100][1] = __IX100_1;
	bool_input[100][2] = __IX100_2;
	bool_input[100][3] = __IX100_3;
	bool_input[100][4] = __IX100_4;
	bool_input[100][5] = __IX100_5;
	bool_input[100][6] = __IX100_6;
	bool_input[100][7] = __IX100_7;
	bool_output[100][0] = __QX100_0;
	bool_output[100][1] = __QX100_1;
	bool_output[100][2] = __QX100_2;
	bool_output[100][3] = __QX100_3;
	bool_output[100][4] = __QX100_4;
	bool_output[100][5] = __QX100_5;
	bool_output[100][6] = __QX100_6;
	bool_output[100][7] = __QX100_7;
	bool_input[101][0] = __IX101_0;
	bool_input[101][1] = __IX101_1;
	bool_input[101][2] = __IX101_2;
	bool_input[101][3] = __IX101_3;
	bool_input[101][4] = __IX101_4;
	bool_input[101][5] = __IX101_5;
	bool_input[101][6] = __IX101_6;
	bool_input[101][7] = __IX101_7;
	bool_output[101][0] = __QX101_0;
	bool_output[101][1] = __QX101_1;
	bool_output[101][2] = __QX101_2;
	bool_output[101][3] = __QX101_3;
	bool_output[101][4] = __QX101_4;
	bool_output[101][5] = __QX101_5;
	bool_output[101][6] = __QX101_6;
	bool_output[101][7] = __QX101_7;

	int_input[100] = __IW100;
	int_input[101] = __IW101;
	int_input[102] = __IW102;
	int_input[103] = __IW103;
	int_input[104] = __IW104;
	int_input[105] = __IW105;
	int_input[106] = __IW106;
	int_input[107] = __IW107;
	int_input[108] = __IW108;
	int_input[109] = __IW109;
	int_input[110] = __IW110;
	int_input[111] = __IW111;
	int_input[112] = __IW112;
	int_input[113] = __IW113;
	int_input[114] = __IW114;
	int_input[115] = __IW115;

	int_output[100] = __QW100;
	int_output[101] = __QW101;
	int_output[102] = __QW102;
	int_output[103] = __QW103;
	int_output[104] = __QW104;
	int_output[105] = __QW105;
	int_output[106] = __QW106;
	int_output[107] = __QW107;	
	int_output[108] = __QW108;
	int_output[109] = __QW109;
	int_output[110] = __QW110;
	int_output[111] = __QW111;
	int_output[112] = __QW112;
	int_output[113] = __QW113;
	int_output[114] = __QW114;
	int_output[115] = __QW115;	

}

void updateTime()
{
	// __CURRENT_TIME.tv_nsec += common_ticktime__;

	// if (__CURRENT_TIME.tv_nsec >= 1000000000)
	// {
	// 	__CURRENT_TIME.tv_nsec -= 1000000000;
	// 	__CURRENT_TIME.tv_sec += 1;
	// }
}