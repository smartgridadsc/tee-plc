//-----------------------------------------------------------------------------
// Copyright 2018 Thiago Alves
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
// This is the main file for the OpenPLC. It contains the initialization
// procedures for the hardware, network and the main loop
// Thiago Alves, Jun 2018
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "iec_types.h"
#include "ladder.h"

#include <tee_client_api.h>
#include <scan_cycle.h>
#include <err.h>


extern int opterr;
//extern int common_ticktime__;
IEC_BOOL __DEBUG;

IEC_LINT cycle_counter = 0;

unsigned long __tick = 0;
// even if we don't need the internal buffer to store the intermediate variables
// we can still reserve the bufferLock, for shared memory region protection
pthread_mutex_t bufferLock; //mutex for the internal buffers
pthread_mutex_t logLock; //mutex for the internal log
uint8_t run_openplc = 1; //Variable to control OpenPLC Runtime execution
unsigned char log_buffer[1000000]; //A very large buffer to store all logs
int log_index = 0;
int log_counter = 0;

int PORT = 43628;


#define ROUND 1000

long timing_init = 0;
long timing_cycle[ROUND] = {0};
long timing_final = 0;
struct timespec tic, toc;

int timing_logic[ROUND] = {0};
int timing_dec[ROUND] = {0};
int timing_enc[ROUND] = {0};

long diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
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
  printf("Writing to log...\n");
  FILE *f;
  f = fopen("cycle_timing.txt", "w+");
  if(f == NULL){
    printf("Failed to open file");
  }
  //fprintf(f, "%ld\n", timing_init);
  for(int i=0; i<ROUND; i++){
    fprintf(f, "%ld\n", timing_cycle[i]);
  }
  //fprintf(f, "%ld\n", timing_final);
  fclose(f);
  printf("Successfully written to log.\n");

  FILE *fd;
  fd = fopen("dec_timing.txt", "w+");
  if(fd == NULL){
    printf("Failed to open dec_timing.txt");
  }
  for(int i=0; i<ROUND; i++){
    fprintf(fd, "%d\n", timing_dec[i]);
  }
  fclose(fd);

  FILE *fe;
  fe = fopen("enc_timing.txt", "w+");
  if(fe == NULL){
    printf("Failed to open enc_timing.txt");
  }
  for(int i=0; i<ROUND; i++){
    fprintf(fe, "%d\n", timing_enc[i]);
  }
  fclose(fe);

  FILE *fl;
  fl = fopen("logic_timing.txt", "w+");
  if(fl == NULL){
    printf("Failed to open logic_timing.txt");
  }
  for(int i=0; i<ROUND; i++){
    fprintf(fl, "%d\n", timing_logic[i]);
  }
  fclose(fl);
}

//-----------------------------------------------------------------------------
// parse the LOCATED_VARIABLES.h and get the starting address of variables
// TODO: currently is hard-coded
//-----------------------------------------------------------------------------
void parsing_located_vars(void** buffer, uint32_t* size){
    extern bool* __IX100_0; // __IX100_0 is defined in glueVars.cpp
    
    printf("__IX100_0 is at %p\n", __IX100_0);
    *buffer = (void*) __IX100_0;
    printf("(void*) buffer content is %p\n", *buffer);
    *size = 32*sizeof(bool) + 32*sizeof(int16_t);
    printf("WARNING: Set the shared memory size manually\n");

    extern int16_t* __QW100;
    printf("__QW100 is %p, should be %p\n", __QW100, __IX100_0+1*32+2*8);

}


//-----------------------------------------------------------------------------
// Helper function - Makes the running thread sleep for the ammount of time
// in milliseconds
//-----------------------------------------------------------------------------
void sleep_until(struct timespec *ts, int delay)
{
    ts->tv_nsec += delay;
    if(ts->tv_nsec >= 1000*1000*1000)
    {
        ts->tv_nsec -= 1000*1000*1000;
        ts->tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts,  NULL);
}

