/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Intel Bluetooth Test Generic Driver - Header
 *
 * Copyright (C) 2026  Your Company/Name
 */

#ifndef __BTINTEL_TEST_GENERIC_DRIVER_H
#define __BTINTEL_TEST_GENERIC_DRIVER_H

/* ============================================================================
 * INCLUDES
 * ============================================================================ */

#include <linux/types.h>
#include <linux/ioctl.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Device configuration */
#define BTINTEL_TEST_DEFAULT_BUFFER_SIZE	4096
#define BTINTEL_TEST_MAX_BUFFER_SIZE		(16 * 1024 * 1024)  /* 16MB */

/* Version code (major.minor.patch -> 0x010000 = 1.0.0) */
#define BTINTEL_TEST_VERSION_CODE		0x010000

/* Magic number for ioctl - use a unique value (ASCII character) */
#define BTINTEL_TEST_IOC_MAGIC			'B'

/* ============================================================================
 * DATA STRUCTURES FOR IOCTL
 * ============================================================================ */

/**
 * struct btintel_test_dev_info - Device information
 * @version: Driver version code
 * @buffer_size: Size of internal device buffer
 * @active: Device active status
 * @refcount: Number of open file descriptors
 */
struct btintel_test_dev_info {
	u32 version;
	size_t buffer_size;
	u8 active;
	u32 refcount;
};

/**
 * struct btintel_test_stats - Device statistics
 * @read_count: Total number of read operations
 * @write_count: Total number of write operations
 * @ioctl_count: Total number of ioctl operations
 * @errors: Total number of errors
 */
struct btintel_test_stats {
	u64 read_count;
	u64 write_count;
	u64 ioctl_count;
	u64 errors;
};

/**
 * struct btintel_test_buffer_data - Buffer size configuration
 * @size: New buffer size
 * @reserved: Padding for future use
 */
struct btintel_test_buffer_data {
	size_t size;
	u64 reserved;
};

/**
 * struct btintel_test_status - Device status
 * @state: Device state flags
 * @error_code: Last error code
 * @reserved: Padding for future use
 */
struct btintel_test_status {
	u32 state;
	u32 error_code;
	u64 reserved;
};

/* ============================================================================
 * IOCTL COMMAND DEFINITIONS
 * ============================================================================ */

/**
 * BTINTEL_TEST_IOC_GET_INFO - Get device information
 * Type: Read (IOR)
 * Argument: pointer to struct btintel_test_dev_info
 */
#define BTINTEL_TEST_IOC_GET_INFO \
	_IOR(BTINTEL_TEST_IOC_MAGIC, 0, struct btintel_test_dev_info)

/**
 * BTINTEL_TEST_IOC_GET_STATS - Get device statistics
 * Type: Read (IOR)
 * Argument: pointer to struct btintel_test_stats
 */
#define BTINTEL_TEST_IOC_GET_STATS \
	_IOR(BTINTEL_TEST_IOC_MAGIC, 1, struct btintel_test_stats)

/**
 * BTINTEL_TEST_IOC_RESET_STATS - Reset device statistics
 * Type: None (IO)
 * Argument: none
 */
#define BTINTEL_TEST_IOC_RESET_STATS \
	_IO(BTINTEL_TEST_IOC_MAGIC, 2)

/**
 * BTINTEL_TEST_IOC_CLEAR_BUFFER - Clear internal device buffer
 * Type: None (IO)
 * Argument: none
 */
#define BTINTEL_TEST_IOC_CLEAR_BUFFER \
	_IO(BTINTEL_TEST_IOC_MAGIC, 3)

/**
 * BTINTEL_TEST_IOC_SET_BUFFER_SIZE - Set device buffer size
 * Type: Write (IOW)
 * Argument: pointer to struct btintel_test_buffer_data
 */
#define BTINTEL_TEST_IOC_SET_BUFFER_SIZE \
	_IOW(BTINTEL_TEST_IOC_MAGIC, 4, struct btintel_test_buffer_data)

/**
 * BTINTEL_TEST_IOC_GET_STATUS - Get device status
 * Type: Read (IOR)
 * Argument: pointer to struct btintel_test_status
 */
#define BTINTEL_TEST_IOC_GET_STATUS \
	_IOR(BTINTEL_TEST_IOC_MAGIC, 5, struct btintel_test_status)

/**
 * BTINTEL_TEST_IOC_ENABLE - Enable device
 * Type: None (IO)
 * Argument: none
 */
#define BTINTEL_TEST_IOC_ENABLE \
	_IO(BTINTEL_TEST_IOC_MAGIC, 6)

/**
 * BTINTEL_TEST_IOC_DISABLE - Disable device
 * Type: None (IO)
 * Argument: none
 */
#define BTINTEL_TEST_IOC_DISABLE \
	_IO(BTINTEL_TEST_IOC_MAGIC, 7)

/* ============================================================================
 * REGISTER DEFINITIONS (if applicable)
 * ============================================================================ */

/* Example register definitions - customize for your hardware */

#define BTINTEL_TEST_CSR_BASE			0x000
#define BTINTEL_TEST_STATUS_REG			(BTINTEL_TEST_CSR_BASE + 0x00)
#define BTINTEL_TEST_CONTROL_REG		(BTINTEL_TEST_CSR_BASE + 0x04)
#define BTINTEL_TEST_VERSION_REG		(BTINTEL_TEST_CSR_BASE + 0x08)

/* Status register bit fields */
#define BTINTEL_TEST_STATUS_READY		BIT(0)
#define BTINTEL_TEST_STATUS_ERROR		BIT(1)
#define BTINTEL_TEST_STATUS_BUSY		BIT(2)

/* Control register bit fields */
#define BTINTEL_TEST_CTRL_ENABLE		BIT(0)
#define BTINTEL_TEST_CTRL_RESET			BIT(1)

/* ============================================================================
 * MACROS FOR REGISTER ACCESS (if using memory-mapped I/O)
 * ============================================================================ */

/**
 * Read 32-bit register
 */
#define BTINTEL_TEST_READ_REG(base, reg) \
	readl((base) + (reg))

/**
 * Write 32-bit register
 */
#define BTINTEL_TEST_WRITE_REG(base, reg, value) \
	writel((value), (base) + (reg))

/**
 * Set specific bits in register
 */
#define BTINTEL_TEST_SET_BITS(base, reg, bits) \
	BTINTEL_TEST_WRITE_REG((base), (reg), \
		BTINTEL_TEST_READ_REG((base), (reg)) | (bits))

/**
 * Clear specific bits in register
 */
#define BTINTEL_TEST_CLEAR_BITS(base, reg, bits) \
	BTINTEL_TEST_WRITE_REG((base), (reg), \
		BTINTEL_TEST_READ_REG((base), (reg)) & ~(bits))

#endif /* __BTINTEL_TEST_GENERIC_DRIVER_H */
