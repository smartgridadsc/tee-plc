#!/usr/bin/env python

# Copyright 2024 Illinois Advanced Research Center at Singapore
#
# This file is part of TEE-PLC Project.
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