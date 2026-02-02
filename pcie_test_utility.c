/*
 * PCIe Test Driver - User Space Test Utility
 *
 * This utility provides a command-line interface to test the PCIe driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <errno.h>
#include <stdint.h>

#include "pcie_test_driver.h"

#define DEVICE_PATH "/dev/pcie_test_0"
#define MAX_DEVICES 16

static int verbose = 0;
static int device_fd = -1;

void print_usage(const char *prog)
{
    printf("Usage: %s [options] <command> [args]\n", prog);
    printf("\nOptions:\n");
    printf("  -d <dev>    Device to use (default: %s)\n", DEVICE_PATH);
    printf("  -v          Verbose output\n");
    printf("  -h          Show this help\n");
    printf("\nCommands:\n");
    printf("  info                          Show device information\n");
    printf("  stats                         Show device statistics\n");
    printf("  bar-read <bar> <offset> <sz>  Read from BAR (sz: 8, 16, or 32 bits)\n");
    printf("  bar-write <bar> <offset> <val> <sz>  Write to BAR (sz: 8, 16, or 32 bits)\n");
    printf("  cfg-read <offset> <sz>       Read config space (sz: 8, 16, or 32 bits)\n");
    printf("  cfg-write <offset> <val> <sz> Write config space (sz: 8, 16, or 32 bits)\n");
    printf("  list                          List all available PCIe devices\n");
}

int open_device(const char *dev_path)
{
    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open device %s: %s\n",
               dev_path, strerror(errno));
        return -1;
    }
    if (verbose)
        printf("Opened device: %s (fd=%d)\n", dev_path, fd);
    return fd;
}

void close_device(int fd)
{
    if (fd >= 0) {
        close(fd);
        if (verbose)
            printf("Closed device\n");
    }
}

void show_device_info(int fd)
{
    struct pcie_device_info info;
    struct pcie_statistics stats;
    int ret;
    
    ret = ioctl(fd, PCIE_IOC_GET_DEVICE_INFO, &info);
    if (ret < 0) {
        fprintf(stderr, "Error: Failed to get device info: %s\n", strerror(errno));
        return;
    }
    
    printf("PCIe Device Information\n");
    printf("=======================\n");
    printf("Vendor ID:           0x%04x\n", info.vendor_id);
    printf("Device ID:           0x%04x\n", info.device_id);
    printf("Domain:Bus:Slot.Func: %04x:%02x:%02x.%d\n",
           info.domain_num, info.bus_num, info.slot_num, info.func_num);
    printf("IRQ:                 %d\n", info.irq);
    printf("BARs Present:        %d\n", info.bar_count);
    
    ret = ioctl(fd, PCIE_IOC_GET_STATISTICS, &stats);
    if (ret == 0) {
        printf("\nStatistics:\n");
        printf("  Reads:  %llu\n", (unsigned long long)stats.read_count);
        printf("  Writes: %llu\n", (unsigned long long)stats.write_count);
        printf("  IRQs:   %llu\n", (unsigned long long)stats.irq_count);
    }
}

void bar_read(int fd, int bar, uint32_t offset, int size)
{
    struct pcie_bar_access access;
    int ret;
    
    access.bar_num = bar;
    access.offset = offset;
    access.value = 0;
    
    switch (size) {
    case 8:
        ret = ioctl(fd, PCIE_IOC_BAR_READ8, &access);
        break;
    case 16:
        ret = ioctl(fd, PCIE_IOC_BAR_READ16, &access);
        break;
    case 32:
        ret = ioctl(fd, PCIE_IOC_BAR_READ32, &access);
        break;
    default:
        fprintf(stderr, "Error: Invalid size %d (use 8, 16, or 32)\n", size);
        return;
    }
    
    if (ret < 0) {
        fprintf(stderr, "Error: BAR read failed: %s\n", strerror(errno));
        return;
    }
    
    printf("BAR%d[0x%x]: 0x%x\n", bar, offset, access.value);
}

void bar_write(int fd, int bar, uint32_t offset, uint32_t value, int size)
{
    struct pcie_bar_access access;
    int ret;
    
    access.bar_num = bar;
    access.offset = offset;
    access.value = value;
    
    switch (size) {
    case 8:
        ret = ioctl(fd, PCIE_IOC_BAR_WRITE8, &access);
        break;
    case 16:
        ret = ioctl(fd, PCIE_IOC_BAR_WRITE16, &access);
        break;
    case 32:
        ret = ioctl(fd, PCIE_IOC_BAR_WRITE32, &access);
        break;
    default:
        fprintf(stderr, "Error: Invalid size %d (use 8, 16, or 32)\n", size);
        return;
    }
    
    if (ret < 0) {
        fprintf(stderr, "Error: BAR write failed: %s\n", strerror(errno));
        return;
    }
    
    printf("Wrote 0x%x to BAR%d[0x%x] (%d-bit)\n", value, bar, offset, size);
}

void cfg_read(int fd, uint16_t offset, int size)
{
    struct pcie_config_access access;
    int ret;
    
    access.offset = offset;
    access.value = 0;
    
    switch (size) {
    case 8:
        ret = ioctl(fd, PCIE_IOC_CONFIG_READ8, &access);
        break;
    case 16:
        ret = ioctl(fd, PCIE_IOC_CONFIG_READ16, &access);
        break;
    case 32:
        ret = ioctl(fd, PCIE_IOC_CONFIG_READ32, &access);
        break;
    default:
        fprintf(stderr, "Error: Invalid size %d (use 8, 16, or 32)\n", size);
        return;
    }
    
    if (ret < 0) {
        fprintf(stderr, "Error: Config read failed: %s\n", strerror(errno));
        return;
    }
    
    printf("Config[0x%x]: 0x%x\n", offset, access.value);
}

void cfg_write(int fd, uint16_t offset, uint32_t value, int size)
{
    struct pcie_config_access access;
    int ret;
    
    access.offset = offset;
    access.value = value;
    
    switch (size) {
    case 8:
        ret = ioctl(fd, PCIE_IOC_CONFIG_WRITE8, &access);
        break;
    case 16:
        ret = ioctl(fd, PCIE_IOC_CONFIG_WRITE16, &access);
        break;
    case 32:
        ret = ioctl(fd, PCIE_IOC_CONFIG_WRITE32, &access);
        break;
    default:
        fprintf(stderr, "Error: Invalid size %d (use 8, 16, or 32)\n", size);
        return;
    }
    
    if (ret < 0) {
        fprintf(stderr, "Error: Config write failed: %s\n", strerror(errno));
        return;
    }
    
    printf("Wrote 0x%x to Config[0x%x] (%d-bit)\n", value, offset, size);
}

void list_devices(void)
{
    int i;
    char dev_path[256];
    FILE *fp;
    
    printf("Available PCIe Test Devices:\n");
    printf("============================\n");
    
    for (i = 0; i < MAX_DEVICES; i++) {
        snprintf(dev_path, sizeof(dev_path), "/dev/pcie_test_%d", i);
        if (access(dev_path, F_OK) == 0) {
            printf("  %s\n", dev_path);
        }
    }
    
    /* Also check sysfs for loaded devices */
    fp = popen("lsmod | grep pcie_test_driver", "r");
    if (fp) {
        char line[256];
        if (fgets(line, sizeof(line), fp)) {
            printf("\nModule Status: Loaded\n");
        }
        pclose(fp);
    }
}

