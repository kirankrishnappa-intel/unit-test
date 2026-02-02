/*
 * PCIe Test Driver - Kernel Module for Testing PCIe Device Operations
 *
 * This driver provides a framework for testing PCIe devices with support for:
 * - Device enumeration and initialization
 * - BAR (Base Address Register) access
 * - Configuration space operations
 * - Interrupts handling
 * - DMA operations
 * - Performance testing
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#define PCIE_TEST_DRIVER_NAME "pcie_test_driver"
#define PCIE_TEST_DRIVER_VERSION "1.0"
#define MAX_PCIE_DEVICES 16
#define DMA_BUFFER_SIZE (4 * 1024)  /* 4KB DMA buffer */

/* Module parameters */
static int debug_level = 0;
module_param(debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_level, "Debug level (0=normal, 1=verbose, 2=very verbose)");

/* Device structure */
struct pcie_test_dev {
    struct pci_dev *pdev;
    void __iomem *bar_virt[6];
    unsigned long bar_len[6];
    unsigned long bar_start[6];
    int bar_flags[6];
    
    /* DMA related */
    dma_addr_t dma_handle;
    void *dma_buffer;
    
    /* Device management */
    struct device *dev;
    int device_id;
    spinlock_t lock;
    
    /* Statistics */
    u64 read_count;
    u64 write_count;
    u64 irq_count;
};

/* Global device list */
static struct pcie_test_dev *pcie_devices[MAX_PCIE_DEVICES];
static int device_count = 0;
static DEFINE_SPINLOCK(device_list_lock);

/* Character device */
static dev_t pcie_test_dev_t;
static struct class *pcie_test_class;

#define DBG(level, fmt, ...) \
    do { \
        if (debug_level >= level) \
            printk(KERN_DEBUG "[%s] " fmt, PCIE_TEST_DRIVER_NAME, ##__VA_ARGS__); \
    } while (0)

#define INFO(fmt, ...) printk(KERN_INFO "[%s] " fmt, PCIE_TEST_DRIVER_NAME, ##__VA_ARGS__)
#define WARN(fmt, ...) printk(KERN_WARNING "[%s] " fmt, PCIE_TEST_DRIVER_NAME, ##__VA_ARGS__)
#define ERR(fmt, ...) printk(KERN_ERR "[%s] " fmt, PCIE_TEST_DRIVER_NAME, ##__VA_ARGS__)

/* ============================================================================
 * BAR Access Functions
 * ============================================================================
 */

static int pcie_test_map_bars(struct pcie_test_dev *dev)
{
    struct pci_dev *pdev = dev->pdev;
    int i, ret = 0;
    
    DBG(1, "Mapping BAR regions for device %04x:%04x\n",
        pdev->vendor, pdev->device);
    
    for (i = 0; i < 6; i++) {
        unsigned long flags = pci_resource_flags(pdev, i);
        unsigned long start = pci_resource_start(pdev, i);
        unsigned long len = pci_resource_len(pdev, i);
        
        if (len == 0)
            continue;
        
        if (!(flags & IORESOURCE_MEM)) {
            DBG(2, "BAR%d is not memory resource (flags=0x%lx)\n", i, flags);
            continue;
        }
        
        dev->bar_start[i] = start;
        dev->bar_len[i] = len;
        dev->bar_flags[i] = flags;
        
        if (request_mem_region(start, len, PCIE_TEST_DRIVER_NAME) == NULL) {
            WARN("Failed to request BAR%d memory region\n", i);
            continue;
        }
        
        dev->bar_virt[i] = ioremap(start, len);
        if (!dev->bar_virt[i]) {
            ERR("Failed to remap BAR%d\n", i);
            release_mem_region(start, len);
            ret = -ENOMEM;
            continue;
        }
        
        INFO("Mapped BAR%d: 0x%lx -> 0x%p (size: 0x%lx)\n",
            i, start, dev->bar_virt[i], len);
    }
    
    return ret;
}

static void pcie_test_unmap_bars(struct pcie_test_dev *dev)
{
    int i;
    
    for (i = 0; i < 6; i++) {
        if (!dev->bar_virt[i])
            continue;
        
        iounmap(dev->bar_virt[i]);
        release_mem_region(dev->bar_start[i], dev->bar_len[i]);
        dev->bar_virt[i] = NULL;
        
        DBG(1, "Unmapped BAR%d\n", i);
    }
}

/* ============================================================================
 * Configuration Space Access
 * ============================================================================
 */

static int pcie_test_read_config_byte(struct pcie_test_dev *dev, int offset, u8 *val)
{
    return pci_read_config_byte(dev->pdev, offset, val);
}

static int pcie_test_write_config_byte(struct pcie_test_dev *dev, int offset, u8 val)
{
    return pci_write_config_byte(dev->pdev, offset, val);
}

static int pcie_test_read_config_word(struct pcie_test_dev *dev, int offset, u16 *val)
{
    return pci_read_config_word(dev->pdev, offset, val);
}

static int pcie_test_write_config_word(struct pcie_test_dev *dev, int offset, u16 val)
{
    return pci_write_config_word(dev->pdev, offset, val);
}

static int pcie_test_read_config_dword(struct pcie_test_dev *dev, int offset, u32 *val)
{
    return pci_read_config_dword(dev->pdev, offset, val);
}

static int pcie_test_write_config_dword(struct pcie_test_dev *dev, int offset, u32 val)
{
    return pci_write_config_dword(dev->pdev, offset, val);
}

/* ============================================================================
 * BAR Read/Write Operations
 * ============================================================================
 */

static u32 pcie_test_bar_read32(struct pcie_test_dev *dev, int bar_num, unsigned long offset)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return 0xFFFFFFFF;
    
    if (offset >= dev->bar_len[bar_num])
        return 0xFFFFFFFF;
    
    return readl(dev->bar_virt[bar_num] + offset);
}

