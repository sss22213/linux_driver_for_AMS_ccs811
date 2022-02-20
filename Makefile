PWD := $(shell pwd)
CC := gcc
KVERSION := $(shell uname -r)
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

MODULE_NAME = ccs811
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs += ccs811_sensor.o ccs811_driver.o

all: clean test
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	rm -f test
	make -C $(KERNEL_DIR) M=$(PWD) clean
test:
	rm -f test
	gcc -o test test.c -g -Wall
