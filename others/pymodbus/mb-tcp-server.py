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


# --------------------------------------------------------------------------- #
# import the various server implementations
# --------------------------------------------------------------------------- #
from pymodbus.version import version
from pymodbus.server.sync import StartTcpServer

from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock, ModbusSparseDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

# --------------------------------------------------------------------------- #
# configure the service logging
# --------------------------------------------------------------------------- #
import logging
FORMAT = ('%(asctime)-15s %(threadName)-15s'
          ' %(levelname)-8s %(module)-15s:%(lineno)-8s %(message)s')
logging.basicConfig(format=FORMAT)
log = logging.getLogger()
log.setLevel(logging.ERROR)

from threading import Thread
import numpy as np
from time import sleep

def initialize_data(UID):
    # ----------------------------------------------------------------------- #
    # initialize your data store
    # ----------------------------------------------------------------------- #
    block_di = ModbusSequentialDataBlock(0x00, [0]*0xff)
    block_co = ModbusSequentialDataBlock(0x00, [0]*0xff)
    block_hr = ModbusSequentialDataBlock(0x00, [0]*0xff)
    block_ir = ModbusSequentialDataBlock(0x00, [0]*0xff)
    store = ModbusSlaveContext(di=block_di, co=block_co, hr=block_hr, ir=block_ir)
    #
    # The server then makes use of a server context that allows the server to
    # respond with different slave contexts for different unit ids. By default
    # it will return the same context for every unit id supplied (broadcast
    # mode).
    # However, this can be overloaded by setting the single flag to False and
    # then supplying a dictionary of unit id to context mapping::
    #
    slaves  = {
        UID: store
    }

    return slaves


def run_server(slaves, PORT):

    context = ModbusServerContext(slaves=slaves, single=False)

    # ----------------------------------------------------------------------- #
    # initialize the server information
    # ----------------------------------------------------------------------- #
    # If you don't set this or any fields, they are defaulted to empty strings.
    # ----------------------------------------------------------------------- #
    identity = ModbusDeviceIdentification()
    identity.VendorName = 'Pymodbus'
    identity.ProductCode = 'PM'
    identity.VendorUrl = 'http://github.com/riptideio/pymodbus/'
    identity.ProductName = 'Pymodbus Server'
    identity.ModelName = 'Pymodbus Server'
    identity.MajorMinorRevision = version.short()

    # ----------------------------------------------------------------------- #
    # run the server you want
    # ----------------------------------------------------------------------- #
    # Tcp:
    StartTcpServer(context, identity=identity, address=("0.0.0.0", PORT))



if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        print("Usage: PORT SLAVE_UID")
        exit(0)

    port = sys.argv[1]
    uid = sys.argv[2]
    print("PORT is: ", port," UID is: ", uid)

    slaves = initialize_data(int(uid))

    # thread = Thread(target=updateData)
    # thread.run()

    run_server(slaves, int(port))