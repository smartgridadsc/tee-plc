global-incdirs-y += include
global-incdirs-y += ../host
global-incdirs-y += ../host/lib


srcs-y += core_logic_only_ta.c

# To remove a certain compiler flag, add a line like this
#cflags-template_ta.c-y += -Wno-strict-prototypes
