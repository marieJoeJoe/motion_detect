ifneq  ($(KERNELRELEASE),)

obj-m:=leds-ws2812b.o

else

#KERDIR := /lib/modules/$(shell uname -r)/build
KERDIR := /home/marie/raspberry-pi/linux
PWD:=$(shell pwd)

all:
	make -C $(KERDIR) M=$(PWD) modules

clean:
	rm -f *.ko *.o *.symvers *.cmd *.cmd.o modules.* *.mod.c
endif
