#!/usr/bin/env python

# Copyright 2024 Illinois Advanced Research Center at Singapore
#
# Copyright 2008-2023 Pymodbus
# 
# TEE-PLC is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# TEE-PLC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with TEE-PLC.  If not, see <http://www.gnu.org/licenses/>.


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