# Linux Driver Skeleton - Complete Guide

## Overview

This is a production-ready skeleton for a Linux PCI device driver. It provides:

- **Full PCI device lifecycle management** (probe, remove)
- **Memory-mapped I/O (BAR) access**
- **Interrupt handling** (MSI, MSI-X ready)
- **DMA support** (mask configuration)
- **IOCTL interface** for user-space communication
- **Comprehensive error handling**
- **Module metadata and versioning**
- **Debugging infrastructure**

## File Structure

```
driver_skeleton.c      - Main driver implementation (400+ lines)
driver_skeleton.h      - Header with IOCTL definitions and register macros
Makefile.skeleton      - Build system with multiple targets
DRIVER_SKELETON_GUIDE.md - This file
```

## Quick Start

### 1. Build the Module

```bash
# In the unit-test directory
make -f Makefile.skeleton modules

# Output: skeleton_driver.ko (if compilation succeeds)
```

### 2. Load the Module

```bash
sudo make -f Makefile.skeleton load

# Check if loaded
lsmod | grep skeleton_driver
```

### 3. Check Status

```bash
make -f Makefile.skeleton status
make -f Makefile.skeleton dmesg
```

### 4. Unload the Module

```bash
sudo make -f Makefile.skeleton unload
```

## Source Code Structure

### driver_skeleton.c

#### 1. **Module Metadata** (lines 30-40)
Declares module information visible in `modinfo`:
```c
MODULE_DESCRIPTION("Linux Kernel Driver Skeleton");
MODULE_AUTHOR("Your Name");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
```

#### 2. **Data Structures** (lines 60-90)
Main device structure `skeleton_device`:
```c
struct skeleton_device {
    struct pci_dev *pdev;        // PCI device
    struct device *dev;          // Device pointer for logging
    void __iomem *bar0;          // Mapped BAR0 memory
    resource_size_t bar_len;     // BAR0 size
    int irq;                      // Interrupt number
    spinlock_t lock;              // Serialization
    bool active;                  // Device state
    struct { ... } stats;         // Statistics
};
```

#### 3. **PCI Device Table** (lines 130-140)
Lists supported devices:
```c
static const struct pci_device_id skeleton_pci_tbl[] = {
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xABCD) },
    { 0 }  // Required terminator
};
```

#### 4. **Interrupt Handler** (lines 160-195)
```c
static irqreturn_t skeleton_interrupt_handler(int irq, void *data)
{
    // Called when device interrupts
    // Must return IRQ_HANDLED or IRQ_NONE
}
```

#### 5. **Device Enable/Disable** (lines 200-260)
```c
static int skeleton_enable_device(struct skeleton_device *skel);
static void skeleton_disable_device(struct skeleton_device *skel);
```

#### 6. **PCI Probe** (lines 270-380)
Called when device is detected. Initializes:
- Allocate device structure (`kzalloc`)
- Enable PCI device (`pci_enable_device`)
- Request memory regions (`pci_request_regions`)
- Set DMA mask (`dma_set_mask`)
- Map BAR0 (`pci_iomap`)
- Request IRQ (`request_irq`)
- Enable hardware

#### 7. **PCI Remove** (lines 385-415)
Called on driver unload. Cleanup in reverse order:
- Disable device
- Free IRQ
- Unmap BAR0
- Release regions
- Disable PCI device
- Free structures

#### 8. **Module Init/Exit** (lines 420-450)
```c
static int __init skeleton_init(void)   // Runs at modprobe
static void __exit skeleton_exit(void)  // Runs at rmmod
```

### driver_skeleton.h

#### 1. **IOCTL Magic Number**
```c
#define SKELETON_IOC_MAGIC 'S'  // Unique identifier
```

#### 2. **IOCTL Commands**
```c
#define SKELETON_IOC_GET_INFO    _IOR(...)  // Read device info
#define SKELETON_IOC_RESET       _IO(...)   // No arguments
#define SKELETON_IOC_BAR_READ    _IOR(...)  // Read BAR register
#define SKELETON_IOC_BAR_WRITE   _IOW(...)  // Write BAR register
#define SKELETON_IOC_GET_STATE   _IOR(...)  // Get device state
#define SKELETON_IOC_SET_STATE   _IOW(...)  // Set device state
```

