global-incdirs-y += include
global-incdirs-y += ${path_to_wolfssl_include}
global-incdirs-y += ../host/lib
global-incdirs-y += .
srcs-y += my_modbus.c Config0.c Res0.c glueVars.c scan_cycle.c

libdirs += .
libnames += wolfssl
libdeps += libwolfssl.a

# To remove a certain compiler flag, add a line like this
cflags-template_ta.c-y += -Wnounused-parameter
