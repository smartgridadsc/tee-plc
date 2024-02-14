//-----------------------------------------------------------------------------
// Copyright 2024 Illinois Advanced Research Center at Singapore Ltd.
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


#include "scan_cycle.h"
#include "my_modbus.h"
#include "arm_user_sysreg.h"
#include "arm64_user_sysreg.h"

#define BUFFER_SIZE        1024

//Booleans
extern uint8_t *bool_input[BUFFER_SIZE][8];
extern uint8_t *bool_output[BUFFER_SIZE][8];

//Analog I/O
extern uint16_t *int_input[BUFFER_SIZE];
extern uint16_t *int_output[BUFFER_SIZE];

int total_slave = 0;
modbus_dev_t* slaves[MAX_SLAVE_NUM];

uint8_t bool_input_buf[MAX_MB_IO];
uint8_t bool_output_buf[MAX_MB_IO];
uint16_t int_input_buf[MAX_MB_IO];
uint16_t int_output_buf[MAX_MB_IO];

// The binding table from handle to isocket_ctx
// The resources are maintained by isocket and wolfssl, we do not need
// to care about the memory allocation and free.
TEE_iSocketHandle tcp_ctx[100];
SSL_t _ssl[100];

const uint16_t timeout = 300;

// TEE time measurement
typedef struct{
	uint64_t tic_enc, toc_enc;
	uint64_t tic_dec, toc_dec;
	uint64_t tic_read, toc_read;
	uint64_t tic_write, toc_write;
    uint64_t time_entry, time_exit;

    uint32_t time_dec, time_enc;
} Measure_t;

Measure_t m;

// timestamp in array
uint8_t ts_idx;
uint16_t ts_arr[32] = {0};

/**
 * @brief translate pctcnt to us
 * from qemu output, the typical value of pct=20871879332, frq=62500000
 * which means the frq is in Hz. Donot need to multiply with 10^6
 * @param cnt : the difference between tic and toc
 * @return unsigned int 
 */
static uint32_t pctcnt2us(uint64_t cnt)
{
	return (cnt * 1000000ULL) / read_cntfrq(); // 19,200,000
}

int my_IOSend(WOLFSSL* ssl, char* buff, int sz, void* ctx);
int my_IORecv(WOLFSSL* ssl, char* buff, int sz, void* ctx);

// factory method
modbus_dev_t* make_slave(char *addr, uint16_t port, uint8_t slave_id, TEE_Result* res){
    modbus_dev_t* slave = (modbus_dev_t*)TEE_Malloc(sizeof(modbus_dev_t),0);
    // DMSG("sizeof(modbus_dev_t)= %lu", sizeof(modbus_dev_t));
    slave->coils.num_regs = 8;
    // DMSG("slave->coil.num_regs is: %d", slave->coils.num_regs);
    if(!slave){
        *res = TEE_ERROR_OUT_OF_MEMORY;
        EMSG("ERROR: Failed to allocate memory for slave.");
        return NULL;
    }
    // tcp_ctx and proto_err is set during connect
    slave->tcp_set.ipVersion = TEE_IP_VERSION_4;
    slave->tcp_set.server_addr = addr;
    slave->tcp_set.server_port = port;
    slave->slave_id = slave_id;
    slave->trans_id = 0;
    *res = TEE_SUCCESS;
    return slave;
}

static void set_slave_dis_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs){
    slave->dis_inputs.num_regs = num_regs;
    slave->dis_inputs.start_address = start_addr;
}

static void set_slave_coils(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs){
    slave->coils.num_regs = num_regs;
    slave->coils.start_address = start_addr;
}

static void set_slave_input_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs){
    slave->input_regs.num_regs = num_regs;
    slave->input_regs.start_address = start_addr;
}

static void set_slave_holding_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs){
    slave->holding_regs.num_regs = num_regs;
    slave->holding_regs.start_address = start_addr;
}

static void set_slave_holding_read_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs){
    slave->holding_read_regs.num_regs = num_regs;
    slave->holding_read_regs.start_address = start_addr;
}

TEE_Result modbus_init(int slave_num){
    wolfSSL_Init();
    modbus_dev_t* slave;
    TEE_Result res;
    uint8_t SLAVE_NUM = slave_num;
    char* slave_addrs = "192.168.1.12";
    uint16_t slave_ports = 802;
    uint8_t slave_id = 1;
    for(int i=0; i<SLAVE_NUM; i++){
        EMSG("modbus_init: slave %d: slave_addr:%s, slave_port: %d", slave_id+i, slave_addrs, slave_ports+i);
        slave = make_slave(slave_addrs, slave_ports+i, slave_id+i, &res);
        if(!slave){
            EMSG("ERROR: Make slave %d failed with NULL. Error Code: %x", slave_id+i, res);
            return res;
        } else if (res != TEE_SUCCESS){
            EMSG("ERROR: Make slave %d failed NON-NULL. Error Code: %x", slave_id+i, res);
            return res;
        }
        set_slave_coils(slave, 0, 8);
        set_slave_dis_regs(slave, 0, 24);
        set_slave_holding_read_regs(slave, 0, 0);
        set_slave_holding_regs(slave, 0, 0);
        set_slave_input_regs(slave, 0, 0);
        slaves[total_slave++] = slave; // add slave into slave list
    }
    return TEE_SUCCESS;
}

