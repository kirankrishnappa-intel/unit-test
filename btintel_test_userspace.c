/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Intel Bluetooth Test Generic Driver - Userspace Test Application
 *
 * Copyright (C) 2026  Your Company/Name
 *
 * This program demonstrates how to interact with the btintel_test_generic_driver
 * using ioctl commands.
 *
 * Build: gcc -o btintel_test_userspace btintel_test_userspace.c
 * Usage: ./btintel_test_userspace
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "btintel_test_userspace.h"

#define DEVICE_PATH "/dev/btintel_test_generic_driver"

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

/**
 * print_error - Print error message with errno
 */
static void print_error(const char *msg)
{
	fprintf(stderr, "ERROR: %s: %s\n", msg, strerror(errno));
}

/**
 * print_info - Print informational message
 */
static void print_info(const char *msg)
{
	printf("[INFO] %s\n", msg);
}

/**
 * print_success - Print success message
 */
static void print_success(const char *msg)
{
	printf("[OK] %s\n", msg);
}

/**
 * open_device - Open the device file
 */
static int open_device(void)
{
	int fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		print_error("Failed to open device");
		return -1;
	}
	printf("Opened device: %s (fd=%d)\n", DEVICE_PATH, fd);
	return fd;
}

/**
 * close_device - Close the device file
 */
static void close_device(int fd)
{
	if (fd >= 0) {
		close(fd);
		print_info("Device closed");
	}
}

/* ============================================================================
 * IOCTL COMMAND TESTS
 * ============================================================================ */

/**
 * test_get_info - Test GET_INFO ioctl
 */
static int test_get_info(int fd)
{
	struct btintel_test_dev_info info;
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_GET_INFO...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_GET_INFO, &info);
	if (ret < 0) {
		print_error("GET_INFO ioctl failed");
		return -1;
	}

	printf("  Device Info:\n");
	printf("    Version:     0x%08x\n", info.version);
	printf("    Buffer Size: %zu bytes\n", info.buffer_size);
	printf("    Active:      %s\n", info.active ? "Yes" : "No");
	printf("    Refcount:    %u\n", info.refcount);

	print_success("GET_INFO completed");
	return 0;
}

/**
 * test_get_stats - Test GET_STATS ioctl
 */
static int test_get_stats(int fd)
{
	struct btintel_test_stats stats;
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_GET_STATS...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_GET_STATS, &stats);
	if (ret < 0) {
		print_error("GET_STATS ioctl failed");
		return -1;
	}

	printf("  Device Statistics:\n");
	printf("    Read Count:  %llu\n", (unsigned long long)stats.read_count);
	printf("    Write Count: %llu\n", (unsigned long long)stats.write_count);
	printf("    Ioctl Count: %llu\n", (unsigned long long)stats.ioctl_count);
	printf("    Errors:      %llu\n", (unsigned long long)stats.errors);

	print_success("GET_STATS completed");
	return 0;
}

/**
 * test_reset_stats - Test RESET_STATS ioctl
 */
static int test_reset_stats(int fd)
{
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_RESET_STATS...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_RESET_STATS);
	if (ret < 0) {
		print_error("RESET_STATS ioctl failed");
		return -1;
	}

	print_success("RESET_STATS completed");
	return 0;
}

/**
 * test_clear_buffer - Test CLEAR_BUFFER ioctl
 */
static int test_clear_buffer(int fd)
{
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_CLEAR_BUFFER...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_CLEAR_BUFFER);
	if (ret < 0) {
		print_error("CLEAR_BUFFER ioctl failed");
		return -1;
	}

	print_success("CLEAR_BUFFER completed");
	return 0;
}

/**
 * test_set_buffer_size - Test SET_BUFFER_SIZE ioctl
 */
static int test_set_buffer_size(int fd, size_t new_size)
{
	struct btintel_test_buffer_data buf_data;
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_SET_BUFFER_SIZE...");
	printf("  Requesting buffer size: %zu bytes\n", new_size);

	buf_data.size = new_size;
	buf_data.reserved = 0;

	ret = ioctl(fd, BTINTEL_TEST_IOC_SET_BUFFER_SIZE, &buf_data);
	if (ret < 0) {
		print_error("SET_BUFFER_SIZE ioctl failed");
		return -1;
	}

	print_success("SET_BUFFER_SIZE completed");
	return 0;
}

