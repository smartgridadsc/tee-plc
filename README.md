# Minimal TEE-PLC

This repository is the source code of Minimal TEE-PLC. The normal world OpenPLC Runtime is in ./host, while the secure world Logic-only TA is in ./ta.

**Note:** Before you start, please make sure you have finished all the steps mentioned in main branch README and successfully run OpenPLC and OP-TEE on RPI3.

## Run Minimal TEE-PLC on RPI 3

### Build Minimal TEE-PLC

Create a folder under the optee project and copy all the files under this repository to this folder. Then run `compile.sh`, the executables will be copied to `<path/to/optee-project>/my_programs_out`. 

``` shell
cd <path/to/optee-project>
mkdir -p my_programs
cp -r <path/to/tee-plc>/.. my_programs
cd my_programs
./compile.sh
```

Open the folder `<path/to/optee-project>/my_programs_out`, you should see two files. One is `openplc`, another is `8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta`.

### Copy OpenPLC & Minimal TEE-PLC to SD card

Download the source code of [OpenPLC v3] under branch `OpenPLC` and copy to SD card.

Then copy `openplc` under `<path/to/OpenPLC/webserver/core`. This is the executable file of OpenPLC and will be executed by python server.

Also copy `8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta` to `/lib/optee_armtz`. It will be called by `openplc` and executed in the secure world.

### Run Minimal TEE-PLC on RPI3

Now you can continue following the `main`` branch README to run Minimal TEE-PLC tests with Modbus clients.