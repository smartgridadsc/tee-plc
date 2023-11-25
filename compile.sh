echo "==============================================="
echo "cd host to compile host"
echo "==============================================="
cd host
echo "Clear Res0.o and Config.o"
rm -f Res0.o Config.o
echo "Successfully cleared!"

echo "Compiling Res0"
$(pwd)/../../../toolchains/aarch64/bin/aarch64-linux-gnu-g++ -std=gnu++11 -I../../../optee_client/out/export/usr/include -I../../include -I./include -I./lib -I./include -I../ta/include -c Res0.c -L../../libs -lstdc++ -w
if [ $? -ne 0 ]; then
    echo "Error compiling Res0"
    echo "Compilation finished with errors!"
    exit 1
fi
echo "Compiled Res0 successfully!"

echo "Compiling Config0"
$(pwd)/../../../toolchains/aarch64/bin/aarch64-linux-gnu-g++ -std=gnu++11 -I ./lib -I../../../optee_client/out/export/usr/include -c Config0.c -lstdc++ -w
if [ $? -ne 0 ]; then
    echo "Error compiling Config0"
    echo "Compilation finished with errors!"
    exit 1
fi
echo "Compiled Res0 successfully!"


echo "Compiling main program..."
$(pwd)/../../../toolchains/aarch64/bin/aarch64-linux-gnu-g++ -std=gnu++11 *.cpp *.o -o openplc -I../../include -I ../ta/include -I ./lib -I ./include -I ../../../optee_client/out/export/usr/include -pthread -fpermissive -L../../libs -lstdc++ -lmodbus -w -L../../../optee_client/out/export/usr/lib -lteec
if [ $? -ne 0 ]; then
    echo "Error compiling openplc"
    echo "Compilation finished with errors!"
    exit 1
fi
echo "Compilation finished successfully!"
cd ..

echo "==============================================="
echo "cd ta to compile ta"
echo "==============================================="
if [ -d "ta" ]; then
    echo "TA folder is found"
    cd ta
    rm -rf 8aaa*
    rm -f *.o
    make CROSS_COMPILE=/home/ang/Intern/op-tee/toolchains/aarch64/bin/aarch64-linux-gnu- PLATFORM=hikey960 TA_DEV_KIT_DIR=../../../optee_os/out/arm/export-ta_arm64
    
    if [ $? -ne 0 ]; then
        echo "Error compiling ta files"
        echo "Compilation finished with errors!"
        exit 1
    fi
    echo "compilation succeeded"
    cd ..
else
    echo "TA folder NOT FOUND, skip"
fi


cur_dir=${PWD##*/}
mkdir -p $(pwd)/../../my_programs_out/${cur_dir}/
cp host/openplc $(pwd)/../../my_programs_out/${cur_dir}/
if [ -d "ta" ]; then
    cp ta/*.ta $(pwd)/../../my_programs_out/${cur_dir}/
fi
echo "copied to my_programs_out/${cur_dir}/"