#### 3. **Register Definitions**
```c
#define SKELETON_STATUS_REG          0x00
#define SKELETON_CONTROL_REG         0x04
#define SKELETON_STATUS_READY        BIT(0)
#define SKELETON_CTRL_ENABLE         BIT(0)
```

#### 4. **Helper Macros**
```c
#define SKELETON_READ_REG(base, reg)   readl((base) + (reg))
#define SKELETON_WRITE_REG(base, reg, val) writel((val), (base) + (reg))
#define SKELETON_SET_BITS(base, reg, bits)
#define SKELETON_CLEAR_BITS(base, reg, bits)
```

#### 5. **Inline Helper Functions**
```c
skeleton_wait_for_condition()  // Poll register for condition
```

## Customization Guide

### 1. Rename Your Driver

Replace all instances of `skeleton`:
```bash
sed -i 's/skeleton/mydriver/g' driver_skeleton.c driver_skeleton.h
sed -i 's/SKELETON/MYDRIVER/g' driver_skeleton.c driver_skeleton.h
```

### 2. Add Supported Devices

In `driver_skeleton.c`, update the PCI device table:
```c
static const struct pci_device_id skeleton_pci_tbl[] = {
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA876) },
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xE476) },
    { PCI_DEVICE(PCI_VENDOR_ID_BROADCOM, 0x1234) },
    { 0 }
};
```

### 3. Add Register Definitions

In `driver_skeleton.h`, replace example registers with your hardware:
```c
#define MYDEV_CSR_BASE              0x000
#define MYDEV_STATUS_REG            (MYDEV_CSR_BASE + 0x00)
#define MYDEV_CONTROL_REG           (MYDEV_CSR_BASE + 0x04)
#define MYDEV_INTERRUPT_STATUS_REG  (MYDEV_CSR_BASE + 0x10)

/* Bit fields */
#define MYDEV_STATUS_READY          BIT(0)
#define MYDEV_STATUS_ERROR          BIT(1)
#define MYDEV_CTRL_ENABLE           BIT(0)
#define MYDEV_CTRL_RESET            BIT(1)
```

### 4. Implement Device Initialization

In `skeleton_enable_device()`, add hardware setup:
```c
static int skeleton_enable_device(struct skeleton_device *skel)
{
    // Set your hardware to active state
    u32 ctrl = SKELETON_READ_REG(skel->bar0, SKELETON_CONTROL_REG);
    ctrl |= SKELETON_CTRL_ENABLE | SKELETON_CTRL_INTERRUPT_EN;
    SKELETON_WRITE_REG(skel->bar0, SKELETON_CONTROL_REG, ctrl);
    
    // Wait for device ready
    return skeleton_wait_for_condition(skel->bar0,
        SKELETON_STATUS_REG,
        SKELETON_STATUS_READY,
        SKELETON_STATUS_READY,
        100);  // 100ms timeout
}
```

### 5. Implement Interrupt Handler

In `skeleton_interrupt_handler()`, add your logic:
```c
static irqreturn_t skeleton_interrupt_handler(int irq, void *data)
{
    struct skeleton_device *skel = data;
    u32 status;
    
    status = SKELETON_READ_REG(skel->bar0, SKELETON_INTERRUPT_STATUS_REG);
    
    if (!status)
        return IRQ_NONE;  // Not our interrupt
    
    // Handle specific interrupt conditions
    if (status & SKELETON_INTR_RX_DONE)
        handle_rx_complete(skel);
    
    if (status & SKELETON_INTR_TX_DONE)
        handle_tx_complete(skel);
    
    // Clear interrupt status
    SKELETON_WRITE_REG(skel->bar0, SKELETON_INTERRUPT_STATUS_REG, status);
    
    skel->stats.interrupts++;
    return IRQ_HANDLED;
}
```

### 6. Add New IOCTL Commands