TEE_Result modbus_connect(modbus_dev_t *slave){
    TEE_Result res;
    slave->s = slave->slave_id; // TODO: this should be changed to sock_fd
    // DMSG("enter modbus_connect");
    // DMSG("slave %d 's fd is %u, tcp_ctx is %p, addr of tcp_ctx is %p, tcp_addr is %s, tcp_port is %u", slave->slave_id,
    //             slave->s, tcp_ctx[slave->s], &tcp_ctx[slave->s], slave->tcp_set.server_addr, slave->tcp_set.server_port);

    res = TEE_tcpSocket->open(&tcp_ctx[slave->s], &(slave->tcp_set), &(slave->proto_err));

    // DMSG("modbus_connect: slave %d 's tcp_ctx index is %u, handle is %p", 
    //         slave->slave_id, idx, tcp_ctx[slave->s]);

    if (res != TEE_SUCCESS) {
        EMSG("ERROR: Failed to open() TCP for slave %d, error: %#010x "
                "protocolError: %#010x\n", slave->slave_id, res, slave->proto_err);
        if (res == TEE_ERROR_BAD_PARAMETERS)
            EMSG("ERROR: Bad parameters.\n");
        if (res == TEE_ERROR_COMMUNICATION)
            EMSG("ERROR: Unable to connect.\n");
        if (res == TEE_ISOCKET_ERROR_TIMEOUT)
            EMSG("ERROR: Connection timeout\n");
        if (res == TEE_ERROR_COMMUNICATION)
            EMSG("ERROR: Error in communication\n");
        if (res == TEE_ISOCKET_ERROR_PROTOCOL &&
                slave->proto_err == TEE_ISOCKET_ERROR_HOSTNAME)
            EMSG("ERROR: Unknown hostname.\n");
        
        IMSG("Closing TEE_tcpSocket for slave %d ...", slave->slave_id);
        if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
            EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        }
        IMSG("Closed TEE_tcpSocket");
        return res;
    }

    // bind to tcp_ctx
    isocket_ctx* tmp_ctx = (isocket_ctx*)tcp_ctx[slave->s];
    uint32_t idx = tmp_ctx->handle;

    // bind to wolfssl
    if((_ssl[slave->s].wfctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL){
        EMSG("ERROR: wolfSSL new CTX failed");
        return -1;
    }

    //DMSG("********************### Loading verify buffer ###***********************");
    if (wolfSSL_CTX_load_verify_buffer(_ssl[slave->s].wfctx, ca_cert_der_2048, sizeof(ca_cert_der_2048), WOLFSSL_FILETYPE_ASN1) != SSL_SUCCESS) {
        EMSG("ERROR: failed to load cert, have you updated system time?\n");
        wolfSSL_CTX_free(_ssl[slave->s].wfctx);
        if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
            EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        }
        return -1;
    }
    //DMSG("********************### Successfully loaded verify buffer ###***********************");

    wolfSSL_SetIORecv(_ssl[slave->s].wfctx, my_IORecv);
    wolfSSL_SetIOSend(_ssl[slave->s].wfctx, my_IOSend);

    //DMSG("********************### creating wolfssl object ###***********************");
    if ((_ssl[slave->s].wfssl = wolfSSL_new(_ssl[slave->s].wfctx)) == NULL) {
        EMSG("Error: failed to create WOLFSSL object\n");
        wolfSSL_CTX_free(_ssl[slave->s].wfctx);
        if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
            EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        }
        return -1;
    }
    //DMSG("********************### Successfully created wolfssl object ###***********************");
    
    if((res = wolfSSL_set_fd(_ssl[slave->s].wfssl, slave->s) != WOLFSSL_SUCCESS)){
        EMSG("ERROR: failed to connect to set the fd\n");
        wolfSSL_free(_ssl[slave->s].wfssl);
        wolfSSL_CTX_free(_ssl[slave->s].wfctx);
        if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
            EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        }
    }

    if (wolfSSL_connect(_ssl[slave->s].wfssl) != SSL_SUCCESS) {
        EMSG("ERROR: failed to connect to wolfSSL\n");
        wolfSSL_free(_ssl[slave->s].wfssl);
        wolfSSL_CTX_free(_ssl[slave->s].wfctx);
        if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
            EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        }
        return -1;
    }

    return TEE_SUCCESS;
}