/**
 * test_get_status - Test GET_STATUS ioctl
 */
static int test_get_status(int fd)
{
	struct btintel_test_status status;
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_GET_STATUS...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_GET_STATUS, &status);
	if (ret < 0) {
		print_error("GET_STATUS ioctl failed");
		return -1;
	}

	printf("  Device Status:\n");
	printf("    State:      0x%08x\n", status.state);
	printf("    Error Code: 0x%08x\n", status.error_code);

	print_success("GET_STATUS completed");
	return 0;
}

/**
 * test_enable - Test ENABLE ioctl
 */
static int test_enable(int fd)
{
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_ENABLE...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_ENABLE);
	if (ret < 0) {
		print_error("ENABLE ioctl failed");
		return -1;
	}

	print_success("ENABLE completed");
	return 0;
}

/**
 * test_disable - Test DISABLE ioctl
 */
static int test_disable(int fd)
{
	int ret;

	print_info("Testing BTINTEL_TEST_IOC_DISABLE...");

	ret = ioctl(fd, BTINTEL_TEST_IOC_DISABLE);
	if (ret < 0) {
		print_error("DISABLE ioctl failed");
		return -1;
	}

	print_success("DISABLE completed");
	return 0;
}

/**
 * test_read_write - Test read/write operations
 */
static int test_read_write(int fd)
{
	char write_buffer[256];
	char read_buffer[256];
	ssize_t ret;

	print_info("Testing read/write operations...");

	/* Prepare write buffer */
	strcpy(write_buffer, "Hello from userspace!");

	/* Write to device */
	print_info("Writing to device...");
	ret = write(fd, write_buffer, strlen(write_buffer));
	if (ret < 0) {
		print_error("Write failed");
		return -1;
	}
	printf("  Wrote %zd bytes\n", ret);

	/* Read from device */
	print_info("Reading from device...");
	ret = read(fd, read_buffer, sizeof(read_buffer) - 1);
	if (ret < 0) {
		print_error("Read failed");
		return -1;
	}
	read_buffer[ret] = '\0';
	printf("  Read %zd bytes: %s\n", ret, read_buffer);

	print_success("read/write completed");
	return 0;
}

/* ============================================================================
 * MAIN PROGRAM
 * ============================================================================ */

int main(int argc, char *argv[])
{
	int fd;
	int ret = 0;

	printf("========================================\n");
	printf("Intel Bluetooth Test Driver - Userspace Test\n");
	printf("========================================\n\n");

	/* Open device */
	fd = open_device();
	if (fd < 0) {
		return EXIT_FAILURE;
	}

	printf("\n--- Basic Device Operations ---\n");

	/* Test get info */
	if (test_get_info(fd) < 0)
		ret = -1;

	printf("\n--- Read/Write Operations ---\n");

	/* Test read/write */
	if (test_read_write(fd) < 0)
		ret = -1;

	printf("\n--- Statistics Operations ---\n");

	/* Reset stats first */
	if (test_reset_stats(fd) < 0)
		ret = -1;

	/* Get stats */
	if (test_get_stats(fd) < 0)
		ret = -1;

	printf("\n--- Buffer Operations ---\n");

	/* Set buffer size */
	if (test_set_buffer_size(fd, 8192) < 0)
		ret = -1;

	/* Clear buffer */
	if (test_clear_buffer(fd) < 0)
		ret = -1;

	printf("\n--- Device Status Operations ---\n");

	/* Get status */
	if (test_get_status(fd) < 0)
		ret = -1;

	printf("\n--- Enable/Disable Operations ---\n");

	/* Disable device */
	if (test_disable(fd) < 0)
		ret = -1;

	/* Get info after disable */
	if (test_get_info(fd) < 0)
		ret = -1;

	/* Enable device */
	if (test_enable(fd) < 0)
		ret = -1;

	/* Get info after enable */
	if (test_get_info(fd) < 0)
		ret = -1;

	printf("\n");

	/* Close device */
	close_device(fd);

	/* Final status */
	if (ret == 0) {
		printf("========================================\n");
		printf("All tests completed successfully!\n");
		printf("========================================\n");
		return EXIT_SUCCESS;
	} else {
		printf("========================================\n");
		printf("Some tests failed!\n");
		printf("========================================\n");
		return EXIT_FAILURE;
	}
}
