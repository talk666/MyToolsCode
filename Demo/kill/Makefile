ccflags-y += -std=gnu9x -Wall -Wno-unused-function -Wno-unused-variable -Wno-format

ifneq ($(KERNELRELEASE),)
        obj-m := kill.o
else
        KERNELDIR ?= /lib/modules/$(shell uname -r)/build
        PWD := $(shell pwd)

default:
	make -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -f *.o *.ko Module.symvers modules.order *.mod.c .*.cmd