TEE_Result modbus_connect_all(void){
    
    TEE_Result res_bit = 0;
    for(int i=0; i<total_slave; i++){
        // DMSG("slave %d: ipv: %d, addr: %s, port: %d", slaves[i]->slave_id, slaves[i]->tcp_set.ipVersion, 
        //         slaves[i]->tcp_set.server_addr, slaves[i]->tcp_set.server_port);
        TEE_Result res;
        res = modbus_connect(slaves[i]);
        if(res == TEE_SUCCESS){ // only when the slave is opened, can set mb_status to connect
            slaves[i]->mb_status = MB_CONNECT;
        } else {
            res_bit |= 1 << i;
        }
    }

    return res_bit;
}

void modbus_build_tcpreq_hdr(modbus_dev_t* slave, int function, int addr, int nb, uint8_t *req){
    /* Increase transaction ID */
    if (slave->trans_id < UINT16_MAX)
        slave->trans_id++;
    else
        slave->trans_id = 0;
    req[0] = slave->trans_id >> 8;
    req[1] = slave->trans_id & 0x00ff;

    /* Protocol Modbus */
    req[2] = 0;
    req[3] = 0;

    /* For modbus tcp, the header length is 6.*/
    int data_len = MB_TCP_PRESET_REQ_LENGTH-6;
    req[4] = data_len >> 8;
    req[5] = data_len & 0x00ff;

    req[6] = slave->slave_id;
    req[7] = function;
    req[8] = addr >> 8;
    req[9] = addr & 0x00ff;
    req[10] = nb >> 8;
    req[11] = nb & 0x00ff;
}

// meta data
uint8_t compute_meta_len_after_function(int function){
    int length;
    switch(function){
    case MB_FC_WRITE_COIL:
    case MB_FC_WRITE_REGISTER:
    case MB_FC_WRITE_MULTIPLE_COILS:
    case MB_FC_WRITE_MULTIPLE_REGISTERS:
        length = 4;
        break;
    default:
        length = 1;
    }

    return length;
}

// length to read
int compute_data_len_after_meta(modbus_dev_t *slave, uint8_t *msg){
    int function = msg[MB_TCP_HEADER_LENGTH];
    int length;

    if(function <= MB_FC_READ_INPUT_REGISTERS){
        length = msg[MB_TCP_HEADER_LENGTH+1];
    } else {
        length = 0;
    }

    length += MB_TCP_CHECKSUM_LENGTH;
    return length;
}

int modbus_receive_msg(modbus_dev_t *slave, uint8_t *msg){
    TEE_Result res;
    int length_to_read;
    int rc;
    int msg_length = 0;
    typedef enum {
        _STEP_FUNCTION,
        _STEP_META,
        _STEP_DATA
    } _step_t;
    _step_t step;

    step = _STEP_FUNCTION;

    length_to_read = MB_TCP_HEADER_LENGTH + 1;

    IMSG("modbus_recv_msg: length_to_read is %d", length_to_read);

    while(length_to_read != 0){
        rc = modbus_recv(slave, msg+msg_length, length_to_read);

        if(rc <= 0){
            EMSG("ERROR: Failed to read response from iSocket");
            return -1;
        }

        msg_length += rc;
        length_to_read -= rc;

        if(length_to_read == 0){
            switch(step){
            case _STEP_FUNCTION:
                length_to_read = compute_meta_len_after_function(msg[MB_TCP_HEADER_LENGTH]);
                if(length_to_read != 0){
                    step = _STEP_META;
                    break;
                }
            case _STEP_META:
                length_to_read = compute_data_len_after_meta(slave, msg);
                if((msg_length + length_to_read) > MB_TCP_MAX_ADU_LENGTH){
                    EMSG("ERROR: To many data");
                    return -1;
                }
                step = _STEP_DATA;
                break;
            default:
                break;
            }
        }
    }

    IMSG("modbus_recv_msg: msg_length is %d", msg_length);
    return msg_length;
}

uint32_t compute_response_length_from_request(uint8_t *req){
    int length;
    const int offset = MB_TCP_HEADER_LENGTH;

    switch (req[offset]) {
    case MB_FC_READ_COILS:
    case MB_FC_READ_INPUTS: {
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    }
        break;
    case MB_FC_READ_HOLDING_REGISTERS:
    case MB_FC_READ_INPUT_REGISTERS:
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    default:
        length = 5;
    }
    return offset + length + MB_TCP_CHECKSUM_LENGTH;
}

