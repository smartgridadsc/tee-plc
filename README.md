# TEE-PLC

This repository is the implementation of TEE-PLC. 

[TODO: What is TEE-PLC]

[TODO: Minimal & Enhanced TEE-PLC]
TEE-PLC contains two different versions, namely Minimal TEE-PLC and Enhanced TEE-PLC.
You can find the source code of each version under corresponding branches: "minimal" is for Minimal TEE-PLC and "enhanced" is for Enhanced TEE-PLC.

To successfully run the TEE-PLC project, please follow the remaining part of this README step by step before you start to build Minimal TEE-PLC or Enhanced TEE-PLC.


## Build OP-TEE Project

We built OP-TEE 3.17 on Ubuntu 20.04 and tested on RPI 3. We strongly encourage you to start with the exact environment as we tested. 

### Prerequisites
First download this repository

``` shell
git clone https://github.com/smartgridadsc/tee-plc/
```

Then install all dependencies needed by OP-TEE.

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

Now you can download OP-TEE. The following commands will download OP-TEE 3.17.0 with RPI3 support.

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

## Test OP-TEE on RPI 3

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

You should be able to see all test cases for OP-TEE with their results.

## Build OpenPLC for RPI3

### Add to buildroot the libraries needed by OpenPLC
[TODO: may also mention additional buildroot dependencies]

## Build Minimal TEE-PLC or Enhanced TEE-PLC

If you have no issues building OP-TEE and running it on RPI3, you can switch to either "minimal" branch for Minimal TEE-PLC or "enhanced" branch Ehanced TEE-PLC to continue.

``` shell
git switch minimal ## or git switch enhanced
```

## Run OpenPLC Webserver on RPI3
If you have completed the steps in either `minimal` or `enhanced` branch, you can continue from here to start OpenPLC Webserver and run TEE-PLC from webbrowser.

### Add Slave Devices to OpenPLC configuration file 
[TODO]

## Appendix: How to Generate Your Own Control Logic for TEE-PLC
We basically follow this [tutorial](https://autonomylogic.com/docs/3-2-creating-your-first-project-on-openplc-editor/) to design and build control logic for TEE-PLC. After successful compilation, you may get following files under `openplc/webserver/core`. These following files are needed for you to customize your own TEE-PLC.

- POUS.c & POUS.h: the code blocks of control logic
- ResX.c & ResX.h: the resources used by control logic
- Config.c & Config.h: the parameters to configure behavior of control logic
- LOCAL_VARIABLES.h: declaration of variables used by control logic
- glue_vars.c: the glue layer to bind declaration to their definition

You will need to move these files to `/ta` and compile into the secure world TA. For more details, you can refer to README in enhanced TEE-PLC.

## Acknowledgement
This research is supported by the National Research Foundation, Prime Minister's Office, Singapore under its Campus for Research Excellence and Technological Enterprise (CREATE) programme.