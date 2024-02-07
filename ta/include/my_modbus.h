/*
 * Modified for TEE-PLC
 * Illinois Advanced Research Center at Singapore Ltd. 2024
 *
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 */

#ifndef _MY_MODBUS_H_
#define _MY_MODBUS_H_

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "tee_tcpsocket.h"
#include "string.h"
#include "string_ext.h"
#include "ssl_backend.h"

typedef enum {
    MB_CLOSED,
    MB_CONNECT,
    MB_CLEARED
} mb_status_t;

typedef struct
{
    uint16_t start_address;
    uint16_t num_regs;
} mb_addr;

typedef struct {
        uint32_t handle;
        uint32_t proto_error;
} isocket_ctx;

typedef struct {
    uint32_t s;
    // TEE_iSocketHandle tcp_ctx;
    TEE_tcpSocket_Setup tcp_set;
    uint32_t proto_err;
    uint8_t slave_id;
    mb_status_t mb_status;
    uint16_t trans_id;

    mb_addr dis_inputs;
    mb_addr coils;
    mb_addr input_regs;
    mb_addr holding_regs;
    mb_addr holding_read_regs;
} modbus_dev_t;

#define MAX_SLAVE_NUM 20

#define MAX_MB_IO              400


#define MB_FC_NONE                      0
#define MB_FC_READ_COILS                1
#define MB_FC_READ_INPUTS               2
#define MB_FC_READ_HOLDING_REGISTERS    3
#define MB_FC_READ_INPUT_REGISTERS      4
#define MB_FC_WRITE_COIL                5
#define MB_FC_WRITE_REGISTER            6
#define MB_FC_WRITE_MULTIPLE_COILS      15
#define MB_FC_WRITE_MULTIPLE_REGISTERS  16
#define MB_FC_ERROR                     255


#define MB_TCP_HEADER_LENGTH            7
#define MB_TCP_PRESET_REQ_LENGTH        12
#define MB_TCP_PRESET_RSP_LENGTH        8
#define MB_TCP_CHECKSUM_LENGTH          0
#define MB_TCP_MAX_ADU_LENGTH           260
#define MAX_MESSAGE_LENGTH              260
#define MB_MAX_READ_REGISTERS           125
#define MB_MAX_WRITE_REGISTERS          123



#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define lowByte(w) ((unsigned char) ((w) & 0xff))
#define highByte(w) ((unsigned char) ((w) >> 8))

// modbus private: TODO: may declare as static and not appear in header file
TEE_Result modbus_connect(modbus_dev_t* slave);
TEE_Result modbus_close(modbus_dev_t* slave);
modbus_dev_t* make_slave(char *addr, uint16_t port, uint8_t slave_id, TEE_Result* res);
int32_t modbus_send(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len);
int32_t modbus_recv(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len);

// modbus test interface
void modbus_poll(void);
static void hex_print(const char* hint, uint8_t* msg, uint32_t msg_len);

// modbus interface
TEE_Result modbus_init(int slave_num);
TEE_Result modbus_connect_all(void);
TEE_Result modbus_close_all(void);
void modbus_clear(void);
uint32_t querySlaveRead(void);
uint32_t querySlaveWrite(void);

// update interface
void updateBuffersIn_MB(void);
void updateBuffersOut_MB(void);

// send timestamp to shared memory
void updateTimeStampInSHM(uint16_t);

#endif /* _MY_MODBUS_H_ */