int check_confirmation(modbus_dev_t *slave, uint8_t *req, uint8_t *rsp, int rsp_length){
    int rc;
    int rsp_len_computed;
    const int offset = MB_TCP_HEADER_LENGTH;
    const int function = rsp[offset];

    /* check transcation ID */
    if(req[0] != rsp[0] || req[1] != rsp[1]){
        EMSG("ERROR: Invalid trans ID received 0x%X (not 0x%X)", (rsp[0]<<8)+rsp[1], (req[0]<<8)+req[1]);
        return -1;
    }

    /* check protocol ID */
    if(rsp[2] != 0x0 && rsp[3] != 0x0){
        EMSG("ERROR: Invalid protocol ID received 0x%X (not 0x0)\n", (rsp[2]<<8)+rsp[3]);
        return -1;
    }
    rsp_len_computed = compute_response_length_from_request(req);
    // DMSG("check_confirmation: rsp_len_computed is %d. Read inputs with 8 bits should be 10.", rsp_len_computed);

    /* Check length */
    if(rsp_length == rsp_len_computed && function < 0x80) {
        int req_nb_value;
        int rsp_nb_value;

        /* check function code */
        if(function != req[offset]){
            EMSG("ERROR: Received function not corresponding to the request (0x%X != 0x%X)\n", function, req[offset]);
            return -1;
        }

        /* Check the number of values is corresponding to the request */
        switch (function) {
        case MB_FC_READ_COILS:
        case MB_FC_READ_INPUTS:
            /* Read functions, 8 values in a byte (nb
             * of values in the request and byte count in
             * the response. */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);
            rsp_nb_value = rsp[offset + 1];
            break;
        case MB_FC_READ_HOLDING_REGISTERS:
        case MB_FC_READ_INPUT_REGISTERS:
            /* Read functions 1 value = 2 bytes */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 1] / 2);
            break;
        case MB_FC_WRITE_MULTIPLE_COILS:
        case MB_FC_WRITE_MULTIPLE_REGISTERS:
            /* N Write functions */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
            break;
        default:
            /* 1 Write functions & others */
            req_nb_value = rsp_nb_value = 1;
        }

        if(req_nb_value == rsp_nb_value){
            rc = rsp_nb_value;
        } else {
            rc = -1;
        }
    } else {
        rc = -1;
    }

    return rc;
}

int modbus_send_msg_pre(uint8_t *req, int req_length){
    /* Subtract the header length to the message length */
    int mbap_length = req_length - 6;

    req[4] = mbap_length >> 8;
    req[5] = mbap_length & 0x00FF;

    return req_length;
}

/**
 * @brief
 * The actual send function. The interface for modbus functions stays the same
 * However, the implementation should be different: tee_tcpSocket or wolfssl
 * @param slave: slave modbus ctx
 * @param msg : the msg to be sent
 * @param msg_len : the length of the msg
 * @return TEE_Result 
 */
int32_t modbus_send(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len){
    int res;
    if(slave->mb_status != MB_CONNECT){
        EMSG("ERROR: Non-connected ctx cannot send");
        return -1;
    }

    m.tic_enc = read_cntpct();
    res = wolfSSL_write(_ssl[slave->s].wfssl, msg, msg_len);
    //DMSG("modbus_send returned with res: %d", res);
    return res;
}

/**
 * @brief 
 * The actual send function. The interface for modbus functions stays the same
 * However, the implementation should be different: tee_tcpSocket or wolfssl
 * @param slave 
 * @param msg 
 * @return TEE_Result 
 */
int32_t modbus_recv(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len){
    uint64_t tic_mbrecv = read_cntpct();

    IMSG("modbus_recv has been called at %u!", pctcnt2us(tic_mbrecv-m.time_entry));

    int res;
    if(slave->mb_status != MB_CONNECT){
        EMSG("ERROR: Non-connected ctx cannot send");
        return -1;
    }
    // DMSG("modbus_recv: before recv: slave %d 's fd is %d, ctx_fd is %d, proto_error is %x",
    //                 slave->slave_id, slave->s, ((isocket_ctx*)tcp_ctx[slave->s])->handle, ((isocket_ctx*)tcp_ctx[slave->s])->proto_error);
    
    res = wolfSSL_read(_ssl[slave->s].wfssl, msg, msg_len);
    m.toc_dec = read_cntpct();

    IMSG("wolfssl decryption time is: %lu us, ts: %lu", 
        pctcnt2us(m.toc_dec-m.tic_dec), pctcnt2us(m.toc_dec-m.time_entry));

    m.time_dec += pctcnt2us(m.toc_dec-m.tic_dec);

    ts_arr[ts_idx++] = pctcnt2us(m.tic_dec - m.time_entry);
    IMSG("ts_arr[%d] is ts for tic_dec", ts_idx);
    ts_arr[ts_idx++] = pctcnt2us(m.toc_dec - m.time_entry);
    IMSG("ts_arr[%d] is ts for toc_dec", ts_idx);
    //DMSG("wolfssl_read returns %d", res);
    if(res > 0){
        // IMSG("Received message from server: %s", msg);
    }
    
    return res;
}

