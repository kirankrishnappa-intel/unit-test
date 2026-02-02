# PCIe Test Driver Framework

A comprehensive Linux kernel module and user-space utilities for testing PCIe devices. This framework provides tools for BAR access, configuration space operations, interrupt handling, and DMA operations.

## Overview

The PCIe Test Driver consists of:

1. **Kernel Module** (`pcie_test_driver.c`) - Core driver module that:
   - Enumerates PCIe devices
   - Maps BAR (Base Address Register) regions
   - Provides BAR read/write operations
   - Handles configuration space access
   - Manages DMA buffers
   - Handles interrupts
   - Exposes sysfs attributes for device monitoring

2. **User-Space Utility** (`pcie_test_utility.c`) - Command-line tool for:
   - Device information queries
   - BAR read/write operations
   - Configuration space access
   - Statistics monitoring
   - Device enumeration

3. **Test Script** (`test_driver.sh`) - Automated testing:
   - Module loading/unloading
   - Basic functionality tests
   - Device status checking

## File Structure

```
unit-test/
├── pcie_test_driver.c      # Main kernel module (2000+ lines)
├── pcie_test_driver.h      # Header with IOCTL definitions
├── pcie_test_utility.c     # User-space test utility
├── test_driver.sh          # Automated test script
├── Makefile                # Build configuration
└── README.md               # This file
```

## Features

### BAR Access
- Read/write operations at 8, 16, and 32-bit granularity
- Support for all 6 BAR regions per device
- Automatic offset validation
- Statistics tracking

### Configuration Space
- PCI configuration space read/write
- Support for 8, 16, and 32-bit operations
- Offset-based access (0x0 - 0x3FF)

### Device Management
- Automatic device enumeration via PCI bus
- Per-device statistics (reads, writes, interrupts)
- Device-specific sysfs attributes
- Hot-plug support

### Interrupt Handling
- Shared interrupt registration
- Interrupt counting and statistics
- Non-blocking interrupt handlers

### DMA Support
- Coherent DMA buffer allocation (4KB)
- Physical address translation
- Memory consistency guarantees

## Building

### Prerequisites

```bash
# Install kernel headers and build tools
sudo apt-get install linux-headers-$(uname -r) build-essential

# For Ubuntu/Debian
sudo apt-get install gcc make

# For RHEL/CentOS
sudo yum install gcc make kernel-devel
```

### Build Steps

```bash
cd unit-test

# Build kernel module
make modules

# Build test utility
gcc -o pcie_test_utility pcie_test_utility.c -Wall -Wextra

# Or use make (shorthand)
make
```

### Build Targets

```bash
make modules      # Build the kernel module
make clean        # Remove build artifacts
make install      # Install module to /lib/modules
make uninstall    # Remove installed module
make load         # Load module into kernel
make unload       # Unload module from kernel
make reload       # Unload and reload module
make info         # Show build information
make help         # Show all targets
```

## Installation

### Install Module

```bash
cd unit-test
sudo make modules
sudo make install
sudo depmod -a
```

### Verify Installation

```bash
# List installed module
modinfo pcie_test_driver

# Or use the test script
sudo ./test_driver.sh check
```

## Usage

### Module Loading

```bash
# Load module manually
sudo insmod pcie_test_driver.ko

# Or use make
sudo make load

# Or use test script
sudo ./test_driver.sh load
```

### Module Parameters

```bash
# Load with debug output
sudo insmod pcie_test_driver.ko debug_level=1

# debug_level values:
# 0 = normal (default)
# 1 = verbose
# 2 = very verbose
```

### Unloading

```bash
# Unload module
sudo rmmod pcie_test_driver

# Or use make
sudo make unload

# Or use test script
sudo ./test_driver.sh unload
```

## Using the Test Utility

### Device Enumeration

```bash
# List all available devices
./pcie_test_utility list

# Output example:
# Available PCIe Test Devices:
# ============================
#   /dev/pcie_test_0
#   /dev/pcie_test_1
```

### Device Information