In `driver_skeleton.h`:
```c
#define SKELETON_IOC_CUSTOM_OP \
    _IOWR(SKELETON_IOC_MAGIC, 10, struct custom_data)
```

In `driver_skeleton.c`, add `unlocked_ioctl` handler.

### 7. Add Debug Output

Enable debug macro:
```c
#define DEBUG  // Uncomment to enable debug prints
```

Or in Makefile:
```makefile
EXTRA_CFLAGS += -DDEBUG -g
```

## Common Issues & Solutions

### Issue: "Device not found"
**Solution:** Check PCI IDs match your hardware:
```bash
lspci -nn | grep Intel
# Compare with device IDs in skeleton_pci_tbl
```

### Issue: "Failed to request IRQ"
**Solution:** IRQ may be in use. Check:
```bash
cat /proc/interrupts | grep <irq_number>
```

### Issue: "BAR mapping failed"
**Solution:** BAR might not be enabled:
```bash
lspci -v | grep -A20 "Bluetooth"
# Check if Memory access is enabled
```

### Issue: Module won't unload
**Solution:** Reference count too high:
```bash
lsmod | grep skeleton_driver
# Close all users (file handles, sysfs, etc.)
```

## Testing

### 1. Basic Module Test
```bash
make -f Makefile.skeleton modules
sudo make -f Makefile.skeleton load
make -f Makefile.skeleton status
sudo make -f Makefile.skeleton unload
```

### 2. Device Detection Test
```bash
sudo dmesg | tail -20
# Should show: "Found device: vendor=0x..., device=0x..."
```

### 3. Interrupt Test
```bash
cat /proc/interrupts | grep skeleton_driver
# After interrupts: count should increase
```

### 4. User-Space IOCTL Test
Create a test program:
```c
#include "driver_skeleton.h"
#include <fcntl.h>
#include <sys/ioctl.h>

int main() {
    int fd = open("/dev/skeleton_device", O_RDWR);
    struct skeleton_dev_info info;
    
    if (ioctl(fd, SKELETON_IOC_GET_INFO, &info) == 0) {
        printf("Device: %04x:%04x\n", info.vendor_id, info.device_id);
    }
    
    close(fd);
    return 0;
}
```

## Makefile Targets

```bash
make -f Makefile.skeleton modules        # Build module
make -f Makefile.skeleton clean          # Clean build files
make -f Makefile.skeleton load           # Load module (root)
make -f Makefile.skeleton unload         # Unload module (root)
make -f Makefile.skeleton reload         # Reload module (root)
make -f Makefile.skeleton status         # Show load status
make -f Makefile.skeleton dmesg          # Show kernel messages
make -f Makefile.skeleton log            # Show systemd log
make -f Makefile.skeleton info           # Show build info
make -f Makefile.skeleton help           # Show all targets
```

## Performance Optimization

### 1. Register Access
Use `readl()/writel()` for 32-bit, `readq()/writeq()` for 64-bit.

### 2. Interrupt Handling
Keep handler short. Move heavy processing to work queue:
```c
schedule_work(&skel->work);  // In interrupt handler
```

### 3. DMA Setup
Ensure 128-byte or 256-byte alignment for optimal performance.

### 4. Locking
Use `spin_lock_irqsave()` in interrupt context, `mutex` elsewhere.

## Security Considerations

1. **Validate user input** in IOCTL handlers
2. **Check permissions** for privileged operations
3. **Prevent buffer overflows** in fixed-size arrays
4. **Use secure allocation** functions
5. **Sanitize debug output** (no secrets in dmesg)

## References

- Linux Kernel Module Programming Guide: https://kernel-team.pages.debian.net/kernel-handbook/
- PCI Subsystem: https://www.kernel.org/doc/Documentation/PCI/
- Device Drivers: https://www.kernel.org/doc/html/latest/driver-api/
- IOCTL Interface: https://www.kernel.org/doc/html/latest/userspace-api/ioctl/

## License

These skeleton files are provided under GPL-2.0-or-later. Modify freely for your use case.

## Questions?

Refer to the inline comments in the source code for detailed explanations of each section.