static int modbus_read_input_bits(modbus_dev_t* slave, uint8_t* tempBuff){
    // ******************************* Query ******************************
    // build the request header
    int nb = slave->dis_inputs.num_regs;
    int addr = slave->dis_inputs.start_address;

    if(tcp_ctx[slave->s] == NULL){
        EMSG("ERROR: ctx of slave %d is null.", slave->slave_id);
        return -1;
    }
    
    uint8_t req[MB_TCP_PRESET_REQ_LENGTH]; // the typical length of Modbus TCP read inputs request
    uint8_t rsp[MAX_MESSAGE_LENGTH]; // MAX_LEN of Modbus TCP 
    uint32_t req_len = MB_TCP_PRESET_REQ_LENGTH;
    int rc = req_len;
    modbus_build_tcpreq_hdr(slave, MB_FC_READ_INPUTS, addr, nb, req);

    // send the request
    rc = modbus_send(slave, req, req_len);

    if(rc > 0){
        int i, temp, bit;
        int pos =0;
        int offset;
        int offset_end;

        rc = modbus_receive_msg(slave, rsp);
        if(rc == -1) return -1;
        
        rc = check_confirmation(slave, req, rsp, rc);

        //DMSG("check_confirmation returns: %d. For read inputs of 8 bits should return 1.", rc);

        if(rc == -1) return -1;

        offset = MB_TCP_HEADER_LENGTH + 2;
        offset_end = offset + rc;

        for(i = offset; i< offset_end; i++){
            temp = rsp[i];
            // DMSG("modbus read input bits: rsp[i] is: %d. should be 148/255.", rsp[i]);
            for (bit = 0x01; (bit & 0xff) && (pos < nb);) {
                tempBuff[pos++] = (temp & bit) ? 1 : 0;
                bit = bit << 1;
            }
        }
    }

    if(rc == -1)
        return rc;
    else
        return nb;
}

int modbus_write_coils(modbus_dev_t* slave, uint8_t* src){
    // ******************************* Query ******************************
    int nb = slave->coils.num_regs;
    int addr = slave->coils.start_address;

    int rc;
    int i;
    int byte_count;
    int req_length = MB_TCP_PRESET_REQ_LENGTH;
    int bit_check = 0;
    int pos = 0;
    uint8_t req[MAX_MESSAGE_LENGTH];

    if(tcp_ctx[slave->s] == NULL){
        EMSG("ERROR: ctx of slave %d is null.", slave->slave_id);
        return -1;
    }

    modbus_build_tcpreq_hdr(slave, MB_FC_WRITE_MULTIPLE_COILS,addr, nb, req);
    byte_count = (nb / 8) + ((nb % 8) ? 1 : 0);
    req[req_length++] = byte_count;

    for (i = 0; i < byte_count; i++) {
        int bit;

        bit = 0x01;
        req[req_length] = 0;

        while ((bit & 0xFF) && (bit_check++ < nb)) {
            if (src[pos++])
                req[req_length] |= bit;
            else
                req[req_length] &=~ bit;

            bit = bit << 1;
        }
        req_length++;
    }

    modbus_send_msg_pre(req, req_length);
    rc = modbus_send(slave, req, req_length);
    if (rc > 0) {
        uint8_t rsp[MAX_MESSAGE_LENGTH];

        rc = modbus_receive_msg(slave, rsp);
        if (rc == -1)
            return -1;

        rc = check_confirmation(slave, req, rsp, rc);
    }

    return rc;
}

static int modbus_write_registers(modbus_dev_t* slave, uint16_t* src){
    int rc;
    int i;
    int req_length = MB_TCP_PRESET_REQ_LENGTH;
    int byte_count;
    uint8_t req[MAX_MESSAGE_LENGTH];

    int nb = slave->holding_regs.num_regs;
    int addr = slave->holding_regs.start_address;

    if(tcp_ctx[slave->s] == NULL){
        EMSG("ERROR: ctx of slave %d is null.", slave->slave_id);
        return -1;
    }

    if(nb > MB_MAX_WRITE_REGISTERS){
        EMSG("Too many registers requested (%d > %d)", nb, MB_MAX_READ_REGISTERS);
        return -1;
    }

    modbus_build_tcpreq_hdr(slave,MB_FC_WRITE_MULTIPLE_REGISTERS,addr, nb, req);
    byte_count = nb * 2;
    req[req_length++] = byte_count;

    for (i = 0; i < nb; i++) {
        req[req_length++] = src[i] >> 8;
        req[req_length++] = src[i] & 0x00FF;
    }

    modbus_send_msg_pre(req, req_length);
    rc = modbus_send(slave, req, req_length);

    if (rc > 0) {
        uint8_t rsp[MAX_MESSAGE_LENGTH];

        rc = modbus_receive_msg(slave, rsp);
        if (rc == -1)
            return -1;

        rc = check_confirmation(slave, req, rsp, rc);
    }

    return rc;
}

/* Reads the data from a remote device and put that data into an array */
static int read_registers(modbus_dev_t *slave, int function, int addr, int nb, uint16_t *dest)
{
    
    int rc;
    int req_length = MB_TCP_PRESET_REQ_LENGTH;
    uint8_t req[MB_TCP_PRESET_REQ_LENGTH];
    uint8_t rsp[MAX_MESSAGE_LENGTH];

    if(nb > MB_MAX_READ_REGISTERS){
        EMSG("Too many registers requested (%d > %d)", nb, MB_MAX_READ_REGISTERS);
        return -1;
    }

    modbus_build_tcpreq_hdr(slave, function, addr, nb, req);

    modbus_send_msg_pre(req, req_length);
    rc = modbus_send(slave, req, req_length);

    if (rc > 0) {
        int offset;
        int i;

        rc = modbus_receive_msg(slave, rsp);
        if (rc == -1)
            return -1;

        rc = check_confirmation(slave, req, rsp, rc);
        if (rc == -1)
            return -1;

        offset = MB_TCP_HEADER_LENGTH;

        for (i = 0; i < rc; i++) {
            /* shift reg hi_byte to temp OR with lo_byte */
            dest[i] = (rsp[offset + 2 + (i << 1)] << 8) |
                rsp[offset + 3 + (i << 1)];
        }
    }

    return rc;
}