```bash
# Get device information
./pcie_test_utility -d /dev/pcie_test_0 info

# Output example:
# PCIe Device Information
# =======================
# Vendor ID:           0x8086
# Device ID:           0x0001
# Domain:Bus:Slot.Func: 0000:02:00.0
# IRQ:                 16
# BARs Present:        2
# 
# Statistics:
#   Reads:  0
#   Writes: 0
#   IRQs:   0
```

### BAR Operations

```bash
# Read 32-bit value from BAR0 at offset 0x100
./pcie_test_utility -d /dev/pcie_test_0 bar-read 0 0x100 32

# Write 32-bit value to BAR0 at offset 0x100
./pcie_test_utility -d /dev/pcie_test_0 bar-write 0 0x100 0xDEADBEEF 32

# Read 16-bit value
./pcie_test_utility -d /dev/pcie_test_0 bar-read 0 0x100 16

# Read 8-bit value
./pcie_test_utility -d /dev/pcie_test_0 bar-read 0 0x100 8
```

### Configuration Space Operations

```bash
# Read device status register (offset 0x06)
./pcie_test_utility -d /dev/pcie_test_0 cfg-read 0x06 16

# Write to configuration register
./pcie_test_utility -d /dev/pcie_test_0 cfg-write 0x04 0x0007 16

# Read vendor ID (offset 0x00)
./pcie_test_utility -d /dev/pcie_test_0 cfg-read 0x00 16
```

### Statistics

```bash
# Get operation statistics
./pcie_test_utility -d /dev/pcie_test_0 stats

# Output example:
# Statistics:
#   Reads:  42
#   Writes: 15
#   IRQs:   0
```

### Verbose Mode

```bash
# Enable verbose output
./pcie_test_utility -v -d /dev/pcie_test_0 info
```

## Sysfs Interface

After module is loaded, device attributes are available via sysfs:

```bash
# List PCIe test devices
ls -la /sys/class/pcie_test/

# View device information
cat /sys/class/pcie_test/pcie_test_0/device_info

# View statistics
cat /sys/class/pcie_test/pcie_test_0/bar_read
cat /sys/class/pcie_test/pcie_test_0/bar_write

# Reset statistics
echo 1 > /sys/class/pcie_test/pcie_test_0/reset_stats
```

## Testing

### Automated Testing

```bash
# Run basic tests
sudo ./test_driver.sh test-basic

# Run all tests
sudo ./test_driver.sh test-all

# Check driver status
sudo ./test_driver.sh check
```

### Manual Testing

```bash
# 1. Load the module
sudo ./test_driver.sh load

# 2. List devices
./pcie_test_utility list

# 3. Get device info
./pcie_test_utility info

# 4. Read from BAR0
./pcie_test_utility bar-read 0 0x00 32

# 5. Read configuration space
./pcie_test_utility cfg-read 0x00 16

# 6. Check statistics
./pcie_test_utility stats

# 7. Unload module
sudo ./test_driver.sh unload
```

## Kernel Logging

Monitor kernel messages:

```bash
# View all kernel messages
dmesg | grep pcie_test_driver

# Follow kernel messages in real-time
sudo tail -f /var/log/kern.log | grep pcie_test_driver

# With debug enabled
sudo insmod pcie_test_driver.ko debug_level=2
dmesg | tail -20
```

## IOCTL Commands

The driver supports the following IOCTL commands (from `pcie_test_driver.h`):

| Command | Purpose | Data Structure |
|---------|---------|-----------------|
| `PCIE_IOC_BAR_READ8` | Read 8-bit from BAR | `pcie_bar_access` |
| `PCIE_IOC_BAR_WRITE8` | Write 8-bit to BAR | `pcie_bar_access` |
| `PCIE_IOC_BAR_READ16` | Read 16-bit from BAR | `pcie_bar_access` |
| `PCIE_IOC_BAR_WRITE16` | Write 16-bit to BAR | `pcie_bar_access` |
| `PCIE_IOC_BAR_READ32` | Read 32-bit from BAR | `pcie_bar_access` |
| `PCIE_IOC_BAR_WRITE32` | Write 32-bit to BAR | `pcie_bar_access` |
| `PCIE_IOC_CONFIG_READ8` | Read config byte | `pcie_config_access` |
| `PCIE_IOC_CONFIG_WRITE8` | Write config byte | `pcie_config_access` |
| `PCIE_IOC_CONFIG_READ16` | Read config word | `pcie_config_access` |
| `PCIE_IOC_CONFIG_WRITE16` | Write config word | `pcie_config_access` |
| `PCIE_IOC_CONFIG_READ32` | Read config dword | `pcie_config_access` |
| `PCIE_IOC_CONFIG_WRITE32` | Write config dword | `pcie_config_access` |
| `PCIE_IOC_GET_DEVICE_INFO` | Get device info | `pcie_device_info` |
| `PCIE_IOC_GET_STATISTICS` | Get statistics | `pcie_statistics` |

