obj-m += hiddenfile.o

all: build

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: build
	sudo insmod hiddenfile.ko

unload: build
	sudo insmod hiddenfile