static int modbus_read_registers(modbus_dev_t* slave, uint16_t* dest){
    int status;
    int nb = slave->holding_read_regs.num_regs;
    int addr = slave->holding_read_regs.start_address;

    if(tcp_ctx[slave->s] == NULL){
        EMSG("ERROR: ctx of slave %d is null.", slave->slave_id);
        return -1;
    }

    status = read_registers(slave, MB_FC_READ_HOLDING_REGISTERS, addr, nb, dest);
    return status;
}

static int modbus_read_input_registers(modbus_dev_t* slave, uint16_t* dest){
    int status;
    int nb = slave->input_regs.num_regs;
    int addr = slave->input_regs.start_address;

    if(tcp_ctx[slave->s] == NULL){
        EMSG("ERROR: ctx of slave %d is null.", slave->slave_id);
        return -1;
    }

    status = read_registers(slave, MB_FC_READ_INPUT_REGISTERS, addr, nb, dest);

    return status;
}

uint32_t querySlaveRead(void){
    ts_idx = 0;
    m.time_entry = read_cntpct();
    m.time_enc = 0;
    m.time_dec = 0;
    ts_arr[ts_idx++] = 0;
    IMSG("ts_arr[%d] is ts for time_entry", ts_idx);

    uint16_t bool_input_index = 0;
    uint16_t int_input_index = 0;

    uint32_t res_bit = 0; // lower 16 bit, one for each slave: 1 for error, 0 for normal

    for(int i=0; i<total_slave; i++){

        if(slaves[i]->mb_status != MB_CONNECT){
            IMSG("WARNING: Slave[%d] is disconnected. Attempting to reconnect...", i);
            // TODO: try to reconnect
            TEE_Result res;
            res = modbus_connect(slaves[i]);
            if(res != TEE_SUCCESS){
                res_bit |= 1 << i;
                IMSG("WARNING: Connection failed on slave[%d]", i);
                // skip the num_regs of the slave
                bool_input_index += slaves[i]->dis_inputs.num_regs;
            } else {
                res_bit &= ~(1 << i);
                IMSG("WARNING: Connected to MB slave[%d]", i);
                slaves[i]->mb_status = MB_CONNECT;
            }
        }

        if(slaves[i]->mb_status == MB_CONNECT){

            // Read discrete input
            if(slaves[i]->dis_inputs.num_regs != 0){
                uint8_t* tempBuff;
                tempBuff = (uint8_t *)TEE_Malloc(slaves[i]->dis_inputs.num_regs, 0);
                int return_val = modbus_read_input_bits(slaves[i], tempBuff);
                if(return_val == -1){
                    res_bit |= 1<<i; // if error, mark the corresponding bit
                    modbus_close(slaves[i]);
                    EMSG("ERROR: Modbus Read Discrete Input failed on slave %d", slaves[i]->slave_id);
                    bool_input_index += slaves[i]->dis_inputs.num_regs;
                } else {
                    // DMSG("Query slave read: return_val is: %d", return_val);
                    for(int j=0; j<return_val; j++){
                        bool_input_buf[bool_input_index] = tempBuff[j];
                        bool_input_index++;
                    }
                }
                TEE_Free(tempBuff);
            }

            // input registers
            if(slaves[i]->input_regs.num_regs != 0){
                uint16_t* tempBuff;
                tempBuff = (uint16_t *)TEE_Malloc(2*(slaves[i]->input_regs.num_regs), 0);
                int return_val = modbus_read_input_registers(slaves[i], tempBuff);
                if(return_val == -1){
                    res_bit |= 1<<i; // if error, mark the corresponding bit
                    modbus_close(slaves[i]);
                    EMSG("ERROR: Modbus Read Input Registers failed on slave %d", slaves[i]->slave_id);
                    int_input_index += slaves[i]->input_regs.num_regs;
                } else {
                    for(int j=0; j<return_val; j++){
                        int_input_buf[int_input_index] = tempBuff[j];
                        int_input_index++;
                    }
                }
                TEE_Free(tempBuff);
            }

            // holding read registers
            if(slaves[i]->holding_read_regs.num_regs != 0){
                uint16_t* tempBuff;
                tempBuff = (uint16_t *)TEE_Malloc(2*(slaves[i]->holding_read_regs.num_regs), 0);
                int return_val = modbus_read_registers(slaves[i], tempBuff);
                if(return_val == -1){
                    res_bit |= 1<<i; // if error, mark the corresponding bit
                    modbus_close(slaves[i]);
                    EMSG("ERROR: Modbus Read Registers failed on slave %d", slaves[i]->slave_id);
                    int_input_index += slaves[i]->holding_read_regs.num_regs;
                } else {
                    for(int j=0; j<return_val; j++){
                        int_input_buf[int_input_index] = tempBuff[j];
                        int_input_index++;
                    }
                }
                TEE_Free(tempBuff);
            }
        }
    }

    return res_bit;
}

