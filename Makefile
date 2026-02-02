# Unit Test Framework - Makefile with Kconfig & Multiple Driver Support
# Supports: PCIe Test Driver, Intel Bluetooth Test Generic Driver

# Kernel build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Module name - can be overridden with BUILD_DRIVER=btintel
BUILD_DRIVER ?= pcie
ifeq ($(BUILD_DRIVER),btintel)
  MODULE_NAME := btintel_test_generic_driver
  obj-m := btintel_test_generic_driver.o
else
  MODULE_NAME := pcie_test_driver
  obj-m := pcie_test_driver.o
endif

# Compiler flags
ccflags-y := -std=gnu99 -Wall -Wextra -O2

# Add debug flags if enabled
ifeq ($(CONFIG_PCIE_TEST_DRIVER_DEBUG),y)
ccflags-y += -DDEBUG -g
endif

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
	@echo "Module path: /lib/modules/$(shell uname -r)/kernel/drivers/misc/"
	@echo ""
	@echo "Kconfig Settings:"
	@echo "CONFIG_PCIE_TEST_DRIVER=$(CONFIG_PCIE_TEST_DRIVER)"
	@echo "CONFIG_PCIE_TEST_DRIVER_DEBUG=$(CONFIG_PCIE_TEST_DRIVER_DEBUG)"
	@echo "CONFIG_BUILD_PCIE_TEST_UTILITY=$(CONFIG_BUILD_PCIE_TEST_UTILITY)"
	@echo "CONFIG_BUILD_PCIE_TEST_SCRIPTS=$(CONFIG_BUILD_PCIE_TEST_SCRIPTS)"

# Help
help:
	@echo "Unit Test Framework - Make targets:"
	@echo ""
	@echo "PCIe Test Driver (default):"
	@echo "  all      - Build the kernel module (default)"
	@echo "  modules  - Build the kernel module"
	@echo "  clean    - Remove build artifacts"
	@echo "  install  - Install the module"
	@echo "  uninstall - Uninstall the module"
	@echo "  load     - Load the module"
	@echo "  unload   - Unload the module"
	@echo "  reload   - Unload and load the module"
	@echo "  info     - Show module information"
	@echo ""
	@echo "Intel Bluetooth Test Generic Driver:"
	@echo "  make -f Makefile.btintel_test modules  - Build btintel test driver"
	@echo "  make -f Makefile.btintel_test load     - Load btintel test driver"
	@echo "  make -f Makefile.btintel_test unload   - Unload btintel test driver"
	@echo "  make -f Makefile.btintel_test status   - Show module status"
	@echo ""
	@echo "  help     - Show this help message"

.PHONY: all modules clean install uninstall load unload reload info help
