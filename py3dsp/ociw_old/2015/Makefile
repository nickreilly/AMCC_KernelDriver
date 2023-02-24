#
# Main Makefile for the ociw S5920 PCI  driver
#
obj-m	:= amcc.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
#KVERSION := $(shell uname -r)

# ccflags-y := -msse2 -mmmx -Wall
# EXTRA_CFLAGS := -msse2 -mmmx
# EXTRA_CFLAGS := -Wall

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko* *.mod.c .tmp_versions

#KDIR := /lib/modules/$(shell uname -r)/build
#PWD := $(shell pwd)
#obj-m := amcc.o
#EXTRA_CFLAGS := -msse2 -mmmx
#EXTRA_CFLAGS := -Wall

#default:
#	$(MAKE) -C $(KDIR) M=$(PWD) modules