//-----------------------------------------------------------------------------
// Helper function - Makes the running thread sleep for the ammount of time
// in milliseconds
//-----------------------------------------------------------------------------
void sleepms(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

//-----------------------------------------------------------------------------
// Helper function - Logs messages and print them on the console
//-----------------------------------------------------------------------------
void log(unsigned char *logmsg)
{
    pthread_mutex_lock(&logLock); //lock mutex
    printf("%s", logmsg);
    for (int i = 0; logmsg[i] != '\0'; i++)
    {
        log_buffer[log_index] = logmsg[i];
        log_index++;
        log_buffer[log_index] = '\0';
    }
    
    log_counter++;
    if (log_counter >= 1000)
    {
        /*Store current log on a file*/
        log_counter = 0;
        log_index = 0;
    }
    pthread_mutex_unlock(&logLock); //unlock mutex
}

//-----------------------------------------------------------------------------
// Interactive Server Thread. Creates the server to listen to commands on
// localhost
//-----------------------------------------------------------------------------
void *interactiveServerThread(void *arg)
{
    startInteractiveServer(PORT);
}

//-----------------------------------------------------------------------------
// Verify if pin is present in one of the ignored vectors
//-----------------------------------------------------------------------------
bool pinNotPresent(int *ignored_vector, int vector_size, int pinNumber)
{
    for (int i = 0; i < vector_size; i++)
    {
        if (ignored_vector[i] == pinNumber)
            return false;
    }
    
    return true;
}

//-----------------------------------------------------------------------------
// Disable all outputs
//-----------------------------------------------------------------------------
void disableOutputs()
{
    //Disable digital outputs
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (bool_output[i][j] != NULL) *bool_output[i][j] = 0;
        }
    }
    
    //Disable byte outputs
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (byte_output[i] != NULL) *byte_output[i] = 0;
    }
    
    //Disable analog outputs
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (int_output[i] != NULL) *int_output[i] = 0;
    }
}

//-----------------------------------------------------------------------------
// Special Functions
//-----------------------------------------------------------------------------
void handleSpecialFunctions()
{
    //current time [%ML1024]
    struct tm *current_time;
    time_t rawtime;
    
    time(&rawtime);
    // store the UTC clock in [%ML1027]
    if (special_functions[3] != NULL) *special_functions[3] = rawtime;

    current_time = localtime(&rawtime);
    
    rawtime = rawtime - timezone;
    if (current_time->tm_isdst > 0) rawtime = rawtime + 3600;
        
    if (special_functions[0] != NULL) *special_functions[0] = rawtime;
    
    //number of cycles [%ML1025]
    cycle_counter++;
    if (special_functions[1] != NULL) *special_functions[1] = cycle_counter;
    
    //comm error counter [%ML1026]
    /* Implemented in modbus_master.cpp */

    //insert other special functions below
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
    printf("Received Ctrl-C, shutting down openplc ... \n");
    run_openplc = false;
}