static void pcie_test_bar_write32(struct pcie_test_dev *dev, int bar_num, 
                                  unsigned long offset, u32 value)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return;
    
    if (offset >= dev->bar_len[bar_num])
        return;
    
    writel(value, dev->bar_virt[bar_num] + offset);
}

static u32 pcie_test_bar_read16(struct pcie_test_dev *dev, int bar_num, unsigned long offset)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return 0xFFFF;
    
    if (offset >= dev->bar_len[bar_num])
        return 0xFFFF;
    
    return readw(dev->bar_virt[bar_num] + offset);
}

static void pcie_test_bar_write16(struct pcie_test_dev *dev, int bar_num,
                                  unsigned long offset, u16 value)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return;
    
    if (offset >= dev->bar_len[bar_num])
        return;
    
    writew(value, dev->bar_virt[bar_num] + offset);
}

static u32 pcie_test_bar_read8(struct pcie_test_dev *dev, int bar_num, unsigned long offset)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return 0xFF;
    
    if (offset >= dev->bar_len[bar_num])
        return 0xFF;
    
    return readb(dev->bar_virt[bar_num] + offset);
}

static void pcie_test_bar_write8(struct pcie_test_dev *dev, int bar_num,
                                 unsigned long offset, u8 value)
{
    if (bar_num >= 6 || !dev->bar_virt[bar_num])
        return;
    
    if (offset >= dev->bar_len[bar_num])
        return;
    
    writeb(value, dev->bar_virt[bar_num] + offset);
}

/* ============================================================================
 * Interrupt Handler
 * ============================================================================
 */

static irqreturn_t pcie_test_interrupt_handler(int irq, void *dev_id)
{
    struct pcie_test_dev *dev = dev_id;
    
    if (!dev)
        return IRQ_NONE;
    
    spin_lock(&dev->lock);
    dev->irq_count++;
    spin_unlock(&dev->lock);
    
    DBG(2, "Interrupt received (count: %llu)\n", dev->irq_count);
    
    return IRQ_HANDLED;
}

/* ============================================================================
 * DMA Operations
 * ============================================================================
 */

static int pcie_test_setup_dma(struct pcie_test_dev *dev)
{
    dev->dma_buffer = dma_alloc_coherent(&dev->pdev->dev, DMA_BUFFER_SIZE,
                                         &dev->dma_handle, GFP_KERNEL);
    
    if (!dev->dma_buffer) {
        ERR("Failed to allocate DMA buffer\n");
        return -ENOMEM;
    }
    
    INFO("Allocated DMA buffer: virt=0x%p, phys=0x%llx, size=%d\n",
        dev->dma_buffer, (unsigned long long)dev->dma_handle, DMA_BUFFER_SIZE);
    
    return 0;
}

