# Minimal tee-plc

This repository is the source code of minimal tee-plc.

The normal world host application is in ./host, while the secure world trusted application is in ./ta.

## Build & Run

First follow the official op-tee [guide](https://optee.readthedocs.io/en/latest/building/devices/qemu.html#qemu-v8) to build and run op-tee. 

Then create a folder in optee project and copy the files under this repository to this folder. Then run `compile.sh`, the executables will be copied to `<path-to-optee-root-dir>/my_programs_out`. 

``` shell
cd <path-to-optee-root-dir>
mkdir -p my_programs
cp -r <path-to-this-folder>/.. my_programs
cd my_programs
./compile.sh
```

