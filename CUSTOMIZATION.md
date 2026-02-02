# PCIe Test Driver - Configuration and Customization Guide

This document explains how to customize the PCIe test driver framework for your specific needs.

## Device Filter Configuration

By default, the driver matches ALL PCIe devices. To restrict to specific devices, modify the device ID table in `pcie_test_driver.c`:

### Current Configuration (Match All)
```c
static struct pci_device_id pcie_test_device_ids[] = {
    /* Wildcard: match any PCIe device */
    { PCI_DEVICE(PCI_ANY_ID, PCI_ANY_ID) },
    { 0 }
};
```

### Example: Match Intel Devices Only
```c
static struct pci_device_id pcie_test_device_ids[] = {
    /* Intel vendor ID = 0x8086 */
    { PCI_DEVICE(0x8086, PCI_ANY_ID) },
    { 0 }
};
```

### Example: Match Specific Device
```c
static struct pci_device_id pcie_test_device_ids[] = {
    /* Intel Ethernet Controller I225-V */
    { PCI_DEVICE(0x8086, 0x15F9) },
    { 0 }
};
```

### Example: Match Multiple Specific Devices
```c
static struct pci_device_id pcie_test_device_ids[] = {
    { PCI_DEVICE(0x8086, 0x0D01) },  /* Device 1 */
    { PCI_DEVICE(0x8086, 0x0D04) },  /* Device 2 */
    { PCI_DEVICE(0x1234, 0x5678) },  /* Custom vendor device */
    { 0 }
};
```

### Finding Your Device Vendor/Device IDs

```bash
# List all PCI devices with IDs
lspci -n

# Output example:
# 00:1f.3 8086:1f3c
#         ^^^^ ^^^^
#         Vendor ID
#         Device ID

# Or use
lspci -v

# Search for specific device
lspci | grep -i "your device name"
```

## Constants Configuration

### Maximum Devices
```c
#define MAX_PCIE_DEVICES 16
```
**Location:** Line ~45  
**Change to:** Increase or decrease based on expected number of devices

### DMA Buffer Size
```c
#define DMA_BUFFER_SIZE (4 * 1024)  /* 4KB DMA buffer */
```
**Location:** Line ~47  
**Common sizes:**
- `(4 * 1024)` = 4 KB (default)
- `(16 * 1024)` = 16 KB
- `(1024 * 1024)` = 1 MB
- `(4 * 1024 * 1024)` = 4 MB

### Module Name and Version
```c
#define PCIE_TEST_DRIVER_NAME "pcie_test_driver"
#define PCIE_TEST_DRIVER_VERSION "1.0"
```
**Location:** Lines ~42-43

## Logging Configuration

### Debug Levels
The driver supports three debug levels:

```bash
# Load with debug level 0 (normal - no extra logging)
sudo insmod pcie_test_driver.ko debug_level=0

# Load with debug level 1 (verbose - important info)
sudo insmod pcie_test_driver.ko debug_level=1

# Load with debug level 2 (very verbose - all details)
sudo insmod pcie_test_driver.ko debug_level=2
```

### Adding Custom Debug Statements
```c
DBG(level, fmt, ...)      // Debug messages
INFO(fmt, ...)            // Informational
WARN(fmt, ...)            // Warning
ERR(fmt, ...)             // Error
```

**Example:**
```c
DBG(1, "Custom device initialized at 0x%lx\n", base_address);
```

## Customizing BAR Access

### Change Memory Access Width
Default operations support 8/16/32-bit access. To add 64-bit support:

```c
static u64 pcie_test_bar_read64(struct pcie_test_dev *dev, int bar_num, 
                                unsigned long offset)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return 0xFFFFFFFFFFFFFFFFULL;
    
    if (offset >= dev->bar_len[bar_num])
        return 0xFFFFFFFFFFFFFFFFULL;
    
    return readq(dev->bar_virt[bar_num] + offset);
}

static void pcie_test_bar_write64(struct pcie_test_dev *dev, int bar_num,
                                  unsigned long offset, u64 value)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return;
    
    if (offset >= dev->bar_len[bar_num])
        return;
    
    writeq(value, dev->bar_virt[bar_num] + offset);
}
```

### Add Hardware-Specific Operations
```c
static int pcie_test_custom_operation(struct pcie_test_dev *dev)
{
    u32 status = pcie_test_bar_read32(dev, 0, 0x00);
    
    if (status & CUSTOM_STATUS_READY) {
        DBG(1, "Device is ready\n");
        return 0;
    }
    
    return -EBUSY;
}
```

## Customizing Sysfs Attributes

### Add New Attribute
```c
static ssize_t custom_attr_show(struct device *d, 
                                struct device_attribute *attr, 
                                char *buf)
{
    struct pcie_test_dev *dev = dev_get_drvdata(d);
    if (!dev)
        return -ENODEV;
    
    return sprintf(buf, "Custom value: %d\n", dev->custom_value);
}

static DEVICE_ATTR_RO(custom_attr);
```

### Register New Attribute
Add to `pcie_test_attrs` array:
```c
static struct attribute *pcie_test_attrs[] = {
    &dev_attr_device_info.attr,
    &dev_attr_bar_read.attr,
    &dev_attr_bar_write.attr,
    &dev_attr_reset_stats.attr,
    &dev_attr_custom_attr.attr,  // Add this
    NULL
};
```

## Customizing Statistics

### Add New Counter
In `struct pcie_test_dev`:
```c
struct pcie_test_dev {
    // ... existing fields ...
    u64 read_count;
    u64 write_count;
    u64 irq_count;
    u64 custom_counter;  // Add this
};
```