uint32_t querySlaveWrite(void){
    uint16_t int_output_index = 0;
    uint16_t bool_output_index = 0;

    uint32_t res_bit = 0; // high 16-bit, one for each slave

    for(int i=0; i<total_slave; i++){

        if(slaves[i]->mb_status != MB_CONNECT){
            IMSG("WARNING: Slave[%d] is disconnected. Attempting to reconnect...", i);
            // TODO: try to reconnect
            TEE_Result res;
            res = modbus_connect(slaves[i]);
            if(res != TEE_SUCCESS){
                res |= 1 << (16+i);
                IMSG("WARNING: Connection failed on slave[%d]", i);
                // skip the num_regs of the slave
                // bool_input_index += slaves[i]->dis_inputs.num_regs;
                bool_output_index += slaves[i]->coils.num_regs;
            } else {
                res &= ~(1 << (16+i)); 
                IMSG("WARNING: Connected to MB slave[%d]", i);
                slaves[i]->mb_status = MB_CONNECT;
            }
        }

        if(slaves[i]->mb_status == MB_CONNECT){
            // Write coils
            if(slaves[i]->coils.num_regs != 0){
                uint8_t *tempBuff;
                tempBuff = (uint8_t*)TEE_Malloc(slaves[i]->coils.num_regs, 0);

                for(int j=0; j<slaves[i]->coils.num_regs; j++){
                    tempBuff[j] = bool_output_buf[bool_output_index];
                    bool_output_index++;
                }

                int return_val = modbus_write_coils(slaves[i], tempBuff);
                if(return_val == -1){
                    res_bit |= 1 << (16+i);
                    modbus_close(slaves[i]);
                    EMSG("ERROR: Modbus write coil failed on slave %d", slaves[i]->slave_id);
                }

                TEE_Free(tempBuff);
            }

            // write holding registers
            if(slaves[i]->holding_regs.num_regs != 0){
                uint16_t *tempBuff;
                tempBuff = (uint16_t*)TEE_Malloc(2*(slaves[i]->holding_regs.num_regs), 0);

                for(int j=0;j<slaves[i]->holding_regs.num_regs; j++){
                    tempBuff[j] = int_output_buf[int_output_index];
                    int_output_index++;
                }

                int return_val = modbus_write_registers(slaves[i], tempBuff);
                if(return_val == -1){
                    res_bit |= 1 << (16+i);
                    modbus_close(slaves[i]);
                    EMSG("ERROR: Modbus write reg failed on slave %d", slaves[i]->slave_id);
                }

                TEE_Free(tempBuff);
            }
        }
    }
    return res_bit;
}

TEE_Result modbus_close(modbus_dev_t* slave){
    TEE_Result res;

    // first shutdown ssl session
    IMSG("WARNING: Closing slave %d ....", slave->slave_id);
    if((res = wolfSSL_shutdown(_ssl[slave->s].wfssl)) != SSL_SUCCESS){
        IMSG("Waiting for peer to close ssl session on slave %d", slave->slave_id);
    }

    // then free ssl ctx
    if(_ssl[slave->s].wfssl)
        wolfSSL_free(_ssl[slave->s].wfssl);
    if(_ssl[slave->s].wfctx)
        wolfSSL_CTX_free(_ssl[slave->s].wfctx);

    // finally shutdown socket
    if((res = TEE_tcpSocket->close(tcp_ctx[slave->s])) != TEE_SUCCESS){
        EMSG("ERROR: Failed to close tee_tcpsocket on slave %d.", slave->slave_id);
        return res;
    }
    slave->mb_status = MB_CLOSED;
    return TEE_SUCCESS;
}

TEE_Result modbus_close_all(void){
    TEE_Result res = TEE_SUCCESS;

    // TODO: retry closing, may need to change
    for(int i=0; i<total_slave; i++){
        if(slaves[i]->mb_status != MB_CLOSED){
            res |= modbus_close(slaves[i]);
        }
    }

    return res;
}

void modbus_clear(void){
    for(int i=0; i<total_slave; i++){
        TEE_Free(slaves[i]);
        slaves[i]->mb_status = MB_CLEARED;
    }
    wolfSSL_Cleanup();
}


void updateBuffersIn_MB()
{
    for (int i = 0; i < MAX_MB_IO; i++)
    {
        if (bool_input[100+(i/8)][i%8] != NULL){
            *bool_input[100+(i/8)][i%8] = bool_input_buf[i];
        }

        if (int_input[100+i] != NULL) *int_input[100+i] = int_input_buf[i];
    }
}

