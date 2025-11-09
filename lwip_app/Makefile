#The same on development VM and target RasPi FS
THIS_MK_DIR := $(shell pwd)

LWIPDIR=/usr/lib/lwip/src
include $(LWIPDIR)/Filelists.mk

#Need to link lwIP files here
#Have a check for Ubuntu dev vs RasPi, change path to lwIP depending on platform

CC ?= $(CROSS_COMPILE)gcc
# CFLAGS ?= -g -Wall -Werror
CFLAGS ?= -g
LDFLAGS ?= -lpthread -lrt

#'-I' Adds to list of directories to search for dependencies
INCLUDES= -I $(LWIPDIR)/include
INCLUDES += -I $(LWIPDIR)/../contrib/ports/unix/port/include
INCLUDES += -I $(THIS_MK_DIR)

$(info INCLUDES: $(INCLUDES))


lwip_app: testlwip.c $(COREFILES) $(CORE4FILES) $(CORE6FILES) $(APIFILES) $(PPPFILES) $(LWIPDIR)/netif/ethernet.c sysarch.c $(LWIPDIR)/../contrib/ports/unix/port/netif/sio.c
	@echo "Using compiler: $(CC)"
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS)

clean:
	rm -f testlwip testlwip.o