int main(int argc,char **argv)
{
    signal(SIGINT, sigintHandler); // register signal handler

    int slave_num = 1;
    if(argc <= 1){
        printf("Usage: openplc <num of slave>\n");
        return 1;
    } else {
        slave_num = atoi(argv[1]);
        printf("Slave num: %d\n", slave_num);
    }

    // if(argc <= 1){
    //     PORT = 43628;
    // } else {
    //     PORT = atoi(argv[1]);
    //     if(PORT < 600 || PORT > 65535){
    //         printf("Invalid port!\n");
    //         PORT = 43628;
    //     }
    // }
    // printf("PORT: %d\n", PORT);
    

    unsigned char log_msg[1000];
    sprintf(log_msg, "OpenPLC Runtime starting on port...\n");
    log(log_msg);

    //======================================================
    //                 PLC INITIALIZATION
    //======================================================
    tzset();
    time(&start_time);
    pthread_t interactive_thread;
    pthread_create(&interactive_thread, NULL, interactiveServerThread, NULL);
    glueVars();

    //======================================================
    //               MUTEX INITIALIZATION
    //======================================================
    if (pthread_mutex_init(&bufferLock, NULL) != 0)
    {
        printf("Mutex init failed\n");
        pthread_join(interactive_thread, NULL);
        exit(1);
    }

    //======================================================
    //              HARDWARE INITIALIZATION
    //======================================================
    initializeHardware();
    // initializeMB(); 
    initCustomLayer();
    updateBuffersIn();
    updateCustomIn();
    updateBuffersOut();
    updateCustomOut();

    //======================================================
    //          PERSISTENT STORAGE INITIALIZATION
    //======================================================
    glueVars();
    mapUnusedIO();
    readPersistentStorage();
    //pthread_t persistentThread;
    //pthread_create(&persistentThread, NULL, persistentStorage, NULL);

#ifdef __linux__
    //======================================================
    //              REAL-TIME INITIALIZATION
    //======================================================
    // Set our thread to real time priority
    struct sched_param sp;
    sp.sched_priority = 30;
    printf("Setting main thread priority to RT\n");
    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
    {
        printf("WARNING: Failed to set main thread to real-time priority\n");
    }

    // Lock memory to ensure no swapping is done.
    printf("Locking main thread memory\n");
    if(mlockall(MCL_FUTURE|MCL_CURRENT))
    {
        printf("WARNING: Failed to lock memory\n");
    }
#endif

    //gets the starting point for the clock
    printf("Getting current time\n");
    struct timespec timer_start;
    clock_gettime(CLOCK_MONOTONIC, &timer_start);


    //======================================================
    //                    TEE INITIALIZATION
    //======================================================
    printf("Num of slave is %d.\n", slave_num);

    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_SCAN_CYCLE_UUID;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS){
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    }

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS){
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        errx(1, "TEEC_OpenSession failed with code 0x%x origin 0x%x",
            res, err_origin);
    }

    memset(&op, 0, sizeof(op));

    //=====================================================
    //             Register Shared Memory
    //=====================================================
    // TODO: If we want to monitor all the variables running in TA
    // we need to register all different kinds of variables
    // located, memory, external, global ...
    // here we register located variable as demonstration
    // another difference with hello_shm is we don't set shm_struct
    // because we only need the value. shm_struct only used by control logic 
    // TODO: here we only set the shared memory as output
    // if we want to force a value, we need other way to do it
    TEEC_SharedMemory shm;
    shm = {0};
    shm.flags = TEEC_MEM_OUTPUT; 
    // notes: we are passing block mem to secure world
    void* shm_buffer;
    uint32_t shm_size;
    parsing_located_vars(&shm_buffer, &shm_size); // TODO: see the declaration of this function
    printf("shm_buffer address is: %p, shm_size is %d \n", shm_buffer, shm_size);
    shm.buffer = shm_buffer; 
    shm.size = shm_size; 

    res = TEEC_RegisterSharedMemory(&ctx, &shm);
    if(res != TEEC_SUCCESS){
        TEEC_ReleaseSharedMemory(&shm);
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        errx(1, "TEEC_RegisterSharedMemory failed with code 0x%x", res);
    }

    op.paramTypes = TEEC_PARAM_TYPES(
                TEEC_VALUE_INOUT,  // for measurement
                TEEC_VALUE_INOUT,  // for measurement
                TEEC_MEMREF_WHOLE, // for shared memory
                TEEC_NONE); // reserved for shared memory

    op.params[0].value.a = slave_num; // for test only
    op.params[0].value.b = 0; // for measurement only
    op.params[1].value.a = 0; // for measurement only
    op.params[1].value.b = 0; // for measurement only
    op.params[2].memref.parent = &shm;


    // initialize the modbus inside TA
    printf("Opening ...\n");
    res = TEEC_InvokeCommand(&sess, TA_SCAN_CYCLE_INIT, &op,
                &err_origin);
    if (res != TEEC_SUCCESS){
        TEEC_CloseSession(&sess);
        TEEC_FinalizeContext(&ctx);
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    }

    //======================================================
    //                    MAIN LOOP
    //======================================================
    printf("Main loop ...\n");
    printf("WARNING: debug mode is on, max iteration is %d\n", ROUND);
    // printf("WARNING: hard-coded interactive server running on 192.168.0.184\n");

    int num_cycle = 0;
    while(run_openplc)
    {
        if(num_cycle >= ROUND) break; // for debugging only

        // invoke the scan cycle
        printf("Scan Cycle %d ... ", num_cycle);
        op.params[0].value.a = num_cycle;
        // ---------------- timing prob core logic tic begin ------------------
        clock_gettime(CLOCK_MONOTONIC, &tic);
        // ----------------- timing prob core logic tic end -------------------
        res = TEEC_InvokeCommand(&sess, TA_SCAN_CYCLE_EXEC, &op,
                    &err_origin);
        if (res != TEEC_SUCCESS){
            res = TEEC_InvokeCommand(&sess, TA_SCAN_CYCLE_EXIT, &op,
                        &err_origin);
            TEEC_CloseSession(&sess);
            TEEC_FinalizeContext(&ctx);
            errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
                res, err_origin);
        }
        // ---------------- timing prob core logic toc begin ------------------
        clock_gettime(CLOCK_MONOTONIC, &toc);
        timing_cycle[num_cycle] = diff(tic, toc);
        // ----------------- timing prob core logic toc end -------------------

        printf("Failed slaves: %x\n", op.params[0].value.b);

        uint8_t *data = (uint8_t*)shm_buffer;

        timing_logic[num_cycle] = data[32+22*2+2+1]<<8 | data[32+22*2+2];
        printf("timing_logic is %u\n", timing_logic[num_cycle]);

        timing_enc[num_cycle] = data[32+23*2+2+1]<<8 | data[32+23*2+2];
        printf("timing_enc is %u\n", timing_enc[num_cycle]);

        timing_dec[num_cycle] = data[32+24*2+2+1]<<8 | data[32+24*2+2];
        printf("timing_dec is %u\n", timing_dec[num_cycle]);

        printf("-----------------------------\n");
        printf("Shared memory value is: \n");
        for(int i=0;i<32; i++){ // TODO: hard-coded size 32
            printf("%x, ", data[i]);
        }
        printf("\n");
        for(int i=32; i<32+32*2; i+=2){ // each register takes 2 bytes, thus 16*2
            printf("%d ",data[i+1]<<8 | data[i]);
        }
        printf("\n-----------------------------\n");
        // we still need the timer here to trigger the core logic execution
        // potential abnormal behaviors:
        // the timer is typically 20ms for a whole cycle
        // if we exceed 20ms, there may be a warning
        // updateTime();
        sleep_until(&timer_start, 2000000ULL);
        num_cycle++;
    }

    //======================================================
    //                shutdown TA session
    //======================================================
    printf("Closing TA Session ...\n");
    res = TEEC_InvokeCommand(&sess, TA_SCAN_CYCLE_EXIT, &op,
                &err_origin);
    if (res != TEEC_SUCCESS){
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
            res, err_origin);
    }
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    printf("Closed TA Session successfully\n");

    
    //======================================================
    //             SHUTTING DOWN OPENPLC RUNTIME
    //======================================================
    write_log();

    printf("Reach an end\n");
    // run_openplc = 0; 
    // since when we are interacting with hmi, we donot need to modify this by hand
    pthread_join(interactive_thread, NULL);
    printf("Disabling outputs\n");
    disableOutputs();
    updateCustomOut();
    updateBuffersOut();
    finalizeHardware();
    printf("Shutting down OpenPLC Runtime...\n");

    exit(0);
}
