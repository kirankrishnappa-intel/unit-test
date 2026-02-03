# Unit Test Framework - Makefile for Intel Bluetooth Test Generic Driver
# Supports: Intel Bluetooth Test Generic Driver

# Kernel build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Module name
MODULE_NAME := btintel_test_generic_driver
obj-m := btintel_test_generic_driver.o

# Userspace application
USERSPACE_APP := btintel_test_userspace
USERSPACE_SRC := btintel_test_userspace.c
USERSPACE_HDR := btintel_test_userspace.h

# Compiler flags
ccflags-y := -std=gnu99 -Wall -Wextra -O2
ccflags-y += -I$(PWD)/../backport-include
ccflags-y += -I$(PWD)/../include
ccflags-y += -I$(PWD)/../drivers/bluetooth

# Userspace compiler flags
USERSPACE_CFLAGS := -Wall -Wextra -O2 -g

# Default target
all: modules userspace

# Build kernel modules
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Build userspace application
userspace: $(USERSPACE_SRC) $(USERSPACE_HDR)
	gcc $(USERSPACE_CFLAGS) -o $(USERSPACE_APP) $(USERSPACE_SRC)
	@echo "Userspace application built: $(USERSPACE_APP)"

# Clean build artifacts
clean: clean-modules clean-userspace
	rm -f .*.cmd

clean-modules:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.mod.c *.mod.o *.o
	rm -f .*.cmd Module.symvers modules.order
	rm -rf .tmp_versions

clean-userspace:
	rm -f $(USERSPACE_APP)
	@echo "Userspace application cleaned"

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

# Run userspace test (requires module to be loaded)
run-userspace: userspace
	./$(USERSPACE_APP)

# Show module info
info:
	@echo "Module: $(MODULE_NAME)"
	@echo "Kernel: $(shell uname -r)"
	@echo "Build dir: $(KDIR)"
	@echo "Userspace app: $(USERSPACE_APP)"

# Help
help:
	@echo "Intel Bluetooth Test Generic Driver - Make targets:"
	@echo ""
	@echo "Kernel Module Targets:"
	@echo "  all        - Build kernel module and userspace app (default)"
	@echo "  modules    - Build the kernel module"
	@echo "  clean      - Remove all build artifacts"
	@echo "  clean-modules - Remove kernel module artifacts only"
	@echo "  install    - Install the module"
	@echo "  uninstall  - Uninstall the module"
	@echo "  load       - Load the module"
	@echo "  unload     - Unload the module"
	@echo "  reload     - Unload and load the module"
	@echo ""
	@echo "Userspace Application Targets:"
	@echo "  userspace      - Build userspace test application"
	@echo "  clean-userspace - Remove userspace application"
	@echo "  run-userspace  - Run userspace test (requires module loaded)"
	@echo ""
	@echo "Info:"
	@echo "  info       - Show module and build information"
	@echo "  help       - Show this help message"

.PHONY: all modules userspace clean clean-modules clean-userspace install uninstall load unload reload run-userspace info help
