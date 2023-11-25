CC      ?= $(CROSS_COMPILE)gcc
CXX     ?= $(CROSS_COMPILE)g++
LD      ?= $(CROSS_COMPILE)ld
AR      ?= $(CROSS_COMPILE)ar
NM      ?= $(CROSS_COMPILE)nm
OBJCOPY ?= $(CROSS_COMPILE)objcopy
OBJDUMP ?= $(CROSS_COMPILE)objdump
READELF ?= $(CROSS_COMPILE)readelf

OBJS = Config0.o Res0.o

OPTEE_FLAGS += -Wall -I../ta/include -I$(TEEC_EXPORT)/include -I./include

# removed the std=gnu++11
OPENPLC_FLAGS := -I ./lib -pthread -fpermissive `pkg-config --cflags --libs libmodbus` -w


#Add/link other required libraries here
LDADD += -lteec -L$(TEEC_EXPORT)/lib

BINARY = optee_example_hello_world

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) *.cpp *.o -o $@ $(OPENPLC_FLAGS) $(LDFLAGS) $(LDADD) 

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

# first compile res0.o and Config0.o
Config0.o: Config0.c
	$(CXX) -std=gnu++11 -I ./lib -c Config0.c -w

# Res0.c is modified with tee client so OPTEE_FLAGS is needed.
Res0.o: Res0.c
	$(CXX) -std=gnu++11 $(OPTEE_FLAGS) -I ./lib -c Res0.c -w