## Architecture

### Kernel Module Components

```
pcie_test_driver.c
├── Device Management
│   ├── pcie_test_probe()
│   ├── pcie_test_remove()
│   └── Device list tracking
├── BAR Operations
│   ├── pcie_test_map_bars()
│   ├── pcie_test_unmap_bars()
│   ├── BAR read (8/16/32-bit)
│   └── BAR write (8/16/32-bit)
├── Configuration Space
│   ├── Read/write byte/word/dword
│   └── Offset-based access
├── Interrupt Handler
│   └── pcie_test_interrupt_handler()
├── DMA Support
│   ├── pcie_test_setup_dma()
│   └── pcie_test_cleanup_dma()
├── Sysfs Interface
│   ├── device_info attribute
│   ├── bar_read attribute
│   ├── bar_write attribute
│   └── reset_stats attribute
└── PCI Driver Registration
    ├── Device ID table
    └── Driver structure
```

## Limitations

1. **Device Filter**: Currently matches all PCI devices. Can be configured with specific vendor/device IDs in `pcie_test_device_ids`

2. **DMA Buffer Size**: Fixed at 4KB. Can be modified by changing `DMA_BUFFER_SIZE`

3. **Maximum Devices**: Limited to 16 devices. Controlled by `MAX_PCIE_DEVICES`

4. **BAR Count**: Supports 6 BAR regions (standard PCIe limit)

5. **Configuration Space**: 10-bit address space (0x000-0x3FF)

## Troubleshooting

### Module fails to load

```bash
# Check kernel compatibility
uname -r
cat /proc/version

# Check for build errors
make clean
make modules

# View detailed dmesg output
dmesg | tail -20
```

### No devices detected

```bash
# Check if PCIe devices are available
lspci

# Check if module is loaded
lsmod | grep pcie_test_driver

# Check kernel logs
dmesg | grep pcie_test_driver

# Load with debug enabled
sudo insmod pcie_test_driver.ko debug_level=2
```

### Permission denied errors

```bash
# Ensure you have root privileges
sudo -i

# Or prefix commands with sudo
sudo ./pcie_test_utility info
```

## Performance Considerations

- BAR reads/writes are direct memory operations (very fast)
- Configuration space access may be slower (PCI configuration bus)
- Interrupt handlers use spinlocks for thread-safe operation
- DMA buffer is coherent (suitable for device communication)

## Security Considerations

- Module requires root privileges for loading/unloading
- Device access requires appropriate file permissions
- Memory-mapped I/O access is unrestricted (for testing)
- Configuration space changes should be made carefully

## Future Enhancements

- [ ] Selectable device filter (vendor/device ID)
- [ ] Configurable DMA buffer size
- [ ] MSI-X interrupt support
- [ ] Performance benchmarking tools
- [ ] Kernel module documentation (kerneldoc)
- [ ] Unit tests with kernel test framework (KTF)
- [ ] Power management (suspend/resume)
- [ ] Multiple device coordination

## License

GPL v2 or later

## Author

Test Framework

## References

- Linux Device Drivers (3rd Edition) by Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman
- PCI Express Base Specification (PCIe specs)
- Linux Kernel Development (2nd Edition) by Robert Love
- Kernel documentation: `/usr/src/linux/Documentation/PCI/`

## Support

For issues and questions:

1. Check kernel logs: `dmesg | grep pcie_test_driver`
2. Verify module is loaded: `lsmod | grep pcie_test_driver`
3. Run automated tests: `./test_driver.sh test-all`
4. Review this README and code comments
