rm -rf 0bbd*

rm -f *.o
rm -f *.dmp
rm -f *.cmd
rm -f *.d
rm -f *.lds
rm -f *.elf

make CROSS_COMPILE=aarch64-linux-gnu- CFG_FTRACE_SUPPORT=n CFG_SYSCALL_FTRACE=n CFG_ULIBS_MCOUNT=n CFG_TA_MCOUNT=n PLATFORM=rpi3 TA_DEV_KIT_DIR=../../../optee_os/out/arm/export-ta_arm64 V=1 debug=j

echo "compilation succeeded"

mkdir -p ../../../my_programs_out/tee_openplc_full/
cp *.ta ../../../my_programs_out/tee_openplc_full/

echo "copied to my_programs_out/tee_openplc_full"