### Update Statistics Collection
```c
void custom_operation(struct pcie_test_dev *dev)
{
    spin_lock(&dev->lock);
    dev->custom_counter++;
    spin_unlock(&dev->lock);
}
```

## Module Parameters

### Add New Parameter
```c
static int custom_param = 0;
module_param(custom_param, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(custom_param, "Custom parameter description");
```

### Load with Parameter
```bash
sudo insmod pcie_test_driver.ko custom_param=42
```

## Interrupt Handling Customization

### Custom Interrupt Handler
```c
static irqreturn_t pcie_test_interrupt_handler(int irq, void *dev_id)
{
    struct pcie_test_dev *dev = dev_id;
    
    if (!dev)
        return IRQ_NONE;
    
    spin_lock(&dev->lock);
    dev->irq_count++;
    
    // Add custom logic here
    u32 status = pcie_test_bar_read32(dev, 0, 0x00);
    if (status & INTERRUPT_SOURCE_MASK) {
        // Handle custom interrupt
        pcie_test_bar_write32(dev, 0, 0x00, status);
    }
    
    spin_unlock(&dev->lock);
    
    return IRQ_HANDLED;
}
```

## DMA Buffer Customization

### Change Buffer Allocation
```c
static int pcie_test_setup_dma(struct pcie_test_dev *dev)
{
    size_t size = 16 * 1024;  // Change size here
    
    dev->dma_buffer = dma_alloc_coherent(&dev->pdev->dev, size,
                                         &dev->dma_handle, GFP_KERNEL);
    
    if (!dev->dma_buffer) {
        ERR("Failed to allocate DMA buffer\n");
        return -ENOMEM;
    }
    
    // Store size for later use
    dev->dma_size = size;
    
    return 0;
}
```

### Use DMA Buffer in Custom Function
```c
void pcie_test_dma_operation(struct pcie_test_dev *dev)
{
    // Write data to DMA buffer
    memset(dev->dma_buffer, 0, dev->dma_size);
    
    // Trigger DMA operation by writing physical address to device
    u32 dma_addr = (u32)dev->dma_handle;
    pcie_test_bar_write32(dev, 0, 0x10, dma_addr);
    
    // Wait for completion
    // ... implementation specific ...
}
```

## Build Configuration

### Optimize Build
Edit `Makefile`:
```makefile
# Add optimization flags
ccflags-y := -std=gnu99 -Wall -Wextra -O3 -DOPTIMIZE_PERFORMANCE

# Or disable for debugging
ccflags-y := -std=gnu99 -Wall -Wextra -O0 -g
```

### Custom Install Location
```makefile
# Change install directory
install: modules
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=/custom/path modules_install
```

## Creating Device-Specific Modules

### Template for Custom Device Driver
```c
/*
 * Based on PCIe Test Driver Framework
 * Customize for your device
 */

#include "pcie_test_driver.h"

#define CUSTOM_DEVICE_VENDOR 0x1234
#define CUSTOM_DEVICE_ID     0x5678

// Custom constants
#define CUSTOM_REG_STATUS    0x00
#define CUSTOM_REG_CONTROL   0x04
#define CUSTOM_REG_DATA      0x08

// Custom operations
static int custom_device_init(struct pcie_test_dev *dev)
{
    u32 status = pcie_test_bar_read32(dev, 0, CUSTOM_REG_STATUS);
    
    INFO("Device status: 0x%x\n", status);
    
    // Initialize device
    pcie_test_bar_write32(dev, 0, CUSTOM_REG_CONTROL, 0x01);
    
    return 0;
}

MODULE_ALIAS("pci:v00001234d00005678sv*sd*bc*sc*i*");
```

## Testing Customizations

### Build and Test
```bash
# Build with changes
make clean
make modules

# Load module
sudo insmod pcie_test_driver.ko debug_level=2

# Test with utility
./pcie_test_utility info

# Check logs
dmesg | tail -20

# Unload
sudo rmmod pcie_test_driver
```

## Debugging Tips

### Enable All Debug Output
```bash
sudo insmod pcie_test_driver.ko debug_level=2
dmesg -w  # Watch in real-time
```

### Check Module Info
```bash
modinfo pcie_test_driver.ko
```

### Monitor Device Operations
```bash
# Watch BAR operations in sysfs
watch -n 1 'cat /sys/class/pcie_test/pcie_test_0/device_info'
```

### Trace System Calls
```bash
strace ./pcie_test_utility -d /dev/pcie_test_0 info
```

## Common Customizations Checklist

- [ ] Set device filter (vendor/device IDs)
- [ ] Adjust DMA buffer size
- [ ] Add custom sysfs attributes
- [ ] Implement device-specific operations
- [ ] Add custom interrupt handling
- [ ] Configure module parameters
- [ ] Update documentation
- [ ] Test with your hardware
- [ ] Verify build with `make clean && make modules`
- [ ] Test loading/unloading

## Performance Tuning

### For Low-Latency Operations
```c
// Use preempt-rt kernel
// Reduce buffer copies
// Use DMA for large transfers
```

### For High-Throughput Operations
```c
// Increase DMA buffer size
// Batch operations
// Use memory pooling
```

## Additional Resources

- Linux Device Drivers (3rd Edition) - Chapter on PCI Drivers
- Linux Kernel documentation: `/usr/src/linux/Documentation/PCI/`
- PCIe Base Specification (available from PCI-SIG)
- Kernel source: `/usr/src/linux/drivers/pci/`

---

For support with customization, refer to the README.md and inline code comments.
