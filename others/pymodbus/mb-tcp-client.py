from pymodbus.client.sync import ModbusTcpClient
from time import sleep

client = ModbusTcpClient('127.0.0.1', port=5020)
# client.write_coil(1, True)
# result = client.read_coils(1,1)
# print(result.bits[0])
# result = client.read_discrete_inputs(1,1)
# print(result.bits[0])

# client.write_coils(address=0,values=[True, True, True, True],unit=1)

for _ in range(10):
    #result = client.read_coils(address=0,count=8,unit=1)
    #print(result.bits)

    result = client.read_input_registers(address=0,count=2,unit=1)
    print(result)

    sleep(2)

client.close()