static void pcie_test_cleanup_dma(struct pcie_test_dev *dev)
{
    if (!dev->dma_buffer)
        return;
    
    dma_free_coherent(&dev->pdev->dev, DMA_BUFFER_SIZE,
                     dev->dma_buffer, dev->dma_handle);
    dev->dma_buffer = NULL;
    
    DBG(1, "Freed DMA buffer\n");
}

/* ============================================================================
 * Sysfs Attributes
 * ============================================================================
 */

static ssize_t device_info_show(struct device *d, struct device_attribute *attr, char *buf)
{
    struct pcie_test_dev *dev = dev_get_drvdata(d);
    int len = 0;
    int i;
    
    if (!dev)
        return -ENODEV;
    
    len += sprintf(buf + len, "PCIe Test Device Information\n");
    len += sprintf(buf + len, "============================\n");
    len += sprintf(buf + len, "Vendor ID:    0x%04x\n", dev->pdev->vendor);
    len += sprintf(buf + len, "Device ID:    0x%04x\n", dev->pdev->device);
    len += sprintf(buf + len, "Bus:Device:Function: %04x:%02x:%02x.%d\n",
                  pci_domain_nr(dev->pdev->bus),
                  dev->pdev->bus->number,
                  PCI_SLOT(dev->pdev->devfn),
                  PCI_FUNC(dev->pdev->devfn));
    len += sprintf(buf + len, "\nBAR Information:\n");
    
    for (i = 0; i < 6; i++) {
        if (dev->bar_len[i] > 0) {
            len += sprintf(buf + len, "  BAR%d: 0x%lx - 0x%lx (Size: 0x%lx, Flags: 0x%x)\n",
                          i, dev->bar_start[i], dev->bar_start[i] + dev->bar_len[i],
                          dev->bar_len[i], dev->bar_flags[i]);
        }
    }
    
    len += sprintf(buf + len, "\nStatistics:\n");
    len += sprintf(buf + len, "  Read Count:  %llu\n", dev->read_count);
    len += sprintf(buf + len, "  Write Count: %llu\n", dev->write_count);
    len += sprintf(buf + len, "  IRQ Count:   %llu\n", dev->irq_count);
    
    return len;
}

static DEVICE_ATTR_RO(device_info);

static ssize_t bar_read_show(struct device *d, struct device_attribute *attr, char *buf)
{
    struct pcie_test_dev *dev = dev_get_drvdata(d);
    
    if (!dev)
        return -ENODEV;
    
    return sprintf(buf, "BAR read count: %llu\n", dev->read_count);
}

static DEVICE_ATTR_RO(bar_read);

static ssize_t bar_write_show(struct device *d, struct device_attribute *attr, char *buf)
{
    struct pcie_test_dev *dev = dev_get_drvdata(d);
    
    if (!dev)
        return -ENODEV;
    
    return sprintf(buf, "BAR write count: %llu\n", dev->write_count);
}

static DEVICE_ATTR_RO(bar_write);

static ssize_t reset_stats_store(struct device *d, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    struct pcie_test_dev *dev = dev_get_drvdata(d);
    
    if (!dev)
        return -ENODEV;
    
    spin_lock(&dev->lock);
    dev->read_count = 0;
    dev->write_count = 0;
    dev->irq_count = 0;
    spin_unlock(&dev->lock);
    
    INFO("Statistics reset\n");
    return count;
}

static DEVICE_ATTR_WO(reset_stats);

static struct attribute *pcie_test_attrs[] = {
    &dev_attr_device_info.attr,
    &dev_attr_bar_read.attr,
    &dev_attr_bar_write.attr,
    &dev_attr_reset_stats.attr,
    NULL
};

ATTRIBUTE_GROUPS(pcie_test);

/* ============================================================================
 * Device Probe/Remove
 * ============================================================================
 */

