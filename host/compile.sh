rm -f *.o

aarch64-linux-gnu-g++ -std=gnu++11 *.cpp -o openplc -I./lib -I../ta/include -I../../../optee_client/out/export/usr/include -pthread -fpermissive -I./modbus/include -L./modbus/lib -lmodbus -w -L../../../optee_client/out/export/usr/lib -lteec

mkdir -p ../../../my_programs_out/tee_openplc_full
cp openplc ../../../my_programs_out/tee_openplc_full/
echo "copied to my_programs_out/tee_openplc_full/"
