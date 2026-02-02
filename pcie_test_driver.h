/*
 * PCIe Test Driver - Header File
 */

#ifndef _PCIE_TEST_DRIVER_H_
#define _PCIE_TEST_DRIVER_H_

#include <linux/types.h>

/* IOCTL Command Codes */
#define PCIE_TEST_IOC_MAGIC 0xE0

/* BAR Access IOCTLs */
#define PCIE_IOC_BAR_READ32  _IOWR(PCIE_TEST_IOC_MAGIC, 0x01, struct pcie_bar_access)
#define PCIE_IOC_BAR_WRITE32 _IOW(PCIE_TEST_IOC_MAGIC, 0x02, struct pcie_bar_access)
#define PCIE_IOC_BAR_READ16  _IOWR(PCIE_TEST_IOC_MAGIC, 0x03, struct pcie_bar_access)
#define PCIE_IOC_BAR_WRITE16 _IOW(PCIE_TEST_IOC_MAGIC, 0x04, struct pcie_bar_access)
#define PCIE_IOC_BAR_READ8   _IOWR(PCIE_TEST_IOC_MAGIC, 0x05, struct pcie_bar_access)
#define PCIE_IOC_BAR_WRITE8  _IOW(PCIE_TEST_IOC_MAGIC, 0x06, struct pcie_bar_access)

/* Configuration Space IOCTLs */
#define PCIE_IOC_CONFIG_READ8   _IOWR(PCIE_TEST_IOC_MAGIC, 0x10, struct pcie_config_access)
#define PCIE_IOC_CONFIG_WRITE8  _IOW(PCIE_TEST_IOC_MAGIC, 0x11, struct pcie_config_access)
#define PCIE_IOC_CONFIG_READ16  _IOWR(PCIE_TEST_IOC_MAGIC, 0x12, struct pcie_config_access)
#define PCIE_IOC_CONFIG_WRITE16 _IOW(PCIE_TEST_IOC_MAGIC, 0x13, struct pcie_config_access)
#define PCIE_IOC_CONFIG_READ32  _IOWR(PCIE_TEST_IOC_MAGIC, 0x14, struct pcie_config_access)
#define PCIE_IOC_CONFIG_WRITE32 _IOW(PCIE_TEST_IOC_MAGIC, 0x15, struct pcie_config_access)

/* Device Info IOCTL */
#define PCIE_IOC_GET_DEVICE_INFO _IOR(PCIE_TEST_IOC_MAGIC, 0x20, struct pcie_device_info)
#define PCIE_IOC_GET_STATISTICS  _IOR(PCIE_TEST_IOC_MAGIC, 0x21, struct pcie_statistics)

/* BAR Access Structure */
struct pcie_bar_access {
    __u8 bar_num;           /* BAR number (0-5) */
    __u32 offset;           /* Offset within BAR */
    __u32 value;            /* Data value */
};

/* Configuration Space Access Structure */
struct pcie_config_access {
    __u16 offset;           /* Configuration space offset */
    __u32 value;            /* Data value */
};

/* Device Information Structure */
struct pcie_device_info {
    __u16 vendor_id;
    __u16 device_id;
    __u8 bus_num;
    __u8 slot_num;
    __u8 func_num;
    __u8 domain_num;
    __u8 bar_count;
    __u8 irq;
};

/* BAR Information Structure */
struct pcie_bar_info {
    __u32 start;
    __u32 size;
    __u32 flags;
    __u8 present;
};

/* Statistics Structure */
struct pcie_statistics {
    __u64 read_count;
    __u64 write_count;
    __u64 irq_count;
};

#endif /* _PCIE_TEST_DRIVER_H_ */