static int pcie_test_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct pcie_test_dev *dev;
    int ret = 0;
    
    INFO("Probing PCIe device %04x:%04x\n", pdev->vendor, pdev->device);
    
    if (device_count >= MAX_PCIE_DEVICES) {
        ERR("Maximum number of devices reached\n");
        return -ENODEV;
    }
    
    /* Allocate device structure */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;
    
    dev->pdev = pdev;
    dev->device_id = device_count;
    spin_lock_init(&dev->lock);
    
    /* Enable PCI device */
    ret = pci_enable_device(pdev);
    if (ret) {
        ERR("Failed to enable PCI device\n");
        goto free_dev;
    }
    
    /* Request IRQ */
    ret = request_irq(pdev->irq, pcie_test_interrupt_handler,
                     IRQF_SHARED, PCIE_TEST_DRIVER_NAME, dev);
    if (ret) {
        WARN("Failed to request IRQ %d (interrupts may not work)\n", pdev->irq);
        /* Don't fail, just warn */
    }
    
    /* Map BARs */
    ret = pcie_test_map_bars(dev);
    if (ret) {
        WARN("Failed to map all BARs\n");
        /* Don't fail, some BARs may not be available */
    }
    
    /* Setup DMA */
    ret = pcie_test_setup_dma(dev);
    if (ret) {
        WARN("Failed to setup DMA\n");
        /* Don't fail, DMA may not be needed */
    }
    
    /* Register device */
    pci_set_drvdata(pdev, dev);
    
    spin_lock(&device_list_lock);
    pcie_devices[device_count] = dev;
    device_count++;
    spin_unlock(&device_list_lock);
    
    INFO("Device registered successfully (ID: %d)\n", dev->device_id);
    
    return 0;

free_dev:
    kfree(dev);
    return ret;
}

static void pcie_test_remove(struct pci_dev *pdev)
{
    struct pcie_test_dev *dev = pci_get_drvdata(pdev);
    
    if (!dev)
        return;
    
    INFO("Removing PCIe device %04x:%04x\n", pdev->vendor, pdev->device);
    
    /* Cleanup DMA */
    pcie_test_cleanup_dma(dev);
    
    /* Unmap BARs */
    pcie_test_unmap_bars(dev);
    
    /* Free IRQ */
    free_irq(pdev->irq, dev);
    
    /* Disable device */
    pci_disable_device(pdev);
    
    /* Remove from device list */
    spin_lock(&device_list_lock);
    if (pcie_devices[dev->device_id] == dev) {
        pcie_devices[dev->device_id] = NULL;
        device_count--;
    }
    spin_unlock(&device_list_lock);
    
    /* Free device structure */
    kfree(dev);
}

/* ============================================================================
 * PCI Device ID Table
 * ============================================================================
 */

static struct pci_device_id pcie_test_device_ids[] = {
    /* Wildcard: match any PCIe device */
    { PCI_DEVICE(PCI_ANY_ID, PCI_ANY_ID) },
    { 0 }
};

MODULE_DEVICE_TABLE(pci, pcie_test_device_ids);

/* ============================================================================
 * PCI Driver Structure
 * ============================================================================
 */

static struct pci_driver pcie_test_driver = {
    .name = PCIE_TEST_DRIVER_NAME,
    .id_table = pcie_test_device_ids,
    .probe = pcie_test_probe,
    .remove = pcie_test_remove,
};

/* ============================================================================
 * Module Init/Exit
 * ============================================================================
 */

static int __init pcie_test_init(void)
{
    int ret;
    
    printk(KERN_INFO "PCIe Test Driver v%s\n", PCIE_TEST_DRIVER_VERSION);
    
    ret = pci_register_driver(&pcie_test_driver);
    if (ret)
        ERR("Failed to register PCI driver\n");
    else
        INFO("Driver registered successfully\n");
    
    return ret;
}

static void __exit pcie_test_exit(void)
{
    pci_unregister_driver(&pcie_test_driver);
    INFO("Driver unregistered\n");
}

module_init(pcie_test_init);
module_exit(pcie_test_exit);

MODULE_AUTHOR("Test Framework");
MODULE_DESCRIPTION("Linux PCIe Test Driver Framework");
MODULE_LICENSE("GPL");
MODULE_VERSION(PCIE_TEST_DRIVER_VERSION);
