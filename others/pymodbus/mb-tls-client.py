from pymodbus.client.sync import ModbusTlsClient
from pymodbus.framer.socket_framer import ModbusSocketFramer
import ssl
from time import sleep

sslctx = ssl.create_default_context(cafile="certs/ca-cert.pem")
sslctx.check_hostname = False
sslctx.options.OP_NO_TLSv1_2
client = ModbusTlsClient('127.0.0.1', port=8020, sslctx=sslctx,framer=ModbusSocketFramer)
# client.write_coil(1, True)
# result = client.read_coils(1,1)
# print(result.bits[0])
# result = client.read_discrete_inputs(1,1)
# print(result.bits[0])

for _ in range(10):
    # client.write_coils(address=0,values=[True, True, True, True],unit=1)
    result = client.read_coils(address=0,count=8,unit=1)
    print(result)

    sleep(2)
client.close()