void updateBuffersOut_MB()
{
    for (int i = 0; i < MAX_MB_IO; i++)
    {
        if (bool_output[100+(i/8)][i%8] != NULL){
            bool_output_buf[i] = *bool_output[100+(i/8)][i%8];
        }

        if (int_output[100+i] != NULL) int_output_buf[i] = *int_output[100+i];
    }
}


void updateTimeStampInSHM(uint16_t core_logic_time)
{
    m.time_exit = read_cntpct();
    ts_arr[ts_idx++] = pctcnt2us(m.time_exit - m.time_entry);
    IMSG("ts_arr[%d] is ts for time_exit", ts_idx);

    IMSG("value of ts_idx is %d, should be no larger than 32.", ts_idx);

    // here reuse the int_buffer to pass the measurement to normal world
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

// ctx here is actually the slave->s
int my_IORecv(WOLFSSL* ssl, char* buff, int sz, void* ctx) {
    uint64_t tic_IORecv = read_cntpct();
    IMSG("my_IORecv has been called at %u!", pctcnt2us(tic_IORecv-m.time_entry));

    TEE_Result res;
    uint32_t length = (uint32_t)sz;

    uint32_t s = *(uint32_t*)ctx;

    //DMSG("My_IORecv: sock_fd recovered is %u.", s);
    //DMSG("My_IORecv: handle recovered is %p.", tcp_ctx[s]);

    m.tic_read = read_cntpct();
    res = TEE_tcpSocket->recv(tcp_ctx[s], buff, &length, timeout);
    m.toc_read = read_cntpct();


    IMSG("isocket recv time is: %lu us, ts: %lu", 
        pctcnt2us(m.toc_read-m.tic_read), pctcnt2us(m.toc_read-m.time_entry));

    ts_arr[ts_idx++] = pctcnt2us(m.tic_read - m.time_entry);
    IMSG("ts_arr[%d] is ts for tic_read", ts_idx);
    ts_arr[ts_idx++] = pctcnt2us(m.toc_read - m.time_entry);
    IMSG("ts_arr[%d] is ts for toc_read", ts_idx);

    m.tic_dec = read_cntpct();

    if (res != TEE_SUCCESS) {
        EMSG("ERROR: My_IORecv error while receiving.");
        if(res == TEE_ISOCKET_ERROR_TIMEOUT){
            EMSG("ERROR_TIMEOUT");
        } else if(res == TEE_ERROR_COMMUNICATION){
            EMSG("ERROR_COMMUNICATION");
        } else {
            EMSG("ERROR code is 0x%x", res);
        }
        return -1;
    } else {
        return length;
    }
}

int my_IOSend(WOLFSSL* ssl, char* buff, int sz, void* ctx) {

    TEE_Result res;
    uint32_t length = (uint32_t)sz;
    uint32_t s = *(uint32_t*)ctx;
    
    if (buff == NULL) {
        EMSG("ERROR: buff == NULL, will panic\n");
    }

    m.toc_enc = read_cntpct();

    IMSG("encryption time is: %lu us, ts: %lu", 
        pctcnt2us(m.toc_enc-m.tic_enc), pctcnt2us(m.toc_enc-m.time_entry));

    m.time_enc += pctcnt2us(m.toc_enc-m.tic_enc);

    ts_arr[ts_idx++] = pctcnt2us(m.tic_enc-m.time_entry);
    IMSG("ts_arr[%d] is ts for tic_enc", ts_idx);
    ts_arr[ts_idx++] = pctcnt2us(m.toc_enc-m.time_entry);
    IMSG("ts_arr[%d] is ts for toc_enc", ts_idx);


    m.tic_write = read_cntpct();
    res = TEE_tcpSocket->send(tcp_ctx[s], buff, &length, timeout);

    m.toc_write = read_cntpct();

    IMSG("isocket send time is: %lu us, ts: %lu", 
        pctcnt2us(m.toc_write-m.tic_write), pctcnt2us(m.toc_write-m.time_entry));

    ts_arr[ts_idx++] = pctcnt2us(m.tic_write-m.time_entry);
    IMSG("ts_arr[%d] is ts for tic_write", ts_idx);
    ts_arr[ts_idx++] = pctcnt2us(m.toc_write-m.time_entry);
    IMSG("ts_arr[%d] is ts for toc_write", ts_idx);

    if (res != TEE_SUCCESS) {
        EMSG("ERROR: error while sending...\n");
        EMSG("ERROR: My_IORecv error while receiving.");
        if(res == TEE_ISOCKET_ERROR_TIMEOUT){
            EMSG("ERROR_TIMEOUT");
        } else if(res == TEE_ERROR_COMMUNICATION){
            EMSG("ERROR_COMMUNICATION");
        } else {
            EMSG("ERROR code is 0x%x", res);
        }
        return -1;
    } else {
        // DMSG("sent %d bytes\n", length);
        return length;
    }
}
