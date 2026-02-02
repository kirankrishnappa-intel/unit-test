# Unit Test Framework - Makefile for Intel Bluetooth Test Generic Driver
# Supports: Intel Bluetooth Test Generic Driver

# Kernel build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Module name
MODULE_NAME := btintel_test_generic_driver
obj-m := btintel_test_generic_driver.o

# Compiler flags
ccflags-y := -std=gnu99 -Wall -Wextra -O2
ccflags-y += -I$(PWD)/../backport-include
ccflags-y += -I$(PWD)/../include
ccflags-y += -I$(PWD)/../drivers/bluetooth

# Default target
all: modules

# Build kernel modules
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.mod.c *.mod.o *.o
	rm -f .*.cmd Module.symvers modules.order
	rm -rf .tmp_versions

# Install module
install: modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

# Uninstall module
uninstall:
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/misc/$(MODULE_NAME).ko
	depmod -a

# Load module
load: modules
	insmod ./$(MODULE_NAME).ko

# Unload module
unload:
	-rmmod $(MODULE_NAME)

# Reload module
reload: unload load

# Show module info
info:
	@echo "Module: $(MODULE_NAME)"
	@echo "Kernel: $(shell uname -r)"
	@echo "Build dir: $(KDIR)"

# Help
help:
	@echo "Intel Bluetooth Test Generic Driver - Make targets:"
	@echo ""
	@echo "  all      - Build the kernel module (default)"
	@echo "  modules  - Build the kernel module"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install the module"
	@echo "  uninstall - Uninstall the module"
	@echo "  load     - Load the module"
	@echo "  unload   - Unload the module"
	@echo "  reload   - Unload and load the module"
	@echo "  info     - Show module information"
	@echo "  help     - Show this help message"

.PHONY: all modules clean install uninstall load unload reload info help
