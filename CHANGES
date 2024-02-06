FIRST RELEASE v1.0 | <2 FEB 2024>

    ******************** Removals: ********************
    1. file host/dnp3.cpp
        deleted file
	
	2. file host/dnp3.cfg 
        deleted file

	3. file host/modbus_master.cpp
	    deleted file

    ******************** Modifications: ********************
    1. file host/interactive_server.cpp
        commented out lines 72-75, 231, 285, 291, 302, 320, 326, 337, 502, 503 to remove dnp3 feature

    ******************** Additions: ********************
    1. file my_modbus.c
modbus_master.cpp: void *querySlaveDevices(void *arg);
uint32_t querySlaveRead(void);
uint32_t querySlaveWrite(void);


modbus_master.cpp: void updateBuffersIn_MB();
void updateBuffersIn_MB();

modbus_master.cpp: void updateBuffersOut_MB();
void updateBuffersOut_MB();

modbus.c: void modbus_close(modbus_t *ctx);
TEE_Result modbus_close(modbus_dev_t* slave);

modbus.c: int modbus_connect(modbus_t *ctx);
TEE_Result modbus_connect(modbus_dev_t *slave);

modbus.cpp: static uint8_t compute_meta_length_after_function(int function, msg_type_t msg_type);
uint8_t compute_meta_len_after_function(int function);

modbus.cpp: static unsigned int compute_response_length_from_request(modbus_t *ctx, uint8_t *req);
uint32_t compute_response_length_from_request(uint8_t *req);

modbus.cpp: static int check_confirmation(modbus_t *ctx, uint8_t *req, uint8_t *rsp, int rsp_length);
int check_confirmation(modbus_dev_t *slave, uint8_t *req, uint8_t *rsp, int rsp_length);

modbus.c: int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
static int modbus_read_input_bits(modbus_dev_t* slave, uint8_t* tempBuff);

modbus.c: static int read_registers(modbus_t *ctx, int function, int addr, int nb, uint16_t *dest);
static int read_registers(modbus_dev_t *slave, int function, int addr, int nb, uint16_t *dest);

modbus.c: int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
static int modbus_read_registers(modbus_dev_t* slave, uint16_t* dest);

modbus.c :int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *src);
int modbus_write_coils(modbus_dev_t* slave, uint8_t* src);

modbus.c: int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *src);
static int modbus_write_registers(modbus_dev_t* slave, uint16_t* src);

modbus.c: int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
static int modbus_read_input_registers(modbus_dev_t* slave, uint16_t* dest);

modbus.c: static int read_registers(modbus_t *ctx, int function, int addr, int nb, uint16_t *dest);
static int read_registers(modbus_dev_t *slave, int function, int addr, int nb, uint16_t *dest);

modbus.c: static int send_msg(modbus_t *ctx, uint8_t *msg, int msg_length);
int32_t modbus_send(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len);

modbus.c: int _modbus_receive_msg(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);
int modbus_receive_msg(modbus_dev_t *slave, uint8_t *msg);

modbus.c: int modbus_receive(modbus_t *ctx, uint8_t *req);
int32_t modbus_recv(modbus_dev_t* slave, uint8_t *msg, uint32_t msg_len);

modbus_tcp.cpp: static int _modbus_tcp_send_msg_pre(uint8_t *req, int req_length);
int modbus_send_msg_pre(uint8_t *req, int req_length);

modbus.c: int modbus_get_header_length(modbus_t *ctx);
void modbus_build_tcpreq_hdr(modbus_dev_t* slave, int function, int addr, int nb, uint8_t *req);



added:

TEE_Result modbus_init(int slave_num);

hook functions for wolfssl sending & receiving:
int my_IORecv(WOLFSSL* ssl, char* buff, int sz, void* ctx);
int my_IOSend(WOLFSSL* ssl, char* buff, int sz, void* ctx);

Initialize slave device registers:
static void set_slave_dis_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs);
static void set_slave_coils(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs);
static void set_slave_input_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs);
static void set_slave_holding_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs);
static void set_slave_holding_read_regs(modbus_dev_t* slave, uint16_t start_addr, uint16_t num_regs);

Update timestamp in shared memory:
void updateTimeStampInSHM(uint16_t core_logic_time);

For debugging:
static void hex_print(const char* hint, uint8_t* msg, uint32_t msg_len);
void modbus_poll(void);

Get high-res timestamp: 
static uint32_t pctcnt2us(uint64_t cnt);

Construct slave devices from config files:
modbus_dev_t* make_slave(char *addr, uint16_t port, uint8_t slave_id, TEE_Result* res);

For scan-cycle ta to control slave devices:
TEE_Result modbus_close_all(void);
TEE_Result modbus_connect_all(void);

Free allocated memory of modbus & wolfssl resources:
void modbus_clear(void);