obj-m := dose.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

%.dtbo: %.dts
	dtc -@ -O dtb -o $@ $<

run: dose.ko
	-sudo rmmod dose.ko
	sudo insmod dose.ko

install: dose.ko
	sudo cp dose.ko /lib/modules/`uname -r`/kernel/drivers/net/ethernet