int main(int argc, char **argv)
{
    const char *device = DEVICE_PATH;
    int opt;
    
    while ((opt = getopt(argc, argv, "d:vh")) != -1) {
        switch (opt) {
        case 'd':
            device = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            print_usage(argv[0]);
            exit(0);
        default:
            print_usage(argv[0]);
            exit(1);
        }
    }
    
    if (optind >= argc) {
        print_usage(argv[0]);
        exit(1);
    }
    
    const char *cmd = argv[optind];
    
    /* Handle list command without opening device */
    if (strcmp(cmd, "list") == 0) {
        list_devices();
        exit(0);
    }
    
    /* Open device for other commands */
    device_fd = open_device(device);
    if (device_fd < 0)
        exit(1);
    
    if (strcmp(cmd, "info") == 0) {
        show_device_info(device_fd);
    }
    else if (strcmp(cmd, "stats") == 0) {
        struct pcie_statistics stats;
        if (ioctl(device_fd, PCIE_IOC_GET_STATISTICS, &stats) == 0) {
            printf("Statistics:\n");
            printf("  Reads:  %llu\n", (unsigned long long)stats.read_count);
            printf("  Writes: %llu\n", (unsigned long long)stats.write_count);
            printf("  IRQs:   %llu\n", (unsigned long long)stats.irq_count);
        } else {
            fprintf(stderr, "Error: Failed to get statistics\n");
        }
    }
    else if (strcmp(cmd, "bar-read") == 0) {
        if (optind + 3 > argc) {
            fprintf(stderr, "Error: bar-read requires BAR, offset, and size\n");
            exit(1);
        }
        int bar = atoi(argv[optind + 1]);
        uint32_t offset = strtoul(argv[optind + 2], NULL, 0);
        int size = atoi(argv[optind + 3]);
        bar_read(device_fd, bar, offset, size);
    }
    else if (strcmp(cmd, "bar-write") == 0) {
        if (optind + 4 > argc) {
            fprintf(stderr, "Error: bar-write requires BAR, offset, value, and size\n");
            exit(1);
        }
        int bar = atoi(argv[optind + 1]);
        uint32_t offset = strtoul(argv[optind + 2], NULL, 0);
        uint32_t value = strtoul(argv[optind + 3], NULL, 0);
        int size = atoi(argv[optind + 4]);
        bar_write(device_fd, bar, offset, value, size);
    }
    else if (strcmp(cmd, "cfg-read") == 0) {
        if (optind + 2 > argc) {
            fprintf(stderr, "Error: cfg-read requires offset and size\n");
            exit(1);
        }
        uint16_t offset = strtoul(argv[optind + 1], NULL, 0);
        int size = atoi(argv[optind + 2]);
        cfg_read(device_fd, offset, size);
    }
    else if (strcmp(cmd, "cfg-write") == 0) {
        if (optind + 3 > argc) {
            fprintf(stderr, "Error: cfg-write requires offset, value, and size\n");
            exit(1);
        }
        uint16_t offset = strtoul(argv[optind + 1], NULL, 0);
        uint32_t value = strtoul(argv[optind + 2], NULL, 0);
        int size = atoi(argv[optind + 3]);
        cfg_write(device_fd, offset, value, size);
    }
    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", cmd);
        print_usage(argv[0]);
        close_device(device_fd);
        exit(1);
    }
    
    close_device(device_fd);
    exit(0);
}
