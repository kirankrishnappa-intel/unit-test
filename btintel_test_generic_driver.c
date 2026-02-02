/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Intel Bluetooth Test Generic Driver with IOCTL Support
 *
 * Copyright (C) 2026  Your Company/Name
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================ */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#include "btintel_test_generic_driver.h"

/* ============================================================================
 * MODULE METADATA
 * ============================================================================ */

#define DRIVER_NAME			"btintel_test_generic_driver"
#define DRIVER_DESC			"Intel Bluetooth Test Generic Driver with IOCTL Support"
#define DRIVER_AUTHOR			"Your Name <your.email@example.com>"
#define DRIVER_VERSION			"1.0.0"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION(DRIVER_DESC);

/* ============================================================================
 * CONSTANTS & MACROS
 * ============================================================================ */

/* Number of devices supported */
#define DEVICE_COUNT			1

/* Debug logging macro */
#ifdef DEBUG
#define pr_debug_dev(fmt, ...) \
	pr_debug("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define pr_debug_dev(fmt, ...) ((void)0)
#endif

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/**
 * struct btintel_test_device - Main device structure
 * @cdev: Character device structure
 * @dev: Device pointer
 * @devt: Device number (major + minor)
 * @lock: Mutex for device state protection
 * @refcount: Open file descriptor reference count
 * @active: Device state (active/inactive)
 * @buffer: Internal device buffer (example)
 * @buffer_size: Size of internal buffer
 * @stats: Device statistics
 */
struct btintel_test_device {
	struct cdev cdev;
	struct device *dev;
	dev_t devt;
	struct mutex lock;
	int refcount;
	bool active;
	void *buffer;
	size_t buffer_size;
	struct {
		unsigned long read_count;
		unsigned long write_count;
		unsigned long ioctl_count;
		unsigned long errors;
	} stats;
};

/* ============================================================================
 * GLOBAL VARIABLES
 * ============================================================================ */

static struct class *btintel_test_class;
static dev_t btintel_test_devt;
static struct btintel_test_device *btintel_test_dev;

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

static int btintel_test_open(struct inode *inode, struct file *filp);
static int btintel_test_release(struct inode *inode, struct file *filp);
static ssize_t btintel_test_read(struct file *filp, char __user *buf,
				  size_t count, loff_t *f_pos);
static ssize_t btintel_test_write(struct file *filp, const char __user *buf,
				   size_t count, loff_t *f_pos);
static long btintel_test_ioctl(struct file *filp, unsigned int cmd,
			       unsigned long arg);

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */

static const struct file_operations btintel_test_fops = {
	.owner = THIS_MODULE,
	.open = btintel_test_open,
	.release = btintel_test_release,
	.read = btintel_test_read,
	.write = btintel_test_write,
	.unlocked_ioctl = btintel_test_ioctl,
};

/* ============================================================================
 * FILE OPERATION IMPLEMENTATIONS
 * ============================================================================ */

/**
 * btintel_test_open - Called when device file is opened
 * @inode: Inode structure
 * @filp: File structure
 *
 * Return: 0 on success, negative error code on failure
 */
static int btintel_test_open(struct inode *inode, struct file *filp)
{
	struct btintel_test_device *dev = container_of(inode->i_cdev,
						   struct btintel_test_device,
						   cdev);

	pr_debug_dev("Device opened\n");

	mutex_lock(&dev->lock);

	if (!dev->active) {
		pr_warn("Device not active\n");
		mutex_unlock(&dev->lock);
		return -ENODEV;
	}

	dev->refcount++;
	filp->private_data = dev;

	mutex_unlock(&dev->lock);

	return 0;
}

/**
 * btintel_test_release - Called when device file is closed
 * @inode: Inode structure
 * @filp: File structure
 *
 * Return: 0 on success
 */
static int btintel_test_release(struct inode *inode, struct file *filp)
{
	struct btintel_test_device *dev = filp->private_data;

	pr_debug_dev("Device released\n");

	mutex_lock(&dev->lock);
	if (dev->refcount > 0)
		dev->refcount--;
	mutex_unlock(&dev->lock);

	return 0;
}

/**
 * btintel_test_read - Called when user reads from device
 * @filp: File structure
 * @buf: User-space buffer
 * @count: Number of bytes to read
 * @f_pos: File position
 *
 * Return: Number of bytes read, or negative error code
 */
static ssize_t btintel_test_read(struct file *filp, char __user *buf,
				  size_t count, loff_t *f_pos)
{
	struct btintel_test_device *dev = filp->private_data;
	ssize_t ret = 0;

	if (!dev || !dev->buffer)
		return -ENODEV;

	mutex_lock(&dev->lock);

	if (*f_pos >= dev->buffer_size) {
		ret = 0;
		goto out;
	}

	count = min(count, dev->buffer_size - (size_t)*f_pos);

	if (copy_to_user(buf, dev->buffer + *f_pos, count)) {
		ret = -EFAULT;
		dev->stats.errors++;
		goto out;
	}

	*f_pos += count;
	ret = count;
	dev->stats.read_count++;

	pr_debug_dev("Read %zd bytes\n", count);

out:
	mutex_unlock(&dev->lock);
	return ret;
}

/**
 * btintel_test_write - Called when user writes to device
 * @filp: File structure
 * @buf: User-space buffer
 * @count: Number of bytes to write
 * @f_pos: File position
 *
 * Return: Number of bytes written, or negative error code
 */
static ssize_t btintel_test_write(struct file *filp, const char __user *buf,
				   size_t count, loff_t *f_pos)
{
	struct btintel_test_device *dev = filp->private_data;
	ssize_t ret = 0;

	if (!dev || !dev->buffer)
		return -ENODEV;

	mutex_lock(&dev->lock);

	if (*f_pos >= dev->buffer_size) {
		ret = -ENOSPC;
		dev->stats.errors++;
		goto out;
	}

	count = min(count, dev->buffer_size - (size_t)*f_pos);

	if (copy_from_user(dev->buffer + *f_pos, buf, count)) {
		ret = -EFAULT;
		dev->stats.errors++;
		goto out;
	}

	*f_pos += count;
	ret = count;
	dev->stats.write_count++;

	pr_debug_dev("Wrote %zd bytes\n", count);

out:
	mutex_unlock(&dev->lock);
	return ret;
}

/**
 * btintel_test_ioctl - Handle IOCTL commands
 * @filp: File structure
 * @cmd: IOCTL command
 * @arg: Argument (user-space pointer or data)
 *
 * Return: 0 on success, negative error code on failure
 */
static long btintel_test_ioctl(struct file *filp, unsigned int cmd,
			       unsigned long arg)
{
	struct btintel_test_device *dev = filp->private_data;
	struct btintel_test_dev_info info;
	struct btintel_test_stats stats;
	struct btintel_test_buffer_data buf_data;
	int ret = 0;

	if (!dev)
		return -ENODEV;

	mutex_lock(&dev->lock);

	switch (cmd) {
	case BTINTEL_TEST_IOC_GET_INFO:
		info.version = BTINTEL_TEST_VERSION_CODE;
		info.buffer_size = dev->buffer_size;
		info.active = dev->active;
		info.refcount = dev->refcount;

		if (copy_to_user((void __user *)arg, &info, sizeof(info))) {
			ret = -EFAULT;
			dev->stats.errors++;
		}
		pr_debug_dev("GET_INFO ioctl\n");
		break;

	case BTINTEL_TEST_IOC_GET_STATS:
		stats.read_count = dev->stats.read_count;
		stats.write_count = dev->stats.write_count;
		stats.ioctl_count = dev->stats.ioctl_count;
		stats.errors = dev->stats.errors;

		if (copy_to_user((void __user *)arg, &stats, sizeof(stats))) {
			ret = -EFAULT;
			dev->stats.errors++;
		}
		pr_debug_dev("GET_STATS ioctl\n");
		break;

	case BTINTEL_TEST_IOC_RESET_STATS:
		dev->stats.read_count = 0;
		dev->stats.write_count = 0;
		dev->stats.ioctl_count = 0;
		dev->stats.errors = 0;
		pr_debug_dev("RESET_STATS ioctl\n");
		break;

	case BTINTEL_TEST_IOC_CLEAR_BUFFER:
		if (dev->buffer)
			memset(dev->buffer, 0, dev->buffer_size);
		pr_debug_dev("CLEAR_BUFFER ioctl\n");
		break;

	case BTINTEL_TEST_IOC_SET_BUFFER_SIZE:
		if (copy_from_user(&buf_data, (void __user *)arg,
				   sizeof(buf_data))) {
			ret = -EFAULT;
			dev->stats.errors++;
			break;
		}

		if (buf_data.size > BTINTEL_TEST_MAX_BUFFER_SIZE) {
			ret = -EINVAL;
			dev->stats.errors++;
			break;
		}

		if (dev->buffer)
			kfree(dev->buffer);

		dev->buffer = kzalloc(buf_data.size, GFP_KERNEL);
		if (!dev->buffer) {
			ret = -ENOMEM;
			dev->stats.errors++;
			break;
		}

		dev->buffer_size = buf_data.size;
		pr_debug_dev("SET_BUFFER_SIZE ioctl (size=%zu)\n",
			     buf_data.size);
		break;

	case BTINTEL_TEST_IOC_GET_STATUS:
		/* TODO: Implement device-specific status */
		pr_debug_dev("GET_STATUS ioctl\n");
		break;

	case BTINTEL_TEST_IOC_ENABLE:
		dev->active = true;
		pr_debug_dev("ENABLE ioctl\n");
		break;

	case BTINTEL_TEST_IOC_DISABLE:
		dev->active = false;
		pr_debug_dev("DISABLE ioctl\n");
		break;

	default:
		pr_warn("Unknown ioctl command: 0x%x\n", cmd);
		ret = -ENOTTY;
		dev->stats.errors++;
		break;
	}

	dev->stats.ioctl_count++;
	mutex_unlock(&dev->lock);

	return ret;
}

/* ============================================================================
 * DEVICE INITIALIZATION & CLEANUP
 * ============================================================================ */

/**
 * btintel_test_device_init - Initialize device structure
 *
 * Return: 0 on success, negative error code on failure
 */
static int btintel_test_device_init(void)
{
	int ret = 0;

	pr_info("Initializing device\n");

	btintel_test_dev = kzalloc(sizeof(*btintel_test_dev), GFP_KERNEL);
	if (!btintel_test_dev) {
		pr_err("Failed to allocate device structure\n");
		return -ENOMEM;
	}

	mutex_init(&btintel_test_dev->lock);
	btintel_test_dev->active = true;
	btintel_test_dev->buffer_size = BTINTEL_TEST_DEFAULT_BUFFER_SIZE;

	/* Allocate internal buffer */
	btintel_test_dev->buffer = kzalloc(btintel_test_dev->buffer_size, GFP_KERNEL);
	if (!btintel_test_dev->buffer) {
		pr_err("Failed to allocate device buffer\n");
		ret = -ENOMEM;
		goto err_buffer;
	}

	return 0;

err_buffer:
	kfree(btintel_test_dev);
	btintel_test_dev = NULL;
	return ret;
}

/**
 * btintel_test_device_cleanup - Cleanup device structure
 */
static void btintel_test_device_cleanup(void)
{
	if (!btintel_test_dev)
		return;

	pr_info("Cleaning up device\n");

	if (btintel_test_dev->buffer) {
		kfree(btintel_test_dev->buffer);
		btintel_test_dev->buffer = NULL;
	}

	kfree(btintel_test_dev);
	btintel_test_dev = NULL;
}

/* ============================================================================
 * CHARACTER DEVICE REGISTRATION
 * ============================================================================ */

/**
 * btintel_test_chrdev_register - Register character device
 *
 * Return: 0 on success, negative error code on failure
 */
static int btintel_test_chrdev_register(void)
{
	int ret = 0;

	pr_info("Registering character device\n");

	/* Allocate device numbers */
	ret = alloc_chrdev_region(&btintel_test_devt, 0, DEVICE_COUNT, DRIVER_NAME);
	if (ret) {
		pr_err("Failed to allocate device number\n");
		return ret;
	}

	pr_info("Allocated device number: %d:%d\n",
		MAJOR(btintel_test_devt), MINOR(btintel_test_devt));

	/* Create device class */
	btintel_test_class = class_create(DRIVER_NAME);
	if (IS_ERR(btintel_test_class)) {
		pr_err("Failed to create device class\n");
		ret = PTR_ERR(btintel_test_class);
		goto err_class_create;
	}

	/* Initialize character device */
	cdev_init(&btintel_test_dev->cdev, &btintel_test_fops);
	btintel_test_dev->cdev.owner = THIS_MODULE;

	/* Add character device */
	ret = cdev_add(&btintel_test_dev->cdev, btintel_test_devt, DEVICE_COUNT);
	if (ret) {
		pr_err("Failed to add character device\n");
		goto err_cdev_add;
	}

	/* Create device file */
	btintel_test_dev->dev = device_create(btintel_test_class, NULL, btintel_test_devt,
					 NULL, DRIVER_NAME "0");
	if (IS_ERR(btintel_test_dev->dev)) {
		pr_err("Failed to create device file\n");
		ret = PTR_ERR(btintel_test_dev->dev);
		goto err_device_create;
	}

	pr_info("Device file created: /dev/%s0\n", DRIVER_NAME);
	return 0;

	/* Error handling */
err_device_create:
	cdev_del(&btintel_test_dev->cdev);
err_cdev_add:
	class_destroy(btintel_test_class);
err_class_create:
	unregister_chrdev_region(btintel_test_devt, DEVICE_COUNT);
	return ret;
}

/**
 * btintel_test_chrdev_unregister - Unregister character device
 */
static void btintel_test_chrdev_unregister(void)
{
	pr_info("Unregistering character device\n");

	if (btintel_test_dev && btintel_test_dev->dev) {
		device_destroy(btintel_test_class, btintel_test_devt);
		cdev_del(&btintel_test_dev->cdev);
	}

	if (btintel_test_class) {
		class_destroy(btintel_test_class);
	}

	unregister_chrdev_region(btintel_test_devt, DEVICE_COUNT);
}

/* ============================================================================
 * MODULE INIT & EXIT
 * ============================================================================ */

/**
 * btintel_test_init - Module initialization
 *
 * Return: 0 on success, negative error code on failure
 */
static int __init btintel_test_init(void)
{
	int ret = 0;

	pr_info("Loading %s driver version %s\n", DRIVER_NAME, DRIVER_VERSION);

	/* Initialize device */
	ret = btintel_test_device_init();
	if (ret) {
		pr_err("Failed to initialize device\n");
		return ret;
	}

	/* Register character device */
	ret = btintel_test_chrdev_register();
	if (ret) {
		pr_err("Failed to register character device\n");
		btintel_test_device_cleanup();
		return ret;
	}

	pr_info("Driver loaded successfully\n");
	return 0;
}

/**
 * btintel_test_exit - Module cleanup
 */
static void __exit btintel_test_exit(void)
{
	pr_info("Unloading %s driver\n", DRIVER_NAME);

	btintel_test_chrdev_unregister();
	btintel_test_device_cleanup();

	pr_info("Driver unloaded\n");
}

module_init(btintel_test_init);
module_exit(btintel_test_exit);

/* ============================================================================
 * EOF
 * ============================================================================ */
