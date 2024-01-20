# Minimal TEE-PLC

This repository is the source code of Minimal TEE-PLC. This README will guide you to create a runnable demo on RPI3.

The normal world host application is in ./host, while the secure world trusted application is in ./ta.

## Build OP-TEE Project

We built this project on Ubuntu 20.04 with op-tee 3.17 and tested on RPI 3. We strongly encourage you to start with the exact environment as we tested. To set up the op-tee project, follow the steps:

### Prerequistes

``` shell
$ sudo dpkg --add-architecture i386
$ sudo apt-get update
$ sudo apt-get install android-tools-adb android-tools-fastboot autoconf \
        automake bc bison build-essential ccache codespell \
        cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl \
        libattr1-dev libcap-dev libcap-ng-dev \
        libfdt-dev libftdi-dev libglib2.0-dev libgmp-dev libhidapi-dev \
        libmpc-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make \
        mtools netcat ninja-build python-crypto python3-crypto python-pyelftools \
        python3-pycryptodome python3-pyelftools python-serial python3-serial \
        rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev
```

### Download OP-TEE repos

``` shell
$ mkdir -p <path/to/your/optee-project>
$ cd <path/to/your/optee-project>
$ repo init -u https://github.com/OP-TEE/manifest.git -m rpi3.xml -b 3.17.0
$ repo sync -j4 --no-clone-bundle
```

### Get the toolchain

``` shell
$ cd <optee-project>/build
$ make -j2 toolchains
```

### Build the project

``` shell
$ make -j `nproc`
```

You can find more detailed info in the [op-tee official guide here](https://optee.readthedocs.io/en/3.17.0/building/gits/build.html#build).

## Test original OP-TEE on RPI 3

If you successfully build the OP-TEE project, you get the bootable OP-TEE awared linux image. The next step is to flash to RPI3 SD card and power up the SBC.

### Flash bootable image to SD card

Run the following command and follow the console output to flash SD card.

``` shell
cd <path/to/optee-project/>
cd build
make img-help
```

### Power up RPI 3 & run xtest

Connect RPI3 uart0 pins (see the following table) to PC with serial cable.

| UART   | pin   | Signal | GPIO   | Mode  | RPi3 Header pin |
|--------|-------|--------|--------|-------|-----------------|
| Black  | (GND) | GND    | N/A    | N/A   | 6               |
| White  | (RXD) | TXD    | GPIO14 | ALT0  | 8               |
| Green  | (TXD) | RXD    | GPIO15 | ALT0  | 10              |

Then plug in SD card and power up RPI3. Open the serial console, and you should see linux booting log from console output.

``` shell
$ picocom -b 115200 /dev/ttyUSB0
```

Then verify OP-TEE driver by running `xtest` in UART console of RPI3.

``` shell
$ xtest # run in serial console
```

## Run TEE-PLC on RPI 3

### Build TEE-PLC

Create a folder under the optee project and copy all the files under this repository to this folder. Then run `compile.sh`, the executables will be copied to `<path/to/optee-project>/my_programs_out`. 

``` shell
cd <path/to/optee-project>
mkdir -p my_programs
cp -r <path/to/tee-plc>/.. my_programs
cd my_programs
./compile.sh
```

Open the folder `<path/to/optee-project>/my_programs_out`, you should see two files. One is `openplc`, another is `8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta`.

### Copy OpenPLC & TEE-PLC to SD card

Download the source code of [OpenPLC v3] under branch `OpenPLC` and copy to SD card.

Then copy `openplc` under `<path/to/OpenPLC/webserver/core`. This is the executable file of OpenPLC and will be executed by python server.

[TODO: complete the path to ta folder]

Also copy `8aaaf200-2450-11e4-abe2-0002a5d5c51b.ta` to `<path/to/ta/files>`. It will be called by `openplc` and executed in the secure world.

### Run TEE-PLC

Please follow the guide under branch `OpenPLC` to run TEE-PLC tests with Modbus clients.