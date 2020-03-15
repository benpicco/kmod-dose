obj-m := dose.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

%.dtbo: %.dts
	dtc -@ -O dtb -o $@ $<

install: dose.ko dose.dtbo
	-sudo rmmod dose.ko
	sudo cp dose.dtbo /boot/overlays/
	sudo cp dose.ko /lib/modules/`uname -r`/kernel/drivers/net/ethernet
	sudo dtoverlay dose
	sudo modprobe dose
