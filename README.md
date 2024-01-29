# Enhanced TEE-PLC

This repository is the source code of Enhanced TEE-PLC. The normal world OpenPLC Runtime is in ./host, while the secure world Scan-cycle TA is in ./ta.

**Note:** Before you start, please make sure you have finished all the steps mentioned in main branch README and successfully run OpenPLC and OP-TEE on RPI3.

## Build & Run

### Prerequisites

- Build this project in Linux. We built this project on Ubuntu20.04.
- Use the toolchain `aarch64-linux-gnu-` offered by op-tee. The building scripts are provided in subfolders `host` and `ta`. You may need to add toolchains to PATH before you run the scripts. The detailed compilation guide for building optee-client and optee-ta can be found in https://optee.readthedocs.io/en/latest/building/gits/optee_examples/optee_examples.html.
- You need to build `optee_os` and `optee_client` seperately before you run the scripts. The detailed building guide can be found in https://optee.readthedocs.io/en/latest/building/gits/optee_client.html and https://optee.readthedocs.io/en/latest/building/gits/optee_os.html.

### Build WolfSSL library
Our implementation relies on WolfSSL for Modbus/TLS connection. Although op-tee is not natively supported by WolfSSL, [this repo](https://github.com/wenshei/wolfssl) has ported WolfSSL to op-tee. You can follow [this guide](https://github.com/wenshei/wolfssl/blob/master/FOR-OPTEE) to build the static library.

By default, the static library will be found in `/home/ubuntu/projects/wolfssl/lib/libwolfssl.a` and header files will be put in `/home/ubuntu/projects/wolfssl/include`. You can change the output directory to whatever you like by running `./configure -prefix="<path/to/output/directory>"`.

Once you get the static library, put it under folder `ta` (`ta/libwolfssl.a`). You also need to modify the `ta/sub.mk` file to tell the compiler the location of the header files of WolfSSL.


### Generate certificates
Run the following command to generate self-signed certificates for testing.

``` shell
cd <path/to/wolfssl>
./certs/renewcerts.sh
```

Copy the following certificates to `host/certs` folder and `others/pymodbus/certs` folder.

- ca-cert.pem
- client-ca.pem
- client-cert.pem
- client-key.pem
- root-ca-cert.pem
- server-cert.pem
- server-key.pem

### Build

Run the following command:

``` shell
cd host
./compile.sh
cd ..
cd ta
./compile_ta.sh
```

If you see any errors, please make sure you followed the steps in prerequisite and build the static library correctly.

## Debug

- You may see `0xffffeeee` errors from ta console. Please check if the IP address hardcoded in `ta/mymodbus.c` is the one of your PC.
- You may see the whole system gets stuck. This is because OP-TEE is scheduled by Linux and SW and NW cannot run simultanously. If SW is stuck, the whole system gets stuck. Please quit qemu and debug before rerun.
- Your may see diagnostic information printed in console. Please use `optee_os/scripts/symbolize.py` to see the last function OP-TEE called before exiting. The instruction can be found from this website https://optee.readthedocs.io/en/latest/debug/abort_dumps.html.
- Please add the following trace message functions to where you want to inspect the value of variables. OP-TEE offers MSG, IMSG, DMSG, FMSG to differentiate logs into different levels. When building TA, you can configure the CFLAG **CFG_TEE_CORE_LOG_LEVEL={0|1|2|3|4}** in file `ta/Makefile` to specify which level of trace messages will be printed. Typically when we want to debug, we set the log level to 4 to print every message. When we clear out all bugs and want to measure the performance, we set the log level to 0.

| Value | Name              | Description with related macros                              |
| ----- | ----------------- | ------------------------------------------------------------ |
| 0     | -                 | All trace messages are disabled                              |
| 1     | Error trace level | Only non-tagged and error trace messages are embedded: MSG(), MSG_RAW(), EMSG(), EMSG_RAW() |
| 2     | Info trace level  | + info trace messages: IMSG(), IMSG_RAW()                    |
| 3     | Debug trace level | + debug trace messages: DMSG(), DMSG_RAW()                   |
| 4     | Flow trace level  | + flow trace messages: FMSG(), FMSG_RAW()                    |

## OpenPLC Runtime Explanation

### Variable mapping

This section talks about how the internal memory used by control logic is mapped to modbus client and is updated by modbus client.

#### Macro definition in `LOCATED_VARIABLES.h`

MatIEC compiles control logic into C code and generates `LOCATED_VARIABLES.h` containing the following macros.

``` c
__LOCATED_VAR(BOOL,__IX100_0,I,X,100,0)
__LOCATED_VAR(BOOL,__IX100_1,I,X,100,1)
```

#### Macro expansion in `glueVars.cpp`

The macros are expanded to boolean pointers `__IX100_0` and boolean value `____IX100_0`. `__IX100_0` is assigned with address of `____IX100_0`:

``` c
// these following lines expands to bool ____IX100_0;
#define __LOCATED_VAR(type, name, ...) type __##name;
#include "LOCATED_VARIABLES.h"
#undef __LOCATED_VAR

// these following lines expands to bool* __IX100_0 = & ____IX100_0;
#define __LOCATED_VAR(type, name, ...) type* name = &__##name;
#include "LOCATED_VARIABLES.h"
#undef __LOCATED_VAR
```

#### Mapping Macros to Variable Names in `POUS.c` and `POUS.h`

Only the pointers are used in control logic file `POUS.c` and `POUS.h`.

``` c
// POUS.h
typedef struct {
    __DECLARE_LOCATED(BOOL,M10)
/* Expands to:
__IEC_BOOL_p M10;
*/
} PROGRAM0;

/* __IEC_BOOL_p:
typedef struct {
    BOOL *value; 
    BYTE flags; 
    BOLL fvalues;
} __IEC_BOOL_p;
*/

// POUS.c
void PROGRAM0_init__(PROGRAM0 *data__, BOOL retain) {
	__INIT_LOCATED(BOOL,__IX100_0,data__->M10,retain)
/* Expands to:
{ extern BOOL *__IX100_0; data__->M10.value = __IX100_0; __INIT_RETAIN(data__->M10, retain) }
*/
}
```

`extern` keyword ask linker to look for __IX100_0 in other `.o` files. Then `M10.value(pointer)` is mapped to `__IX100_0`. 

#### Mapping Macros to Internal Buffers in `glueVar.cpp`

Internal buffers used by control logic are basically arrays of boolean and int. These arrays will be assigned to the located variables one by one.

```c
bool_input[100][0] = __IX100_0;
```

#### Internal Buffers updated in `modbus_master.cpp` 

The `modbus_master.cpp` is the modbus server for HMI. It reads the value from internal buffer and reflects to HMI monitor page.

``` c
// in modbus_master.cpp

void updateBuffersIn_MB()
{
    pthread_mutex_lock(&ioLock);

    for (int i = 0; i < MAX_MB_IO; i++)
    {
        if (bool_input[100+(i/8)][i%8] != NULL) *bool_input[100+(i/8)][i%8] = bool_input_buf[i];
        if (int_input[100+i] != NULL) *int_input[100+i] = int_input_buf[i];
    }

    pthread_mutex_unlock(&ioLock);
}
```

#### Internal Buffers updated in `modbus.cpp`

The file `modbus.cpp` is the modbus client which polls data from modbus slaves. Internal buffer `bool_input` is assigned with value in `bool_input_buf` which is the sensor reading from modbus slave registers.

``` c
// in modbus.cpp
void ReadDiscreteInputs(unsigned char *buffer, int bufferSize)
{
	if (bool_input[position/8][position%8] != NULL){
		bitWrite(buffer[9 + i], j, *bool_input[position/8][position%8]);
	}
}
```

## Port control logic and Modbus Client to TA

For openplc runtime in NW, we only need to copy the value from SW through shared memory. Thus shared memory registration can be done by a simple parser parsing the variables declared in `LOCATED_VARIABLES.h`. Also, since we donot need modbus_master in NW, we donot need `modbus_master.cpp`. In addition, we donot run control logic in NW, so we donot need the variables in POUS.c. Finally, the mapping from `bool_input` to `IX100_0` has been done by `glueVars.cpp`, we donot need to map ourselves. For shared memory registration, we only need to set the shared memory starting from `IX100_0` and its size to the sum of all the macros.

For openplc SW, we need all the mappings from macro to internal memory, to modbus_buffers, to have a complete compatibility with IEC61311-3.

One thing to mention is the timer in openplc. Timer is set in NW by openplc as sleep_until() function. Since SW cannot invoke itself, there are two ways to do this.

- Depend on NW to invoke TA
- Implement a timer in SW to periodically execute scan cycle. This need a interrupt handler to run FIQ context.

For the first solution, we may be not able to avoid the DoS attack. For the second solution, we need to carefully implement the functionality. For now let's just stick on the first solution.

In summary, the modification to the original openplc generated files should be like this:

| Files                             | Functions                                                    | NW        | SW                    |
| --------------------------------- | ------------------------------------------------------------ | --------- | --------------------- |
| glueVars.cpp                      | 1. Expand macros (L55)<br />2. map internal buffer to macros (L65) | no change | no change             |
| LOCATED_VARIABLES.h               | Define macros<br />Host/TA can know shm size by parsing it   | no change | no change             |
| modbus_master.cpp                 | Serves python HMI and reflects internal buffer to the monitor page | no need   | change to my_modbus.c |
| modbus.cpp                        | Modbus client to poll data from slaves                       | no need   | no need               |
| POUS.h                            | map macros to var (pointers)<br />`IEC_BOOL_p M10`           | no need   | no change             |
| POUS.c                            | POUS and var initialization                                  | no need   | no change             |
| Res0.h                            | No such file                                                 | N/A       | N/A                   |
| Res0.c                            | invoke POUS in POUS.c                                        | no need   | no change             |
| Config0.h                         | Empty                                                        | no need   | no change             |
| Config0.c                         | set up ticktime used by scan cycle                           | no need   | no change             |
| lib/iec_xxx.h<br />lib/accessor.h | IEC 61131-3 compatible libraries. <br />We need these headers to have full support for IEC 61131-3 in TA. | no need   | no change             |

## Note

#### 1. Reversed Located Variables

Located variables in SW may be allocated in memory from high address to low address. However, in NW, shared memory is allocated from low address to high address by default. That means when pass data from SW to NW through shared memory, we may not pass the correct buffer. In this case we need to reverse the variables in `LOCATED_VARIABLES.h`. The script is provided as `ta/reverse_lines.py`. To check how the located variables is arranged in memory, you can run `readelf -s 0bbd4f5b-ab7b-4dcd-a1f5-9b2eedaf0d1e.elf | grep ____IX100` and can see the variables as follows:

``` shell
  1254: 00000000000a5e2d     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_5
  1351: 00000000000a5e2f     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_7
  1468: 00000000000a5e2e     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_6
  1603: 00000000000a5e28     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_0
  1665: 00000000000a5e2a     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_2
  1751: 00000000000a5e29     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_1
  1845: 00000000000a5e2b     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_3
  2066: 00000000000a5e2c     1 OBJECT  GLOBAL DEFAULT   14 ____IX100_4
```

From which we see the size of each symbol is 1byte, meaning these are boolean values not pointers. We can find `____IX100_0` is at `a5e28` and `____IX100_1` is at `a5e29`. This means the variables are allocated from low to high and is in the same order in NW. We donot need to reverse the variables in `LOCATED_VARIABLES.h`.

#### 2. `int_input[]` and `int_output[]` are used as shared memory to pass data from SW to NW

This makes the code very confusing and not scalable. A better design is to register another shared memory and use this piece of memory as the dedicated shared memory to pass data from SW to NW.

``` c
// my_modbus.c : 1240
void updateTimeStampInSHM(uint16_t core_logic_time)
{
    m.time_exit = read_cntpct();
    ts_arr[ts_idx++] = pctcnt2us(m.time_exit - m.time_entry);
    IMSG("ts_arr[%d] is ts for time_exit", ts_idx);

    IMSG("value of ts_idx is %d, should be no larger than 32.", ts_idx);

    // reuse the int_buffer to pass the measurement to normal world
    for(int i=0; i<16; i++){
        *int_input[100+i] = ts_arr[i];
    }

    for(int i=0; i<6; i++){
        *int_output[100+i] = ts_arr[i+16];
    }

    *int_output[107] = core_logic_time;
    *int_output[108] = m.time_enc;
    *int_output[109] = m.time_dec; 
}
```

#### 3.Port `newlib` to fully support IEC 61131-3 function blocks

The IEC 61131-3 libraries rely on math functions in libc. However, optee_os does not support all math functions in libc. Thus some function blocks provided by IEC 61131-3 libraries cannot compile successfully. OP-TEE suggests to port newlib to `optee_os` since `optee_os` has already ported partial newlib. The project of newlib can be found from https://sourceware.org/newlib/

Current implement of Enhanced TEE-PLC doesnot use IEC 61131-3 function blocks. We only use headers such as `iec_types_all.h` and `accessor.h`. We also copied following macro expansion code from `iec_std_lib.h` to `POUS.c`, instead of directly include `iec_std_lib.h` in `POUS.c` which can leads to compilation error.

``` c
#define __lit(type,value,...) (type)value##__VA_ARGS__
// Keep this macro expention step to let sfx(__VA_ARGS__) change into L or LL
#define __literal(type,value,...) __lit(type,value,__VA_ARGS__)
#define __BOOL_LITERAL(value) __literal(BOOL,value)
```

